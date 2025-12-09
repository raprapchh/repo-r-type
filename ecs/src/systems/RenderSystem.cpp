#include "../../include/systems/RenderSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/HitBox.hpp"
#include <SFML/Graphics.hpp>

namespace rtype::ecs {

RenderSystem::RenderSystem(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures)
    : window_(window), textures_(textures) {
}

void RenderSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Position, component::Drawable>();

    for (auto entity : view) {
        auto& pos = registry.getComponent<component::Position>(static_cast<size_t>(entity));
        auto& drawable = registry.getComponent<component::Drawable>(static_cast<size_t>(entity));

        if (textures_.find(drawable.texture_name) == textures_.end()) {
            continue;
        }

        if (drawable.frame_count > 1) {
            drawable.animation_timer += static_cast<float>(dt);
            if (drawable.animation_timer >= drawable.animation_speed) {
                drawable.animation_timer = 0.0f;
                if (drawable.loop) {
                    drawable.animation_frame =
                        (drawable.animation_frame + 1) % static_cast<uint32_t>(drawable.frame_count);
                } else {
                    if (drawable.animation_frame < static_cast<uint32_t>(drawable.frame_count) - 1) {
                        drawable.animation_frame++;
                    }
                }
            }
        }

        const sf::Texture& texture = textures_.at(drawable.texture_name);
        sf::Vector2u texture_size = texture.getSize();

        sf::IntRect texture_rect;

        if (drawable.rect_width > 0 && drawable.rect_height > 0) {
            int current_frame = drawable.animation_frame;
            texture_rect = sf::IntRect(drawable.rect_x + (current_frame * drawable.rect_width), drawable.rect_y,
                                       drawable.rect_width, drawable.rect_height);
        } else if (drawable.texture_name == "obstacle_1") {
            texture_rect = sf::IntRect(0, 1, texture_size.x, texture_size.y - 1);
        } else {
            uint32_t sprite_width = texture_size.x / 5;
            uint32_t sprite_height = texture_size.y / 5;

            uint32_t row = drawable.sprite_index;
            uint32_t col = drawable.animation_frame % 5;

            texture_rect = sf::IntRect(col * sprite_width, row * sprite_height, sprite_width, sprite_height);
        }

        sf::Sprite sprite(texture, texture_rect);
        sprite.setPosition(pos.x, pos.y);
        sprite.setScale(drawable.scale_x, drawable.scale_y);

        window_.draw(sprite);

        sf::FloatRect bounds = sprite.getGlobalBounds();
        sf::RectangleShape hitbox(sf::Vector2f(bounds.width, bounds.height));
        hitbox.setPosition(bounds.left, bounds.top);
        hitbox.setFillColor(sf::Color::Transparent);
        hitbox.setOutlineColor(sf::Color::Red);
        hitbox.setOutlineThickness(2.0f);
        window_.draw(hitbox);
    }

    auto view_hitbox = registry.view<component::Position, component::HitBox>();
    for (auto entity : view_hitbox) {
        auto& pos = registry.getComponent<component::Position>(static_cast<size_t>(entity));
        auto& hitbox = registry.getComponent<component::HitBox>(static_cast<size_t>(entity));

        sf::RectangleShape debug_box(sf::Vector2f(hitbox.width, hitbox.height));
        debug_box.setPosition(pos.x, pos.y);
        debug_box.setFillColor(sf::Color::Transparent);
        debug_box.setOutlineColor(sf::Color::Green);
        debug_box.setOutlineThickness(2.0f);
        window_.draw(debug_box);
    }
}

} // namespace rtype::ecs
