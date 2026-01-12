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
    solo_button_text_.setCharacterSize(25);
    solo_button_text_.setFillColor(sf::Color::White);

    multiplayer_button_.setSize(sf::Vector2f(300, 60));
    multiplayer_button_.setFillColor(sf::Color(100, 150, 200));
    multiplayer_button_.setOutlineColor(sf::Color::White);
    multiplayer_button_.setOutlineThickness(2);

    multiplayer_button_text_.setFont(font_);
    multiplayer_button_text_.setString("MULTIPLAYER");
    multiplayer_button_text_.setCharacterSize(25);
    multiplayer_button_text_.setFillColor(sf::Color::White);

    back_button_.setSize(sf::Vector2f(300, 60));
    back_button_.setFillColor(sf::Color(150, 150, 150));
    back_button_.setOutlineColor(sf::Color::White);
    back_button_.setOutlineThickness(2);

    back_button_text_.setFont(font_);
    back_button_text_.setString("BACK");
    back_button_text_.setCharacterSize(25);
    back_button_text_.setFillColor(sf::Color::White);
}

void ModeSelectionState::on_enter(Renderer& renderer, Client& client) {
    (void)client;
    update_positions(renderer.get_window_size());
}

void ModeSelectionState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
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
            if (event.key.code == sf::Keyboard::Up) {
                if (selected_button_ == SelectedButton::MULTIPLAYER) {
                    selected_button_ = SelectedButton::SOLO;
                } else if (selected_button_ == SelectedButton::BACK) {
                    selected_button_ = SelectedButton::MULTIPLAYER;
                }
            } else if (event.key.code == sf::Keyboard::Down) {
                if (selected_button_ == SelectedButton::SOLO) {
                    selected_button_ = SelectedButton::MULTIPLAYER;
                } else if (selected_button_ == SelectedButton::MULTIPLAYER) {
                    selected_button_ = SelectedButton::BACK;
                }
            } else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                if (selected_button_ == SelectedButton::SOLO) {
                    state_manager.change_state(std::make_unique<GameState>(false));
                } else if (selected_button_ == SelectedButton::MULTIPLAYER) {
                    state_manager.change_state(std::make_unique<LobbyState>());
                } else if (selected_button_ == SelectedButton::BACK) {
                    state_manager.change_state(std::make_unique<MenuState>());
                }
            } else if (event.key.code == sf::Keyboard::Escape) {
                state_manager.change_state(std::make_unique<MenuState>());
            }
        }
    }
}

void ModeSelectionState::handle_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager) {
    if (solo_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<GameState>(false));
    } else if (multiplayer_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<LobbyState>());
    } else if (back_button_.getGlobalBounds().contains(mouse_pos)) {
        state_manager.change_state(std::make_unique<MenuState>());
    }
}

void ModeSelectionState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)client;
    (void)state_manager;
    (void)delta_time;
    sf::Vector2f mouse_pos = renderer.get_mouse_position();

    ColorBlindMode mode = renderer.get_accessibility_manager().get_current_mode();

    sf::Color button_normal(100, 150, 200);
    sf::Color button_hover(120, 170, 220);
    sf::Color button_selected(200, 200, 0);
    sf::Color back_normal(150, 150, 150);
    sf::Color back_hover(170, 170, 170);
    sf::Color back_selected(200, 200, 0);

    switch (mode) {
    case ColorBlindMode::Deuteranopia:
        button_normal = sf::Color(0, 0, 255);
        button_hover = sf::Color(50, 50, 255);
        button_selected = sf::Color(0, 255, 255);
        back_normal = sf::Color(255, 165, 0);
        back_hover = sf::Color(255, 185, 20);
        back_selected = sf::Color(0, 255, 255);
        break;
    case ColorBlindMode::Protanopia:
        button_normal = sf::Color(0, 100, 255);
        button_hover = sf::Color(50, 120, 255);
        button_selected = sf::Color(0, 255, 255);
        back_normal = sf::Color(255, 255, 0);
        back_hover = sf::Color(255, 255, 50);
        back_selected = sf::Color(0, 255, 255);
        break;
    case ColorBlindMode::Tritanopia:
        button_normal = sf::Color(255, 0, 0);
        button_hover = sf::Color(255, 50, 50);
        button_selected = sf::Color(0, 255, 255);
        back_normal = sf::Color(0, 200, 200);
        back_hover = sf::Color(0, 220, 220);
        back_selected = sf::Color(255, 255, 0);
        break;
    case ColorBlindMode::None:
    default:
        break;
    }

    if (solo_button_.getGlobalBounds().contains(mouse_pos)) {
        solo_button_.setFillColor(button_hover);
    } else if (selected_button_ == SelectedButton::SOLO) {
        solo_button_.setFillColor(button_selected);
    } else {
        solo_button_.setFillColor(button_normal);
    }

    if (multiplayer_button_.getGlobalBounds().contains(mouse_pos)) {
        multiplayer_button_.setFillColor(button_hover);
    } else if (selected_button_ == SelectedButton::MULTIPLAYER) {
        multiplayer_button_.setFillColor(button_selected);
    } else {
        multiplayer_button_.setFillColor(button_normal);
    }

    if (back_button_.getGlobalBounds().contains(mouse_pos)) {
        back_button_.setFillColor(back_hover);
    } else if (selected_button_ == SelectedButton::BACK) {
        back_button_.setFillColor(back_selected);
    } else {
        back_button_.setFillColor(back_normal);
    }
}

void ModeSelectionState::render(Renderer& renderer, Client& /* client */) {
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
        multiplayer_button_.getPosition().y +
            (button_height - multiplayer_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    back_button_.setSize(sf::Vector2f(button_width, button_height));
    back_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y + button_spacing * 2);
    back_button_text_.setPosition(
        back_button_.getPosition().x + (button_width - back_button_text_.getLocalBounds().width) / 2.0f,
        back_button_.getPosition().y + (button_height - back_button_text_.getLocalBounds().height) / 2.0f - 5.0f);
}

} // namespace rtype::client
