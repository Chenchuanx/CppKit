#include <iostream>
#include <thread>
#include <chrono>
#include "stateless/stateless.hpp"

enum class State {
    INIT,
    START,
    STOP,
    ERROR,
    UNKNOWN
};

enum class Trigger {
    CLICK_START,
    CLICK_STOP,
    RESTART,
    ERROR_OCCURRED,
    UNKNOWN_TRIGGER
};

std::string stateToString(State state) {
    switch (state) {
        case State::INIT:
            return "INIT";
        case State::START:
            return "START";
        case State::STOP:
            return "STOP";
        case State::ERROR:
            return "ERROR";
        default:
            return "UNKNOWN";
    }
}

int main() {
    stateless::StateMachine<State, Trigger> stateMachine(State::INIT);

    
    // 配置状态
    stateMachine.configure(State::INIT)
        ->on_entry([]() {
            std::cout << "Entry state INIT" << std::endl;
        })
        ->on_exit([]() {
            std::cout << "Exit state INIT" << std::endl;
        })
        ->permit(Trigger::CLICK_START, State::START)
        ->permit(Trigger::CLICK_STOP, State::STOP)
        ->permit(Trigger::ERROR_OCCURRED, State::ERROR);

    stateMachine.configure(State::START)
        ->on_entry([]() {
            std::cout << "Entry state START" << std::endl;
        })
        ->on_exit([]() {
            std::cout << "Exit state START" << std::endl;
        })
        ->permit(Trigger::CLICK_STOP, State::STOP)
        ->permit(Trigger::ERROR_OCCURRED, State::ERROR);

    stateMachine.configure(State::STOP)
        ->on_entry([]() {
            std::cout << "Entry state STOP" << std::endl;
        })
        ->on_exit([]() {
            std::cout << "Exit state STOP" << std::endl;
        })
        ->permit(Trigger::CLICK_START, State::START);
    
    stateMachine.configure(State::ERROR)
        ->on_entry([]() {
            std::cout << "Entry state ERROR" << std::endl;
        })
        ->on_exit([]() {
            std::cout << "Exit state ERROR" << std::endl;
        })
        ->permit(Trigger::RESTART, State::INIT);

    // 激活状态机，执行初始状态的 on_entry 动作
    // 方式1：显式激活（配置完成后立即执行初始状态的 on_entry）
    stateMachine.activate();
    
    // 方式2：不调用 activate()，在第一次 fire 时自动激活

    std::string input;
    while (true) {
        std::cout << "Current state: " << stateToString(stateMachine.currentState()) << std::endl;
        std::cout << "Please input a command: ";
        std::cin >> input;
        if (input == "start") {
            stateMachine.fire(Trigger::CLICK_START);
        } else if (input == "stop") {
            stateMachine.fire(Trigger::CLICK_STOP);
        } else if (input == "restart") {
            stateMachine.fire(Trigger::RESTART);
        } else if (input == "error") {
            stateMachine.fire(Trigger::ERROR_OCCURRED);
        } else if (input == "show") {
            std::cout << "Current state: " << stateToString(stateMachine.currentState()) << std::endl;
        } else if (input == "exit") {
            break;
        } else if (input == "fire") {
            stateMachine.fire(Trigger::UNKNOWN_TRIGGER);
        }else {
            std::cout << "Invalid input" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    return 0;
}