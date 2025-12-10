#include "../include/Renderer.hpp"
#include <iostream>
#include <algorithm>

namespace rtype::client {

Renderer::Renderer(uint32_t width, uint32_t height)
    : window_(std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height), "R-Type Client")), background_x_(0.0f),
      background_x_stars_(0.0f), background_x_stars2_(0.0f) {
    window_->setFramerateLimit(60);
    window_->setKeyRepeatEnabled(false);
    view_.setSize(static_cast<float>(width), static_cast<float>(height));
    view_.setCenter(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
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
    for (const auto& pair : entities_) {
        sf::Sprite sprite = create_sprite(pair.second);
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
    window_->draw(lives_text_);

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
        window_->draw(bg_sprite);

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
    load_texture("client/sprites/players_ship.png", "player");
    load_texture("client/sprites/map_1.png", "background");
    load_texture("client/sprites/star_bg.png", "background_stars");
    load_texture("client/sprites/star_2_bg.png", "background_stars2");
    load_texture("client/sprites/monster_0.png", "enemy_basic");
    load_texture("client/sprites/r-typesheet2-ezgif.com-crop.gif", "shot");
    load_texture("client/sprites/shot_death.png", "death");
    load_texture("client/sprites/players_ship.png", "default");
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
    view_.setSize(static_cast<float>(width), static_cast<float>(height));
    view_.setCenter(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
    window_->setView(view_);
}

} // namespace rtype::client