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

    if (!logo_texture_.loadFromFile("client/sprites/r-type_logo.png")) {
        std::cerr << "Warning: Could not load logo from client/sprites/r-type_logo.png" << std::endl;
    }
    logo_sprite_.setTexture(logo_texture_);
    logo_sprite_.setScale(1.5f, 1.5f);

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

    settings_button_.setSize(sf::Vector2f(300, 60));
    settings_button_.setFillColor(sf::Color(100, 100, 100));
    settings_button_.setOutlineColor(sf::Color::White);
    settings_button_.setOutlineThickness(2);

    settings_button_text_.setFont(font_);
    settings_button_text_.setString("SETTINGS");
    settings_button_text_.setCharacterSize(40);
    settings_button_text_.setFillColor(sf::Color::White);

    accessibility_cycle_button_.setSize(sf::Vector2f(400, 60));
    accessibility_cycle_button_.setFillColor(sf::Color(50, 50, 150));
    accessibility_cycle_button_.setOutlineColor(sf::Color::White);
    accessibility_cycle_button_.setOutlineThickness(2);

    accessibility_cycle_text_.setFont(font_);
    accessibility_cycle_text_.setString("Mode: None");
    accessibility_cycle_text_.setCharacterSize(22);
    accessibility_cycle_text_.setFillColor(sf::Color::White);

    confirm_button_.setSize(sf::Vector2f(150, 50));
    confirm_button_.setFillColor(sf::Color(0, 200, 0));
    confirm_button_.setOutlineColor(sf::Color::White);
    confirm_button_.setOutlineThickness(2);

    confirm_button_text_.setFont(font_);
    confirm_button_text_.setString("CONFIRM");
    confirm_button_text_.setCharacterSize(20);
    confirm_button_text_.setFillColor(sf::Color::White);

    cancel_button_.setSize(sf::Vector2f(150, 50));
    cancel_button_.setFillColor(sf::Color(200, 0, 0));
    cancel_button_.setOutlineColor(sf::Color::White);
    cancel_button_.setOutlineThickness(2);

    cancel_button_text_.setFont(font_);
    cancel_button_text_.setString("CANCEL");
    cancel_button_text_.setCharacterSize(20);
    cancel_button_text_.setFillColor(sf::Color::White);
}

void MenuState::on_enter(Renderer& renderer, Client& client) {
    (void)client;
    update_positions(renderer.get_window_size());
}

void MenuState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
}

void MenuState::handle_input(Renderer& renderer, StateManager& state_manager) {
    rtype::client::Event event;
    while (renderer.poll_event(event)) {
        if (event.type == rtype::client::Event::Closed) {
            renderer.close_window();
        } else if (event.type == rtype::client::Event::Resized) {
            renderer.handle_resize(event.size.width, event.size.height);
            update_positions(sf::Vector2u(event.size.width, event.size.height));
        } else if (event.type == rtype::client::Event::MouseButtonPressed) {
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
                sf::Vector2f mouse_pos = renderer.get_mouse_position();
                handle_button_click(mouse_pos, state_manager);
            }
        } else if (event.type == rtype::client::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                handle_button_click(sf::Vector2f(start_button_.getPosition().x + start_button_.getSize().x / 2.0f,
                                                 start_button_.getPosition().y + start_button_.getSize().y / 2.0f),
                                    state_manager);
            } else if (event.key.code == sf::Keyboard::Key::Escape) {
                renderer.close_window();
            }
        }
    }
}

void MenuState::handle_button_click(const sf::Vector2f& mouse_pos, StateManager& state_manager) {
    if (show_settings_) {
        if (accessibility_cycle_button_.getGlobalBounds().contains(mouse_pos)) {
            // Cycle through preview modes
            switch (pending_mode_) {
            case ColorBlindMode::None:
                pending_mode_ = ColorBlindMode::Deuteranopia;
                break;
            case ColorBlindMode::Deuteranopia:
                pending_mode_ = ColorBlindMode::Protanopia;
                break;
            case ColorBlindMode::Protanopia:
                pending_mode_ = ColorBlindMode::Tritanopia;
                break;
            case ColorBlindMode::Tritanopia:
                pending_mode_ = ColorBlindMode::None;
                break;
            }

            // Update display
            std::string mode_name;
            switch (pending_mode_) {
            case ColorBlindMode::None:
                mode_name = "None";
                break;
            case ColorBlindMode::Deuteranopia:
                mode_name = "Deuteranopia";
                break;
            case ColorBlindMode::Protanopia:
                mode_name = "Protanopia";
                break;
            case ColorBlindMode::Tritanopia:
                mode_name = "Tritanopia";
                break;
            }
            accessibility_cycle_text_.setString("Mode: " + mode_name);
            update_positions(state_manager.get_renderer().get_window_size());
            return;
        }

        if (confirm_button_.getGlobalBounds().contains(mouse_pos)) {
            // Apply the pending mode
            ColorBlindMode current = state_manager.get_renderer().get_accessibility_manager().get_current_mode();
            while (current != pending_mode_) {
                state_manager.get_renderer().get_accessibility_manager().cycle_mode();
                current = state_manager.get_renderer().get_accessibility_manager().get_current_mode();
            }
            show_settings_ = false;
            return;
        }

        if (cancel_button_.getGlobalBounds().contains(mouse_pos)) {
            // Revert to current mode
            pending_mode_ = state_manager.get_renderer().get_accessibility_manager().get_current_mode();
            std::string mode_name = state_manager.get_renderer().get_accessibility_manager().get_mode_name();
            accessibility_cycle_text_.setString("Mode: " + mode_name);
            show_settings_ = false;
            update_positions(state_manager.get_renderer().get_window_size());
            return;
        }
    } else {
        if (start_button_.getGlobalBounds().contains(mouse_pos)) {
            state_manager.change_state(std::make_unique<ModeSelectionState>());
        } else if (quit_button_.getGlobalBounds().contains(mouse_pos)) {
            state_manager.get_renderer().close_window();
        } else if (settings_button_.getGlobalBounds().contains(mouse_pos)) {
            show_settings_ = true;
            pending_mode_ = state_manager.get_renderer().get_accessibility_manager().get_current_mode();
            std::string mode_name = state_manager.get_renderer().get_accessibility_manager().get_mode_name();
            accessibility_cycle_text_.setString("Mode: " + mode_name);
            update_positions(state_manager.get_renderer().get_window_size());
        }
    }
}

void MenuState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)client;
    (void)state_manager;
    (void)delta_time;
    sf::Vector2f mouse_pos = renderer.get_mouse_position();

    ColorBlindMode mode = renderer.get_accessibility_manager().get_current_mode();

    sf::Color start_normal(100, 150, 200);
    sf::Color start_hover(120, 170, 220);
    sf::Color quit_normal(200, 100, 100);
    sf::Color quit_hover(220, 120, 120);
    sf::Color settings_normal(100, 100, 100);
    sf::Color settings_hover(120, 120, 120);

    switch (mode) {
    case ColorBlindMode::Deuteranopia:
        start_normal = sf::Color(0, 0, 255);
        start_hover = sf::Color(50, 50, 255);
        quit_normal = sf::Color(255, 165, 0);
        quit_hover = sf::Color(255, 185, 20);
        settings_normal = sf::Color(50, 50, 50);
        settings_hover = sf::Color(70, 70, 70);
        break;
    case ColorBlindMode::Protanopia:
        start_normal = sf::Color(0, 100, 255);
        start_hover = sf::Color(50, 120, 255);
        quit_normal = sf::Color(255, 255, 0);
        quit_hover = sf::Color(255, 255, 50);
        settings_normal = sf::Color(80, 80, 80);
        settings_hover = sf::Color(100, 100, 100);
        break;
    case ColorBlindMode::Tritanopia:
        start_normal = sf::Color(255, 0, 0);
        start_hover = sf::Color(255, 50, 50);
        quit_normal = sf::Color(0, 200, 200);
        quit_hover = sf::Color(0, 220, 220);
        settings_normal = sf::Color(100, 100, 100);
        settings_hover = sf::Color(120, 120, 120);
        break;
    case ColorBlindMode::None:
    default:
        break;
    }

    if (start_button_.getGlobalBounds().contains(mouse_pos)) {
        start_button_.setFillColor(start_hover);
    } else {
        start_button_.setFillColor(start_normal);
    }

    if (settings_button_.getGlobalBounds().contains(mouse_pos)) {
        settings_button_.setFillColor(settings_hover);
    } else {
        settings_button_.setFillColor(settings_normal);
    }

    if (quit_button_.getGlobalBounds().contains(mouse_pos)) {
        quit_button_.setFillColor(quit_hover);
    } else {
        quit_button_.setFillColor(quit_normal);
    }
}

void MenuState::render(Renderer& renderer, Client& /* client */) {
    renderer.clear();

    if (font_loaded_) {
        renderer.get_window()->draw(logo_sprite_);
        renderer.draw_rectangle(start_button_);
        renderer.draw_text(start_button_text_);

        renderer.draw_rectangle(settings_button_);
        renderer.draw_text(settings_button_text_);

        renderer.draw_rectangle(quit_button_);
        renderer.draw_text(quit_button_text_);

        if (show_settings_) {
            sf::RectangleShape modal_bg(sf::Vector2f(renderer.get_window_size()));
            modal_bg.setFillColor(sf::Color(0, 0, 0, 200));
            renderer.draw_rectangle(modal_bg);

            renderer.draw_rectangle(accessibility_cycle_button_);
            renderer.draw_text(accessibility_cycle_text_);

            renderer.draw_rectangle(confirm_button_);
            renderer.draw_text(confirm_button_text_);
            renderer.draw_rectangle(cancel_button_);
            renderer.draw_text(cancel_button_text_);
        }
    }

    renderer.display();
}

void MenuState::update_positions(const sf::Vector2u& window_size) {
    float title_y = window_size.y * 0.15f;
    float button_start_y = window_size.y * 0.5f;
    float button_spacing = window_size.y * 0.12f;

    logo_sprite_.setPosition((window_size.x - logo_sprite_.getGlobalBounds().width) / 2.0f, title_y);

    float button_width = std::min(300.0f, window_size.x * 0.25f);
    float button_height = std::min(60.0f, window_size.y * 0.08f);
    start_button_.setSize(sf::Vector2f(button_width, button_height));
    start_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y);
    start_button_text_.setPosition(
        start_button_.getPosition().x + (button_width - start_button_text_.getLocalBounds().width) / 2.0f,
        start_button_.getPosition().y + (button_height - start_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    quit_button_.setSize(sf::Vector2f(button_width, button_height));
    quit_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y + button_spacing * 2);
    quit_button_text_.setPosition(
        quit_button_.getPosition().x + (button_width - quit_button_text_.getLocalBounds().width) / 2.0f,
        quit_button_.getPosition().y + (button_height - quit_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    settings_button_.setSize(sf::Vector2f(button_width, button_height));
    settings_button_.setPosition((window_size.x - button_width) / 2.0f, button_start_y + button_spacing);
    settings_button_text_.setPosition(
        settings_button_.getPosition().x + (button_width - settings_button_text_.getLocalBounds().width) / 2.0f,
        settings_button_.getPosition().y + (button_height - settings_button_text_.getLocalBounds().height) / 2.0f -
            5.0f);

    if (show_settings_) {
        float center_x = window_size.x / 2.0f;
        float center_y = window_size.y * 0.4f;

        accessibility_cycle_button_.setPosition(center_x - accessibility_cycle_button_.getSize().x / 2.0f, center_y);

        sf::FloatRect cycle_text_bounds = accessibility_cycle_text_.getLocalBounds();
        accessibility_cycle_text_.setOrigin(cycle_text_bounds.left + cycle_text_bounds.width / 2.0f,
                                            cycle_text_bounds.top + cycle_text_bounds.height / 2.0f);
        accessibility_cycle_text_.setPosition(
            accessibility_cycle_button_.getPosition().x + accessibility_cycle_button_.getSize().x / 2.0f,
            accessibility_cycle_button_.getPosition().y + accessibility_cycle_button_.getSize().y / 2.0f);

        float buttons_y = center_y + 100.0f;
        confirm_button_.setPosition(center_x - 160.0f, buttons_y);
        cancel_button_.setPosition(center_x + 10.0f, buttons_y);

        sf::FloatRect confirm_text_bounds = confirm_button_text_.getLocalBounds();
        confirm_button_text_.setOrigin(confirm_text_bounds.left + confirm_text_bounds.width / 2.0f,
                                       confirm_text_bounds.top + confirm_text_bounds.height / 2.0f);
        confirm_button_text_.setPosition(confirm_button_.getPosition().x + confirm_button_.getSize().x / 2.0f,
                                         confirm_button_.getPosition().y + confirm_button_.getSize().y / 2.0f);

        sf::FloatRect cancel_text_bounds = cancel_button_text_.getLocalBounds();
        cancel_button_text_.setOrigin(cancel_text_bounds.left + cancel_text_bounds.width / 2.0f,
                                      cancel_text_bounds.top + cancel_text_bounds.height / 2.0f);
        cancel_button_text_.setPosition(cancel_button_.getPosition().x + cancel_button_.getSize().x / 2.0f,
                                        cancel_button_.getPosition().y + cancel_button_.getSize().y / 2.0f);
    }
}

} // namespace rtype::client
