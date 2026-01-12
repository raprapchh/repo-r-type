#include "Client.hpp"
#include "Renderer.hpp"
#include "States.hpp"
#include "MenuState.hpp"
#include "../../shared/GameConstants.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include <cstdint>

int main(int argc, char* argv[]) {
    try {
        std::string server_host = "127.0.0.1";
        uint16_t server_port = 4242;
        uint32_t session_id = 1;

        if (argc > 1) {
            server_host = argv[1];
        }
        if (argc > 2) {
            server_port = static_cast<uint16_t>(std::stoi(argv[2]));
        }
        if (argc > 3) {
            session_id = static_cast<uint32_t>(std::stoul(argv[3]));
            if (session_id == 0)
                session_id = 1;
        }

        rtype::client::Renderer renderer(static_cast<uint32_t>(rtype::constants::SCREEN_WIDTH),
                                         static_cast<uint32_t>(rtype::constants::SCREEN_HEIGHT));
        rtype::client::Client client(server_host, server_port, renderer);
        client.set_session_id(session_id);
        rtype::client::StateManager state_manager(renderer, client);

        auto menu_state = std::make_unique<rtype::client::MenuState>();
        state_manager.change_state(std::move(menu_state));

        sf::Clock clock;
        while (renderer.is_open()) {
            float delta_time = clock.restart().asSeconds();

            renderer.handle_input();
            client.update(delta_time);

            state_manager.handle_input();
            state_manager.update(delta_time);
            state_manager.render();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        client.disconnect();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
