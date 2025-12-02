#include "Client.hpp"
#include "Renderer.hpp"
#include "../shared/utils/Logger.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    try {
        rtype::client::Renderer renderer(1280, 720);
        rtype::client::Client client("127.0.0.1", 4242, renderer);
        client.connect();

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        sf::Clock clock;
        while (renderer.is_open()) {
            sf::Event event;
            while (renderer.poll_event(event)) {
                if (event.type == sf::Event::Closed) {
                    renderer.close_window();
                }
            }
            renderer.handle_input();

            if (renderer.is_moving_up() || renderer.is_moving_down() || renderer.is_moving_left() ||
                renderer.is_moving_right()) {
                int8_t dx = 0, dy = 0;
                if (renderer.is_moving_left())
                    dx = -1;
                if (renderer.is_moving_right())
                    dx = 1;
                if (renderer.is_moving_up())
                    dy = -1;
                if (renderer.is_moving_down())
                    dy = 1;
                client.send_move(dx, dy);
            }

            if (renderer.is_shooting()) {
                client.send_shoot(0, 0);
            }
            renderer.render_frame();

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        client.disconnect();
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    return 0;
}
