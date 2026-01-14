#pragma once

#include "States.hpp"
#include "utils/GameRules.hpp"
#include <SFML/Graphics.hpp>
#include <atomic>
#include <map>

namespace rtype::client {

enum class LobbyMode { MAIN_MENU, CREATE_ROOM, BROWSE_ROOMS, IN_ROOM };

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

    LobbyMode current_mode_;
    enum class MainMenuButton { CREATE, JOIN, BACK };
    MainMenuButton selected_main_menu_button_ = MainMenuButton::CREATE;

    enum class CreateRoomButton { INPUT, CREATE, BACK };
    CreateRoomButton selected_create_room_button_ = CreateRoomButton::INPUT;

    enum class BrowseRoomsButton { REFRESH, BACK };
    BrowseRoomsButton selected_browse_rooms_button_ = BrowseRoomsButton::REFRESH;

    enum class InRoomButton { START, LEAVE };
    InRoomButton selected_in_room_button_ = InRoomButton::START;

    sf::RectangleShape create_button_;
    sf::Text create_button_text_;
    sf::RectangleShape join_button_;
    sf::Text join_button_text_;
    sf::RectangleShape back_button_;
    sf::Text back_button_text_;
    sf::RectangleShape leave_room_button_;
    sf::Text leave_room_button_text_;

    struct RoomEntry {
        uint32_t session_id;
        uint8_t player_count;
        uint8_t max_players;
        std::string status;
        std::string room_name;
    };
    std::vector<RoomEntry> available_rooms_;
    std::mutex rooms_mutex_;
    std::atomic<bool> room_list_needs_update_;
    std::atomic<bool> lobby_update_pending_;
    std::atomic<int8_t> pending_player_count_;
    std::atomic<int8_t> pending_your_player_id_;
    sf::Text rooms_title_text_;
    sf::Text create_room_button_text_;
    sf::RectangleShape create_room_button_;
    sf::Text refresh_button_text_;
    sf::RectangleShape refresh_button_;
    std::vector<sf::Text> room_texts_;
    std::vector<sf::RectangleShape> room_buttons_;
    int selected_room_index_;
    bool is_typing_room_name_;
    std::string current_room_name_input_;
    sf::RectangleShape room_input_background_;
    sf::Text room_input_label_;
    sf::Text room_input_text_;
    bool was_room_backspace_pressed_;
    float room_backspace_timer_;
    float room_backspace_delay_;

    std::mutex pending_mutex_;
    std::vector<std::pair<uint32_t, std::string>> pending_player_joins_;
    std::vector<std::pair<uint32_t, std::string>> pending_name_updates_;
    std::vector<std::pair<std::string, std::string>> pending_chat_messages_;

    sf::RectangleShape chat_background_;
    sf::RectangleShape chat_input_background_;
    sf::Text chat_input_label_;
    sf::Text chat_input_text_;
    std::string current_chat_input_;
    std::vector<std::pair<std::string, std::string>> chat_messages_;
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
    void update_room_list_display();
    void add_room(uint32_t session_id, uint8_t player_count, uint8_t max_players, uint8_t status,
                  const std::string& room_name);

    rtype::config::GameMode selected_game_mode_;
    rtype::config::Difficulty selected_difficulty_;
    bool selected_friendly_fire_;
    sf::Text game_mode_label_;
    sf::Text game_mode_text_;
    sf::RectangleShape game_mode_button_;
    sf::Text difficulty_label_;
    sf::Text difficulty_text_;
    sf::RectangleShape difficulty_button_;
    sf::Text friendly_fire_label_;
    sf::Text friendly_fire_text_;
    sf::RectangleShape friendly_fire_button_;
};

} // namespace rtype::client
