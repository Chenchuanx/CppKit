#include "reactor.h"
#include <iostream>
#include <stdexcept>

int main() {
    try {
        Reactor reactor(8080);
        reactor.Run();
    } catch (const std::exception& e) {
        std::cerr << "Server startup failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}