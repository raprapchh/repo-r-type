#include "../include/Renderer.hpp"
#include <iostream>

namespace rtype::client {

Renderer::Renderer(uint32_t width, uint32_t height)
    : window_(std::make_unique<sf::RenderWindow>(sf::VideoMode(width, height), "R-Type Client"))
      // player_x_(100.0f), player_y_(height / 2.0f)
      ,
      background_x_(0.0f) {
    window_->setFramerateLimit(60);
    window_->setKeyRepeatEnabled(false);
    view_.setSize(static_cast<float>(width), static_cast<float>(height));
    view_.setCenter(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);
    window_->setView(view_);
    std::fill(std::begin(keys_), std::end(keys_), false);
    load_sprites();
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
    keys_[sf::Keyboard::Up] = sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    keys_[sf::Keyboard::Down] = sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
    keys_[sf::Keyboard::Left] = sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
    keys_[sf::Keyboard::Right] = sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
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
        texture_name = "player";
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

    sprite.setPosition(entity.x, entity.y);
    return sprite;
}

void Renderer::draw_ui() {
}

void Renderer::render_frame() {
    clear();

    if (textures_.count("background")) {
        sf::Sprite bg_sprite(textures_["background"]);
        bg_sprite.setPosition(background_x_, 0);
        window_->draw(bg_sprite);

        sf::Sprite bg2_sprite(textures_["background"]);
        bg2_sprite.setPosition(background_x_ + bg_sprite.getLocalBounds().width, 0);
        window_->draw(bg2_sprite);

        background_x_ -= 1.0f;
        if (background_x_ < -bg_sprite.getLocalBounds().width) {
            background_x_ = 0;
        }
    }

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
    load_texture("client/sprites/r-typesheet5.gif", "player");
    load_texture("client/sprites/r-typesheet8.gif", "background");
    load_texture("client/sprites/r-typesheet3.gif", "enemy_basic");
    load_texture("client/sprites/r-typesheet2.gif", "shot");
    load_texture("client/sprites/r-typesheet7.gif", "default");
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
