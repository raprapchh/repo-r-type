#include "../../include/systems/RenderSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/Explosion.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/NetworkId.hpp"
#include "../../include/components/HitFlash.hpp"
#include "../../../client/include/AccessibilityManager.hpp"
#include "../../../shared/net/MessageData.hpp"
#include <algorithm>
#include <vector>

namespace rtype::ecs {

RenderSystem::RenderSystem(std::shared_ptr<rtype::rendering::IRenderer> renderer,
                           rtype::client::AccessibilityManager* accessibility_mgr)
    : renderer_(renderer), accessibility_manager_(accessibility_mgr) {
}

void RenderSystem::update(GameEngine::Registry& registry, double dt) {
    if (!renderer_ || !renderer_->is_open()) {
        return;
    }

    auto view = registry.view<component::Position, component::Drawable>();
    std::vector<GameEngine::entity_t> explosions_to_destroy;

    for (auto entity : view) {
        GameEngine::entity_t entity_id = static_cast<GameEngine::entity_t>(entity);
        auto& pos = registry.getComponent<component::Position>(static_cast<size_t>(entity));
        auto& drawable = registry.getComponent<component::Drawable>(static_cast<size_t>(entity));

        uint32_t texture_width = 0, texture_height = 0;
        if (!renderer_->get_texture_size(drawable.texture_name, texture_width, texture_height)) {
            continue;
        }

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

        rtype::rendering::RenderData render_data;
        render_data.x = pos.x;
        render_data.y = pos.y;
        render_data.scale_x = drawable.scale_x;
        render_data.scale_y = drawable.scale_y;
        render_data.texture_name = drawable.texture_name;
        render_data.current_sprite = drawable.current_sprite;
        render_data.frame_count = drawable.frame_count;
        render_data.sprite_index = drawable.sprite_index;
        render_data.rect_x = drawable.rect_x;
        render_data.rect_y = drawable.rect_y;
        render_data.rect_width = drawable.rect_width;
        render_data.rect_height = drawable.rect_height;
        render_data.visible = true;

        if (accessibility_manager_) {
            uint16_t entity_type = rtype::net::EntityType::ENEMY;

            if (registry.hasComponent<component::NetworkId>(entity_id)) {
                entity_type = rtype::net::EntityType::PLAYER;
            } else if (drawable.texture_name.find("player") != std::string::npos ||
                       drawable.texture_name == "player_ships") {
                entity_type = rtype::net::EntityType::PLAYER;
            } else if (drawable.texture_name.find("enemy") != std::string::npos ||
                       drawable.texture_name.find("monster") != std::string::npos) {
                entity_type = rtype::net::EntityType::ENEMY;
            }

            sf::Color color = accessibility_manager_->get_entity_color(entity_type);
            render_data.color_r = color.r;
            render_data.color_g = color.g;
            render_data.color_b = color.b;
            render_data.color_a = color.a;
        } else {
            render_data.color_r = 255;
            render_data.color_g = 255;
            render_data.color_b = 255;
            render_data.color_a = 255;
        }

        // Apply hit flash effect (override to red/pink tint when recently damaged)
        if (registry.hasComponent<component::HitFlash>(entity_id)) {
            auto& flash = registry.getComponent<component::HitFlash>(entity_id);
            if (flash.active) {
                // Use red tint instead of white (white = original color in SFML)
                render_data.color_r = 255;
                render_data.color_g = 100;
                render_data.color_b = 100;
                render_data.color_a = 255;
                // Decrement timer
                flash.timer -= static_cast<float>(dt);
                if (flash.timer <= 0.0f) {
                    flash.active = false;
                    flash.timer = 0.0f;
                }
            }
        }

        renderer_->draw_sprite(render_data);
    }

    for (auto explosion_entity : explosions_to_destroy) {
        registry.destroyEntity(explosion_entity);
    }
}

void RenderSystem::set_renderer(std::shared_ptr<rtype::rendering::IRenderer> renderer) {
    renderer_ = renderer;
}

std::shared_ptr<rtype::rendering::IRenderer> RenderSystem::get_renderer() const {
    return renderer_;
}

} // namespace rtype::ecs
