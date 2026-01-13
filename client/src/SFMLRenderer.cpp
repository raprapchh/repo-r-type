#include "../include/SFMLRenderer.hpp"
#include <algorithm>

namespace rtype::rendering {

SFMLRenderer::SFMLRenderer(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures)
    : window_(window), textures_(textures) {
}

void SFMLRenderer::draw_sprite(const RenderData& data) {
    if (data.texture_name == "__RECTANGLE__") {
        sf::RectangleShape rect(sf::Vector2f(data.rect_width, data.rect_height));
        rect.setPosition(data.x, data.y);
        rect.setFillColor(sf::Color(data.color_r, data.color_g, data.color_b, data.color_a));
        window_.draw(rect);
        return;
    }

    if (textures_.find(data.texture_name) == textures_.end()) {
        return;
    }

    const sf::Texture& texture = textures_.at(data.texture_name);
    uint32_t texture_width = texture.getSize().x;
    uint32_t texture_height = texture.getSize().y;

    sf::IntRect texture_rect = calculate_texture_rect(data, texture_width, texture_height);

    sf::Sprite sprite(texture, texture_rect);
    sprite.setPosition(data.x, data.y);
    sprite.setScale(data.scale_x, data.scale_y);
    sprite.setRotation(data.rotation);
    if (data.rotation != 0.0f) {
        sf::FloatRect bounds = sprite.getLocalBounds();
        sprite.setOrigin(bounds.width, 0.0f);
    }
    sprite.setColor(sf::Color(data.color_r, data.color_g, data.color_b, data.color_a));

    window_.draw(sprite);
}

void SFMLRenderer::clear() {
    window_.clear(sf::Color::Black);
}

void SFMLRenderer::display() {
    window_.display();
}

bool SFMLRenderer::is_open() const {
    return window_.isOpen();
}

bool SFMLRenderer::get_texture_size(const std::string& texture_name, uint32_t& out_width, uint32_t& out_height) const {
    if (texture_name == "__RECTANGLE__") {
        out_width = 100;
        out_height = 100;
        return true;
    }

    auto it = textures_.find(texture_name);
    if (it == textures_.end()) {
        return false;
    }

    sf::Vector2u size = it->second.getSize();
    out_width = size.x;
    out_height = size.y;
    return true;
}

sf::IntRect SFMLRenderer::calculate_texture_rect(const RenderData& data, uint32_t texture_width,
                                                 uint32_t texture_height) const {
    if (data.texture_name == "player_ships") {
        const uint32_t columns = 5;
        const uint32_t rows = 5;
        uint32_t sprite_width = texture_width / columns;
        uint32_t sprite_height = texture_height / rows;
        uint32_t row = data.sprite_index;
        uint32_t col = data.current_sprite % columns;
        return sf::IntRect(col * sprite_width, row * sprite_height, sprite_width, sprite_height);
    }

    if (data.rect_width > 0 && data.rect_height > 0) {
        int rect_width = data.rect_width;
        int rect_height = data.rect_height;

        if (static_cast<int>(data.current_sprite) == static_cast<int>(data.frame_count) - 1) {
            int calculated_pos = data.rect_x + (static_cast<int>(data.current_sprite) * data.rect_width);
            int remaining_width = static_cast<int>(texture_width) - calculated_pos;
            if (remaining_width > 0 && remaining_width < data.rect_width) {
                rect_width = remaining_width;
            }
        }

        return sf::IntRect(data.rect_x + (data.current_sprite * data.rect_width), data.rect_y, rect_width, rect_height);
    }

    if (data.texture_name == "obstacle_1") {
        return sf::IntRect(0, 1, texture_width, texture_height - 1);
    }

    uint32_t frame_count = (data.frame_count > 0) ? data.frame_count : 1;
    uint32_t sprite_width = texture_width / frame_count;
    uint32_t sprite_height = texture_height;
    return sf::IntRect(data.current_sprite * sprite_width, 0, sprite_width, sprite_height);
}

void SFMLRenderer::draw_parallax_background(const std::string& texture_name, float view_y) {
    if (textures_.find(texture_name) == textures_.end()) {
        return;
    }

    sf::Texture& texture = textures_.at(texture_name);
    if (!texture.isRepeated()) {
        texture.setRepeated(true);
    }

    sf::Vector2u size = window_.getSize();
    float parallax_factor = 0.5f; // Adjust strictly for visual effect
    int texture_y = static_cast<int>(view_y * parallax_factor);

    // We want the background to cover the whole view
    sf::IntRect texture_rect(0, -texture_y, size.x, size.y);

    sf::Sprite sprite(texture, texture_rect);
    sprite.setPosition(window_.mapPixelToCoords(sf::Vector2i(0, 0))); // Draw relative to view

    // Since we are using a view, we need to make sure the sprite is drawn at the top-left of the view
    // However, mapPixelToCoords(0,0) gives us exactly that.
    // Ensure the size matches the window size in world coordinates if the view is zoomed,
    // but here we assume 1:1 pixel mapping for background simplicity or adjust scale.
    // For now, let's just draw it covering the current view area.

    sf::View current_view = window_.getView();
    sf::Vector2f view_center = current_view.getCenter();
    sf::Vector2f view_size = current_view.getSize();

    sprite.setPosition(view_center.x - view_size.x / 2.0f, view_center.y - view_size.y / 2.0f);
    sprite.setTextureRect(sf::IntRect(0, -static_cast<int>(view_y * parallax_factor), static_cast<int>(view_size.x),
                                      static_cast<int>(view_size.y)));

    window_.draw(sprite);
}

} // namespace rtype::rendering
