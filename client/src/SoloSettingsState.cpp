#include "SoloSettingsState.hpp"
#include "GameState.hpp"
#include "ModeSelectionState.hpp"
#include <iostream>

namespace rtype::client {

SoloSettingsState::SoloSettingsState()
    : font_loaded_(false), selected_difficulty_(rtype::config::Difficulty::NORMAL), selected_lives_(3) {
    setup_ui();
}

void SoloSettingsState::setup_ui() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr << "Warning: Could not load font. UI will not display text." << std::endl;
        font_loaded_ = false;
        return;
    }
    font_loaded_ = true;

    title_text_.setFont(font_);
    title_text_.setString("SOLO SETTINGS");
    title_text_.setCharacterSize(50);
    title_text_.setFillColor(sf::Color::White);
    title_text_.setStyle(sf::Text::Bold);

    difficulty_label_.setFont(font_);
    difficulty_label_.setString("DIFFICULTY");
    difficulty_label_.setCharacterSize(20);
    difficulty_label_.setFillColor(sf::Color::White);

    difficulty_button_.setSize(sf::Vector2f(200, 50));
    difficulty_button_.setFillColor(sf::Color(80, 80, 80));
    difficulty_button_.setOutlineColor(sf::Color::White);
    difficulty_button_.setOutlineThickness(2);

    difficulty_text_.setFont(font_);
    difficulty_text_.setString("NORMAL");
    difficulty_text_.setCharacterSize(18);
    difficulty_text_.setFillColor(sf::Color::White);

    lives_label_.setFont(font_);
    lives_label_.setString("LIVES");
    lives_label_.setCharacterSize(20);
    lives_label_.setFillColor(sf::Color::White);

    lives_button_.setSize(sf::Vector2f(200, 50));
    lives_button_.setFillColor(sf::Color(80, 80, 80));
    lives_button_.setOutlineColor(sf::Color::White);
    lives_button_.setOutlineThickness(2);

    lives_text_.setFont(font_);
    lives_text_.setString("3");
    lives_text_.setCharacterSize(18);
    lives_text_.setFillColor(sf::Color::White);

    start_button_.setSize(sf::Vector2f(300, 60));
    start_button_.setFillColor(sf::Color(100, 150, 200));
    start_button_.setOutlineColor(sf::Color::White);
    start_button_.setOutlineThickness(2);

    start_button_text_.setFont(font_);
    start_button_text_.setString("START GAME");
    start_button_text_.setCharacterSize(25);
    start_button_text_.setFillColor(sf::Color::White);

    back_button_.setSize(sf::Vector2f(300, 60));
    back_button_.setFillColor(sf::Color(150, 150, 150));
    back_button_.setOutlineColor(sf::Color::White);
    back_button_.setOutlineThickness(2);

    back_button_text_.setFont(font_);
    back_button_text_.setString("BACK");
    back_button_text_.setCharacterSize(25);
    back_button_text_.setFillColor(sf::Color::White);
}

void SoloSettingsState::on_enter(Renderer& renderer, Client& client) {
    (void)client;
    update_positions(renderer.get_window_size());
}

void SoloSettingsState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
}

void SoloSettingsState::handle_input(Renderer& renderer, StateManager& state_manager) {
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

                if (difficulty_button_.getGlobalBounds().contains(mouse_pos)) {
                    if (selected_difficulty_ == rtype::config::Difficulty::EASY) {
                        selected_difficulty_ = rtype::config::Difficulty::NORMAL;
                        difficulty_text_.setString("NORMAL");
                    } else if (selected_difficulty_ == rtype::config::Difficulty::NORMAL) {
                        selected_difficulty_ = rtype::config::Difficulty::HARD;
                        difficulty_text_.setString("HARD");
                    } else {
                        selected_difficulty_ = rtype::config::Difficulty::EASY;
                        difficulty_text_.setString("EASY");
                    }
                } else if (lives_button_.getGlobalBounds().contains(mouse_pos)) {
                    selected_lives_ = (selected_lives_ >= 5) ? 2 : selected_lives_ + 1;
                    lives_text_.setString(std::to_string(selected_lives_));
                } else if (start_button_.getGlobalBounds().contains(mouse_pos)) {
                    state_manager.change_state(
                        std::make_unique<GameState>(false, selected_difficulty_, selected_lives_));
                } else if (back_button_.getGlobalBounds().contains(mouse_pos)) {
                    state_manager.change_state(std::make_unique<ModeSelectionState>());
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Up) {
                if (selected_button_ == SelectedButton::LIVES) {
                    selected_button_ = SelectedButton::DIFFICULTY;
                } else if (selected_button_ == SelectedButton::START) {
                    selected_button_ = SelectedButton::LIVES;
                } else if (selected_button_ == SelectedButton::BACK) {
                    selected_button_ = SelectedButton::START;
                }
            } else if (event.key.code == sf::Keyboard::Down) {
                if (selected_button_ == SelectedButton::DIFFICULTY) {
                    selected_button_ = SelectedButton::LIVES;
                } else if (selected_button_ == SelectedButton::LIVES) {
                    selected_button_ = SelectedButton::START;
                } else if (selected_button_ == SelectedButton::START) {
                    selected_button_ = SelectedButton::BACK;
                }
            } else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                if (selected_button_ == SelectedButton::DIFFICULTY) {
                    if (selected_difficulty_ == rtype::config::Difficulty::EASY) {
                        selected_difficulty_ = rtype::config::Difficulty::NORMAL;
                        difficulty_text_.setString("NORMAL");
                    } else if (selected_difficulty_ == rtype::config::Difficulty::NORMAL) {
                        selected_difficulty_ = rtype::config::Difficulty::HARD;
                        difficulty_text_.setString("HARD");
                    } else {
                        selected_difficulty_ = rtype::config::Difficulty::EASY;
                        difficulty_text_.setString("EASY");
                    }
                } else if (selected_button_ == SelectedButton::LIVES) {
                    selected_lives_ = (selected_lives_ >= 5) ? 2 : selected_lives_ + 1;
                    lives_text_.setString(std::to_string(selected_lives_));
                } else if (selected_button_ == SelectedButton::START) {
                    state_manager.change_state(
                        std::make_unique<GameState>(false, selected_difficulty_, selected_lives_));
                } else if (selected_button_ == SelectedButton::BACK) {
                    state_manager.change_state(std::make_unique<ModeSelectionState>());
                }
            } else if (event.key.code == sf::Keyboard::Escape) {
                state_manager.change_state(std::make_unique<ModeSelectionState>());
            }
        }
    }
}

void SoloSettingsState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)client;
    (void)state_manager;
    (void)delta_time;
    sf::Vector2f mouse_pos = renderer.get_mouse_position();

    ColorBlindMode mode = renderer.get_accessibility_manager().get_current_mode();

    sf::Color button_normal(100, 150, 200);
    sf::Color button_hover(120, 170, 220);
    sf::Color button_selected(200, 200, 0);
    sf::Color setting_normal(80, 80, 80);
    sf::Color setting_hover(100, 100, 100);
    sf::Color setting_selected(150, 150, 0);
    sf::Color back_normal(150, 150, 150);
    sf::Color back_hover(170, 170, 170);
    sf::Color back_selected(200, 200, 0);

    switch (mode) {
    case ColorBlindMode::Deuteranopia:
        button_normal = sf::Color(0, 0, 255);
        button_hover = sf::Color(50, 50, 255);
        button_selected = sf::Color(0, 255, 255);
        setting_normal = sf::Color(100, 100, 100);
        setting_hover = sf::Color(120, 120, 120);
        setting_selected = sf::Color(0, 200, 200);
        back_normal = sf::Color(255, 165, 0);
        back_hover = sf::Color(255, 185, 20);
        back_selected = sf::Color(0, 255, 255);
        break;
    case ColorBlindMode::Protanopia:
        button_normal = sf::Color(0, 100, 255);
        button_hover = sf::Color(50, 120, 255);
        button_selected = sf::Color(0, 255, 255);
        setting_normal = sf::Color(100, 100, 100);
        setting_hover = sf::Color(120, 120, 120);
        setting_selected = sf::Color(0, 200, 200);
        back_normal = sf::Color(255, 255, 0);
        back_hover = sf::Color(255, 255, 50);
        back_selected = sf::Color(0, 255, 255);
        break;
    case ColorBlindMode::Tritanopia:
        button_normal = sf::Color(255, 0, 0);
        button_hover = sf::Color(255, 50, 50);
        button_selected = sf::Color(0, 255, 255);
        setting_normal = sf::Color(100, 100, 100);
        setting_hover = sf::Color(120, 120, 120);
        setting_selected = sf::Color(200, 200, 0);
        back_normal = sf::Color(0, 200, 200);
        back_hover = sf::Color(0, 220, 220);
        back_selected = sf::Color(255, 255, 0);
        break;
    case ColorBlindMode::None:
    default:
        break;
    }

    if (difficulty_button_.getGlobalBounds().contains(mouse_pos)) {
        difficulty_button_.setFillColor(setting_hover);
    } else if (selected_button_ == SelectedButton::DIFFICULTY) {
        difficulty_button_.setFillColor(setting_selected);
    } else {
        difficulty_button_.setFillColor(setting_normal);
    }

    if (lives_button_.getGlobalBounds().contains(mouse_pos)) {
        lives_button_.setFillColor(setting_hover);
    } else if (selected_button_ == SelectedButton::LIVES) {
        lives_button_.setFillColor(setting_selected);
    } else {
        lives_button_.setFillColor(setting_normal);
    }

    if (start_button_.getGlobalBounds().contains(mouse_pos)) {
        start_button_.setFillColor(button_hover);
    } else if (selected_button_ == SelectedButton::START) {
        start_button_.setFillColor(button_selected);
    } else {
        start_button_.setFillColor(button_normal);
    }

    if (back_button_.getGlobalBounds().contains(mouse_pos)) {
        back_button_.setFillColor(back_hover);
    } else if (selected_button_ == SelectedButton::BACK) {
        back_button_.setFillColor(back_selected);
    } else {
        back_button_.setFillColor(back_normal);
    }
}

void SoloSettingsState::render(Renderer& renderer, Client& /* client */) {
    renderer.clear();

    if (font_loaded_) {
        renderer.draw_text(title_text_);

        renderer.draw_text(difficulty_label_);
        renderer.draw_rectangle(difficulty_button_);
        renderer.draw_text(difficulty_text_);

        renderer.draw_text(lives_label_);
        renderer.draw_rectangle(lives_button_);
        renderer.draw_text(lives_text_);

        renderer.draw_rectangle(start_button_);
        renderer.draw_text(start_button_text_);

        renderer.draw_rectangle(back_button_);
        renderer.draw_text(back_button_text_);
    }

    renderer.display();
}

void SoloSettingsState::update_positions(const sf::Vector2u& window_size) {
    float title_y = window_size.y * 0.1f;
    float settings_start_y = window_size.y * 0.3f;
    float setting_spacing = window_size.y * 0.12f;
    float button_start_y = window_size.y * 0.65f;
    float button_spacing = window_size.y * 0.1f;

    title_text_.setPosition((window_size.x - title_text_.getLocalBounds().width) / 2.0f, title_y);

    float label_x = window_size.x * 0.3f;
    float button_x = window_size.x * 0.55f;
    float button_width = 200.0f;
    float button_height = 50.0f;

    difficulty_label_.setPosition(label_x, settings_start_y);
    difficulty_button_.setPosition(button_x, settings_start_y - 5.0f);
    difficulty_text_.setPosition(button_x + (button_width - difficulty_text_.getLocalBounds().width) / 2.0f,
                                 settings_start_y + (button_height - difficulty_text_.getLocalBounds().height) / 2.0f -
                                     10.0f);

    lives_label_.setPosition(label_x, settings_start_y + setting_spacing);
    lives_button_.setPosition(button_x, settings_start_y + setting_spacing - 5.0f);
    lives_text_.setPosition(button_x + (button_width - lives_text_.getLocalBounds().width) / 2.0f,
                            settings_start_y + setting_spacing +
                                (button_height - lives_text_.getLocalBounds().height) / 2.0f - 10.0f);

    float main_button_width = std::min(300.0f, window_size.x * 0.25f);
    float main_button_height = std::min(60.0f, window_size.y * 0.08f);

    start_button_.setSize(sf::Vector2f(main_button_width, main_button_height));
    start_button_.setPosition((window_size.x - main_button_width) / 2.0f, button_start_y);
    start_button_text_.setPosition(start_button_.getPosition().x +
                                       (main_button_width - start_button_text_.getLocalBounds().width) / 2.0f,
                                   start_button_.getPosition().y +
                                       (main_button_height - start_button_text_.getLocalBounds().height) / 2.0f - 5.0f);

    back_button_.setSize(sf::Vector2f(main_button_width, main_button_height));
    back_button_.setPosition((window_size.x - main_button_width) / 2.0f, button_start_y + button_spacing);
    back_button_text_.setPosition(
        back_button_.getPosition().x + (main_button_width - back_button_text_.getLocalBounds().width) / 2.0f,
        back_button_.getPosition().y + (main_button_height - back_button_text_.getLocalBounds().height) / 2.0f - 5.0f);
}

} // namespace rtype::client
