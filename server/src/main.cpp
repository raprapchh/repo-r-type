#include "Server.hpp"
#include <csignal>
#include <iostream>

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

int main() {
    std::signal(SIGINT, signal_handler);

    try {
        g_server = std::make_unique<rtype::server::Server>(4242);
        g_server->run();
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
