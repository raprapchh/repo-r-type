#include "../../include/systems/BoundarySystem.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/HitBox.hpp"
#include "../../include/components/MapBounds.hpp"
#include "../../../shared/GameConstants.hpp"
#include "../../include/Registry.hpp"

namespace rtype::ecs {

void BoundarySystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    float minX = 0.0f;
    float minY = 0.0f;
    float maxX = rtype::constants::SCREEN_WIDTH;
    float maxY = rtype::constants::SCREEN_HEIGHT;

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
    }

    auto view = registry.view<component::Position>();

    view.each([&registry, minX, minY, maxX, maxY](const auto entity, component::Position& pos) {
        float width = 0.0f;
        float height = 0.0f;

        if (registry.hasComponent<component::HitBox>(static_cast<std::size_t>(entity))) {
            auto& hitbox = registry.getComponent<component::HitBox>(static_cast<std::size_t>(entity));
            width = hitbox.width;
            height = hitbox.height;
        }

        if (pos.x < minX)
            pos.x = minX;
        if (pos.x + width > maxX)
            pos.x = maxX - width;

        if (pos.y < minY)
            pos.y = minY;
        if (pos.y + height > maxY)
            pos.y = maxY - height;
    });
}

} // namespace rtype::ecs
