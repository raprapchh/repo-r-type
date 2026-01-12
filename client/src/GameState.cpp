#include "../include/GameState.hpp"
#include "../include/MenuState.hpp"
#include "../include/SFMLRenderer.hpp"
#include "../../ecs/include/systems/InputSystem.hpp"
#include "../../ecs/include/systems/RenderSystem.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/Tag.hpp"
#include "../../ecs/include/components/Lives.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/Weapon.hpp"
#include "../../ecs/include/components/Score.hpp"
#include <thread>
#include <chrono>

namespace rtype::client {

GameState::GameState() {
    setup_pause_ui();
}

void GameState::on_enter(Renderer& renderer, Client& client) {
    (void)renderer;
    client_ = &client;
    score_saved_ = false;
    initial_player_count_ = 0;
    max_score_reached_ = 0;
    if (!client.is_connected()) {
        client.connect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    client.send_game_start_request();

    // Wait a bit for players to load and count them
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    GameEngine::Registry& registry = client_->get_registry();
    std::mutex& registry_mutex = client_->get_registry_mutex();
    std::lock_guard<std::mutex> lock(registry_mutex);
    auto view = registry.view<rtype::ecs::component::Tag>();
    for (auto entity : view) {
        auto& tag = registry.getComponent<rtype::ecs::component::Tag>(static_cast<GameEngine::entity_t>(entity));
        if (tag.name == "Player") {
            initial_player_count_++;
        }
    }

    std::cout << "GameState: Counted " << initial_player_count_ << " player(s) in game" << std::endl;

    // Start background music
    client.get_audio_system().startBackgroundMusic();
}

void GameState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;

    // Stop background music
    if (client_) {
        client_->get_audio_system().stopBackgroundMusic();
    }

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
                // client_->send_map_resize(static_cast<float>(event.size.width),
                // static_cast<float>(event.size.height));

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
                if (client_) {
                    GameEngine::Registry& registry = client_->get_registry();
                    std::mutex& registry_mutex = client_->get_registry_mutex();
                    std::lock_guard<std::mutex> lock(registry_mutex);

                    int player_count = 0;
                    auto player_view = registry.view<rtype::ecs::component::NetworkId, rtype::ecs::component::Weapon>();
                    for (auto entity : player_view) {
                        (void)entity;
                        player_count++;
                    }

                    if (player_count <= 1) {
                        is_paused_ = !is_paused_;
                        if (is_paused_) {
                            update_pause_ui_positions(renderer.get_window_size());
                        } else {
                            show_settings_panel_ = false;
                        }
                    }
                }
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
            if (is_paused_) {
                sf::Vector2f mouse_pos = renderer.get_mouse_position();
                handle_pause_button_click(mouse_pos, state_manager);
            } else if (game_over_ && all_players_dead_) {
                sf::Vector2f mouse_pos = renderer.get_window()->mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), renderer.get_window()->getDefaultView());

                if (renderer.is_game_over_back_to_menu_clicked(mouse_pos)) {
                    if (client_) {
                        GameEngine::Registry& registry = client_->get_registry();
                        std::mutex& registry_mutex = client_->get_registry_mutex();
                        std::lock_guard<std::mutex> lock(registry_mutex);
                        registry.clear();
                    }
                    game_over_ = false;
                    all_players_dead_ = false;
                    score_saved_ = false;
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

    if (is_paused_) {
        return;
    }

    client.update(delta_time);
    renderer.update(delta_time);

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
                if (registry.hasComponent<rtype::ecs::component::Score>(static_cast<GameEngine::entity_t>(entity))) {
                    auto& score =
                        registry.getComponent<rtype::ecs::component::Score>(static_cast<GameEngine::entity_t>(entity));
                    if (static_cast<uint32_t>(score.value) > max_score_reached_) {
                        max_score_reached_ = static_cast<uint32_t>(score.value);
                    }
                }
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

                // Save score when player dies (only once)
                if (!score_saved_) {
                    score_saved_ = true;
                    uint32_t final_score = max_score_reached_;
                    std::string player_name = client.get_player_name();
                    if (player_name.empty() || player_name == "Player") {
                        player_name = "Player" + std::to_string(client.get_player_id());
                    }

                    std::cout << "Saving score: " << final_score << " for player '" << player_name << "' ("
                              << initial_player_count_ << " player(s) detected)" << std::endl;

                    if (initial_player_count_ <= 1) {
                        std::cout << "-> Solo mode" << std::endl;
                        client.get_scoreboard_manager().add_solo_score(player_name, final_score);
                    } else {
                        std::cout << "-> Multi mode" << std::endl;
                        client.get_scoreboard_manager().add_multi_score(player_name, final_score);
                    }
                }

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

        if (game_over_ && all_players_dead_) {
            return;
        }

        if (game_over_ && all_players_dead_) {
            return;
        }

        bool window_has_focus = renderer.get_window() && renderer.get_window()->hasFocus();

        if (window_has_focus && !game_over_) {
            {
                rtype::ecs::InputSystem input_system(renderer.is_moving_up(), renderer.is_moving_down(),
                                                     renderer.is_moving_left(), renderer.is_moving_right(), 400.0f);
                input_system.update(registry, delta_time);
            }

            {
                rtype::ecs::MovementSystem movement_system;
                movement_system.update(registry, delta_time);
            }

            // Client-side player boundary clamping (visual only)
            {
                auto view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Position>();
                for (auto entity : view) {
                    auto& pos = registry.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                    // Hardcoded map bounds for client visual clamping
                    if (pos.x < 0)
                        pos.x = 0;
                    if (pos.x > 1920 - 100)
                        pos.x = 1920 - 100; // Assuming player width
                    if (pos.y < 0)
                        pos.y = 0;
                    if (pos.y > 1080 - 100)
                        pos.y = 1080 - 100; // Assuming player height
                }
            }

            {
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
            auto controllable_view =
                registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Velocity>();
            for (auto entity : controllable_view) {
                GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
                auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(entity_id);
                vel.vx = 0.0f;
                vel.vy = 0.0f;
            }
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

        auto sfml_renderer =
            std::make_shared<rtype::rendering::SFMLRenderer>(*renderer.get_window(), renderer.get_textures());
        rtype::ecs::RenderSystem render_system(sfml_renderer, &renderer.get_accessibility_manager());
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

    // Stage cleared victory screen
    renderer.draw_stage_cleared();

    if (game_over_) {
        renderer.draw_game_over(all_players_dead_);
    }

    if (is_paused_) {
        render_pause_overlay(renderer);
    }

    renderer.display();
}

void GameState::setup_pause_ui() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr << "Warning: Could not load font from client/fonts/Ethnocentric-Regular.otf. Pause UI will not "
                     "display text."
                  << std::endl;
        font_loaded_ = false;
        return;
    }
    font_loaded_ = true;

    pause_title_text_.setFont(font_);
    pause_title_text_.setString("PAUSE");
    pause_title_text_.setCharacterSize(80);
    pause_title_text_.setFillColor(sf::Color::White);

    settings_button_.setSize(sf::Vector2f(300, 60));
    settings_button_.setFillColor(sf::Color(100, 100, 100));
    settings_button_.setOutlineColor(sf::Color::White);
    settings_button_.setOutlineThickness(2);

    settings_button_text_.setFont(font_);
    settings_button_text_.setString("SETTINGS");
    settings_button_text_.setCharacterSize(30);
    settings_button_text_.setFillColor(sf::Color::White);

    accessibility_cycle_button_.setSize(sf::Vector2f(400, 60));
    accessibility_cycle_button_.setFillColor(sf::Color(50, 50, 150));
    accessibility_cycle_button_.setOutlineColor(sf::Color::White);
    accessibility_cycle_button_.setOutlineThickness(2);

    accessibility_cycle_text_.setFont(font_);
    accessibility_cycle_text_.setString("Mode: None");
    accessibility_cycle_text_.setCharacterSize(22);
    accessibility_cycle_text_.setFillColor(sf::Color::White);
}

void GameState::handle_pause_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager) {
    if (show_settings_panel_) {
        if (accessibility_cycle_button_.getGlobalBounds().contains(mouse_pos)) {
            state_manager.get_renderer().get_accessibility_manager().cycle_mode();
            std::string mode_name = state_manager.get_renderer().get_accessibility_manager().get_mode_name();
            accessibility_cycle_text_.setString("Mode: " + mode_name);
            update_pause_ui_positions(state_manager.get_renderer().get_window_size());
            return;
        }
    } else {
        if (settings_button_.getGlobalBounds().contains(mouse_pos)) {
            show_settings_panel_ = true;
            std::string mode_name = state_manager.get_renderer().get_accessibility_manager().get_mode_name();
            accessibility_cycle_text_.setString("Mode: " + mode_name);
            update_pause_ui_positions(state_manager.get_renderer().get_window_size());
            return;
        }
    }
}

void GameState::update_pause_ui_positions(const sf::Vector2u& window_size) {
    float center_x = window_size.x / 2.0f;
    float center_y = window_size.y / 2.0f;

    sf::FloatRect title_bounds = pause_title_text_.getLocalBounds();
    pause_title_text_.setPosition(center_x - title_bounds.width / 2.0f, center_y - 200.0f);

    if (show_settings_panel_) {
        accessibility_cycle_button_.setPosition(center_x - accessibility_cycle_button_.getSize().x / 2.0f, center_y);

        sf::FloatRect cycle_text_bounds = accessibility_cycle_text_.getLocalBounds();
        accessibility_cycle_text_.setOrigin(cycle_text_bounds.left + cycle_text_bounds.width / 2.0f,
                                            cycle_text_bounds.top + cycle_text_bounds.height / 2.0f);
        accessibility_cycle_text_.setPosition(
            accessibility_cycle_button_.getPosition().x + accessibility_cycle_button_.getSize().x / 2.0f,
            accessibility_cycle_button_.getPosition().y + accessibility_cycle_button_.getSize().y / 2.0f);
    } else {
        settings_button_.setPosition(center_x - 150.0f, center_y - 30.0f);

        sf::FloatRect settings_text_bounds = settings_button_text_.getLocalBounds();
        settings_button_text_.setPosition(
            settings_button_.getPosition().x + (settings_button_.getSize().x - settings_text_bounds.width) / 2.0f,
            settings_button_.getPosition().y + (settings_button_.getSize().y - settings_text_bounds.height) / 2.0f -
                5.0f);
    }
}

void GameState::render_pause_overlay(Renderer& renderer) {
    if (!font_loaded_) {
        return;
    }

    sf::RectangleShape modal_bg(sf::Vector2f(renderer.get_window_size()));
    modal_bg.setFillColor(sf::Color(0, 0, 0, 180));
    renderer.draw_rectangle(modal_bg);

    renderer.draw_text(pause_title_text_);

    if (show_settings_panel_) {
        renderer.draw_rectangle(accessibility_cycle_button_);
        renderer.draw_text(accessibility_cycle_text_);
    } else {
        renderer.draw_rectangle(settings_button_);
        renderer.draw_text(settings_button_text_);
    }
}

} // namespace rtype::client
