#include "../include/SFMLRenderer.hpp"
#include <algorithm>

namespace rtype::rendering {

SFMLRenderer::SFMLRenderer(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures)
    : window_(window), textures_(textures) {
}

void SFMLRenderer::draw_sprite(const RenderData& data) {
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

}
