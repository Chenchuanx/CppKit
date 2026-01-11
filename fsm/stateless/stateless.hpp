#include <memory>
#include <unordered_map>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <tuple>
#include <utility>
#include <cstddef>

#include "trigger_parameters.hpp"

namespace stateless
{

    template<typename TState, typename TTrigger>
    class StateConfiguration;

    template<typename TState, typename TTrigger>
    class StateRepresentation;

    /**
     * @brief 状态机
     * 
     * @tparam TState 
     * @tparam TTrigger 
     */
    template<typename TState, typename TTrigger>
    class StateMachine {
        using TriggerWithParametersPtr = std::unique_ptr<TriggerWithParameters<TTrigger>>;

    public:
        StateMachine(TState initialState) 
            : currentState_(initialState)
        {
            triggerQueueThread_ = std::thread([this]() {
                processTriggerQueue();
            });
        }

        ~StateMachine()
        {
            running_ = false;
            triggerQueueCV_.notify_all();
            triggerQueueThread_.join();
        }

        void fire(TTrigger trigger)
        {
            std::lock_guard<std::mutex> lock(triggerQueueMutex_);
            triggerQueue_.push(std::unique_ptr<TriggerWithParametersNoArgs<TTrigger>>(new TriggerWithParametersNoArgs<TTrigger>(trigger)));
            triggerQueueCV_.notify_one();
        }

        template<typename... Args>
        void fire(TTrigger trigger, Args&&... args)
        {
            std::lock_guard<std::mutex> lock(triggerQueueMutex_);
            triggerQueue_.push(std::unique_ptr<TriggerWithParametersImpl<TTrigger, Args...>>(new TriggerWithParametersImpl<TTrigger, Args...>(trigger, std::forward<Args>(args)...)));
            triggerQueueCV_.notify_one();
        }
        
        bool can_fire(TTrigger trigger) const
        {
            auto stateRepresentation = stateRepresentations_.find(currentState_);
            if (stateRepresentation != stateRepresentations_.end()) {
                return stateRepresentation->second->can_fire(trigger);
            }
            return false;
        }

        std::shared_ptr<StateConfiguration<TState, TTrigger>> configure(TState state) 
        {   
            if (stateRepresentations_.find(state) == stateRepresentations_.end()) {
                stateRepresentations_[state] = std::unique_ptr<StateRepresentation<TState, TTrigger>>(
                    new StateRepresentation<TState, TTrigger>(state)
                );
            }
            return std::make_shared<StateConfiguration<TState, TTrigger>>(state, stateRepresentations_[state].get());
        }

        TState currentState() const
        {
            return currentState_;
        }

    private:
        void processTriggerQueue()
        {
            while (running_)
            {
                std::unique_lock<std::mutex> lock(triggerQueueMutex_);
                triggerQueueCV_.wait(lock, [this]() {
                    return !triggerQueue_.empty() || !running_;
                });
                if (!running_) {
                    break;
                }
                auto triggerWithParams = std::move(triggerQueue_.front());
                triggerQueue_.pop();
                lock.unlock();  // 主动释放锁，允许添加新触发器
                processTrigger(std::move(triggerWithParams));
            }
        }

        void processTrigger(std::unique_ptr<TriggerWithParameters<TTrigger>> triggerWithParams)
        {
            TTrigger trigger = triggerWithParams->trigger();
            if (!can_fire(trigger)) {
                return;
            }
            
            // 从当前状态的 StateRepresentation 中获取目标状态
            auto it = stateRepresentations_.find(currentState_);
            if (it == stateRepresentations_.end() || it->second == nullptr) {
                throw std::runtime_error("State representation not found for state: " + std::to_string(static_cast<int>(currentState_)));
            }
            
            auto currentStateRep = it->second.get();
            TState destinationState = currentStateRep->getDestinationState(trigger);
            
            // 执行退出当前状态的动作
            try {
                currentStateRep->runOnExitAction();
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Error running on exit action: " + std::string(e.what()));
            }
            
            // 获取目标状态的 StateRepresentation
            auto destIt = stateRepresentations_.find(destinationState);
            if (destIt == stateRepresentations_.end() || destIt->second == nullptr) {
                throw std::runtime_error("State representation not found for destination state: " + std::to_string(static_cast<int>(destinationState)));
            }
            
            // 执行进入目标状态的动作
            try {
                destIt->second->runOnEntryAction(std::move(triggerWithParams));
            }
            catch (const std::exception& e) {
                throw std::runtime_error("Error running on entry action: " + std::string(e.what()));
            }
            
            // 更新当前状态
            currentState_ = destinationState;
        }

    private:
        TState currentState_;
        std::queue<std::unique_ptr<TriggerWithParameters<TTrigger>>> triggerQueue_;
        std::unordered_map<TState, std::unique_ptr<StateRepresentation<TState, TTrigger>>> stateRepresentations_;

        bool running_{true};
        std::thread triggerQueueThread_;
        std::mutex triggerQueueMutex_;
        std::condition_variable triggerQueueCV_;
    };

    /**
     * @brief 状态配置
     * 
     * @tparam TState 
     * @tparam TTrigger 
     */
    template<typename TState, typename TTrigger>
    class StateConfiguration {
        using TriggerWithParametersPtr = std::unique_ptr<TriggerWithParameters<TTrigger>>;

    public: 
        StateConfiguration(TState state, StateRepresentation<TState, TTrigger>* stateRepresentation) 
            : state_(state), state_representation_(stateRepresentation)
        {
        }

        StateConfiguration *permit(TTrigger trigger, TState destinationState)
        {
            state_representation_->addPermittedTrigger(trigger, destinationState);
            return this;
        }

        // 无参数的 on_entry
        template<typename Function>
        StateConfiguration *on_entry(Function&& action)
        {

            state_representation_->setOnEntryAction(
                [action](TriggerWithParametersPtr triggerWithParams) {
                    action();
                });
            return this;
        }

        // 带一个参数的 on_entry
        template<typename Function, typename TArgs0>
        StateConfiguration *on_entry(Function&& action, TArgs0&&)
        {
            state_representation_->setOnEntryAction(
                [action](TriggerWithParametersPtr triggerWithParams) {
                    auto* impl = dynamic_cast<TriggerWithParametersImpl<TTrigger, TArgs0>*>(triggerWithParams.get());
                    if (impl) {
                        const auto& params = impl->parameters();
                        action(std::get<0>(params));
                    }
                }
            );
            return this;
        }

        // 带两个参数的 on_entry
        template<typename Function, typename TArgs0, typename TArgs1>
        StateConfiguration *on_entry(Function&& action, TArgs0&&, TArgs1&&)
        {
            state_representation_->setOnEntryAction(
                [action](TriggerWithParametersPtr triggerWithParams) {
                    auto* impl = dynamic_cast<TriggerWithParametersImpl<TTrigger, TArgs0, TArgs1>*>(triggerWithParams.get());
                    if (impl) {
                        const auto& params = impl->parameters();
                        action(std::get<0>(params), std::get<1>(params));
                    }
                }
            );
            return this;
        }

        // 带三个参数的 on_entry
        template<typename Function, typename TArgs0, typename TArgs1, typename TArgs2>
        StateConfiguration *on_entry(Function&& action, TArgs0&&, TArgs1&&, TArgs2&&)
        {
            state_representation_->setOnEntryAction(
                [action](TriggerWithParametersPtr triggerWithParams) {
                    auto* impl = dynamic_cast<TriggerWithParametersImpl<TTrigger, TArgs0, TArgs1, TArgs2>*>(triggerWithParams.get());
                    if (impl) {
                        const auto& params = impl->parameters();
                        action(std::get<0>(params), std::get<1>(params), std::get<2>(params));
                    }
                }
            );
            return this;
        }

        // 无参数的 on_exit
        template<typename Function>
        StateConfiguration *on_exit(Function&& action)
        {
            state_representation_->setOnExitAction(
                [action]() {
                    action();
                });
            return this;
        }

    private:
        TState state_;
        StateRepresentation<TState, TTrigger>* state_representation_;
    };

    template<typename TState, typename TTrigger>
    class StateRepresentation {
        using TriggerWithParametersPtr = std::unique_ptr<stateless::TriggerWithParameters<TTrigger>>;   

    public:
        StateRepresentation(TState state) 
            : state_(state)
        {
        }

        void addPermittedTrigger(TTrigger trigger, TState destinationState)
        {
            permittedTriggers_[trigger] = destinationState;
        }

        
        void setOnEntryAction(std::function<void(TriggerWithParametersPtr)> action)
        {   
            onEntryAction_ = std::move(action);
        }

        void setOnExitAction(std::function<void()> action)
        {
            onExitAction_ = std::move(action);
        }

        void runOnEntryAction(TriggerWithParametersPtr triggerWithParams)
        {
            if (onEntryAction_) {
                onEntryAction_(std::move(triggerWithParams));
            }
        }

        void runOnExitAction()
        {
            if (onExitAction_) {
                onExitAction_();
            }
        }

        bool can_fire(TTrigger trigger) const
        {
            return permittedTriggers_.find(trigger) != permittedTriggers_.end();
        }

        TState getDestinationState(TTrigger trigger) const
        {
            auto it = permittedTriggers_.find(trigger);
            if (it != permittedTriggers_.end()) {
                return it->second;
            }
            throw std::runtime_error("No destination state found for trigger");
        }

    private:
        TState state_;
        std::unordered_map<TTrigger, TState> permittedTriggers_;
        std::function<void(TriggerWithParametersPtr)> onEntryAction_;
        std::function<void()> onExitAction_;
    };

}