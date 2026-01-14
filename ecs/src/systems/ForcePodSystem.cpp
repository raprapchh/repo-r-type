#include "systems/ForcePodSystem.hpp"
#include "components/Parent.hpp"
#include "components/Position.hpp"
#include "components/Weapon.hpp"
#include "components/Tag.hpp"

namespace rtype::ecs {

void ForcePodSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    auto podView = registry.view<component::Parent, component::Position, component::Weapon, component::Tag>();

    podView.each([&registry](auto entity, component::Parent& parent, component::Position& pos,
                             component::Weapon& weapon, component::Tag& tag) {
        (void)entity;

        // Only process ForcePod entities
        if (tag.name != "ForcePod") {
            return;
        }

        // 1. Follow owner position
        auto ownerId = static_cast<GameEngine::entity_t>(parent.ownerId);
        if (registry.isValid(ownerId) && registry.hasComponent<component::Position>(ownerId)) {
            auto& ownerPos = registry.getComponent<component::Position>(ownerId);
            pos.x = ownerPos.x + parent.offsetX;
            pos.y = ownerPos.y + parent.offsetY;
        }

        // 2. Sync shooting with owner
        if (registry.isValid(ownerId) && registry.hasComponent<component::Weapon>(ownerId)) {
            auto& ownerWeapon = registry.getComponent<component::Weapon>(ownerId);
            weapon.isShooting = ownerWeapon.isShooting;
        }
    });
}

} // namespace rtype::ecs
