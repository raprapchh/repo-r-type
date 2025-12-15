#include "../include/Renderer.hpp"
#include "../../shared/GameConstants.hpp"
#include <iostream>
#include <algorithm>

namespace rtype::client {

Renderer::Renderer(uint32_t width, uint32_t height)
    : window_(std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height), "R-Type Client")), background_x_(0.0f),
      background_x_stars_(0.0f), background_x_stars2_(0.0f) {
    window_->setFramerateLimit(60);
    window_->setKeyRepeatEnabled(false);
    view_.setSize(rtype::constants::SCREEN_WIDTH, rtype::constants::SCREEN_HEIGHT);
    view_.setCenter(rtype::constants::SCREEN_WIDTH / 2.0f, rtype::constants::SCREEN_HEIGHT / 2.0f);
    window_->setView(view_);
    std::fill(std::begin(keys_), std::end(keys_), false);
    load_sprites();
    load_fonts();
}

void Renderer::load_fonts() {
    if (!font_.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
        std::cerr << "Erreur: Impossible de charger la police client/fonts/Ethnocentric-Regular.otf" << std::endl;
    }

    score_text_.setFont(font_);
    score_text_.setCharacterSize(24);
    score_text_.setFillColor(sf::Color::White);
    score_text_.setPosition(10.0f, 10.0f);

    lives_text_.setFont(font_);
    lives_text_.setCharacterSize(24);
    lives_text_.setFillColor(sf::Color::White);
    lives_text_.setPosition(10.0f, 40.0f);

    wave_text_.setFont(font_);
    wave_text_.setCharacterSize(24);
    wave_text_.setFillColor(sf::Color::White);
    wave_text_.setPosition(600.0f, 10.0f);
}

Renderer::~Renderer() = default;

bool Renderer::is_open() const {
    return window_->isOpen();
}

bool Renderer::poll_event(sf::Event& event) {
    return window_->pollEvent(event);
}

void Renderer::clear() {
    window_->clear(sf::Color::Black);
}

void Renderer::display() {
    window_->display();
}

void Renderer::handle_input() {
    if (!window_ || !window_->hasFocus()) {
        std::fill(std::begin(keys_), std::end(keys_), false);
        return;
    }

    keys_[sf::Keyboard::Up] =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::Z);
    keys_[sf::Keyboard::Down] =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S);
    keys_[sf::Keyboard::Left] =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
    keys_[sf::Keyboard::Right] =
        sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D);
    keys_[sf::Keyboard::Space] = sf::Keyboard::isKeyPressed(sf::Keyboard::Space);
}

void Renderer::update_entity(const Entity& entity) {
    entities_[entity.id] = entity;
}

void Renderer::spawn_entity(const Entity& entity) {
    std::cout << "Spawning entity with ID: " << entity.id << ", Type: " << entity.type << std::endl;
    entities_[entity.id] = entity;
}

void Renderer::remove_entity(uint32_t entity_id) {
    entities_.erase(entity_id);
}

void Renderer::update_animations(float delta_time) {
    for (auto& [id, entity] : entities_) {
        if (entity.type == rtype::net::EntityType::PROJECTILE) {
            entity.animation_timer += delta_time;
            if (entity.animation_timer >= 0.1f) {
                entity.animation_timer = 0.0f;
                entity.animation_frame = (entity.animation_frame + 1) % 4;
            }
        }
        if (entity.type == rtype::net::EntityType::PLAYER) {
            entity.animation_timer += delta_time;
            if (entity.animation_timer >= 0.1f && entity.player_state == 1) {
                entity.animation_timer = 0.0f;
                entity.animation_frame = (entity.animation_frame + 1) % 4;
            } else if (entity.animation_timer >= 0.1f && entity.player_state == 2) {
                entity.animation_timer = 0.0f;
                entity.animation_frame = (entity.animation_frame - 1) % 4;
            } else {
                entity.animation_frame = 2;
                entity.player_state = 0;
            }
        }
    }
}

void Renderer::update_game_state(const rtype::net::GameStateData& state) {
    game_state_ = state;
}

void Renderer::draw_entities() {
    static bool logged = false;
    ColorBlindMode current_mode = accessibility_manager_.get_current_mode();
    if (!logged && current_mode != ColorBlindMode::None) {
        std::cout << "[DEBUG] Colorblind mode is ACTIVE (" << accessibility_manager_.get_mode_name()
                  << ") - applying entity colors" << std::endl;
        logged = true;
    } else if (logged && current_mode == ColorBlindMode::None) {
        std::cout << "[DEBUG] Colorblind mode is INACTIVE - applying normal colors" << std::endl;
        logged = false;
    }

    for (const auto& pair : entities_) {
        sf::Sprite sprite = create_sprite(pair.second);
        sf::Color color = accessibility_manager_.get_entity_color(pair.second.type);
        sprite.setColor(color);
        window_->draw(sprite);
    }
}

sf::Sprite Renderer::create_sprite(const Entity& entity) {
    sf::Sprite sprite;
    std::string texture_name;

    switch (entity.type) {
    case rtype::net::EntityType::PLAYER:
        texture_name = "player_ships";
        break;
    case rtype::net::EntityType::ENEMY:
        texture_name = "enemy_basic";
        sprite.setScale(6.0f, 6.0f);
        break;
    case rtype::net::EntityType::PROJECTILE:
        texture_name = "shot";
        break;
    default:
        texture_name = "default";
        break;
    }

    if (textures_.count(texture_name)) {
        sprite.setTexture(textures_[texture_name]);
    } else if (textures_.count("default")) {
        sprite.setTexture(textures_["default"]);
    }

    if (entity.type == rtype::net::EntityType::PROJECTILE) {
        sprite.setTextureRect(sf::IntRect(entity.animation_frame * 29, 0, 29, 33));
    } else if (entity.type == rtype::net::EntityType::PLAYER) {
        if (textures_.count("player_ships")) {
            sf::Vector2u texture_size = textures_["player_ships"].getSize();
            uint32_t columns = 5;
            uint32_t rows = 5;
            uint32_t sprite_width = texture_size.x / columns;
            uint32_t sprite_height = texture_size.y / rows;

            uint32_t sprite_index = (entity.id - 1) % 4;
            uint32_t row = sprite_index;
            uint32_t col = entity.animation_frame % columns;

            sprite.setTextureRect(sf::IntRect(col * sprite_width, row * sprite_height, sprite_width, sprite_height));
            sprite.setScale(1.7f, 1.7f);
        }
    }

    sprite.setPosition(entity.x, entity.y);
    return sprite;
}

void Renderer::draw_ui() {
    sf::View current_view = window_->getView();
    window_->setView(window_->getDefaultView());

    score_text_.setString("Score: " + std::to_string(game_state_.score));
    lives_text_.setString("Lives: " + std::to_string(static_cast<int>(game_state_.lives)));

    window_->draw(score_text_);
    window_->draw(score_text_);
    window_->draw(lives_text_);

    int level = game_state_.wave_number / 100;
    int wave = game_state_.wave_number % 100;
    wave_text_.setString("Level " + std::to_string(level + 1) + " - Wave " + std::to_string(wave));
    window_->draw(wave_text_);

    if (charge_percentage_ > 0.0f) {
        sf::RectangleShape charge_bar_bg(sf::Vector2f(200.0f, 20.0f));
        charge_bar_bg.setFillColor(sf::Color(50, 50, 50));
        charge_bar_bg.setPosition(10.0f, 70.0f);
        window_->draw(charge_bar_bg);

        sf::RectangleShape charge_bar_fg(sf::Vector2f(200.0f * charge_percentage_, 20.0f));
        charge_bar_fg.setFillColor(sf::Color(0, 255, 255));
        charge_bar_fg.setPosition(10.0f, 70.0f);
        window_->draw(charge_bar_fg);
    }

    window_->setView(current_view);
}

void Renderer::draw_game_over(bool all_players_dead) {
    sf::View current_view = window_->getView();
    window_->setView(window_->getDefaultView());

    sf::Vector2u window_size = window_->getSize();
    float center_x = static_cast<float>(window_size.x) / 2.0f;
    float center_y = static_cast<float>(window_size.y) / 2.0f;

    sf::Color game_over_color = sf::Color::Red;
    sf::Color button_color = sf::Color(70, 130, 180);

    ColorBlindMode mode = accessibility_manager_.get_current_mode();
    switch (mode) {
    case ColorBlindMode::Deuteranopia:
        game_over_color = sf::Color(255, 165, 0);
        button_color = sf::Color(0, 0, 255);
        break;
    case ColorBlindMode::Protanopia:
        game_over_color = sf::Color(255, 255, 0);
        button_color = sf::Color(0, 100, 255);
        break;
    case ColorBlindMode::Tritanopia:
        game_over_color = sf::Color(0, 200, 200);
        button_color = sf::Color(255, 0, 0);
        break;
    case ColorBlindMode::None:
    default:
        break;
    }

    sf::Text game_over_text;
    game_over_text.setFont(font_);
    game_over_text.setString("GAME OVER");
    game_over_text.setCharacterSize(72);
    game_over_text.setFillColor(game_over_color);
    game_over_text.setStyle(sf::Text::Bold);

    sf::FloatRect text_bounds = game_over_text.getLocalBounds();
    game_over_text.setOrigin(text_bounds.left + text_bounds.width / 2.0f, text_bounds.top + text_bounds.height / 2.0f);
    game_over_text.setPosition(center_x, center_y - 80.0f);

    window_->draw(game_over_text);

    if (!all_players_dead) {
        sf::Text waiting_text;
        waiting_text.setFont(font_);
        waiting_text.setString("Waiting for other players...");
        waiting_text.setCharacterSize(24);
        waiting_text.setFillColor(sf::Color::White);

        sf::FloatRect waiting_bounds = waiting_text.getLocalBounds();
        waiting_text.setOrigin(waiting_bounds.left + waiting_bounds.width / 2.0f,
                               waiting_bounds.top + waiting_bounds.height / 2.0f);
        waiting_text.setPosition(center_x, center_y);

        window_->draw(waiting_text);
    }

    if (all_players_dead) {
        back_to_menu_button_.setSize(sf::Vector2f(280.0f, 65.0f));
        back_to_menu_button_.setFillColor(button_color);
        back_to_menu_button_.setOutlineThickness(2);
        back_to_menu_button_.setOutlineColor(sf::Color::White);

        sf::FloatRect button_bounds = back_to_menu_button_.getLocalBounds();
        back_to_menu_button_.setOrigin(button_bounds.width / 2.0f, button_bounds.height / 2.0f);
        back_to_menu_button_.setPosition(center_x, center_y + 60.0f);

        back_to_menu_text_.setFont(font_);
        back_to_menu_text_.setString("BACK TO MENU");
        back_to_menu_text_.setCharacterSize(23);
        back_to_menu_text_.setFillColor(sf::Color::White);

        sf::FloatRect text_bounds_btn = back_to_menu_text_.getLocalBounds();
        back_to_menu_text_.setOrigin(text_bounds_btn.left + text_bounds_btn.width / 2.0f,
                                     text_bounds_btn.top + text_bounds_btn.height / 2.0f);
        back_to_menu_text_.setPosition(center_x, center_y + 60.0f);

        window_->draw(back_to_menu_button_);
        window_->draw(back_to_menu_text_);
    }

    window_->setView(current_view);
}
void Renderer::draw_background() {
    if (textures_.count("background")) {
        window_->setView(view_);

        sf::Sprite bg_sprite(textures_["background"]);

        float window_height = view_.getSize().y;
        float window_width = view_.getSize().x;
        float texture_height = bg_sprite.getLocalBounds().height;
        float scale = window_height / texture_height;

        sf::Sprite bg3_sprite(textures_["background_stars2"]);
        bg3_sprite.setOrigin(0, texture_height);
        bg3_sprite.setScale(scale, scale);
        bg3_sprite.setPosition(background_x_stars2_, window_height);
        window_->draw(bg3_sprite);

        sf::Sprite bg2_sprite(textures_["background_stars"]);
        bg2_sprite.setOrigin(0, texture_height);
        bg2_sprite.setScale(scale, scale);
        bg2_sprite.setPosition(background_x_stars_, window_height);
        window_->draw(bg2_sprite);

        bg_sprite.setOrigin(0, texture_height);
        bg_sprite.setScale(scale, scale);
        bg_sprite.setPosition(background_x_, window_height);

        background_x_ -= 3.0f;
        background_x_stars_ -= 5.0f;
        background_x_stars2_ -= 10.0f;

        float bg_width = bg_sprite.getLocalBounds().width * scale;
        float bg2_width = bg2_sprite.getLocalBounds().width * scale;
        float bg3_width = bg3_sprite.getLocalBounds().width * scale;

        if (background_x_ < -bg_width + window_width) {
            background_x_ = 0;
        }
        if (background_x_stars_ < -bg2_width + window_width) {
            background_x_stars_ = 0;
        }
        if (background_x_stars2_ < -bg3_width + window_width) {
            background_x_stars2_ = 0;
        }
    }
}

void Renderer::render_frame() {
    clear();
    draw_background();
    draw_entities();
    draw_ui();
    display();
}

void Renderer::load_texture(const std::string& path, const std::string& name) {
    sf::Texture texture;
    if (texture.loadFromFile(path)) {
        textures_[name] = texture;
    } else {
        std::cerr << "Erreur: Impossible de charger la texture " << path << std::endl;
    }
}

void Renderer::load_sprites() {
    load_texture("client/sprites/players_ship.png", "player_ships");
    load_texture("client/sprites/map_1.png", "background");
    load_texture("client/sprites/star_bg.png", "background_stars");
    load_texture("client/sprites/star_2_bg.png", "background_stars2");
    load_texture("client/sprites/monster_0.png", "enemy_basic");
    load_texture("client/sprites/monster_0-top.gif", "monster_0-top");
    load_texture("client/sprites/monster_0-bot.gif", "monster_0-bot");
    load_texture("client/sprites/monster_0-left.gif", "monster_0-left");
    load_texture("client/sprites/monster_0-right.gif", "monster_0-right");
    load_texture("client/sprites/monster_0-ball.gif", "monster_0-ball");
    load_texture("client/sprites/r-typesheet2-ezgif.com-crop.gif", "shot");
    load_texture("client/sprites/shot_death.png", "death");
    load_texture("client/sprites/obstacle1.png", "obstacle_1");
    load_texture("client/sprites/floor_obstacle.png", "floor_obstacle");
    load_texture("client/sprites/shot_death-charge1.gif", "shot_death-charge1");
    load_texture("client/sprites/shot_death-charge2.gif", "shot_death-charge2");
    load_texture("client/sprites/shot_death-charge3.gif", "shot_death-charge3");
    load_texture("client/sprites/shot_death-charge4.gif", "shot_death-charge4");
    load_texture("client/sprites/shot_death-charge-paricule.gif", "charge_particle");
    load_texture("client/sprites/explosion.gif", "explosion");
    load_texture("client/sprites/players_ship.png", "default");
}

void Renderer::draw_charge_effect(const sf::Vector2f& position, float delta_time) {
    if (textures_.find("charge_particle") == textures_.end())
        return;

    charge_particle_timer_ += delta_time;
    if (charge_particle_timer_ >= 0.1f) {
        charge_particle_timer_ = 0.0f;
        charge_particle_frame_ = (charge_particle_frame_ + 1) % 5;
    }

    charge_particle_sprite_.setTexture(textures_["charge_particle"]);

    sf::Vector2u texture_size = textures_["charge_particle"].getSize();
    int frame_width = texture_size.x / 5;
    int frame_height = texture_size.y;

    charge_particle_sprite_.setTextureRect(
        sf::IntRect(charge_particle_frame_ * frame_width, 0, frame_width, frame_height));

    charge_particle_sprite_.setScale(3.0f, 3.0f);
    charge_particle_sprite_.setPosition(position.x + 130.0f, position.y + 10.0f);

    window_->draw(charge_particle_sprite_);
}

sf::Vector2f Renderer::get_shoot_direction() const {
    return {1.0f, 0.0f};
}

void Renderer::close_window() {
    window_->close();
}

sf::Vector2f Renderer::get_player_position(uint32_t player_id) const {
    auto it = entities_.find(player_id);
    if (it != entities_.end()) {
        return {it->second.x, it->second.y};
    }
    return {0.0f, 0.0f};
}

void Renderer::draw_text(const sf::Text& text) {
    window_->draw(text);
}

void Renderer::draw_rectangle(const sf::RectangleShape& rectangle) {
    window_->draw(rectangle);
}

sf::Vector2u Renderer::get_window_size() const {
    return window_->getSize();
}

sf::Vector2f Renderer::get_mouse_position() const {
    return window_->mapPixelToCoords(sf::Mouse::getPosition(*window_));
}

void Renderer::handle_resize(uint32_t width, uint32_t height) {
    (void)width;
    (void)height;
    view_.setSize(rtype::constants::SCREEN_WIDTH, rtype::constants::SCREEN_HEIGHT);
    view_.setCenter(rtype::constants::SCREEN_WIDTH / 2.0f, rtype::constants::SCREEN_HEIGHT / 2.0f);
    window_->setView(view_);
}

bool Renderer::is_game_over_back_to_menu_clicked(const sf::Vector2f& mouse_pos) const {
    return back_to_menu_button_.getGlobalBounds().contains(mouse_pos);
}

} // namespace rtype::client
