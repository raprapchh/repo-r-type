#include "GameState.hpp"
#include "MenuState.hpp"
#include "SFMLRenderer.hpp"
#include "GameConstants.hpp"
#include "systems/InputSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/TextureAnimationSystem.hpp"
#include "systems/MovementSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/BoundarySystem.hpp"
#include "systems/SpawnEffectSystem.hpp"
#include "systems/ProjectileSystem.hpp"
#include "systems/ForcePodSystem.hpp"
#include "systems/FpsSystem.hpp"
#include "systems/DevToolsSystem.hpp"
#include "systems/UIRenderSystem.hpp"
#include "components/MapBounds.hpp"
#include "components/NetworkId.hpp"
#include "components/NetworkInterpolation.hpp"
#include "components/Tag.hpp"
#include "components/Lives.hpp"
#include "components/Health.hpp"
#include "components/HitBox.hpp"
#include "components/CollisionLayer.hpp"
#include "components/Controllable.hpp"
#include "components/Weapon.hpp"
#include "components/Score.hpp"
#include "components/Projectile.hpp"
#include "components/TextDrawable.hpp"
#include "components/UITag.hpp"
#include "components/FpsCounter.hpp"
#include "components/PingStats.hpp"
#include "components/CpuStats.hpp"
#include "components/LagometerComponent.hpp"
#include "systems/PingSystem.hpp"
#include "systems/CpuMetricSystem.hpp"
#include "systems/LagometerSystem.hpp"
#include "components/SpectatorComponent.hpp"
#include "systems/SpectatorSystem.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace rtype::client {

GameState::GameState(bool multiplayer, rtype::config::Difficulty difficulty, uint8_t lives)
    : multiplayer_(multiplayer), solo_difficulty_(difficulty), solo_lives_(lives) {
    setup_pause_ui();
    setup_spectator_ui();
}

void GameState::on_enter(Renderer& renderer, Client& client) {
    (void)renderer;
    client_ = &client;
    score_saved_ = false;
    score_saved_ = false;
    initial_player_count_ = 0;
    max_score_reached_ = 0;

    renderer.reset_game_state();

    spectator_system_ = std::make_shared<rtype::ecs::SpectatorSystem>();

    if (multiplayer_) {
        if (!client.is_connected()) {
            client.connect();
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        client.send_game_start_request();
    } else {
        if (!client.is_connected()) {
            client.connect();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        {
            auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                              std::chrono::system_clock::now().time_since_epoch())
                              .count();
            client.create_room("SOLO-" + std::to_string(now_ms), 1, rtype::config::GameMode::COOP, solo_difficulty_,
                               false, solo_lives_);
        }
        game_start_sent_ = false;

        GameEngine::SystemManager& system_manager = client.get_system_manager();
        system_manager.clear();
        system_manager.addSystem<rtype::ecs::MovementSystem>();
        system_manager.addSystem<rtype::ecs::TextureAnimationSystem>();
        system_manager.addSystem<rtype::ecs::SpawnEffectSystem>();
        system_manager.addSystem<rtype::ecs::CollisionSystem>();
        system_manager.addSystem<rtype::ecs::BoundarySystem>();
        system_manager.addSystem<rtype::ecs::ProjectileSystem>();
        system_manager.addSystem<rtype::ecs::ForcePodSystem>();
    }

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

    // FPS Counter: Only spawn in Multiplayer Mode
    if (multiplayer_) {
        sf::Vector2u windowSize = renderer.get_window_size();
        createFpsCounter(registry, static_cast<float>(windowSize.x));
        createDevMetrics(registry, static_cast<float>(windowSize.x));
        createLagometer(registry, static_cast<float>(windowSize.x));
    }

    client.get_audio_system().startBackgroundMusic();
}

void GameState::on_exit(Renderer& renderer, Client& client) {
    // Reset spectator state logic
    has_chosen_spectate_ = false;
    spectator_choice_pending_ = false;

    // Reset renderer state to avoid leftover visuals on restart
    renderer.reset_game_state();

    if (client_) {
        client_->get_audio_system().stopBackgroundMusic();
    }

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
            } else if (event.key.code == sf::Keyboard::R) {
                if (client_) {
                    client_->send_shoot(0, 0, 4);
                }
            } else if (event.key.code == renderer.get_key_binding(Renderer::Action::Shoot)) {
                if (!is_charging_) {
                    is_charging_ = true;
                    charge_start_time_ = std::chrono::steady_clock::now();
                }
            }
        } else if (event.type == sf::Event::KeyReleased) {
            if (event.key.code == renderer.get_key_binding(Renderer::Action::Shoot)) {
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

                        // Play shoot sound locally
                        if (chargeLevel > 0) {
                            client_->get_audio_system().playSound("player_missile");
                        } else {
                            client_->get_audio_system().playSound("player_shoot");
                        }
                    }
                }
            } else if (event.key.code == sf::Keyboard::F3) {
                // Toggle Dev Tools (FPS Counter) visibility - Multiplayer only
                if (multiplayer_ && client_) {
                    GameEngine::Registry& registry = client_->get_registry();
                    std::mutex& registry_mutex = client_->get_registry_mutex();
                    std::lock_guard<std::mutex> lock(registry_mutex);

                    rtype::ecs::DevToolsSystem dev_tools_system;
                    dev_tools_system.setTogglePressed(true);
                    dev_tools_system.update(registry, 0.0);
                }
            } else if (event.key.code == sf::Keyboard::L) {
                if (multiplayer_ && client_) {
                    GameEngine::Registry& registry = client_->get_registry();
                    std::mutex& registry_mutex = client_->get_registry_mutex();
                    std::lock_guard<std::mutex> lock(registry_mutex);

                    auto view = registry.view<rtype::ecs::component::LagometerComponent>();
                    for (auto entity : view) {
                        auto& lagometer = registry.getComponent<rtype::ecs::component::LagometerComponent>(
                            static_cast<GameEngine::entity_t>(entity));
                        lagometer.visible = !lagometer.visible;
                    }
                }
            }
        } else if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
            // Handle spectator choice dialog clicks FIRST
            if (spectator_choice_pending_) {
                // Recalculate positions/bounds dynamically to ensure accuracy (handling window resize, etc.)
                sf::Vector2u windowSize = renderer.get_window_size();
                float center_x = windowSize.x / 2.0f;
                float center_y = windowSize.y / 2.0f;

                sf::FloatRect continue_bounds(center_x - 200.0f, center_y, 400.0f, 60.0f);
                sf::FloatRect menu_bounds(center_x - 200.0f, center_y + 80.0f, 400.0f, 60.0f);

                // Expand hitboxes, but be careful of overlap (20px gap vs 10px padding * 2 = touch)
                // Using 5px vertical padding ensures 10px gap remains safe
                float padding_h = 10.0f;
                float padding_v = 5.0f;
                continue_bounds.left -= padding_h;
                continue_bounds.top -= padding_v;
                continue_bounds.width += padding_h * 2;
                continue_bounds.height += padding_v * 2;

                menu_bounds.left -= padding_h;
                menu_bounds.top -= padding_v;
                menu_bounds.width += padding_h * 2;
                menu_bounds.height += padding_v * 2;

                sf::Vector2f mouse_pos = renderer.get_window()->mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), renderer.get_window()->getDefaultView());

                if (continue_bounds.contains(mouse_pos)) {
                    // Continue spectating
                    spectator_choice_pending_ = false;
                    has_chosen_spectate_ = true;
                } else if (menu_bounds.contains(mouse_pos)) {
                    // Back to menu

                    if (client_) {
                        GameEngine::Registry& registry = client_->get_registry();
                        std::mutex& registry_mutex = client_->get_registry_mutex();
                        std::lock_guard<std::mutex> lock(registry_mutex);
                        registry.clear();
                    }
                    spectator_choice_pending_ = false;
                    has_chosen_spectate_ = false; // Ensure explicit false
                    game_over_ = false;
                    all_players_dead_ = false;
                    score_saved_ = false;
                    state_manager.change_state(std::make_unique<MenuState>());
                } else {
                    // Clicked outside
                }
            } else if (is_paused_) {
                sf::Vector2f mouse_pos = renderer.get_mouse_position();
                handle_pause_button_click(mouse_pos, state_manager);
            } else if (has_chosen_spectate_ && !game_over_) {
                // Handle clicks on HUD Exit button in Spectator Mode
                sf::Vector2f mouse_pos = renderer.get_window()->mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), renderer.get_window()->getDefaultView());

                // Recalculate HUD button position (Moved to avoid overlap with FPS metrics)
                sf::Vector2u window_size = renderer.get_window_size();
                sf::FloatRect exit_bounds(window_size.x - 120.0f, 100.0f, 100.0f, 30.0f);

                if (exit_bounds.contains(mouse_pos)) {
                    if (client_) {
                        GameEngine::Registry& registry = client_->get_registry();
                        std::mutex& registry_mutex = client_->get_registry_mutex();
                        std::lock_guard<std::mutex> lock(registry_mutex);
                        registry.clear();
                    }
                    has_chosen_spectate_ = false;
                    spectator_choice_pending_ = false;
                    game_over_ = false;
                    all_players_dead_ = false;
                    score_saved_ = false;
                    state_manager.change_state(std::make_unique<MenuState>());
                } else {
                    // Check for settings button or other UI? (Renderer doesn't expose it yet)
                }

            } else if (game_over_ && all_players_dead_) {
                sf::Vector2f mouse_pos = renderer.get_window()->mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), renderer.get_window()->getDefaultView());

                // Multiplayer: Handle restart vote system
                if (multiplayer_ && renderer.is_restart_vote_active()) {
                    if (renderer.is_play_again_clicked(mouse_pos)) {
                        if (client_) {
                            client_->send_restart_vote(true);
                        }
                    } else if (renderer.is_game_over_back_to_menu_clicked(mouse_pos)) {
                        if (client_) {
                            client_->send_restart_vote(false);
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
                    // Solo: Handle direct restart
                } else if (!multiplayer_ && renderer.is_game_over_restart_clicked(mouse_pos)) {
                    if (client_) {
                        client_->restart_session();
                    }
                    game_over_ = false;
                    all_players_dead_ = false;
                    score_saved_ = false;
                    state_manager.change_state(std::make_unique<GameState>(false, solo_difficulty_, solo_lives_));
                    // Both modes: Handle back to menu
                } else if (renderer.is_game_over_back_to_menu_clicked(mouse_pos)) {
                    if (client_) {
                        client_->leave_room();
                    }
                    game_over_ = false;
                    all_players_dead_ = false;
                    score_saved_ = false;
                    state_manager.change_state(std::make_unique<MenuState>());
                }
            } else if (renderer.is_stage_cleared() && renderer.is_game_finished()) {
                sf::Vector2f mouse_pos = renderer.get_window()->mapPixelToCoords(
                    sf::Vector2i(event.mouseButton.x, event.mouseButton.y), renderer.get_window()->getDefaultView());

                if (renderer.is_victory_back_to_menu_clicked(mouse_pos)) {
                    if (client_) {
                        client_->leave_room();
                    }
                    game_over_ = false;
                    all_players_dead_ = false;
                    score_saved_ = false;
                    state_manager.change_state(std::make_unique<MenuState>());
                }
            }
        }
    }

    if (renderer.get_window() && renderer.get_window()->hasFocus() && sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        is_firing_laser_ = true;
    } else {
        is_firing_laser_ = false;
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

    if (is_firing_laser_) {
        if (laser_energy_ > 0.0f) {
            laser_energy_ -= delta_time;
            if (laser_energy_ < 0.0f)
                laser_energy_ = 0.0f;
            if (client_) {
                client_->send_shoot(0, 0, 4);
            }
        }
    } else {
        laser_energy_ += delta_time * 0.5f; // Recharge in 2 seconds (1.0 / 0.5 = 2.0)
        if (laser_energy_ > 1.0f)
            laser_energy_ = 1.0f;
    }
    renderer.set_laser_energy(laser_energy_);

    renderer.update(delta_time);

    GameEngine::Registry& registry = client.get_registry();
    std::mutex& registry_mutex = client.get_registry_mutex();

    {
        std::lock_guard<std::mutex> lock(registry_mutex);

        if (!multiplayer_) {
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

        // If server sent restart vote status, it means all players are dead - force game over
        if (multiplayer_ && renderer.is_restart_vote_active() && !game_over_) {
            game_over_ = true;
            all_players_dead_ = true;
            spectator_choice_pending_ = false;
        }

        // If player is dead OR entity removed in multiplayer, show choice dialog
        // BUT if all players are dead, show standard Game Over instead
        if (multiplayer_ && client.is_connected() && player_id != 0 && (!player_exists || player_dead) &&
            !spectator_choice_pending_ && !has_chosen_spectate_ && !all_players_dead_) {
            spectator_choice_pending_ = true;
        }

        if (client.is_connected() && player_id != 0 &&
            ((!multiplayer_ && (!player_exists || player_dead)) || (multiplayer_ && all_players_dead_) ||
             ((!player_exists || player_dead) && !spectator_choice_pending_ && !has_chosen_spectate_))) {
            if (!game_over_) {
                game_over_ = true;

                if (!score_saved_) {
                    score_saved_ = true;
                    uint32_t final_score = max_score_reached_;
                    std::string player_name = client.get_player_name();
                    if (player_name.empty() || player_name == "Player") {
                        player_name = "Player" + std::to_string(client.get_player_id());
                    }

                    if (initial_player_count_ <= 1) {
                        client.get_scoreboard_manager().add_solo_score(player_name, final_score);
                    } else {
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
            if (multiplayer_ && renderer.is_restart_triggered()) {
                renderer.reset_game_state();
                game_over_ = false;
                all_players_dead_ = false;
                score_saved_ = false;
                max_score_reached_ = 0;
                has_chosen_spectate_ = false;
                spectator_choice_pending_ = false;
            }
            return;
        }

        bool window_has_focus = renderer.get_window() && renderer.get_window()->hasFocus();

        if (window_has_focus && !game_over_) {
            if (!multiplayer_ && !game_start_sent_ && client.is_connected()) {
                client.send_game_start_request();
                game_start_sent_ = true;
            }
            {
                rtype::ecs::InputSystem input_system(renderer.is_moving_up(), renderer.is_moving_down(),
                                                     renderer.is_moving_left(), renderer.is_moving_right(), 400.0f);
                input_system.update(registry, delta_time);
            }

            {
                rtype::ecs::InputSystem input_system(renderer.is_moving_up(), renderer.is_moving_down(),
                                                     renderer.is_moving_left(), renderer.is_moving_right(), 400.0f);
                input_system.update(registry, delta_time);
            }

            {
                auto view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Position>();
                for (auto entity : view) {
                    auto& pos = registry.getComponent<rtype::ecs::component::Position>(static_cast<size_t>(entity));
                    if (pos.x < 0)
                        pos.x = 0;
                    if (pos.x > 1920 - 100)
                        pos.x = 1920 - 100;
                    if (pos.y < 0)
                        pos.y = 0;
                    if (pos.y > 1080 - 100)
                        pos.y = 1080 - 100;
                }
            }

            {
                auto view = registry.view<rtype::ecs::component::Controllable, rtype::ecs::component::Velocity>();
                for (auto entity : view) {
                    auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(static_cast<size_t>(entity));
                    if (client.is_connected()) {
                        client.send_move(vel.vx, vel.vy);
                    }
                }
            }

            if (shoot_requested_) {
                if (client.is_connected()) {
                    client.send_shoot(0, 0);
                }
                shoot_requested_ = false;
            }
        }

        // Send ping if requested by PingSystem (always, regardless of focus or game state)
        if (multiplayer_ && client.is_connected()) {
            auto ping_view = registry.view<rtype::ecs::component::PingStats>();
            for (auto entity : ping_view) {
                auto& stats = registry.getComponent<rtype::ecs::component::PingStats>(entity);
                if (stats.pingRequested) {
                    auto now = std::chrono::steady_clock::now();
                    uint64_t timestamp =
                        std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
                    client.send_ping(timestamp);
                    stats.pingRequested = false;
                }
            }
        }

        if (!window_has_focus || game_over_) {
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
        GameEngine::Registry& registry = client.get_registry();
        std::mutex& registry_mutex = client.get_registry_mutex();
        std::lock_guard<std::mutex> lock(registry_mutex);

        // Check if there's a boss entity in the registry
        bool boss_present = false;
        auto view = registry.view<rtype::ecs::component::Tag>();
        for (auto entity : view) {
            auto& tag = registry.getComponent<rtype::ecs::component::Tag>(static_cast<GameEngine::entity_t>(entity));
            if (tag.name == "Boss_1" || tag.name == "Boss_2") {
                boss_present = true;
                break;
            }
        }
        renderer.set_boss_active(boss_present);

        renderer.draw_background();
        renderer.get_window()->setView(renderer.get_window()->getDefaultView());
    }

    if (renderer.get_window()) {
        GameEngine::Registry& registry = client.get_registry();
        std::mutex& registry_mutex = client.get_registry_mutex();
        std::lock_guard<std::mutex> lock(registry_mutex);

        auto sfml_renderer =
            std::make_shared<rtype::rendering::SFMLRenderer>(*renderer.get_window(), renderer.get_textures());

        rtype::ecs::RenderSystem render_system(sfml_renderer, &renderer.get_accessibility_manager());
        render_system.update(registry, 0.016f);

        if (multiplayer_) {
            rtype::ecs::LagometerSystem lagometer_system;
            lagometer_system.update(registry, 0.016, *renderer.get_window());
        }

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

    // FPS Counter: Update FPS calculation and render UI overlay (Multiplayer only)
    if (multiplayer_ && renderer.get_window()) {
        float fps_dt = fps_clock_.restart().asSeconds();

        GameEngine::Registry& registry = client.get_registry();
        std::mutex& registry_mutex = client.get_registry_mutex();
        std::lock_guard<std::mutex> lock(registry_mutex);

        // Update FPS calculation with REAL delta time
        rtype::ecs::FpsSystem fps_system;
        fps_system.update(registry, static_cast<double>(fps_dt));

        // Update Dev Metrics with REAL delta time
        rtype::ecs::PingSystem ping_system;
        ping_system.update(registry, static_cast<double>(fps_dt));

        rtype::ecs::CpuMetricSystem cpu_metric_system;
        cpu_metric_system.update(registry, static_cast<double>(fps_dt));

        if (has_chosen_spectate_ && spectator_system_) {
            spectator_system_->update(registry, static_cast<double>(fps_dt));
        }

        // Render UI overlay (FPS counter, Ping, CPU)
        rtype::ecs::UIRenderSystem ui_render_system(renderer.get_window());
        ui_render_system.update(registry, static_cast<double>(fps_dt));
    }

    // Stage cleared victory screen
    renderer.draw_stage_cleared();

    // Only show game over if victory screen is NOT displayed
    if (game_over_ && !renderer.is_stage_cleared()) {
        renderer.draw_game_over(all_players_dead_, !multiplayer_);
    }

    if (is_paused_) {
        render_pause_overlay(renderer);
    }

    // Stage cleared victory screen
    renderer.draw_stage_cleared();

    // Spectator choice dialog - shown when player becomes spectator
    if (spectator_choice_pending_) {
        sf::Vector2u windowSize = renderer.get_window_size();
        float center_x = windowSize.x / 2.0f;
        float center_y = windowSize.y / 2.0f;

        // Semi-transparent background
        sf::RectangleShape modal_bg(sf::Vector2f(static_cast<float>(windowSize.x), static_cast<float>(windowSize.y)));
        modal_bg.setFillColor(sf::Color(0, 0, 0, 200));
        renderer.draw_rectangle(modal_bg);

        // Title
        sf::FloatRect title_bounds = spectator_title_text_.getLocalBounds();
        spectator_title_text_.setPosition(center_x - title_bounds.width / 2.0f, center_y - 150.0f);
        renderer.draw_text(spectator_title_text_);

        // Continue button
        spectator_continue_button_.setPosition(center_x - 200.0f, center_y);

        sf::FloatRect continue_text_bounds = spectator_continue_text_.getLocalBounds();
        spectator_continue_text_.setPosition(
            spectator_continue_button_.getPosition().x + (400.0f - continue_text_bounds.width) / 2.0f,
            spectator_continue_button_.getPosition().y + (60.0f - continue_text_bounds.height) / 2.0f - 5.0f);

        renderer.draw_rectangle(spectator_continue_button_);
        renderer.draw_text(spectator_continue_text_);

        // Menu button
        spectator_menu_button_.setPosition(center_x - 200.0f, center_y + 80.0f);

        sf::FloatRect menu_text_bounds = spectator_menu_text_.getLocalBounds();
        spectator_menu_text_.setPosition(
            spectator_menu_button_.getPosition().x + (400.0f - menu_text_bounds.width) / 2.0f,
            spectator_menu_button_.getPosition().y + (60.0f - menu_text_bounds.height) / 2.0f - 5.0f);

        renderer.draw_rectangle(spectator_menu_button_);
        renderer.draw_text(spectator_menu_text_);
    }

    if (has_chosen_spectate_ && !game_over_) {
        sf::Vector2u window_size = renderer.get_window_size();
        sf::FloatRect text_bounds = spectator_mode_text_.getLocalBounds();
        spectator_mode_text_.setPosition(window_size.x / 2.0f - text_bounds.width / 2.0f, 50.0f);
        renderer.draw_text(spectator_mode_text_);

        // Draw HUD Exit Button (Top-Right)
        float btn_x = window_size.x - 120.0f;
        float btn_y = 100.0f;
        spectator_hud_exit_button_.setPosition(btn_x, btn_y);
        renderer.draw_rectangle(spectator_hud_exit_button_);

        sf::FloatRect exit_text_bounds = spectator_hud_exit_text_.getLocalBounds();
        spectator_hud_exit_text_.setPosition(btn_x + 50.0f - exit_text_bounds.width / 2.0f,
                                             btn_y + 15.0f - exit_text_bounds.height / 2.0f - 4.0f); // Centered
        renderer.draw_text(spectator_hud_exit_text_);
    }

    // Only show game over if victory screen is NOT displayed AND NOT in spectator choice
    // But IF all players are dead, force show game over
    if (game_over_ && !renderer.is_stage_cleared() && !spectator_choice_pending_) {
        // Draw game over: show restart button only in solo mode (!multiplayer_)
        renderer.draw_game_over(all_players_dead_, !multiplayer_);
        // Multiplayer: show restart vote UI when all players are dead
        if (multiplayer_ && all_players_dead_ && renderer.is_restart_vote_active()) {
            renderer.draw_restart_vote_ui();
        }
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

void GameState::setup_spectator_ui() {
    if (!font_loaded_) {
        return;
    }

    // Choice Dialog
    spectator_title_text_.setFont(font_);
    spectator_title_text_.setString("GAME OVER");
    spectator_title_text_.setCharacterSize(60);
    spectator_title_text_.setFillColor(sf::Color::Red);

    spectator_continue_button_.setSize(sf::Vector2f(400, 60));
    spectator_continue_button_.setFillColor(sf::Color(70, 130, 180)); // Same blue as Play Again button
    spectator_continue_button_.setOutlineColor(sf::Color::White);
    spectator_continue_button_.setOutlineThickness(2);

    spectator_continue_text_.setFont(font_);
    spectator_continue_text_.setString("CONTINUE TO SPECTATE");
    spectator_continue_text_.setCharacterSize(23);
    spectator_continue_text_.setFillColor(sf::Color::White);

    spectator_menu_button_.setSize(sf::Vector2f(400, 60));
    spectator_menu_button_.setFillColor(sf::Color(150, 50, 50)); // Red-ish
    spectator_menu_button_.setOutlineColor(sf::Color::White);
    spectator_menu_button_.setOutlineThickness(2);

    spectator_menu_text_.setFont(font_);
    spectator_menu_text_.setString("BACK TO MENU");
    spectator_menu_text_.setCharacterSize(24);
    spectator_menu_text_.setFillColor(sf::Color::White);

    // Spectator HUD
    spectator_mode_text_.setFont(font_);
    spectator_mode_text_.setString("SPECTATOR MODE");
    spectator_mode_text_.setCharacterSize(40);
    spectator_mode_text_.setFillColor(sf::Color(255, 255, 0)); // Yellow

    spectator_hud_exit_button_.setSize(sf::Vector2f(100, 30));
    spectator_hud_exit_button_.setFillColor(sf::Color(150, 50, 50, 180));
    spectator_hud_exit_button_.setOutlineColor(sf::Color::White);
    spectator_hud_exit_button_.setOutlineThickness(1);

    spectator_hud_exit_text_.setFont(font_);
    spectator_hud_exit_text_.setString("EXIT");
    spectator_hud_exit_text_.setCharacterSize(15);
    spectator_hud_exit_text_.setFillColor(sf::Color::White);
}

void GameState::spawn_enemy_solo(GameEngine::Registry& registry) {
    auto entity = registry.createEntity();
    float x = 1400.0f + (rand() % 400);
    float y = 100.0f + (rand() % 800);

    registry.addComponent<rtype::ecs::component::NetworkId>(entity, next_enemy_id_++);
    registry.addComponent<rtype::ecs::component::Position>(entity, x, y);
    registry.addComponent<rtype::ecs::component::Velocity>(entity, -200.0f, 0.0f);

    int sub_type = 1 + (rand() % 4);
    std::string sprite_name;
    if (sub_type == 1)
        sprite_name = "monster_0-top";
    else if (sub_type == 2)
        sprite_name = "monster_0-bot";
    else if (sub_type == 3)
        sprite_name = "monster_0-left";
    else
        sprite_name = "monster_0-right";

    registry.addComponent<rtype::ecs::component::Drawable>(entity, sprite_name, 0, 0, 0, 0, 4.0f, 4.0f, 1, 0.1f, true);

    registry.addComponent<rtype::ecs::component::Health>(entity, 100.0f, 100.0f);
    registry.addComponent<rtype::ecs::component::HitBox>(entity, 100.0f, 100.0f);
    registry.addComponent<rtype::ecs::component::Collidable>(entity, rtype::ecs::component::CollisionLayer::Enemy);
    registry.addComponent<rtype::ecs::component::Tag>(entity, "EnemyMonster");
    registry.addComponent<rtype::ecs::component::NetworkInterpolation>(entity, x, y, -200.0f, 0.0f);
}

void GameState::spawn_player_projectile(GameEngine::Registry& registry, GameEngine::entity_t player_entity) {
    if (!registry.hasComponent<rtype::ecs::component::Position>(player_entity) ||
        !registry.hasComponent<rtype::ecs::component::Weapon>(player_entity)) {
        return;
    }

    auto& pos = registry.getComponent<rtype::ecs::component::Position>(player_entity);
    auto& weapon = registry.getComponent<rtype::ecs::component::Weapon>(player_entity);

    auto projectile = registry.createEntity();
    registry.addComponent<rtype::ecs::component::NetworkId>(projectile, next_projectile_id_++);
    float spawn_x = pos.x + weapon.spawnOffsetX;
    float spawn_y = pos.y + weapon.spawnOffsetY;
    registry.addComponent<rtype::ecs::component::Position>(projectile, spawn_x, spawn_y);
    registry.addComponent<rtype::ecs::component::Velocity>(projectile, weapon.projectileSpeed, 0.0f);
    registry.addComponent<rtype::ecs::component::Projectile>(projectile, weapon.damage, weapon.projectileLifetime,
                                                             static_cast<std::size_t>(player_entity));
    registry.addComponent<rtype::ecs::component::HitBox>(projectile, 32.0f, 32.0f);
    registry.addComponent<rtype::ecs::component::Collidable>(projectile,
                                                             rtype::ecs::component::CollisionLayer::PlayerProjectile);
    registry.addComponent<rtype::ecs::component::Tag>(projectile, "PlayerProjectile");
    registry.addComponent<rtype::ecs::component::Drawable>(projectile, "shot", 0, 0, 29, 33, 3.0f, 3.0f, 4, 0.05f,
                                                           true);
    registry.addComponent<rtype::ecs::component::NetworkInterpolation>(projectile, spawn_x, spawn_y,
                                                                       weapon.projectileSpeed, 0.0f);
}

void GameState::createFpsCounter(GameEngine::Registry& registry, float windowWidth) {
    if (!dev_font_) {
        dev_font_ = std::make_shared<sf::Font>();
        if (!dev_font_->loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
            std::cerr << "Warning: Could not load client/fonts/Ethnocentric-Regular.otf for FPS Counter" << std::endl;
            // Fallback to default arial if possible, or just don't crash
        }
    }

    fps_counter_entity_ = registry.createEntity();

    float x = windowWidth - 230.0f;
    float y = 10.0f;

    registry.addComponent<rtype::ecs::component::Position>(fps_counter_entity_, x, y);
    registry.addComponent<rtype::ecs::component::UITag>(fps_counter_entity_);
    registry.addComponent<rtype::ecs::component::FpsCounter>(fps_counter_entity_);

    // Text Setup
    // Using green color for high visibility
    sf::Color textColor = sf::Color::Green;
    registry.addComponent<rtype::ecs::component::TextDrawable>(fps_counter_entity_, dev_font_, "FPS: --", 20,
                                                               textColor);
}

void GameState::createDevMetrics(GameEngine::Registry& registry, float windowWidth) {
    // Ping Entity
    auto ping_entity = registry.createEntity();
    float x = windowWidth - 230.0f;
    float y_ping = 40.0f;

    registry.addComponent<rtype::ecs::component::Position>(ping_entity, x, y_ping);
    registry.addComponent<rtype::ecs::component::UITag>(ping_entity);
    registry.addComponent<rtype::ecs::component::PingStats>(ping_entity);
    registry.addComponent<rtype::ecs::component::TextDrawable>(ping_entity, dev_font_, "PING: -- ms", 20,
                                                               sf::Color::Green);

    // CPU Entity
    auto cpu_entity = registry.createEntity();
    float y_cpu = 70.0f;

    registry.addComponent<rtype::ecs::component::Position>(cpu_entity, x, y_cpu);
    registry.addComponent<rtype::ecs::component::UITag>(cpu_entity);
    registry.addComponent<rtype::ecs::component::CpuStats>(cpu_entity);
    registry.addComponent<rtype::ecs::component::TextDrawable>(cpu_entity, dev_font_, "CPU: -- ms", 20,
                                                               sf::Color::Green);
}

void GameState::createLagometer(GameEngine::Registry& registry, float windowWidth) {
    (void)windowWidth;
    lagometer_entity_ = registry.createEntity();

    float x = 20.0f;
    float y = 600.0f;

    registry.addComponent<rtype::ecs::component::Position>(lagometer_entity_, x, y);
    registry.addComponent<rtype::ecs::component::UITag>(lagometer_entity_);
    registry.addComponent<rtype::ecs::component::LagometerComponent>(lagometer_entity_);
}

} // namespace rtype::client
