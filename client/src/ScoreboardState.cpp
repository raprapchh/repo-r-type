#include "../include/ScoreboardState.hpp"
#include "../include/MenuState.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>

namespace rtype::client {

ScoreboardState::ScoreboardState(ScoreboardManager& scoreboard_manager)
    : scoreboard_manager_(scoreboard_manager), font_loaded_(false), current_tab_(TabMode::Solo) {
    setup_ui();
}

void ScoreboardState::setup_ui() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr << "Warning: Could not load font. Scoreboard UI will not display text." << std::endl;
        font_loaded_ = false;
        return;
    }
    font_loaded_ = true;

    title_text_.setFont(font_);
    title_text_.setString("SCOREBOARD");
    title_text_.setCharacterSize(60);
    title_text_.setFillColor(sf::Color::White);

    solo_tab_.setSize(sf::Vector2f(250, 50));
    solo_tab_.setFillColor(sf::Color(100, 150, 200));
    solo_tab_.setOutlineColor(sf::Color::White);
    solo_tab_.setOutlineThickness(2);

    multi_tab_.setSize(sf::Vector2f(250, 50));
    multi_tab_.setFillColor(sf::Color(80, 80, 80));
    multi_tab_.setOutlineColor(sf::Color::White);
    multi_tab_.setOutlineThickness(2);

    solo_tab_text_.setFont(font_);
    solo_tab_text_.setString("SOLO");
    solo_tab_text_.setCharacterSize(25);
    solo_tab_text_.setFillColor(sf::Color::White);

    multi_tab_text_.setFont(font_);
    multi_tab_text_.setString("MULTI");
    multi_tab_text_.setCharacterSize(25);
    multi_tab_text_.setFillColor(sf::Color::White);

    solo_title_text_.setFont(font_);
    solo_title_text_.setString("Best Personal Scores");
    solo_title_text_.setCharacterSize(30);
    solo_title_text_.setFillColor(sf::Color::Yellow);

    multi_title_text_.setFont(font_);
    multi_title_text_.setString("Global Leaderboard");
    multi_title_text_.setCharacterSize(30);
    multi_title_text_.setFillColor(sf::Color::Yellow);

    back_button_.setSize(sf::Vector2f(200, 50));
    back_button_.setFillColor(sf::Color(200, 100, 100));
    back_button_.setOutlineColor(sf::Color::White);
    back_button_.setOutlineThickness(2);

    back_button_text_.setFont(font_);
    back_button_text_.setString("BACK");
    back_button_text_.setCharacterSize(30);
    back_button_text_.setFillColor(sf::Color::White);
}

void ScoreboardState::on_enter(Renderer& renderer, Client& client) {
    (void)client;
    scoreboard_manager_.load();
    update_score_display();
    update_positions(renderer.get_window_size());
}

void ScoreboardState::on_exit(Renderer& renderer, Client& client) {
    (void)renderer;
    (void)client;
}

void ScoreboardState::update_score_display() {
    solo_score_texts_.clear();
    multi_score_texts_.clear();

    const auto& data = scoreboard_manager_.get_data();

    int rank = 1;
    for (const auto& entry : data.solo_scores) {
        sf::Text text;
        text.setFont(font_);

        std::ostringstream oss;
        oss << std::setw(2) << rank << ". " << std::setw(20) << std::left << entry.player_name << std::setw(10)
            << std::right << entry.score;

        text.setString(oss.str());
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        solo_score_texts_.push_back(text);
        rank++;
    }

    rank = 1;
    for (const auto& entry : data.multi_scores) {
        sf::Text text;
        text.setFont(font_);

        std::ostringstream oss;
        oss << std::setw(2) << rank << ". " << std::setw(20) << std::left << entry.player_name << std::setw(10)
            << std::right << entry.score;

        text.setString(oss.str());
        text.setCharacterSize(24);
        text.setFillColor(sf::Color::White);
        multi_score_texts_.push_back(text);
        rank++;
    }
}

void ScoreboardState::update_positions(const sf::Vector2u& window_size) {
    float center_x = window_size.x / 2.0f;
    float top_margin = 50.0f;

    title_text_.setPosition(center_x - title_text_.getGlobalBounds().width / 2.0f, top_margin);

    float tab_y = top_margin + 100.0f;
    solo_tab_.setPosition(center_x - 260.0f, tab_y);
    multi_tab_.setPosition(center_x + 10.0f, tab_y);

    solo_tab_text_.setPosition(solo_tab_.getPosition().x +
                                   (solo_tab_.getSize().x - solo_tab_text_.getGlobalBounds().width) / 2.0f,
                               solo_tab_.getPosition().y + 10.0f);

    multi_tab_text_.setPosition(multi_tab_.getPosition().x +
                                    (multi_tab_.getSize().x - multi_tab_text_.getGlobalBounds().width) / 2.0f,
                                multi_tab_.getPosition().y + 10.0f);

    float content_y = tab_y + 80.0f;
    if (current_tab_ == TabMode::Solo) {
        solo_title_text_.setPosition(center_x - solo_title_text_.getGlobalBounds().width / 2.0f, content_y);
    } else {
        multi_title_text_.setPosition(center_x - multi_title_text_.getGlobalBounds().width / 2.0f, content_y);
    }

    float score_y = content_y + 60.0f;
    float score_spacing = 40.0f;

    auto& texts = (current_tab_ == TabMode::Solo) ? solo_score_texts_ : multi_score_texts_;
    for (auto& text : texts) {
        text.setPosition(center_x - 300.0f, score_y);
        score_y += score_spacing;
    }

    back_button_.setPosition(center_x - back_button_.getSize().x / 2.0f, window_size.y - 100.0f);
    back_button_text_.setPosition(back_button_.getPosition().x +
                                      (back_button_.getSize().x - back_button_text_.getGlobalBounds().width) / 2.0f,
                                  back_button_.getPosition().y + 10.0f);
}

void ScoreboardState::handle_input(Renderer& renderer, StateManager& state_manager) {
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

                if (back_button_.getGlobalBounds().contains(mouse_pos)) {
                    state_manager.change_state(std::make_unique<MenuState>());
                }

                if (solo_tab_.getGlobalBounds().contains(mouse_pos)) {
                    current_tab_ = TabMode::Solo;
                    solo_tab_.setFillColor(sf::Color(100, 150, 200));
                    multi_tab_.setFillColor(sf::Color(80, 80, 80));
                    update_positions(renderer.get_window_size());
                }

                if (multi_tab_.getGlobalBounds().contains(mouse_pos)) {
                    current_tab_ = TabMode::Multi;
                    multi_tab_.setFillColor(sf::Color(100, 150, 200));
                    solo_tab_.setFillColor(sf::Color(80, 80, 80));
                    update_positions(renderer.get_window_size());
                }
            }
        } else if (event.type == sf::Event::KeyPressed) {
            if (event.key.code == sf::Keyboard::Escape) {
                state_manager.change_state(std::make_unique<MenuState>());
            } else if (event.key.code == sf::Keyboard::Tab) {
                if (current_tab_ == TabMode::Solo) {
                    current_tab_ = TabMode::Multi;
                    multi_tab_.setFillColor(sf::Color(100, 150, 200));
                    solo_tab_.setFillColor(sf::Color(80, 80, 80));
                } else {
                    current_tab_ = TabMode::Solo;
                    solo_tab_.setFillColor(sf::Color(100, 150, 200));
                    multi_tab_.setFillColor(sf::Color(80, 80, 80));
                }
                update_positions(renderer.get_window_size());
            }
        }
    }
}

void ScoreboardState::update(Renderer& renderer, Client& client, StateManager& state_manager, float delta_time) {
    (void)renderer;
    (void)client;
    (void)state_manager;
    (void)delta_time;
}

void ScoreboardState::render(Renderer& renderer, Client& client) {
    (void)client;

    if (!font_loaded_) {
        return;
    }

    renderer.draw_text(title_text_);

    renderer.draw_rectangle(solo_tab_);
    renderer.draw_rectangle(multi_tab_);
    renderer.draw_text(solo_tab_text_);
    renderer.draw_text(multi_tab_text_);

    if (current_tab_ == TabMode::Solo) {
        renderer.draw_text(solo_title_text_);
        for (auto& text : solo_score_texts_) {
            renderer.draw_text(text);
        }
    } else {
        renderer.draw_text(multi_title_text_);
        for (auto& text : multi_score_texts_) {
            renderer.draw_text(text);
        }
    }

    renderer.draw_rectangle(back_button_);
    renderer.draw_text(back_button_text_);
}

} // namespace rtype::client
