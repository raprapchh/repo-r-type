#pragma once

#include "States.hpp"
#include <SFML/Graphics.hpp>
#include <atomic>
#include <vector>

namespace rtype::client {

class LobbyState : public IState {
  public:
    LobbyState();
    ~LobbyState() override = default;

    void handle_input(Renderer& renderer, StateManager& state_manager) override;
    void update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) override;
    void render(Renderer& renderer, Client& client) override;
    void on_enter(Renderer& renderer, Client& client) override;
    void on_exit(Renderer& renderer, Client& client) override;
    StateType get_type() const override {
        return StateType::Lobby;
    }

    void add_player(uint32_t player_id);
    void remove_player(uint32_t player_id);
    void set_player_count(uint8_t count);

  private:
    void setup_ui();
    void update_player_display();
    void update_positions(const sf::Vector2u& window_size);

    sf::Font font_;
    sf::Text title_text_;
    sf::Text waiting_text_;
    sf::Text players_text_;
    sf::Text player_list_text_;
    sf::Text start_button_text_;
    sf::RectangleShape start_button_;
    bool font_loaded_;
    std::vector<uint32_t> connected_players_;
    uint8_t player_count_;
    std::atomic<bool> game_started_;
    Renderer* renderer_ref_;
};

} // namespace rtype::client
