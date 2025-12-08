#include "../include/GameState.hpp"
#include "../../ecs/include/systems/InputSystem.hpp"
#include "../../ecs/include/systems/RenderSystem.hpp"
#include <thread>
#include <chrono>

namespace rtype::client {

GameState::GameState() {
}

void GameState::on_enter(Renderer& renderer, Client& client) {
    if (!client.is_connected()) {
        client.connect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
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
    client.update();

    GameEngine::Registry& registry = client.get_registry();

    bool window_has_focus = renderer.get_window() && renderer.get_window()->hasFocus();

    if (window_has_focus) {
        rtype::ecs::InputSystem input_system(renderer.is_moving_up(), renderer.is_moving_down(), renderer.is_moving_left(),
                                             renderer.is_moving_right(), 200.0f);
        input_system.update(registry, delta_time);

        auto controllable_view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Position, rtype::ecs::component::Velocity>();
        for (auto entity : controllable_view) {
            GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
            auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity_id);
            auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(entity_id);
            pos.x += vel.vx * delta_time;
            pos.y += vel.vy * delta_time;
        }

        auto view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Velocity>();
        for (auto entity : view) {
            auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));
            client.send_move(vel.vx, vel.vy);
        }

        if (renderer.is_shooting()) {
            client.send_shoot(0, 0);
        }
    } else {
        auto controllable_view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Velocity>();
        for (auto entity : controllable_view) {
            GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
            auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(entity_id);
            vel.vx = 0.0f;
            vel.vy = 0.0f;
        }
    }
}

void GameState::render(Renderer& renderer, Client& client) {
    renderer.clear();

    if (renderer.get_window() && renderer.get_textures().count("background")) {
        sf::Sprite bg_sprite(renderer.get_textures().at("background"));
        bg_sprite.setScale(4.0f, 4.0f);
        bg_sprite.setPosition(0, 0);
        renderer.get_window()->draw(bg_sprite);
    }

    if (renderer.get_window()) {
        GameEngine::Registry& registry = client.get_registry();
        rtype::ecs::RenderSystem render_system(*renderer.get_window(), renderer.get_textures());
        render_system.update(registry, 0.0);
    }

    renderer.display();
}

} // namespace rtype::client
