#pragma once
#include <memory>
#include <tuple>
#include <utility>
#include <cstddef>

namespace stateless
{
    /**
    * @brief 带参数的触发器基类接口（类型擦除）
    * 
    * @tparam TTrigger 触发器类型
    */
    template<typename TTrigger>
    class TriggerWithParameters {
    public:
        virtual ~TriggerWithParameters() = default;
        
        virtual TTrigger trigger() const = 0;
    };

    /**
    * @brief 带参数的触发器具体实现
    * 
    * @tparam TTrigger 触发器类型
    * @tparam TArgs 参数类型列表
    */
    template<typename TTrigger, typename... TArgs>
    class TriggerWithParametersImpl : public TriggerWithParameters<TTrigger> {
    public:
        template<typename... Args>
        TriggerWithParametersImpl(TTrigger trigger, Args&&... args)
            : trigger_(trigger), parameters_(std::forward<Args>(args)...)
        {
        }

        TTrigger trigger() const override
        {
            return trigger_;
        }

        const std::tuple<TArgs...>& parameters() const
        {
            return parameters_;
        }

        std::tuple<TArgs...>& parameters()
        {
            return parameters_;
        }

    private:
        TTrigger trigger_;                          // 触发器
        std::tuple<TArgs...> parameters_;           // 参数元组
    };

    /**
    * @brief 无参数的触发器包装类
    * 
    * @tparam TTrigger 触发器类型
    */
    template<typename TTrigger>
    class TriggerWithParametersNoArgs : public TriggerWithParameters<TTrigger> {
    public:
        explicit TriggerWithParametersNoArgs(TTrigger trigger)
            : trigger_(trigger)
        {
        }

        TTrigger trigger() const override
        {
            return trigger_;
        }

    private:
        TTrigger trigger_;
    };
}