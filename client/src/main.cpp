#include "Client.hpp"
#include "Renderer.hpp"
#include "States.hpp"
#include "MenuState.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <memory>

int main() {
    try {
        rtype::client::Renderer renderer(1280, 720);
        rtype::client::Client client("127.0.0.1", 4242, renderer);
        rtype::client::StateManager state_manager(renderer, client);

        auto menu_state = std::make_unique<rtype::client::MenuState>();
        state_manager.change_state(std::move(menu_state));

        sf::Clock clock;
        while (renderer.is_open()) {
            float delta_time = clock.restart().asSeconds();

            sf::Event event;
            while (renderer.poll_event(event)) {
                if (event.type == sf::Event::Closed) {
                    renderer.close_window();
                }
            }
            renderer.handle_input();
            client.update();

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
