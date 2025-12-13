#include "../include/GameState.hpp"
#include "../include/MenuState.hpp"
#include "../../ecs/include/systems/InputSystem.hpp"
#include "../../ecs/include/systems/RenderSystem.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/Lives.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/Weapon.hpp"
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
    client.send_game_start_request();
}

void GameState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
    client_ = nullptr;
}

void GameState::handle_input(Renderer& renderer, StateManager& state_manager) {
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
                if (!is_charging_) {
                    is_charging_ = true;
                    charge_start_time_ = std::chrono::steady_clock::now();
                }
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == sf::Keyboard::Space) {
                if (is_charging_) {
                    auto now = std::chrono::steady_clock::now();
                    auto duration =
                        std::chrono::duration_cast<std::chrono::milliseconds>(now - charge_start_time_).count();
                    int chargeLevel = 0;

                    if (duration < 500) {
                        chargeLevel = 0;
                    } else if (duration < 1000) {
                        chargeLevel = 1;
                    } else if (duration < 1500) {
                        chargeLevel = 2;
                    } else {
                        chargeLevel = 3;
                    }

                    is_charging_ = false;
                    renderer.set_charge_percentage(0.0f);
                    if (client_) {
                        client_->send_shoot(0, 0, chargeLevel);
                    }
                }
            }
        } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            if (game_over_ && all_players_dead_) {
                sf::Vector2f mouse_pos = renderer.get_window()->mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), renderer.get_window()->getDefaultView());

                if (renderer.is_game_over_back_to_menu_clicked(mouse_pos)) {
                    state_manager.change_state(std::make_unique<MenuState>());
                }
            }
        }
    }
    if (is_charging_) {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - charge_start_time_).count();
        float percentage = static_cast<float>(duration) / 1500.0f;
        if (percentage > 1.0f)
            percentage = 1.0f;
        renderer.set_charge_percentage(percentage);
    }

    renderer.handle_input();
}

void GameState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)state_manager;
    client.update(delta_time);

    GameEngine::Registry& registry = client.get_registry();
    std::mutex& registry_mutex = client.get_registry_mutex();

    {
        std::lock_guard<std::mutex> lock(registry_mutex);

        if (!client.is_connected()) {
            return;
        }

        uint32_t player_id = client.get_player_id();
        bool player_exists = false;
        bool player_dead = false;

        auto view = registry.view<rtype::ecs::component::NetworkId>();
        for (auto entity : view) {
            auto& net_id =
                registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<GameEngine::entity_t>(entity));
            if (net_id.id == player_id) {
                player_exists = true;
                if (registry.hasComponent<rtype::ecs::component::Lives>(static_cast<GameEngine::entity_t>(entity))) {
                    auto& lives =
                        registry.getComponent<rtype::ecs::component::Lives>(static_cast<GameEngine::entity_t>(entity));
                    if (lives.remaining <= 0) {
                        player_dead = true;
                    }
                } else if (registry.hasComponent<rtype::ecs::component::Health>(
                               static_cast<GameEngine::entity_t>(entity))) {
                    auto& health =
                        registry.getComponent<rtype::ecs::component::Health>(static_cast<GameEngine::entity_t>(entity));
                    if (health.hp <= 0) {
                        player_dead = true;
                    }
                }
                break;
            }
        }

        if (!player_exists || player_dead) {
            if (!game_over_) {
                game_over_ = true;

                auto player_view = registry.view<rtype::ecs::component::NetworkId>();
                for (auto entity : player_view) {
                    auto& net_id = registry.getComponent<rtype::ecs::component::NetworkId>(
                        static_cast<GameEngine::entity_t>(entity));
                    if (net_id.id == player_id && registry.hasComponent<rtype::ecs::component::Drawable>(
                                                      static_cast<GameEngine::entity_t>(entity))) {
                        registry.removeComponent<rtype::ecs::component::Drawable>(
                            static_cast<GameEngine::entity_t>(entity));
                    }
                }
            }
        }

        if (game_over_) {
            int alive_players = 0;
            auto all_players_view = registry.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Weapon>();
            for (auto entity : all_players_view) {
                auto& net_id =
                    registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<GameEngine::entity_t>(entity));
                if (net_id.id != player_id) {
                    bool other_player_alive = true;
                    if (registry.hasComponent<rtype::ecs::component::Lives>(
                            static_cast<GameEngine::entity_t>(entity))) {
                        auto& lives = registry.getComponent<rtype::ecs::component::Lives>(
                            static_cast<GameEngine::entity_t>(entity));
                        if (lives.remaining <= 0) {
                            other_player_alive = false;
                        }
                    } else if (registry.hasComponent<rtype::ecs::component::Health>(
                                   static_cast<GameEngine::entity_t>(entity))) {
                        auto& health = registry.getComponent<rtype::ecs::component::Health>(
                            static_cast<GameEngine::entity_t>(entity));
                        if (health.hp <= 0) {
                            other_player_alive = false;
                        }
                    }
                    if (other_player_alive) {
                        alive_players++;
                    }
                }
            }
            all_players_dead_ = (alive_players == 0);
        }
    }

    if (game_over_ && all_players_dead_) {
        return;
    }

    bool window_has_focus = renderer.get_window() && renderer.get_window()->hasFocus();

    if (window_has_focus && !game_over_) {
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
        rtype::ecs::RenderSystem render_system(*renderer.get_window(), renderer.get_textures(),
                                               &renderer.get_accessibility_manager());
        render_system.update(registry, 0.016f);

        if (is_charging_) {
            auto view = registry.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Position>();
            for (auto entity : view) {
                auto& net_id =
                    registry.getComponent<rtype::ecs::component::NetworkId>(static_cast<GameEngine::entity_t>(entity));
                if (net_id.id == client.get_player_id()) {
                    auto& pos = registry.getComponent<rtype::ecs::component::Position>(
                        static_cast<GameEngine::entity_t>(entity));
                    renderer.draw_charge_effect(sf::Vector2f(pos.x, pos.y), 0.016f);
                    break;
                }
            }
        }
    }

    renderer.draw_ui();

    if (game_over_) {
        renderer.draw_game_over(all_players_dead_);
    }

    renderer.display();
}

} // namespace rtype::client
