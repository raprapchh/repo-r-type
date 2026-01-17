#pragma once

#include "States.hpp"
#include "utils/GameRules.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::client {

class SoloSettingsState : public IState {
  public:
    SoloSettingsState();
    ~SoloSettingsState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::SoloSettings;
    }

  private:
    void setup_ui();
    void update_positions(const sf::Vector2u& window_size);

    enum class SelectedButton { DIFFICULTY, LIVES, START, BACK };
    SelectedButton selected_button_ = SelectedButton::START;

    sf::Font font_;
    bool font_loaded_;

    sf::Text title_text_;

    sf::Text difficulty_label_;
    sf::Text difficulty_text_;
    sf::RectangleShape difficulty_button_;
    rtype::config::Difficulty selected_difficulty_;

    sf::Text lives_label_;
    sf::Text lives_text_;
    sf::RectangleShape lives_button_;
    uint8_t selected_lives_;

    sf::Text start_button_text_;
    sf::RectangleShape start_button_;

    sf::Text back_button_text_;
    sf::RectangleShape back_button_;
};

} // namespace rtype::client
