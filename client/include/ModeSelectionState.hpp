#pragma once

#include "States.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::client {

class ModeSelectionState : public IState {
  public:
    ModeSelectionState();
    ~ModeSelectionState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::ModeSelection;
    }

  private:
    void setup_ui();
    void handle_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager);
    void update_positions(const sf::Vector2u& window_size);

    enum class SelectedButton { SOLO, MULTIPLAYER, BACK };
    SelectedButton selected_button_ = SelectedButton::SOLO;

    sf::Font font_;
    sf::Text title_text_;
    sf::Text solo_button_text_;
    sf::Text multiplayer_button_text_;
    sf::Text back_button_text_;
    sf::RectangleShape solo_button_;
    sf::RectangleShape multiplayer_button_;
    sf::RectangleShape back_button_;
    bool font_loaded_;
};

} // namespace rtype::client
