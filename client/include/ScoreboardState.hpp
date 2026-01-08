#pragma once

#include "States.hpp"
#include "ScoreboardManager.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::client {

/// @brief State that displays the scoreboard
class ScoreboardState : public IState {
  public:
    ScoreboardState(ScoreboardManager& scoreboard_manager);
    ~ScoreboardState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::Scoreboard;
    }

  private:
    void setup_ui();
    void update_positions(const sf::Vector2u& window_size);
    void update_score_display();

    ScoreboardManager& scoreboard_manager_;

    sf::Font font_;
    bool font_loaded_;

    // UI Elements
    sf::Text title_text_;
    sf::Text solo_title_text_;
    sf::Text multi_title_text_;

    std::vector<sf::Text> solo_score_texts_;
    std::vector<sf::Text> multi_score_texts_;

    sf::RectangleShape back_button_;
    sf::Text back_button_text_;

    sf::RectangleShape solo_tab_;
    sf::RectangleShape multi_tab_;
    sf::Text solo_tab_text_;
    sf::Text multi_tab_text_;

    enum class TabMode { Solo, Multi };
    TabMode current_tab_;
};

} // namespace rtype::client
