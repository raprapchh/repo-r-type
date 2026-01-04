#include "../include/LobbyState.hpp"
#include "../include/GameState.hpp"
#include "../include/States.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace rtype::client {

LobbyState::LobbyState()
    : is_typing_name_(false), backspace_timer_(0.0f), backspace_delay_(INITIAL_BACKSPACE_DELAY),
      was_backspace_pressed_(false), font_loaded_(false), player_count_(0), game_started_(false),
      renderer_ref_(nullptr), local_player_id_(0) {
    setup_ui();
}

void LobbyState::setup_ui() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr
            << "Warning: Could not load font from client/fonts/Ethnocentric-Regular.otf. UI will not display text."
            << std::endl;
        font_loaded_ = false;
        return;
    }
    font_loaded_ = true;

    title_text_.setFont(font_);
    title_text_.setString("LOBBY");
    title_text_.setCharacterSize(60);
    title_text_.setFillColor(sf::Color::White);
    title_text_.setStyle(sf::Text::Bold);

    waiting_text_.setFont(font_);
    waiting_text_.setString("Waiting for players...");
    waiting_text_.setCharacterSize(30);
    waiting_text_.setFillColor(sf::Color::Yellow);

    players_text_.setFont(font_);
    players_text_.setString("Players: 0/4");
    players_text_.setCharacterSize(24);
    players_text_.setFillColor(sf::Color::White);

    player_list_text_.setFont(font_);
    player_list_text_.setString("");
    player_list_text_.setCharacterSize(20);
    player_list_text_.setFillColor(sf::Color::Cyan);

    start_button_.setSize(sf::Vector2f(300, 60));
    start_button_.setFillColor(sf::Color(100, 150, 200));
    start_button_.setOutlineColor(sf::Color::White);
    start_button_.setOutlineThickness(2);

    start_button_text_.setFont(font_);
    start_button_text_.setString("START GAME");
    start_button_text_.setString("START GAME");
    start_button_text_.setCharacterSize(30);
    start_button_text_.setFillColor(sf::Color::White);

    // Name Input UI
    input_background_.setSize(sf::Vector2f(300, 40));
    input_background_.setFillColor(sf::Color(30, 30, 30, 200));  // Darker, slight transparency
    input_background_.setOutlineColor(sf::Color(100, 100, 100)); // Softer outline
    input_background_.setOutlineThickness(2);

    name_input_label_.setFont(font_);
    name_input_label_.setString("ENTER NAME:"); // All caps to match style
    name_input_label_.setCharacterSize(18);     // Slightly smaller
    name_input_label_.setFillColor(sf::Color(200, 200, 200));

    name_input_text_.setFont(font_);
    name_input_text_.setString("");
    name_input_text_.setCharacterSize(20);
    name_input_text_.setFillColor(sf::Color::White);

    is_typing_name_ = false;
    current_name_input_ = "";
}

void LobbyState::on_enter(Renderer& renderer, Client& client) {
    renderer_ref_ = &renderer;
    update_positions(renderer.get_window_size());

    connected_players_.clear();
    player_count_ = 0;
    game_started_ = false;

    if (!client.is_connected()) {
        client.connect();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    add_player(client.get_player_id(), client.get_player_name());
    local_player_id_ = client.get_player_id(); // Store local player ID
    set_player_count(static_cast<uint8_t>(connected_players_.size()));

    client.set_player_join_callback(
        [this](uint32_t player_id, const std::string& name) { add_player(player_id, name); });

    client.set_player_name_callback([this](uint32_t player_id, const std::string& name) {
        if (connected_players_.find(player_id) != connected_players_.end()) {
            std::string final_name = name;
            // Robust check for default names
            if (final_name.empty() || final_name == "Player" || final_name == "PLAYER" || final_name == "player" ||
                final_name == "Player 0" || final_name == "PLAYER 0") {
                final_name = "Player " + std::to_string(player_id);
            }
            connected_players_[player_id] = final_name;
            update_player_display();
        }
    });

    client.set_game_start_callback([this]() { game_started_.store(true); });

    // Initialize current name input with local player name if available, or fetch it
    current_name_input_ = client.get_player_name();
    if (current_name_input_.empty()) {
        current_name_input_ = "Player " + std::to_string(local_player_id_);
    }
    name_input_text_.setString(current_name_input_);

    // Immediately send this default name to server so others see "Player X" instead of empty/default
    if (client.is_connected()) {
        client.send_player_name_update(current_name_input_);
    }
}

void LobbyState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
}

void LobbyState::handle_input(Renderer& renderer, StateManager& state_manager) {
    (void)state_manager;
    sf::Event event;
    while (renderer.poll_event(event)) {
        if (event.type == sf::Event::Closed) {
            renderer.close_window();
        } else if (event.type == sf::Event::Resized) {
            renderer.handle_resize(event.size.width, event.size.height);
            update_positions(sf::Vector2u(event.size.width, event.size.height));
        } else if (event.type == sf::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2f mouse_pos = renderer.get_mouse_position();
                if (player_count_ >= 2 && start_button_.getGlobalBounds().contains(mouse_pos)) {
                    state_manager.get_client().send_game_start_request();
                }

                // Check name input click
                if (input_background_.getGlobalBounds().contains(mouse_pos)) {
                    is_typing_name_ = true;
                    input_background_.setOutlineColor(sf::Color::Cyan);
                } else {
                    is_typing_name_ = false;
                    input_background_.setOutlineColor(sf::Color::White);
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (is_typing_name_) {
                if (event.key.code == sf::Keyboard::Return) {
                    is_typing_name_ = false;
                    input_background_.setOutlineColor(sf::Color(100, 100, 100));

                    // Immediate update
                    if (current_name_input_.empty()) {
                        current_name_input_ = "Player " + std::to_string(local_player_id_);
                    }
                    connected_players_[local_player_id_] = current_name_input_;
                    update_player_display();
                    name_input_text_.setString(current_name_input_);

                    state_manager.get_client().send_player_name_update(current_name_input_);
                }
                // Backspace handled in update() for smooth repeat
            } else {
                if (event.key.code == sf::Keyboard::Escape) {
                } else if ((event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) &&
                           player_count_ >= 2) {
                    state_manager.get_client().send_game_start_request();
                }
            }
        } else if (event.type == sf::Event::TextEntered) {
            if (is_typing_name_) {
                if (event.text.unicode < 128 && event.text.unicode > 31 && current_name_input_.length() < 16) {
                    current_name_input_ += static_cast<char>(event.text.unicode);
                    name_input_text_.setString(current_name_input_);
                }
            }
        }
    }
}

void LobbyState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)client;
    // Backspace handling
    if (is_typing_name_ && sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)) {
        bool delete_char = false;

        if (!was_backspace_pressed_) {
            // First press: delete immediately
            delete_char = true;
            backspace_timer_ = 0.0f;
            backspace_delay_ = INITIAL_BACKSPACE_DELAY;
        } else {
            // Held down: wait for delay
            backspace_timer_ += delta_time;
            if (backspace_timer_ >= backspace_delay_) {
                delete_char = true;
                backspace_timer_ = 0.0f;
                backspace_delay_ = REPEAT_BACKSPACE_DELAY;
            }
        }

        if (delete_char && !current_name_input_.empty()) {
            current_name_input_.pop_back();
            name_input_text_.setString(current_name_input_);
        }
        was_backspace_pressed_ = true;
    } else {
        was_backspace_pressed_ = false;
        backspace_timer_ = 0.0f;
    }

    // Update local ID and default name if it was 0
    if (local_player_id_ == 0 && client.is_connected()) {
        local_player_id_ = client.get_player_id();
        if (local_player_id_ != 0) {
            // If name was "Player 0" or empty, update it
            if (current_name_input_.empty() || current_name_input_ == "Player" || current_name_input_ == "Player 0") {
                current_name_input_ = "Player " + std::to_string(local_player_id_);
                name_input_text_.setString(current_name_input_);
                client.send_player_name_update(current_name_input_);
            }
        }
    }

    if (game_started_.load()) {
        state_manager.change_state(std::make_unique<GameState>());
    }

    ColorBlindMode mode = renderer.get_accessibility_manager().get_current_mode();
    sf::Color start_normal(100, 150, 200);
    sf::Color start_hover(120, 170, 220);
    sf::Color players_text_color = sf::Color::White;
    sf::Color waiting_text_color = sf::Color::Yellow;

    switch (mode) {
    case ColorBlindMode::Deuteranopia:
        start_normal = sf::Color(0, 0, 255);
        start_hover = sf::Color(50, 50, 255);
        players_text_color = sf::Color(100, 100, 255);
        waiting_text_color = sf::Color(255, 165, 0);
        break;
    case ColorBlindMode::Protanopia:
        start_normal = sf::Color(0, 100, 255);
        start_hover = sf::Color(50, 120, 255);
        players_text_color = sf::Color(150, 150, 255);
        waiting_text_color = sf::Color(255, 255, 0);
        break;
    case ColorBlindMode::Tritanopia:
        start_normal = sf::Color(255, 0, 0);
        start_hover = sf::Color(255, 50, 50);
        players_text_color = sf::Color(255, 100, 100);
        waiting_text_color = sf::Color(0, 200, 200);
        break;
    case ColorBlindMode::None:
    default:
        break;
    }

    players_text_.setFillColor(players_text_color);
    waiting_text_.setFillColor(waiting_text_color);

    sf::Vector2f mouse_pos = renderer.get_mouse_position();
    if (player_count_ >= 2 && start_button_.getGlobalBounds().contains(mouse_pos)) {
        start_button_.setFillColor(start_hover);
    } else {
        start_button_.setFillColor(start_normal);
    }
}

void LobbyState::render(Renderer& renderer, Client& /* client */) {
    renderer.clear();

    if (font_loaded_) {
        renderer.draw_text(title_text_);
        renderer.draw_text(waiting_text_);
        renderer.draw_text(players_text_);
        renderer.draw_text(player_list_text_);
        if (player_count_ >= 2 && local_player_id_ == 1) {
            renderer.draw_rectangle(start_button_);
            renderer.draw_text(start_button_text_);
        }

        // Draw Name Input
        renderer.draw_rectangle(input_background_);
        renderer.draw_text(name_input_label_);
        renderer.draw_text(name_input_text_);
    }

    renderer.display();
}

void LobbyState::add_player(uint32_t player_id, const std::string& name) {
    if (connected_players_.find(player_id) == connected_players_.end()) {
        std::string final_name = name;
        if (final_name.empty() || final_name == "Player" || final_name == "PLAYER" || final_name == "player" ||
            final_name == "Player 0" || final_name == "PLAYER 0") {
            final_name = "Player " + std::to_string(player_id);
        }
        connected_players_[player_id] = final_name;
        set_player_count(static_cast<uint8_t>(connected_players_.size()));
    }
}

void LobbyState::remove_player(uint32_t player_id) {
    if (connected_players_.erase(player_id)) {
        set_player_count(static_cast<uint8_t>(connected_players_.size()));
    }
}

void LobbyState::set_player_count(uint8_t count) {
    player_count_ = count;
    update_player_display();
}

void LobbyState::update_player_display() {
    if (!font_loaded_)
        return;

    players_text_.setString("Players: " + std::to_string(player_count_) + "/4");

    std::string player_list = "Connected: ";
    int idx = 0;
    for (const auto& [id, name] : connected_players_) {
        if (idx > 0)
            player_list += ", ";
        player_list += name; // Display name
        idx++;
    }
    player_list_text_.setString(player_list);

    if (renderer_ref_) {
        update_positions(renderer_ref_->get_window_size());
    }
}

void LobbyState::update_positions(const sf::Vector2u& window_size) {
    float title_y = window_size.y * 0.1f;
    float waiting_y = window_size.y * 0.35f;
    float players_y = window_size.y * 0.45f;
    float list_y = window_size.y * 0.53f;
    float button_y = window_size.y * 0.7f;

    title_text_.setPosition((window_size.x - title_text_.getLocalBounds().width) / 2.0f, title_y);
    waiting_text_.setPosition((window_size.x - waiting_text_.getLocalBounds().width) / 2.0f, waiting_y);
    players_text_.setPosition((window_size.x - players_text_.getLocalBounds().width) / 2.0f, players_y);
    player_list_text_.setPosition((window_size.x - player_list_text_.getLocalBounds().width) / 2.0f, list_y);

    float button_width = std::min(300.0f, window_size.x * 0.25f);
    float button_height = std::min(60.0f, window_size.y * 0.08f);
    start_button_.setSize(sf::Vector2f(button_width, button_height));
    start_button_.setPosition((window_size.x - button_width) / 2.0f, button_y);
    start_button_text_.setPosition(
        start_button_.getPosition().x + (button_width - start_button_text_.getLocalBounds().width) / 2.0f,
        start_button_.getPosition().y + (button_height - start_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    float input_y = window_size.y * 0.25f;
    input_background_.setPosition((window_size.x - 300) / 2.0f, input_y);
    // Position label above the input box centered
    name_input_label_.setPosition((window_size.x - name_input_label_.getLocalBounds().width) / 2.0f, input_y - 25);
    // Position text inside input box
    name_input_text_.setPosition(input_background_.getPosition().x + 10, input_y + 8);
}

} // namespace rtype::client
