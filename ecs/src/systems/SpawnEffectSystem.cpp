#include "systems/SpawnEffectSystem.hpp"
#include "components/SpawnEffect.hpp"
#include "components/Drawable.hpp"
#include <vector>

namespace rtype::ecs {

void SpawnEffectSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::SpawnEffect, component::Drawable>();

    std::vector<GameEngine::entity_t> toRemove;

    view.each([&](auto entity, component::SpawnEffect& effect, component::Drawable& drawable) {
        effect.elapsed += static_cast<float>(dt);

        float progress = effect.elapsed / effect.duration;
        if (progress > 1.0f) {
            progress = 1.0f;
        }

        float currentScale = effect.startScale + (effect.endScale - effect.startScale) * progress;
        drawable.scale_x = currentScale;
        drawable.scale_y = currentScale;

        if (effect.elapsed >= effect.duration) {
            toRemove.push_back(static_cast<GameEngine::entity_t>(entity));
        }
    });

    for (auto entity : toRemove) {
        registry.removeComponent<component::SpawnEffect>(entity);
    }
}

} // namespace rtype::ecs
