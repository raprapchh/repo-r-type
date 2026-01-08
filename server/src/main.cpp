#include "Server.hpp"
#include "../../ecs/include/Registry.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include <csignal>
#include <iostream>
#include <string>
#include <memory>
#include <ctime>
#include <cstdlib>

std::unique_ptr<rtype::server::Server> g_server;
GameEngine::Registry g_registry;

void signal_handler(int signal) {
    if (signal == SIGINT) {

        if (g_server) {
            g_server->stop();
        }
        exit(0);
    }
}

int main(int argc, char* argv[]) {
    std::srand(std::time(nullptr));
    std::signal(SIGINT, signal_handler);

    try {
        unsigned int port = 4242;
        if (argc > 1 && std::stoi(argv[1]) < 65535) {
            port = static_cast<unsigned int>(std::stoi(argv[1]));
        }

        g_server = std::make_unique<rtype::server::Server>(g_registry, port);
        g_server->run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}