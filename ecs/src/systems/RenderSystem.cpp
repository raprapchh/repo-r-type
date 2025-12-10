#include "../../include/systems/RenderSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Drawable.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>

namespace rtype::ecs {

RenderSystem::RenderSystem(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures)
    : window_(window), textures_(textures) {
}

void RenderSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Position, component::Drawable>();

    for (auto entity : view) {
        auto& pos = registry.getComponent<component::Position>(static_cast<size_t>(entity));
        auto& drawable = registry.getComponent<component::Drawable>(static_cast<size_t>(entity));

        if (textures_.find(drawable.texture_name) == textures_.end())
            continue;

        // Animation basée sur les états
        if (!drawable.current_state.empty() && drawable.animation_sequences.count(drawable.current_state)) {
            const auto& sequence = drawable.animation_sequences.at(drawable.current_state);
            if (!sequence.empty()) {
                // Reset si changement d'état
                if (drawable.last_state != drawable.current_state) {
                    drawable.animation_index = 0;
                    drawable.last_state = drawable.current_state;
                    drawable.current_sprite = sequence[drawable.animation_index];
                    drawable.animation_timer = 0.0f;
                } else {
                    drawable.animation_timer += static_cast<float>(dt);

                    // Boucle pour gérer gros dt et éviter les saccades
                    while (drawable.animation_timer >= drawable.animation_speed) {
                        drawable.animation_timer -= drawable.animation_speed;
                        drawable.animation_index++;
                        if (drawable.loop)
                            drawable.animation_index %= sequence.size();
                        else
                            drawable.animation_index = std::min(drawable.animation_index, static_cast<uint32_t>(sequence.size() - 1));

                        drawable.current_sprite = sequence[drawable.animation_index];
                    }
                }
            }
        }
        // Animation simple basée sur frame_count
        else if (drawable.frame_count > 1) {
            drawable.animation_timer += static_cast<float>(dt);
            while (drawable.animation_timer >= drawable.animation_speed) {
                drawable.animation_timer -= drawable.animation_speed;

                if (drawable.loop)
                    drawable.current_sprite = (drawable.current_sprite + 1) % drawable.frame_count;
                else {
                    uint32_t next_frame = drawable.current_sprite + 1;
                    drawable.current_sprite = (next_frame < drawable.frame_count) ? next_frame : drawable.frame_count - 1;
                }
            }
        }

        // Gestion du sprite à afficher
        const sf::Texture& texture = textures_.at(drawable.texture_name);
        sf::Vector2u texture_size = texture.getSize();
        sf::IntRect texture_rect;

        if (drawable.rect_width > 0 && drawable.rect_height > 0) {
            texture_rect = sf::IntRect(
                drawable.rect_x + (drawable.current_sprite * drawable.rect_width),
                drawable.rect_y,
                drawable.rect_width,
                drawable.rect_height
            );
        } else {
            const uint32_t columns = 5;
            uint32_t sprite_width = texture_size.x / columns;
            uint32_t sprite_height = texture_size.y / 5;
            uint32_t row = drawable.sprite_index;
            uint32_t col = drawable.current_sprite % columns;

            texture_rect = sf::IntRect(col * sprite_width, row * sprite_height, sprite_width, sprite_height);
        }

        sf::Sprite sprite(texture, texture_rect);
        sprite.setPosition(pos.x, pos.y);
        sprite.setScale(drawable.scale_x, drawable.scale_y);

        window_.draw(sprite);
    }
}

} // namespace rtype::ecs
