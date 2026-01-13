#include "../../include/systems/PlatformerPhysicsSystem.hpp"
#include <iostream>

#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Gravity.hpp"
#include "../../include/components/Jump.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/CollisionLayer.hpp"

namespace rtype::ecs {

void PlatformerPhysicsSystem::update(GameEngine::Registry& registry, double dt) {
    auto view =
        registry.view<rtype::ecs::component::Position, rtype::ecs::component::Velocity, rtype::ecs::component::Gravity,
                      rtype::ecs::component::Jump, rtype::ecs::component::HitBox>();
    auto platforms =
        registry
            .view<rtype::ecs::component::Position, rtype::ecs::component::HitBox, rtype::ecs::component::Collidable>();

    for (auto entity : view) {
        auto& pos = registry.getComponent<rtype::ecs::component::Position>(entity);
        auto& vel = registry.getComponent<rtype::ecs::component::Velocity>(entity);
        auto& gravity = registry.getComponent<rtype::ecs::component::Gravity>(entity);
        auto& jump = registry.getComponent<rtype::ecs::component::Jump>(entity);
        auto& hitbox = registry.getComponent<rtype::ecs::component::HitBox>(entity);

        vel.vy += gravity.force * static_cast<float>(dt);

        float next_y = pos.y + vel.vy * static_cast<float>(dt);

        bool on_ground = false;

        for (auto plat : platforms) {
            if (entity == plat)
                continue;

            auto& plat_pos = registry.getComponent<rtype::ecs::component::Position>(plat);
            auto& plat_box = registry.getComponent<rtype::ecs::component::HitBox>(plat);

            bool match_x = (pos.x + hitbox.width > plat_pos.x && pos.x < plat_pos.x + plat_box.width);

            // Debug logging
            if (entity != plat) {
                // std::cout << "Player Y: " << pos.y << " Next Y: " << next_y << " Plat Y: " << plat_pos.y << " VY: "
                // << vel.vy << " Match X: " << match_x << std::endl;
            }

            if (match_x) {
                if (vel.vy > 0 && pos.y + hitbox.height <= plat_pos.y && next_y + hitbox.height >= plat_pos.y) {
                    std::cout << "Collision! Resetting position. Old Y: " << pos.y
                              << " New Y: " << plat_pos.y - hitbox.height << std::endl;
                    pos.y = plat_pos.y - hitbox.height;
                    vel.vy = jump.strength;
                    on_ground = false;
                } else if (match_x && vel.vy > 0) {
                    // std::cout << "Missed collision check details: "
                    //           << "Bottom: " << (pos.y + hitbox.height)
                    //           << " <= PlatY: " << plat_pos.y
                    //           << " && NextBottom: " << (next_y + hitbox.height)
                    //           << " >= PlatY: " << plat_pos.y
                    //           << std::endl;
                }
            }
        }

        jump.can_jump = on_ground;
    }
}

} // namespace rtype::ecs
