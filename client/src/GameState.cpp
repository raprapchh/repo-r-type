#include "../include/GameState.hpp"

namespace rtype::client {

GameState::GameState() {
}

void GameState::on_enter(Renderer& renderer, Client& client) {
}

void GameState::on_exit(Renderer& renderer, Client& client) {
}

void GameState::handle_input(Renderer& renderer, StateManager& state_manager) {
    sf::Event event;
    while (renderer.poll_event(event)) {
        if (event.type == sf::Event::Closed) {
            renderer.close_window();
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
            }
        }
    }
    renderer.handle_input();
}

void GameState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
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
}

void GameState::render(Renderer& renderer) {
    renderer.render_frame();
}

} // namespace rtype::client
