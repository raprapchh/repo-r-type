#include "../include/MenuState.hpp"
#include "../include/ModeSelectionState.hpp"
#include <iostream>
#include <memory>

namespace rtype::client {

MenuState::MenuState() : font_loaded_(false) {
    setup_ui();
}

void MenuState::setup_ui() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr
            << "Warning: Could not load font from client/fonts/Ethnocentric-Regular.otf. UI will not display text."
            << std::endl;
        font_loaded_ = false;
        return;
    }
    font_loaded_ = true;

    title_text_.setFont(font_);
    title_text_.setString("R-TYPE");
    title_text_.setCharacterSize(80);
    title_text_.setFillColor(sf::Color::White);
    title_text_.setStyle(sf::Text::Bold);

    start_button_.setSize(sf::Vector2f(300, 60));
    start_button_.setFillColor(sf::Color(100, 150, 200));
    start_button_.setOutlineColor(sf::Color::White);
    start_button_.setOutlineThickness(2);

    start_button_text_.setFont(font_);
    start_button_text_.setString("START");
    start_button_text_.setCharacterSize(40);
    start_button_text_.setFillColor(sf::Color::White);

    quit_button_.setSize(sf::Vector2f(300, 60));
    quit_button_.setFillColor(sf::Color(200, 100, 100));
    quit_button_.setOutlineColor(sf::Color::White);
    quit_button_.setOutlineThickness(2);

    quit_button_text_.setFont(font_);
    quit_button_text_.setString("QUIT");
    quit_button_text_.setCharacterSize(40);
    quit_button_text_.setFillColor(sf::Color::White);
}

void MenuState::on_enter(Renderer& renderer, Client& client) {
    update_positions(renderer.get_window_size());
}

void MenuState::on_exit(Renderer& renderer, Client& client) {
}

void MenuState::handle_input(Renderer& renderer, StateManager& state_manager) {
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
            if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                handle_button_click(sf::Vector2f(start_button_.getPosition().x + start_button_.getSize().x / 2.0f,
                                                 start_button_.getPosition().y + start_button_.getSize().y / 2.0f),
                                    state_manager);
            } else if (event.key.code == sf::Keyboard::Escape) {
                renderer.close_window();
            }
        }
    }
}

void MenuState::handle_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager) {
    if (start_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<ModeSelectionState>());
    } else if (quit_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.get_renderer().close_window();
    }
}

void MenuState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    sf::Vector2f mouse_pos = renderer.get_mouse_position();

    if (start_button_.getGlobalBounds().contains(mouse_pos)) {
        start_button_.setFillColor(sf::Color(120, 170, 220));
    } else {
        start_button_.setFillColor(sf::Color(100, 150, 200));
    }

    if (quit_button_.getGlobalBounds().contains(mouse_pos)) {
        quit_button_.setFillColor(sf::Color(220, 120, 120));
    } else {
        quit_button_.setFillColor(sf::Color(200, 100, 100));
    }
}

void MenuState::render(Renderer& renderer) {
    renderer.clear();

    if (font_loaded_) {
        renderer.draw_text(title_text_);
        renderer.draw_rectangle(start_button_);
        renderer.draw_text(start_button_text_);
        renderer.draw_rectangle(quit_button_);
        renderer.draw_text(quit_button_text_);
    }

    renderer.display();
}

void MenuState::update_positions(const sf::Vector2u& window_size) {
    float title_y = window_size.y * 0.15f;
    float button_start_y = window_size.y * 0.5f;
    float button_spacing = window_size.y * 0.12f;

    title_text_.setPosition((window_size.x - title_text_.getLocalBounds().width) / 2.0f, title_y);

    float button_width = std::min(300.0f, window_size.x * 0.25f);
    float button_height = std::min(60.0f, window_size.y * 0.08f);
    start_button_.setSize(sf::Vector2f(button_width, button_height));
    start_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y);
    start_button_text_.setPosition(
        start_button_.getPosition().x + (button_width - start_button_text_.getLocalBounds().width) / 2.0f,
        start_button_.getPosition().y + (button_height - start_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    quit_button_.setSize(sf::Vector2f(button_width, button_height));
    quit_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y + button_spacing);
    quit_button_text_.setPosition(
        quit_button_.getPosition().x + (button_width - quit_button_text_.getLocalBounds().width) / 2.0f,
        quit_button_.getPosition().y + (button_height - quit_button_text_.getLocalBounds().height) / 2.0f - 5.0f);
}

} // namespace rtype::client
