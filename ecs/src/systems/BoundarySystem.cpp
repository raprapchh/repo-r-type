#include "../../include/systems/BoundarySystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Weapon.hpp"
#include "../../include/components/Projectile.hpp"
#include "../../include/components/MapBounds.hpp"
#include "../../include/Registry.hpp"

namespace rtype::ecs {

void BoundarySystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = 1920.0f;
    float maxY = 1060.0f;

    try {
        auto mapBoundsView = registry.view<component::MapBounds>();
        for (auto entity : mapBoundsView) {
            auto& bounds = registry.getComponent<component::MapBounds>(static_cast<std::size_t>(entity));
            minX = bounds.minX;
            minY = bounds.minY;
            maxX = bounds.maxX;
            maxY = bounds.maxY;
            break;
        }
    } catch (const std::exception&) {
        // std::cerr << "BoundarySystem: MapBounds not found, using defaults.\n";
    }

    auto view = registry.view<component::Position>();

    std::vector<GameEngine::entity_t> entities_to_destroy;

    view.each([&](const auto entity, component::Position& pos) {
        float width = 0.0f;
        float height = 0.0f;

        if (registry.hasComponent<component::HitBox>(static_cast<std::size_t>(entity))) {
            auto& hitbox = registry.getComponent<component::HitBox>(static_cast<std::size_t>(entity));
            width = hitbox.width;
            height = hitbox.height;
        }

        // Check if entity is an enemy (has Health, no Weapon)
        bool is_enemy = registry.hasComponent<component::Health>(static_cast<std::size_t>(entity)) &&
                        !registry.hasComponent<component::Weapon>(static_cast<std::size_t>(entity));

        // Destroy enemies when they go off-screen to the left
        if (is_enemy && pos.x + width < -100.0f) {
            entities_to_destroy.push_back(static_cast<GameEngine::entity_t>(entity));
            return;
        }

        // Don't clamp enemies - let them move freely until destroyed
        if (is_enemy) {
            return;
        }

        if (registry.hasComponent<component::Projectile>(static_cast<std::size_t>(entity))) {
            return;
        }

        // For players and other entities, clamp to boundaries
        if (pos.x < 0)
            pos.x = 0;
        if (pos.x + width > 1920)
            pos.x = 1920 - width;
        if (pos.x < minX)
            pos.x = minX;
        if (pos.x + width > maxX)
            pos.x = maxX - width;

        if (pos.y < minY) {
            pos.y = minY;
        }
        if (pos.y + height > maxY) {
            pos.y = maxY - height;
        }
    });

    // Destroy all marked entities
    for (auto entity : entities_to_destroy) {
        registry.destroyEntity(entity);
    }
}

} // namespace rtype::ecs
