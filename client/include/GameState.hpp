#pragma once

#include "States.hpp"
#include <chrono>

namespace rtype::client {

class GameState : public IState {
  public:
    GameState();
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
};

} // namespace rtype::client
