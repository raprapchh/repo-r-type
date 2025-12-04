#include "../include/ModeSelectionState.hpp"
#include "../include/GameState.hpp"
#include "../include/LobbyState.hpp"
#include "../include/MenuState.hpp"
#include <iostream>
#include <memory>

namespace rtype::client {

ModeSelectionState::ModeSelectionState() : font_loaded_(false) {
    setup_ui();
}

void ModeSelectionState::setup_ui() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr
            << "Warning: Could not load font from client/fonts/Ethnocentric-Regular.otf. UI will not display text."
            << std::endl;
        font_loaded_ = false;
        return;
    }
    font_loaded_ = true;

    title_text_.setFont(font_);
    title_text_.setString("SELECT MODE");
    title_text_.setCharacterSize(60);
    title_text_.setFillColor(sf::Color::White);
    title_text_.setStyle(sf::Text::Bold);

    solo_button_.setSize(sf::Vector2f(300, 60));
    solo_button_.setFillColor(sf::Color(100, 150, 200));
    solo_button_.setOutlineColor(sf::Color::White);
    solo_button_.setOutlineThickness(2);

    solo_button_text_.setFont(font_);
    solo_button_text_.setString("SOLO");
    solo_button_text_.setCharacterSize(40);
    solo_button_text_.setFillColor(sf::Color::White);

    multiplayer_button_.setSize(sf::Vector2f(300, 60));
    multiplayer_button_.setFillColor(sf::Color(100, 150, 200));
    multiplayer_button_.setOutlineColor(sf::Color::White);
    multiplayer_button_.setOutlineThickness(2);

    multiplayer_button_text_.setFont(font_);
    multiplayer_button_text_.setString("MULTIPLAYER");
    multiplayer_button_text_.setCharacterSize(40);
    multiplayer_button_text_.setFillColor(sf::Color::White);

    back_button_.setSize(sf::Vector2f(300, 60));
    back_button_.setFillColor(sf::Color(150, 150, 150));
    back_button_.setOutlineColor(sf::Color::White);
    back_button_.setOutlineThickness(2);

    back_button_text_.setFont(font_);
    back_button_text_.setString("BACK");
    back_button_text_.setCharacterSize(40);
    back_button_text_.setFillColor(sf::Color::White);
}

void ModeSelectionState::on_enter(Renderer& renderer, Client& client) {
    update_positions(renderer.get_window_size());
}

void ModeSelectionState::on_exit(Renderer& renderer, Client& client) {
}

void ModeSelectionState::handle_input(Renderer& renderer, StateManager& state_manager) {
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
                handle_button_click(mouse_pos, state_manager);
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                renderer.close_window();
            }
        }
    }
}

void ModeSelectionState::handle_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager) {
    if (solo_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<GameState>());
    } else if (multiplayer_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<LobbyState>());
    } else if (back_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<MenuState>());
    }
}

void ModeSelectionState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    sf::Vector2f mouse_pos = renderer.get_mouse_position();

    if (solo_button_.getGlobalBounds().contains(mouse_pos)) {
        solo_button_.setFillColor(sf::Color(120, 170, 220));
    } else {
        solo_button_.setFillColor(sf::Color(100, 150, 200));
    }

    if (multiplayer_button_.getGlobalBounds().contains(mouse_pos)) {
        multiplayer_button_.setFillColor(sf::Color(120, 170, 220));
    } else {
        multiplayer_button_.setFillColor(sf::Color(100, 150, 200));
    }

    if (back_button_.getGlobalBounds().contains(mouse_pos)) {
        back_button_.setFillColor(sf::Color(170, 170, 170));
    } else {
        back_button_.setFillColor(sf::Color(150, 150, 150));
    }
}

void ModeSelectionState::render(Renderer& renderer) {
    renderer.clear();

    if (font_loaded_) {
        renderer.draw_text(title_text_);
        renderer.draw_rectangle(solo_button_);
        renderer.draw_text(solo_button_text_);
        renderer.draw_rectangle(multiplayer_button_);
        renderer.draw_text(multiplayer_button_text_);
        renderer.draw_rectangle(back_button_);
        renderer.draw_text(back_button_text_);
    }

    renderer.display();
}

void ModeSelectionState::update_positions(const sf::Vector2u& window_size) {
    float title_y = window_size.y * 0.15f;
    float button_start_y = window_size.y * 0.4f;
    float button_spacing = window_size.y * 0.1f;

    title_text_.setPosition((window_size.x - title_text_.getLocalBounds().width) / 2.0f, title_y);

    float button_width = std::min(300.0f, window_size.x * 0.25f);
    float button_height = std::min(60.0f, window_size.y * 0.08f);
    
    solo_button_.setSize(sf::Vector2f(button_width, button_height));
    solo_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y);
    solo_button_text_.setPosition(
        solo_button_.getPosition().x + (button_width - solo_button_text_.getLocalBounds().width) / 2.0f,
        solo_button_.getPosition().y + (button_height - solo_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    multiplayer_button_.setSize(sf::Vector2f(button_width, button_height));
    multiplayer_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y + button_spacing);
    multiplayer_button_text_.setPosition(
        multiplayer_button_.getPosition().x + (button_width - multiplayer_button_text_.getLocalBounds().width) / 2.0f,
        multiplayer_button_.getPosition().y + (button_height - multiplayer_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    back_button_.setSize(sf::Vector2f(button_width, button_height));
    back_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y + button_spacing * 2);
    back_button_text_.setPosition(
        back_button_.getPosition().x + (button_width - back_button_text_.getLocalBounds().width) / 2.0f,
        back_button_.getPosition().y + (button_height - back_button_text_.getLocalBounds().height) / 2.0f - 5.0f);
}

} // namespace rtype::client

