#pragma once

#include "States.hpp"
#include "utils/GameRules.hpp"
#include <chrono>
#include <memory>

namespace rtype::ecs {
class SpectatorSystem;
}

namespace sf {
class Font;
}

namespace rtype::client {

class GameState : public IState {
  public:
    explicit GameState(bool multiplayer = true,
                       rtype::config::Difficulty difficulty = rtype::config::Difficulty::NORMAL, uint8_t lives = 3);
    ~GameState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::Game;
    }

  private:
    void setup_pause_ui();
    void handle_pause_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager);
    void update_pause_ui_positions(const sf::Vector2u& window_size);
    void render_pause_overlay(Renderer& renderer);

    bool shoot_requested_ = false;
    Client* client_ = nullptr;
    bool game_over_ = false;
    bool all_players_dead_ = false;
    bool score_saved_ = false;
    int initial_player_count_ = 0;
    uint32_t max_score_reached_ = 0;
    bool is_charging_ = false;
    std::chrono::steady_clock::time_point charge_start_time_;

    bool is_paused_ = false;
    bool show_settings_panel_ = false;
    bool font_loaded_ = false;

    sf::Font font_;
    sf::Text pause_title_text_;
    sf::RectangleShape settings_button_;
    sf::Text settings_button_text_;

    sf::RectangleShape accessibility_cycle_button_;
    sf::Text accessibility_cycle_text_;

    bool multiplayer_ = true;
    rtype::config::Difficulty solo_difficulty_ = rtype::config::Difficulty::NORMAL;
    uint8_t solo_lives_ = 3;
    float spawn_timer_ = 0.0f;
    float spawn_interval_ = 1.5f;
    uint32_t next_enemy_id_ = 1000;
    uint32_t next_projectile_id_ = 5000;
    bool game_start_sent_ = false;
    float laser_energy_ = 3.0f;
    bool is_firing_laser_ = false;

    // FPS Counter (Developer Console)
    std::shared_ptr<sf::Font> dev_font_;
    bool dev_tools_visible_ = true;
    GameEngine::entity_t fps_counter_entity_ = 0;
    sf::Clock fps_clock_;

    void spawn_enemy_solo(GameEngine::Registry& registry);
    void spawn_player_projectile(GameEngine::Registry& registry, GameEngine::entity_t player_entity);
    void createFpsCounter(GameEngine::Registry& registry, float windowWidth);
    void createDevMetrics(GameEngine::Registry& registry, float windowWidth);

    // Spectator mode
    std::shared_ptr<rtype::ecs::SpectatorSystem> spectator_system_;

    void setup_spectator_ui();

    bool spectator_choice_pending_ = false;
    bool has_chosen_spectate_ = false;

    // UI elements for spectator choice
    sf::Text spectator_title_text_;
    sf::RectangleShape spectator_continue_button_;
    sf::Text spectator_continue_text_;
    sf::RectangleShape spectator_menu_button_;
    sf::Text spectator_menu_text_;

    // UI elements for spectator HUD
    sf::Text spectator_mode_text_;
    sf::RectangleShape spectator_hud_exit_button_;
    sf::Text spectator_hud_exit_text_;
};

} // namespace rtype::client
