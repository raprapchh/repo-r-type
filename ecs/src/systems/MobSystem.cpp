#include "../../include/systems/MobSystem.hpp"
#include "../../include/components/ScreenMode.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Tag.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <vector>

namespace rtype::ecs {

void MobSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::ScreenMode, component::Position, component::Velocity, component::Tag>();

    std::vector<GameEngine::entity_t> mobs_to_switch;

    view.each([&](auto entity, component::ScreenMode&, component::Position& pos, component::Velocity& vel,
                  component::Tag& tag) {
        bool switch_mode = false;
        if (tag.name == "Monster_0_Top") {
            if (pos.y > rtype::config::MAP_MAX_Y) {
                switch_mode = true;
            }
        } else if (tag.name == "Monster_0_Bot") {
            if (pos.y < rtype::config::MAP_MIN_Y) {
                switch_mode = true;
            }
        }

        if (switch_mode) {
            mobs_to_switch.push_back(static_cast<GameEngine::entity_t>(entity));
            vel.vx -= rtype::config::SCROLL_SPEED;
        }
    });

    for (auto entity : mobs_to_switch) {
        registry.removeComponent<component::ScreenMode>(entity);
    }

    auto proj_view = registry.view<component::ScreenMode, component::Projectile, component::Velocity>();
    std::vector<GameEngine::entity_t> projs_to_switch;

    proj_view.each([&](auto entity, component::ScreenMode&, component::Projectile& proj, component::Velocity& vel) {
        bool owner_active = false;
        if (registry.isValid(static_cast<GameEngine::entity_t>(proj.owner_id))) {
            if (registry.hasComponent<component::ScreenMode>(static_cast<std::size_t>(proj.owner_id))) {
                owner_active = true;
            }
        }

        if (!owner_active) {
            projs_to_switch.push_back(static_cast<GameEngine::entity_t>(entity));
            vel.vx -= rtype::config::SCROLL_SPEED;
        }
    });

    for (auto entity : projs_to_switch) {
        registry.removeComponent<component::ScreenMode>(entity);
    }
}

} // namespace rtype::ecs
