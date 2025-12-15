#include "../../include/systems/AnimationSystem.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/Velocity.hpp"
#include <iostream>

namespace rtype::ecs {

void AnimationSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Drawable, component::Velocity>();

    for (auto entity : view) {
        auto& drawable = registry.getComponent<component::Drawable>(static_cast<std::size_t>(entity));
        auto& vel = registry.getComponent<component::Velocity>(static_cast<std::size_t>(entity));

        if (drawable.animation_sequences.empty())
            continue;

        if (vel.vy < 0)
            drawable.current_state = "up";
        else if (vel.vy > 0)
            drawable.current_state = "down";
        else
            drawable.current_state = "idle";

        if (drawable.current_state != drawable.last_state) {
            drawable.animation_frame = 0;
            const auto& seq = drawable.animation_sequences[drawable.current_state];
            if (!seq.empty())
                drawable.current_sprite = seq[0];
            drawable.animation_timer = 0.0f;
            drawable.last_state = drawable.current_state;
            continue;
        }

        const auto& seq = drawable.animation_sequences[drawable.current_state];
        if (seq.empty())
            continue;

        drawable.animation_timer += static_cast<float>(dt);
        if (drawable.animation_timer >= drawable.animation_speed) {
            drawable.animation_timer = 0.0f;
            if (drawable.loop) {
                drawable.animation_frame = (drawable.animation_frame + 1) % seq.size();
            } else {
                if (drawable.animation_frame + 1 < seq.size())
                    drawable.animation_frame++;
            }
            drawable.current_sprite = seq[drawable.animation_frame];

            if (drawable.current_sprite >= 5) {
                drawable.current_sprite = 2;
            }
        }
    }
}

} // namespace rtype::ecs
