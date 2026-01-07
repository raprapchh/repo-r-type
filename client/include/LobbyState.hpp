#pragma once

#include "States.hpp"
#include <SFML/Graphics.hpp>
#include <atomic>
#include <map>

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

    void add_player(uint32_t player_id, const std::string& name);
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
    sf::RectangleShape input_background_;
    sf::Text name_input_label_;
    sf::Text name_input_text_;
    bool is_typing_name_;
    std::string current_name_input_;
    float backspace_timer_;
    float backspace_delay_;
    bool was_backspace_pressed_;
    static constexpr float INITIAL_BACKSPACE_DELAY = 0.4f;
    static constexpr float REPEAT_BACKSPACE_DELAY = 0.05f;

    bool font_loaded_;
    std::map<uint32_t, std::string> connected_players_;
    uint8_t player_count_;
    std::atomic<bool> game_started_;
    Renderer* renderer_ref_;
    uint32_t local_player_id_;

    // Thread-safe pending queues for callbacks (network thread -> main thread)
    std::mutex pending_mutex_;
    std::vector<std::pair<uint32_t, std::string>> pending_player_joins_;
    std::vector<std::pair<uint32_t, std::string>> pending_name_updates_;
    std::vector<std::pair<std::string, std::string>> pending_chat_messages_;

    // Chat UI
    sf::RectangleShape chat_background_;
    sf::RectangleShape chat_input_background_;
    sf::Text chat_input_label_;
    sf::Text chat_input_text_;
    std::string current_chat_input_;
    std::vector<std::pair<std::string, std::string>> chat_messages_; // {player_name, message}
    std::vector<sf::Text> chat_message_texts_;
    bool is_typing_chat_;
    float chat_backspace_timer_;
    float chat_backspace_delay_;
    bool was_chat_backspace_pressed_;
    static constexpr size_t MAX_CHAT_MESSAGES = 9;
    static constexpr size_t MAX_CHAT_INPUT_LENGTH = 100;

    void add_chat_message(const std::string& player_name, const std::string& message);
    void update_chat_display();
    void process_pending_events();
};

} // namespace rtype::client
