#include "../../include/systems/RenderSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/Explosion.hpp"
#include "../../include/components/HitBox.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <vector>

namespace rtype::ecs {

RenderSystem::RenderSystem(sf::RenderWindow& window, std::unordered_map<std::string, sf::Texture>& textures)
    : window_(window), textures_(textures) {
}

void RenderSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Position, component::Drawable>();
    std::vector<GameEngine::entity_t> explosions_to_destroy;

    for (auto entity : view) {
        GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
        auto& pos = registry.getComponent<component::Position>(static_cast<size_t>(entity));
        auto& drawable = registry.getComponent<component::Drawable>(static_cast<size_t>(entity));

        if (textures_.find(drawable.texture_name) == textures_.end())
            continue;

        bool is_explosion = registry.hasComponent<component::Explosion>(entity_id);

        if (!drawable.current_state.empty() && drawable.animation_sequences.count(drawable.current_state)) {
            const auto& sequence = drawable.animation_sequences.at(drawable.current_state);
            if (!sequence.empty()) {
                if (drawable.last_state != drawable.current_state) {
                    drawable.animation_index = 0;
                    drawable.last_state = drawable.current_state;
                    drawable.current_sprite = sequence[drawable.animation_index];
                    drawable.animation_timer = 0.0f;
                } else {
                    drawable.animation_timer += static_cast<float>(dt);

                    while (drawable.animation_timer >= drawable.animation_speed) {
                        drawable.animation_timer -= drawable.animation_speed;
                        drawable.animation_index++;
                        if (drawable.loop)
                            drawable.animation_index %= sequence.size();
                        else
                            drawable.animation_index =
                                std::min(drawable.animation_index, static_cast<uint32_t>(sequence.size() - 1));

                        drawable.current_sprite = sequence[drawable.animation_index];
                    }
                }

                if (is_explosion && !drawable.loop &&
                    drawable.animation_index >= static_cast<uint32_t>(sequence.size() - 1) &&
                    drawable.animation_timer >= drawable.animation_speed * 0.9f) {
                    explosions_to_destroy.push_back(entity_id);
                    continue;
                }
            }
        } else if (drawable.frame_count > 1) {
            drawable.animation_timer += static_cast<float>(dt);
            while (drawable.animation_timer >= drawable.animation_speed) {
                drawable.animation_timer -= drawable.animation_speed;

                if (drawable.loop)
                    drawable.current_sprite =
                        (drawable.current_sprite + 1) % static_cast<uint32_t>(drawable.frame_count);
                else {
                    uint32_t next_frame = drawable.current_sprite + 1;
                    uint32_t frame_count_uint = static_cast<uint32_t>(drawable.frame_count);
                    drawable.current_sprite = (next_frame < frame_count_uint) ? next_frame : frame_count_uint - 1;
                }
            }

            if (is_explosion && !drawable.loop &&
                drawable.current_sprite >= static_cast<uint32_t>(drawable.frame_count - 1) &&
                drawable.animation_timer >= drawable.animation_speed * 0.9f) {
                explosions_to_destroy.push_back(entity_id);
                continue;
            }
        }

        const sf::Texture& texture = textures_.at(drawable.texture_name);
        sf::Vector2u texture_size = texture.getSize();
        sf::IntRect texture_rect;

        if (drawable.rect_width > 0 && drawable.rect_height > 0) {
            int rect_width = drawable.rect_width;
            int rect_height = drawable.rect_height;

            if (is_explosion &&
                static_cast<int>(drawable.current_sprite) == static_cast<int>(drawable.frame_count) - 1) {
                const sf::Texture& texture_check = textures_.at(drawable.texture_name);
                int total_width = static_cast<int>(texture_check.getSize().x);
                int calculated_pos =
                    drawable.rect_x + (static_cast<int>(drawable.current_sprite) * drawable.rect_width);
                int remaining_width = total_width - calculated_pos;
                if (remaining_width > 0 && remaining_width < drawable.rect_width) {
                    rect_width = remaining_width;
                }
            }

            texture_rect = sf::IntRect(drawable.rect_x + (drawable.current_sprite * drawable.rect_width),
                                       drawable.rect_y, rect_width, rect_height);
        } else if (drawable.texture_name == "obstacle_1") {
            texture_rect = sf::IntRect(0, 1, texture_size.x, texture_size.y - 1);
        } else {
            if (drawable.texture_name.find("monster_0-") == 0) {
                texture_rect = sf::IntRect(0, 0, texture_size.x, texture_size.y);
            } else {
                const uint32_t columns = 5;
                uint32_t sprite_width = texture_size.x / columns;
                uint32_t sprite_height = texture_size.y / 5;
                uint32_t row = drawable.sprite_index;
                uint32_t col = drawable.current_sprite % columns;

                texture_rect = sf::IntRect(col * sprite_width, row * sprite_height, sprite_width, sprite_height);
            }
        }

        sf::Sprite sprite(texture, texture_rect);
        sprite.setPosition(pos.x, pos.y);
        sprite.setScale(drawable.scale_x, drawable.scale_y);

        window_.draw(sprite);
    }

    for (auto explosion_entity : explosions_to_destroy) {
        registry.destroyEntity(explosion_entity);
    }
}

} // namespace rtype::ecs
