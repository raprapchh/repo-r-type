#include "systems/ProjectileSystem.hpp"
#include "components/Projectile.hpp"
#include <vector>

namespace rtype::ecs {

void ProjectileSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Projectile>();
    std::vector<GameEngine::entity_t> to_destroy;

    view.each([&](auto entity, component::Projectile& projectile) {
        projectile.lifetime -= dt;

        if (projectile.lifetime <= 0.0f) {
            to_destroy.push_back(static_cast<GameEngine::entity_t>(entity));
        }
    });

    for (auto entity : to_destroy) {
        registry.destroyEntity(entity);
    }
}

} // namespace rtype::ecs
