#include "../include/GameState.hpp"
#include "../../ecs/include/systems/InputSystem.hpp"
#include "../../ecs/include/systems/RenderSystem.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include <thread>
#include <chrono>

namespace rtype::client {

GameState::GameState() {
}

void GameState::on_enter(Renderer& renderer, Client& client) {
    (void)renderer;
    client_ = &client;
    if (!client.is_connected()) {
        client.connect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void GameState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
    client_ = nullptr;
}

void GameState::handle_input(Renderer& renderer, StateManager& state_manager) {
    (void)state_manager;
    sf::Event event;
    while (renderer.poll_event(event)) {
        if (event.type == sf::Event::Closed) {
            renderer.close_window();
        } else if (event.type == sf::Event::Resized) {
            renderer.handle_resize(event.size.width, event.size.height);
            if (client_) {
                client_->send_map_resize(static_cast<float>(event.size.width), static_cast<float>(event.size.height));

                GameEngine::Registry& registry = client_->get_registry();
                std::mutex& registry_mutex = client_->get_registry_mutex();
                std::lock_guard<std::mutex> lock(registry_mutex);

                try {
                    auto view = registry.view<rtype::ecs::component::MapBounds>();
                    bool found = false;
                    for (auto entity : view) {
                        auto& bounds =
                            registry.getComponent<rtype::ecs::component::MapBounds>(static_cast<std::size_t>(entity));
                        bounds.maxX = static_cast<float>(event.size.width);
                        bounds.maxY = static_cast<float>(event.size.height);
                        found = true;
                        break;
                    }
                    if (!found) {
                        auto entity = registry.createEntity();
                        registry.addComponent<rtype::ecs::component::MapBounds>(entity, 0.0f, 0.0f,
                                                                                static_cast<float>(event.size.width),
                                                                                static_cast<float>(event.size.height));
                    }
                } catch (const std::exception&) {
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
            } else if (event.key.code == sf::Keyboard::Space) {
                shoot_requested_ = true;
            }
        }
    }
    renderer.handle_input();
}

void GameState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)state_manager;
    client.update(delta_time);

    GameEngine::Registry& registry = client.get_registry();
    std::mutex& registry_mutex = client.get_registry_mutex();

    bool window_has_focus = renderer.get_window() && renderer.get_window()->hasFocus();

    if (window_has_focus) {
        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            rtype::ecs::InputSystem input_system(renderer.is_moving_up(), renderer.is_moving_down(),
                                                 renderer.is_moving_left(), renderer.is_moving_right(), 200.0f);
            input_system.update(registry, delta_time);
        }

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            rtype::ecs::MovementSystem movement_system;
            movement_system.update(registry, delta_time);
        }

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            rtype::ecs::BoundarySystem boundary_system;
            boundary_system.update(registry, delta_time);
            rtype::ecs::CollisionSystem collision_system;
            collision_system.update(registry, delta_time);
        }

        {
            std::lock_guard<std::mutex> lock(registry_mutex);
            auto view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Velocity>();
            for (auto entity : view) {
                auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));
                client.send_move(vel.vx, vel.vy);
            }
        }

        if (shoot_requested_) {
            client.send_shoot(0, 0);
            shoot_requested_ = false;
        }
    } else {
        std::lock_guard<std::mutex> lock(registry_mutex);
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

    if (renderer.get_window()) {
        renderer.draw_background();
    }

    if (renderer.get_window()) {
        GameEngine::Registry& registry = client.get_registry();
        std::mutex& registry_mutex = client.get_registry_mutex();
        std::lock_guard<std::mutex> lock(registry_mutex);
        rtype::ecs::RenderSystem render_system(*renderer.get_window(), renderer.get_textures());
        render_system.update(registry, 0.016f);
    }

    renderer.draw_ui();

    renderer.display();
}

} // namespace rtype::client
