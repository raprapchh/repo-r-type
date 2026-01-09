#include "../include/LobbyState.hpp"
#include "../include/GameState.hpp"
#include "../include/States.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

namespace rtype::client {

LobbyState::LobbyState()
    : is_typing_name_(false), backspace_timer_(0.0f), backspace_delay_(INITIAL_BACKSPACE_DELAY),
      was_backspace_pressed_(false), font_loaded_(false), player_count_(0), game_started_(false),
      renderer_ref_(nullptr), local_player_id_(0), current_mode_(LobbyMode::MAIN_MENU),
      selected_room_index_(-1), is_typing_room_name_(false),
      was_room_backspace_pressed_(false), room_backspace_timer_(0.0f),
      room_backspace_delay_(INITIAL_BACKSPACE_DELAY), is_typing_chat_(false), chat_backspace_timer_(0.0f),
      chat_backspace_delay_(INITIAL_BACKSPACE_DELAY), was_chat_backspace_pressed_(false) {
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

    // Chat UI setup - Professional Gaming Style
    chat_background_.setSize(sf::Vector2f(600, 200));
    chat_background_.setFillColor(sf::Color(15, 15, 20, 240));
    chat_background_.setOutlineColor(sf::Color(0, 180, 220, 180)); // Cyan accent
    chat_background_.setOutlineThickness(2);

    chat_input_background_.setSize(sf::Vector2f(600, 40));
    chat_input_background_.setFillColor(sf::Color(25, 25, 35, 240));
    chat_input_background_.setOutlineColor(sf::Color(0, 180, 220, 120)); // Cyan accent
    chat_input_background_.setOutlineThickness(1);

    chat_input_label_.setFont(font_);
    chat_input_label_.setString("CHAT:");
    chat_input_label_.setCharacterSize(14);
    chat_input_label_.setFillColor(sf::Color(0, 200, 255)); // Cyan text

    chat_input_text_.setFont(font_);
    chat_input_text_.setString("");
    chat_input_text_.setCharacterSize(14);
    chat_input_text_.setFillColor(sf::Color(220, 220, 220));

    is_typing_chat_ = false;
    current_chat_input_ = "";

    // Room browser UI
    rooms_title_text_.setFont(font_);
    rooms_title_text_.setString("AVAILABLE ROOMS");
    rooms_title_text_.setCharacterSize(36);
    rooms_title_text_.setFillColor(sf::Color::White);
    rooms_title_text_.setStyle(sf::Text::Bold);

    create_room_button_.setSize(sf::Vector2f(250, 50));
    create_room_button_.setFillColor(sf::Color(50, 150, 50));
    create_room_button_.setOutlineColor(sf::Color::White);
    create_room_button_.setOutlineThickness(2);

    create_room_button_text_.setFont(font_);
    create_room_button_text_.setString("CREATE ROOM");
    create_room_button_text_.setCharacterSize(20);
    create_room_button_text_.setFillColor(sf::Color::White);

    refresh_button_.setSize(sf::Vector2f(250, 50));
    refresh_button_.setFillColor(sf::Color(100, 100, 150));
    refresh_button_.setOutlineColor(sf::Color::White);
    refresh_button_.setOutlineThickness(2);

    refresh_button_text_.setFont(font_);
    refresh_button_text_.setString("REFRESH");
    refresh_button_text_.setCharacterSize(20);
    refresh_button_text_.setFillColor(sf::Color::White);

    room_input_background_.setSize(sf::Vector2f(400, 40));
    room_input_background_.setFillColor(sf::Color(30, 30, 30, 200));
    room_input_background_.setOutlineColor(sf::Color(100, 100, 100));
    room_input_background_.setOutlineThickness(2);

    room_input_label_.setFont(font_);
    room_input_label_.setString("ROOM NAME:");
    room_input_label_.setCharacterSize(18);
    room_input_label_.setFillColor(sf::Color(200, 200, 200));

    room_input_text_.setFont(font_);
    room_input_text_.setString("");
    room_input_text_.setCharacterSize(20);
    room_input_text_.setFillColor(sf::Color::White);
    
    // Main menu buttons
    title_text_.setString("R-TYPE MULTIPLAYER");
    
    create_button_.setSize(sf::Vector2f(400, 80));
    create_button_.setFillColor(sf::Color(50, 150, 50));
    create_button_.setOutlineColor(sf::Color::White);
    create_button_.setOutlineThickness(3);
    
    create_button_text_.setFont(font_);
    create_button_text_.setString("CREER UNE ROOM");
    create_button_text_.setCharacterSize(30);
    create_button_text_.setFillColor(sf::Color::White);
    
    join_button_.setSize(sf::Vector2f(400, 80));
    join_button_.setFillColor(sf::Color(100, 100, 200));
    join_button_.setOutlineColor(sf::Color::White);
    join_button_.setOutlineThickness(3);
    
    join_button_text_.setFont(font_);
    join_button_text_.setString("REJOINDRE UNE ROOM");
    join_button_text_.setCharacterSize(30);
    join_button_text_.setFillColor(sf::Color::White);
    
    back_button_.setSize(sf::Vector2f(200, 50));
    back_button_.setFillColor(sf::Color(150, 50, 50));
    back_button_.setOutlineColor(sf::Color::White);
    back_button_.setOutlineThickness(2);
    
    back_button_text_.setFont(font_);
    back_button_text_.setString("RETOUR");
    back_button_text_.setCharacterSize(20);
    back_button_text_.setFillColor(sf::Color::White);
    
    leave_room_button_.setSize(sf::Vector2f(250, 50));
    leave_room_button_.setFillColor(sf::Color(180, 50, 50));
    leave_room_button_.setOutlineColor(sf::Color::White);
    leave_room_button_.setOutlineThickness(2);
    
    leave_room_button_text_.setFont(font_);
    leave_room_button_text_.setString("QUITTER LA ROOM");
    leave_room_button_text_.setCharacterSize(18);
    leave_room_button_text_.setFillColor(sf::Color::White);
}

void LobbyState::on_enter(Renderer& renderer, Client& client) {
    renderer_ref_ = &renderer;
    update_positions(renderer.get_window_size());

    connected_players_.clear();
    player_count_ = 0;
    game_started_ = false;

    // DO NOT connect here - wait for user to click CREATE or JOIN
    // Connection will happen when user selects a room action

    // Only add player if already connected
    if (client.is_connected()) {
        add_player(client.get_player_id(), client.get_player_name());
        local_player_id_ = client.get_player_id();
        set_player_count(static_cast<uint8_t>(connected_players_.size()));
    }

    client.set_player_join_callback([this](uint32_t player_id, const std::string& name) {
        std::lock_guard<std::mutex> lock(pending_mutex_);
        pending_player_joins_.emplace_back(player_id, name);
    });

    client.set_player_name_callback([this](uint32_t player_id, const std::string& name) {
        std::lock_guard<std::mutex> lock(pending_mutex_);
        pending_name_updates_.emplace_back(player_id, name);
    });

    client.set_game_start_callback([this]() { game_started_.store(true); });

    // Thread-safe chat callback
    client.set_chat_message_callback(
        [this](uint32_t /*player_id*/, const std::string& player_name, const std::string& message) {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            pending_chat_messages_.emplace_back(player_name, message);
        });

    // Room list callback
    client.set_room_list_callback([this](uint32_t session_id, uint8_t player_count, uint8_t max_players,
                                          uint8_t status, const std::string& room_name) {
        add_room(session_id, player_count, max_players, status, room_name);
    });

    // Start at main menu
    current_mode_ = LobbyMode::MAIN_MENU;
    available_rooms_.clear();
    selected_room_index_ = -1;

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

    // Setup chat message callback
    chat_messages_.clear();
    chat_message_texts_.clear();
    // Callback already set above
}

void LobbyState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;

    // CRITICAL: Clear all callbacks to prevent dangling pointer crashes
    // These callbacks capture 'this' and will be called from network thread
    client.set_player_join_callback(nullptr);
    client.set_player_name_callback(nullptr);
    client.set_game_start_callback(nullptr);
    client.set_chat_message_callback(nullptr);
    client.set_room_list_callback(nullptr);

    // Clear chat data
    chat_messages_.clear();
    chat_message_texts_.clear();
    
    // Clear room data
    available_rooms_.clear();
    room_texts_.clear();
    room_buttons_.clear();
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
                
                // Navigation selon le mode
                if (current_mode_ == LobbyMode::MAIN_MENU) {
                    // Menu principal: Créer ou Rejoindre
                    if (create_button_.getGlobalBounds().contains(mouse_pos)) {
                        // Connecter au serveur AVANT d'aller en mode CREATE_ROOM
                        if (!state_manager.get_client().is_connected()) {
                            state_manager.get_client().connect();
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        }
                        // Add player now that we're connected
                        if (state_manager.get_client().is_connected()) {
                            add_player(state_manager.get_client().get_player_id(), 
                                     state_manager.get_client().get_player_name());
                            local_player_id_ = state_manager.get_client().get_player_id();
                        }
                        current_mode_ = LobbyMode::CREATE_ROOM;
                        current_room_name_input_.clear();
                        room_input_text_.setString("");
                        if (renderer_ref_) {
                            update_positions(renderer_ref_->get_window_size());
                        }
                    } else if (join_button_.getGlobalBounds().contains(mouse_pos)) {
                        // Connecter au serveur AVANT d'aller en mode BROWSE_ROOMS
                        if (!state_manager.get_client().is_connected()) {
                            state_manager.get_client().connect();
                            std::this_thread::sleep_for(std::chrono::milliseconds(500));
                        }
                        // Add player now that we're connected
                        if (state_manager.get_client().is_connected()) {
                            add_player(state_manager.get_client().get_player_id(), 
                                     state_manager.get_client().get_player_name());
                            local_player_id_ = state_manager.get_client().get_player_id();
                        }
                        current_mode_ = LobbyMode::BROWSE_ROOMS;
                        available_rooms_.clear();
                        selected_room_index_ = -1;
                        state_manager.get_client().request_room_list();
                        if (renderer_ref_) {
                            update_positions(renderer_ref_->get_window_size());
                        }
                        update_room_list_display();
                    }
                }
                else if (current_mode_ == LobbyMode::CREATE_ROOM) {
                    // Page de création de room
                    if (back_button_.getGlobalBounds().contains(mouse_pos)) {
                        current_mode_ = LobbyMode::MAIN_MENU;
                        is_typing_room_name_ = false;
                        room_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                        if (renderer_ref_) {
                            update_positions(renderer_ref_->get_window_size());
                        }
                    } else if (create_room_button_.getGlobalBounds().contains(mouse_pos)) {
                        if (!current_room_name_input_.empty()) {
                            state_manager.get_client().create_room(current_room_name_input_);
                            current_mode_ = LobbyMode::IN_ROOM;
                            is_typing_room_name_ = false;
                            room_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                            if (renderer_ref_) {
                                update_positions(renderer_ref_->get_window_size());
                            }
                        }
                    } else if (room_input_background_.getGlobalBounds().contains(mouse_pos)) {
                        is_typing_room_name_ = true;
                        room_input_background_.setOutlineColor(sf::Color::Cyan);
                    } else {
                        is_typing_room_name_ = false;
                        room_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                    }
                }
                else if (current_mode_ == LobbyMode::BROWSE_ROOMS) {
                    // Page de navigation des rooms
                    if (back_button_.getGlobalBounds().contains(mouse_pos)) {
                        current_mode_ = LobbyMode::MAIN_MENU;
                        if (renderer_ref_) {
                            update_positions(renderer_ref_->get_window_size());
                        }
                    } else if (refresh_button_.getGlobalBounds().contains(mouse_pos)) {
                        available_rooms_.clear();
                        selected_room_index_ = -1;
                        state_manager.get_client().request_room_list();
                        update_room_list_display();
                    } else {
                        // Check room selection
                        for (size_t i = 0; i < room_buttons_.size(); ++i) {
                            if (room_buttons_[i].getGlobalBounds().contains(mouse_pos)) {
                                if (static_cast<int>(i) == selected_room_index_) {
                                    // Double click - join room
                                    state_manager.get_client().join_room(available_rooms_[i].session_id);
                                    current_mode_ = LobbyMode::IN_ROOM;
                                    if (renderer_ref_) {
                                        update_positions(renderer_ref_->get_window_size());
                                    }
                                } else {
                                    selected_room_index_ = static_cast<int>(i);
                                }
                                update_room_list_display();
                                break;
                            }
                        }
                    }
                }
                else if (current_mode_ == LobbyMode::IN_ROOM) {
                    // Dans une room - interface classique du lobby
                    
                    // Leave room button
                    if (leave_room_button_.getGlobalBounds().contains(mouse_pos)) {
                        // Leave room but stay connected to server
                        state_manager.get_client().leave_room();
                        current_mode_ = LobbyMode::MAIN_MENU;
                        connected_players_.clear();
                        player_count_ = 0;
                        chat_messages_.clear();
                        chat_message_texts_.clear();
                        if (renderer_ref_) {
                            update_positions(renderer_ref_->get_window_size());
                        }
                    }
                    else if (player_count_ >= 1 && local_player_id_ == 1 && start_button_.getGlobalBounds().contains(mouse_pos)) {
                        state_manager.get_client().send_game_start_request();
                    }
                    
                    // Check name input click
                    if (input_background_.getGlobalBounds().contains(mouse_pos)) {
                        is_typing_name_ = true;
                        is_typing_chat_ = false;
                        input_background_.setOutlineColor(sf::Color::Cyan);
                        chat_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                    } else if (chat_input_background_.getGlobalBounds().contains(mouse_pos)) {
                        is_typing_chat_ = true;
                        is_typing_name_ = false;
                        chat_input_background_.setOutlineColor(sf::Color::Cyan);
                        input_background_.setOutlineColor(sf::Color(100, 100, 100));
                    } else {
                        is_typing_name_ = false;
                        is_typing_chat_ = false;
                        input_background_.setOutlineColor(sf::Color(100, 100, 100));
                        chat_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                    }
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            // Gestion des touches selon le mode
            if (current_mode_ == LobbyMode::CREATE_ROOM) {
                if (is_typing_room_name_ && event.key.code == sf::Keyboard::Return) {
                    if (!current_room_name_input_.empty()) {
                        state_manager.get_client().create_room(current_room_name_input_);
                        current_mode_ = LobbyMode::IN_ROOM;
                    }
                    is_typing_room_name_ = false;
                    room_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                } else if (event.key.code == sf::Keyboard::Escape) {
                    current_mode_ = LobbyMode::MAIN_MENU;
                    is_typing_room_name_ = false;
                    room_input_background_.setOutlineColor(sf::Color(100, 100, 100));
                }
            }
            else if (current_mode_ == LobbyMode::BROWSE_ROOMS) {
                if (event.key.code == sf::Keyboard::Return && selected_room_index_ >= 0 &&
                    selected_room_index_ < static_cast<int>(available_rooms_.size())) {
                    state_manager.get_client().join_room(available_rooms_[selected_room_index_].session_id);
                    current_mode_ = LobbyMode::IN_ROOM;
                } else if (event.key.code == sf::Keyboard::Escape) {
                    current_mode_ = LobbyMode::MAIN_MENU;
                }
            }
            else if (current_mode_ == LobbyMode::IN_ROOM) {
                if (is_typing_name_) {
                    if (event.key.code == sf::Keyboard::Return) {
                        is_typing_name_ = false;
                        input_background_.setOutlineColor(sf::Color(100, 100, 100));

                        if (current_name_input_.empty()) {
                            current_name_input_ = "Player " + std::to_string(local_player_id_);
                        }
                        connected_players_[local_player_id_] = current_name_input_;
                        update_player_display();
                        name_input_text_.setString(current_name_input_);

                        state_manager.get_client().send_player_name_update(current_name_input_);
                    }
                } else if (is_typing_chat_) {
                    if (event.key.code == sf::Keyboard::Return) {
                        if (!current_chat_input_.empty()) {
                            state_manager.get_client().send_chat_message(current_chat_input_);
                            current_chat_input_.clear();
                            chat_input_text_.setString("");
                        }
                    }
                } else {
                    if ((event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) &&
                        player_count_ >= 2) {
                        state_manager.get_client().send_game_start_request();
                    }
                }
            }
        } else if (event.type == sf::Event::TextEntered) {
            if (is_typing_room_name_) {
                if (event.text.unicode < 128 && event.text.unicode > 31 && current_room_name_input_.length() < 30) {
                    current_room_name_input_ += static_cast<char>(event.text.unicode);
                    room_input_text_.setString(current_room_name_input_);
                }
            } else if (is_typing_name_) {
                if (event.text.unicode < 128 && event.text.unicode > 31 && current_name_input_.length() < 16) {
                    current_name_input_ += static_cast<char>(event.text.unicode);
                    name_input_text_.setString(current_name_input_);
                }
            } else if (is_typing_chat_) {
                if (event.text.unicode < 128 && event.text.unicode > 31 &&
                    current_chat_input_.length() < MAX_CHAT_INPUT_LENGTH) {
                    current_chat_input_ += static_cast<char>(event.text.unicode);
                    if (current_chat_input_.length() > 40) {
                        chat_input_text_.setString("..." +
                                                   current_chat_input_.substr(current_chat_input_.length() - 37));
                    } else {
                        chat_input_text_.setString(current_chat_input_);
                    }
                }
            }
        }
    }
}

void LobbyState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)client;
    // Room name backspace handling
    if (is_typing_room_name_ && sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)) {
        bool delete_char = false;

        if (!was_room_backspace_pressed_) {
            delete_char = true;
            room_backspace_timer_ = 0.0f;
            room_backspace_delay_ = INITIAL_BACKSPACE_DELAY;
        } else {
            room_backspace_timer_ += delta_time;
            if (room_backspace_timer_ >= room_backspace_delay_) {
                delete_char = true;
                room_backspace_timer_ = 0.0f;
                room_backspace_delay_ = REPEAT_BACKSPACE_DELAY;
            }
        }

        if (delete_char && !current_room_name_input_.empty()) {
            current_room_name_input_.pop_back();
            room_input_text_.setString(current_room_name_input_);
        }
        was_room_backspace_pressed_ = true;
    } else {
        was_room_backspace_pressed_ = false;
        room_backspace_timer_ = 0.0f;
    }

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

    // Chat backspace handling
    if (is_typing_chat_ && sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace)) {
        bool delete_char = false;

        if (!was_chat_backspace_pressed_) {
            delete_char = true;
            chat_backspace_timer_ = 0.0f;
            chat_backspace_delay_ = INITIAL_BACKSPACE_DELAY;
        } else {
            chat_backspace_timer_ += delta_time;
            if (chat_backspace_timer_ >= chat_backspace_delay_) {
                delete_char = true;
                chat_backspace_timer_ = 0.0f;
                chat_backspace_delay_ = REPEAT_BACKSPACE_DELAY;
            }
        }

        if (delete_char && !current_chat_input_.empty()) {
            current_chat_input_.pop_back();
            if (current_chat_input_.length() > 40) {
                chat_input_text_.setString("..." + current_chat_input_.substr(current_chat_input_.length() - 37));
            } else {
                chat_input_text_.setString(current_chat_input_);
            }
        }
        was_chat_backspace_pressed_ = true;
    } else {
        was_chat_backspace_pressed_ = false;
        chat_backspace_timer_ = 0.0f;
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

    // Process pending network events on main thread
    process_pending_events();

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
        if (current_mode_ == LobbyMode::MAIN_MENU) {
            // Menu principal
            renderer.draw_text(title_text_);
            renderer.draw_rectangle(create_button_);
            renderer.draw_text(create_button_text_);
            renderer.draw_rectangle(join_button_);
            renderer.draw_text(join_button_text_);
        }
        else if (current_mode_ == LobbyMode::CREATE_ROOM) {
            // Page de création
            renderer.draw_text(title_text_);
            renderer.draw_rectangle(room_input_background_);
            renderer.draw_text(room_input_label_);
            renderer.draw_text(room_input_text_);
            renderer.draw_rectangle(create_room_button_);
            renderer.draw_text(create_room_button_text_);
            renderer.draw_rectangle(back_button_);
            renderer.draw_text(back_button_text_);
        }
        else if (current_mode_ == LobbyMode::BROWSE_ROOMS) {
            // Page de navigation des rooms - afficher rooms_title_text_ au lieu de title_text_
            renderer.draw_text(rooms_title_text_);
            for (const auto& button : room_buttons_) {
                renderer.draw_rectangle(button);
            }
            for (const auto& text : room_texts_) {
                renderer.draw_text(text);
            }
            renderer.draw_rectangle(refresh_button_);
            renderer.draw_text(refresh_button_text_);
            renderer.draw_rectangle(back_button_);
            renderer.draw_text(back_button_text_);
        }
        else if (current_mode_ == LobbyMode::IN_ROOM) {
            // Lobby classique dans une room
            renderer.draw_text(title_text_);
            renderer.draw_text(waiting_text_);
            renderer.draw_text(players_text_);
            renderer.draw_text(player_list_text_);
            
            // Show START button for player 1 with at least 1 player (allow solo)
            if (local_player_id_ == 1) {
                renderer.draw_rectangle(start_button_);
                renderer.draw_text(start_button_text_);
            }
            
            // Leave room button for everyone
            renderer.draw_rectangle(leave_room_button_);
            renderer.draw_text(leave_room_button_text_);

            // Draw Name Input
            renderer.draw_rectangle(input_background_);
            renderer.draw_text(name_input_label_);
            renderer.draw_text(name_input_text_);

            // Draw Chat UI
            renderer.draw_rectangle(chat_background_);
            renderer.draw_rectangle(chat_input_background_);
            renderer.draw_text(chat_input_label_);
            renderer.draw_text(chat_input_text_);
            for (const auto& msg_text : chat_message_texts_) {
                renderer.draw_text(msg_text);
            }
        }
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
    float center_x = window_size.x / 2.0f;
    float center_y = window_size.y / 2.0f;
    
    // Title - change selon le mode
    float title_y = 30.0f;
    if (current_mode_ == LobbyMode::BROWSE_ROOMS) {
        rooms_title_text_.setString("AVAILABLE ROOMS");
        rooms_title_text_.setPosition((window_size.x - rooms_title_text_.getLocalBounds().width) / 2.0f, title_y);
    } else {
        title_text_.setString("R-TYPE MULTIPLAYER");
        title_text_.setPosition((window_size.x - title_text_.getLocalBounds().width) / 2.0f, title_y);
    }
    
    // Menu principal - Deux gros boutons centrés
    float button_width = std::min(400.0f, window_size.x * 0.6f);
    float button_height = 80.0f;
    create_button_.setSize(sf::Vector2f(button_width, button_height));
    create_button_.setPosition(center_x - button_width/2, center_y - 60);
    create_button_text_.setPosition(
        center_x - create_button_text_.getLocalBounds().width / 2.0f,
        center_y - 35);
    
    join_button_.setSize(sf::Vector2f(button_width, button_height));
    join_button_.setPosition(center_x - button_width/2, center_y + 40);
    join_button_text_.setPosition(
        center_x - join_button_text_.getLocalBounds().width / 2.0f,
        center_y + 65);
    
    // Bouton retour (bas gauche)
    back_button_.setPosition(20, window_size.y - 70);
    back_button_text_.setPosition(
        back_button_.getPosition().x + (200 - back_button_text_.getLocalBounds().width) / 2.0f,
        back_button_.getPosition().y + 15);
    
    // Page CREATE_ROOM - centré
    float input_width = std::min(400.0f, window_size.x * 0.6f);
    room_input_label_.setPosition(center_x - input_width/2, center_y - 80);
    room_input_background_.setSize(sf::Vector2f(input_width, 40));
    room_input_background_.setPosition(center_x - input_width/2, center_y - 50);
    room_input_text_.setPosition(center_x - input_width/2 + 10, center_y - 45);
    
    create_room_button_.setSize(sf::Vector2f(250, 50));
    create_room_button_.setPosition(center_x - 125, center_y + 20);
    create_room_button_text_.setPosition(
        center_x - create_room_button_text_.getLocalBounds().width / 2.0f,
        center_y + 35);
    
    // Page BROWSE_ROOMS - refresh button en haut à droite
    refresh_button_.setPosition(window_size.x - 270, 80);
    refresh_button_text_.setPosition(
        refresh_button_.getPosition().x + (250 - refresh_button_text_.getLocalBounds().width) / 2.0f,
        refresh_button_.getPosition().y + 15);
    
    update_room_list_display();
    
    // Mode IN_ROOM - Lobby avec positionnement responsive
    float waiting_y = 120.0f;
    float players_y = 180.0f;
    float list_y = 220.0f;
    float start_button_y = 280.0f;

    waiting_text_.setPosition((window_size.x - waiting_text_.getLocalBounds().width) / 2.0f, waiting_y);
    players_text_.setPosition((window_size.x - players_text_.getLocalBounds().width) / 2.0f, players_y);
    player_list_text_.setPosition((window_size.x - player_list_text_.getLocalBounds().width) / 2.0f, list_y);

    float start_button_width = 250.0f;
    float start_button_height = 60.0f;
    start_button_.setSize(sf::Vector2f(start_button_width, start_button_height));
    start_button_.setPosition(center_x - start_button_width/2, start_button_y);
    start_button_text_.setPosition(
        center_x - start_button_text_.getLocalBounds().width / 2.0f,
        start_button_y + 15);
    
    // Leave room button (en haut à droite)
    leave_room_button_.setPosition(window_size.x - 270, 20);
    leave_room_button_text_.setPosition(
        leave_room_button_.getPosition().x + (250 - leave_room_button_text_.getLocalBounds().width) / 2.0f,
        leave_room_button_.getPosition().y + 15);

    float input_y = window_size.y * 0.20f;
    input_background_.setPosition((window_size.x - 300) / 2.0f, input_y);
    // Position label above the input box centered
    name_input_label_.setPosition((window_size.x - name_input_label_.getLocalBounds().width) / 2.0f, input_y - 25);
    // Position text inside input box
    name_input_text_.setPosition(input_background_.getPosition().x + 10, input_y + 8);

    // Chat UI positioning - Bottom Center
    float chat_width = 600.0f;
    float chat_height = 200.0f;
    float chat_x = (window_size.x - chat_width) / 2.0f;
    float chat_y = window_size.y - chat_height - 20.0f; // Minimal padding from bottom

    chat_background_.setPosition(chat_x, chat_y);
    chat_input_background_.setPosition(chat_x, chat_y + chat_height + 5.0f);
    // Center text vertically in the 40px height input box
    // Box Y is chat_y + chat_height + 5.0f
    // Text size is 14px. 40/2 = 20. Text/2 = 7. Offset = 20 - 7 = 13.
    float input_text_y = chat_y + chat_height + 5.0f + 13.0f;
    chat_input_label_.setPosition(chat_x + 10, input_text_y);
    chat_input_text_.setPosition(chat_x + 70, input_text_y); // After "CHAT:" label (reduced gap slightly)

    // Update chat message text positions
    update_chat_display();
}

void LobbyState::add_chat_message(const std::string& player_name, const std::string& message) {
    chat_messages_.push_back({player_name, message});

    // Keep only last MAX_CHAT_MESSAGES
    if (chat_messages_.size() > MAX_CHAT_MESSAGES) {
        chat_messages_.erase(chat_messages_.begin());
    }

    update_chat_display();
}

void LobbyState::update_chat_display() {
    if (!font_loaded_)
        return;

    // Clear and rebuild text objects
    chat_message_texts_.clear();
    chat_message_texts_.reserve(chat_messages_.size());

    float chat_x = chat_background_.getPosition().x + 8.0f;
    float chat_y = chat_background_.getPosition().y + 8.0f;
    float line_height = 20.0f;

    // Show last MAX_CHAT_MESSAGES messages (scroll effect)
    size_t start_idx = chat_messages_.size() > MAX_CHAT_MESSAGES ? chat_messages_.size() - MAX_CHAT_MESSAGES : 0;
    size_t display_count = 0;

    for (size_t i = start_idx; i < chat_messages_.size() && display_count < MAX_CHAT_MESSAGES; ++i, ++display_count) {
        sf::Text msg_text;
        msg_text.setFont(font_);
        msg_text.setCharacterSize(13);

        std::string name = chat_messages_[i].first;
        std::string msg = chat_messages_[i].second;
        if (name.length() > 16)
            name = name.substr(0, 14) + "..";
        if (msg.length() > 60)
            msg = msg.substr(0, 57) + "...";

        msg_text.setString(name + ": " + msg);
        msg_text.setFillColor(sf::Color(200, 200, 200));
        msg_text.setPosition(chat_x, chat_y + (static_cast<float>(display_count) * line_height));
        chat_message_texts_.push_back(std::move(msg_text));
    }
}

void LobbyState::process_pending_events() {
    std::lock_guard<std::mutex> lock(pending_mutex_);

    // Process player joins
    for (const auto& player : pending_player_joins_) {
        add_player(player.first, player.second);
    }
    pending_player_joins_.clear();

    // Process name updates
    for (const auto& update : pending_name_updates_) {
        uint32_t player_id = update.first;
        std::string name = update.second;

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
    }
    pending_name_updates_.clear();

    // Process chat messages
    for (const auto& msg : pending_chat_messages_) {
        add_chat_message(msg.first, msg.second);
    }
    pending_chat_messages_.clear();
}

void LobbyState::add_room(uint32_t session_id, uint8_t player_count, uint8_t max_players,
                          uint8_t status, const std::string& room_name) {
    std::lock_guard<std::mutex> lock(pending_mutex_);
    
    // Convert status to string
    std::string status_str = (status == 0) ? "waiting" : "playing";
    
    std::cout << "[ROOM_LIST] Added room: " << room_name << " (ID: " << session_id 
              << ") - Players: " << (int)player_count << "/" << (int)max_players << std::endl;
    
    // Check if room already exists and update it
    for (auto& room : available_rooms_) {
        if (room.session_id == session_id) {
            room.player_count = player_count;
            room.max_players = max_players;
            room.status = status_str;
            room.room_name = room_name;
            return;
        }
    }
    
    // Add new room
    RoomEntry entry;
    entry.session_id = session_id;
    entry.player_count = player_count;
    entry.max_players = max_players;
    entry.status = status_str;
    entry.room_name = room_name;
    available_rooms_.push_back(entry);
}

void LobbyState::update_room_list_display() {
    if (!font_loaded_ || !renderer_ref_)
        return;
    
    room_texts_.clear();
    room_buttons_.clear();
    
    // Position dynamique basée sur la taille de la fenêtre
    sf::Vector2u window_size = renderer_ref_->get_window_size();
    float start_y = 150.0f;  // Après le titre
    const float room_height = 70.0f;
    const float room_spacing = 10.0f;
    float room_width = std::min(800.0f, window_size.x - 100.0f);
    float room_x = (window_size.x - room_width) / 2.0f;  // Centrer horizontalement
    
    for (size_t i = 0; i < available_rooms_.size() && i < 6; ++i) {  // Max 6 rooms visibles
        const auto& room = available_rooms_[i];
        float y_pos = start_y + (static_cast<float>(i) * (room_height + room_spacing));
        
        // Room button background
        sf::RectangleShape room_bg;
        room_bg.setSize(sf::Vector2f(room_width, room_height));
        room_bg.setPosition(room_x, y_pos);
        
        if (static_cast<int>(i) == selected_room_index_) {
            room_bg.setFillColor(sf::Color(80, 80, 120, 180));
            room_bg.setOutlineColor(sf::Color::Cyan);
        } else if (room.status == "waiting") {
            room_bg.setFillColor(sf::Color(50, 100, 50, 150));
            room_bg.setOutlineColor(sf::Color(100, 200, 100));
        } else {
            room_bg.setFillColor(sf::Color(100, 50, 50, 100));
            room_bg.setOutlineColor(sf::Color(200, 100, 100));
        }
        room_bg.setOutlineThickness(2);
        room_buttons_.push_back(room_bg);
        
        // Room name
        sf::Text name_text;
        name_text.setFont(font_);
        name_text.setString(room.room_name.empty() ? "Room " + std::to_string(room.session_id) : room.room_name);
        name_text.setCharacterSize(24);
        name_text.setFillColor(sf::Color::White);
        name_text.setPosition(room_x + 20.0f, y_pos + 8.0f);
        room_texts_.push_back(name_text);
        
        // Room info (players/status)
        sf::Text info_text;
        info_text.setFont(font_);
        info_text.setString("Players: " + std::to_string(room.player_count) + "/" + std::to_string(room.max_players) + 
                           " | Status: " + room.status);
        info_text.setCharacterSize(16);
        info_text.setFillColor(sf::Color(180, 180, 180));
        info_text.setPosition(room_x + 20.0f, y_pos + 38.0f);
        room_texts_.push_back(info_text);
    }
}

} // namespace rtype::client
