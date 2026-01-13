#include "../../include/systems/MovementSystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/MovementPattern.hpp"
#include <cmath>

namespace rtype::ecs {

void MovementSystem::update(GameEngine::Registry& registry, double dt) {
    auto patternView = registry.view<component::MovementPattern, component::Velocity>();
    patternView.each([dt]([[maybe_unused]] auto entity, component::MovementPattern& pattern, component::Velocity& vel) {
        pattern.timer += static_cast<float>(dt);

        if (pattern.type == component::MovementPatternType::Circular) {
            float angle = pattern.timer * pattern.frequency;
            vel.vx = -std::sin(angle) * pattern.amplitude * pattern.frequency - 400.0f;
            vel.vy = std::cos(angle) * pattern.amplitude * pattern.frequency;
        } else if (pattern.type == component::MovementPatternType::Sinusoidal) {
            float angle = pattern.timer * pattern.frequency;
            vel.vy = std::cos(angle) * pattern.amplitude;
        } else if (pattern.type == component::MovementPatternType::RandomVertical) {
            if (pattern.timer >= pattern.frequency) {
                pattern.timer = 0;
                int dir = rand() % 3 - 1;
                vel.vy = dir * pattern.amplitude;
            }
        }
    });

    auto view = registry.view<component::Position, component::Velocity>();

    view.each([&registry, dt](auto entity, component::Position& pos, component::Velocity& vel) {
        pos.x += vel.vx * static_cast<float>(dt);
        pos.y += vel.vy * static_cast<float>(dt);

        if (registry.hasComponent<component::MovementPattern>(entity)) {
            auto& pattern = registry.getComponent<component::MovementPattern>(entity);
            if (pattern.type == component::MovementPatternType::RandomVertical) {
                if (pos.y < 50.0f && vel.vy < 0) {
                    vel.vy = -vel.vy;
                    pos.y = 50.0f;
                } else if (pos.y > 750.0f && vel.vy > 0) {
                    vel.vy = -vel.vy;
                    pos.y = 750.0f;
                }
            }
        }
    });
}

} // namespace rtype::ecs
