#include "../../include/systems/ProjectileSystem.hpp"
#include "../../include/components/Projectile.hpp"

namespace rtype::ecs {

void ProjectileSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Projectile>();

    view.each([&registry, dt](auto entity, component::Projectile& projectile) {
        projectile.lifetime -= dt;

        if (projectile.lifetime <= 0.0f) {
            registry.destroyEntity(static_cast<GameEngine::entity_t>(entity));
        }
    });
}

} // namespace rtype::ecs
