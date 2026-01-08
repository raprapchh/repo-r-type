#pragma once

#include "States.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::client {

class MenuState : public IState {
  public:
    MenuState();
    ~MenuState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::Menu;
    }

  private:
    void setup_ui();
    void handle_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager);
    void update_positions(const sf::Vector2u& window_size);
    void update_scoreboard_display(Client& client);

    sf::Font font_;
    sf::Texture logo_texture_;
    sf::Sprite logo_sprite_;
    sf::Text start_button_text_;
    sf::Text quit_button_text_;
    sf::RectangleShape start_button_;
    sf::RectangleShape settings_button_;
    sf::RectangleShape quit_button_;
    
    // Scoreboard display
    sf::Text solo_scores_title_;
    sf::Text multi_scores_title_;
    std::vector<sf::Text> solo_score_texts_;
    std::vector<sf::Text> multi_score_texts_;

    bool show_settings_ = false;
    ColorBlindMode pending_mode_ = ColorBlindMode::None;
    sf::RectangleShape accessibility_cycle_button_;
    sf::Text accessibility_cycle_text_;
    sf::Text settings_button_text_;

    sf::RectangleShape confirm_button_;
    sf::RectangleShape cancel_button_;
    sf::Text confirm_button_text_;
    sf::Text cancel_button_text_;

    bool font_loaded_;
};

} // namespace rtype::client
