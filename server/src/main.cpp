#include "Server.hpp"
#include "../../ecs/include/Registry.hpp"
#include <csignal>
#include <iostream>
#include <cstdint>
#include <string>
#include <memory>

std::unique_ptr<rtype::server::Server> g_server;

void signal_handler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\nShutting down server..." << std::endl;
        if (g_server) {
            g_server->stop();
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);

    try {
        uint16_t port = 4242;
        if (argc > 1) {
            port = static_cast<uint16_t>(std::stoi(argv[1]));
        }

        GameEngine::Registry registry;
        g_server = std::make_unique<rtype::server::Server>(registry, port);
        g_server->run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
