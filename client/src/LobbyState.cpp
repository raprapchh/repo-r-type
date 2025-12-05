#include "../include/LobbyState.hpp"
#include "../include/GameState.hpp"
#include "../include/States.hpp"
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <memory>

namespace rtype::client {

LobbyState::LobbyState() : font_loaded_(false), player_count_(0), game_started_(false), renderer_ref_(nullptr) {
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

    add_player(client.get_player_id());
    set_player_count(1);

    client.set_player_join_callback([this](uint32_t player_id) {
        add_player(player_id);
        set_player_count(static_cast<uint8_t>(connected_players_.size()));
    });

    client.set_game_start_callback([this]() { game_started_.store(true); });
}

void LobbyState::on_exit(Renderer& renderer, Client& client) {
}

void LobbyState::handle_input(Renderer& renderer, StateManager& state_manager) {
    sf::Event event;
    while (renderer.poll_event(event)) {
        if (event.type == sf::Event::Closed) {
            renderer.close_window();
        } else if (event.type == sf::Event::Resized) {
            renderer.handle_resize(event.size.width, event.size.height);
            update_positions(sf::Vector2u(event.size.width, event.size.height));
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
            }
        }
    }
}

void LobbyState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    if (game_started_.load()) {
        state_manager.change_state(std::make_unique<GameState>());
    }
}

void LobbyState::render(Renderer& renderer, Client& /* client */) {
    renderer.clear();

    if (font_loaded_) {
        renderer.draw_text(title_text_);
        renderer.draw_text(waiting_text_);
        renderer.draw_text(players_text_);
        renderer.draw_text(player_list_text_);
    }

    renderer.display();
}

void LobbyState::add_player(uint32_t player_id) {
    if (std::find(connected_players_.begin(), connected_players_.end(), player_id) == connected_players_.end()) {
        connected_players_.push_back(player_id);
        update_player_display();
    }
}

void LobbyState::remove_player(uint32_t player_id) {
    connected_players_.erase(std::remove(connected_players_.begin(), connected_players_.end(), player_id),
                             connected_players_.end());
    update_player_display();
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
    for (size_t i = 0; i < connected_players_.size(); ++i) {
        if (i > 0)
            player_list += ", ";
        player_list += "Player " + std::to_string(connected_players_[i]);
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

    title_text_.setPosition((window_size.x - title_text_.getLocalBounds().width) / 2.0f, title_y);
    waiting_text_.setPosition((window_size.x - waiting_text_.getLocalBounds().width) / 2.0f, waiting_y);
    players_text_.setPosition((window_size.x - players_text_.getLocalBounds().width) / 2.0f, players_y);
    player_list_text_.setPosition((window_size.x - player_list_text_.getLocalBounds().width) / 2.0f, list_y);
}

} // namespace rtype::client
