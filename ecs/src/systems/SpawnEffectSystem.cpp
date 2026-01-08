#include "../../include/systems/SpawnEffectSystem.hpp"
#include "../../include/components/SpawnEffect.hpp"
#include "../../include/components/Drawable.hpp"
#include <vector>

namespace rtype::ecs {

void SpawnEffectSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::SpawnEffect, component::Drawable>();

    std::vector<GameEngine::entity_t> toRemove;

    view.each([&](auto entity, component::SpawnEffect& effect, component::Drawable& drawable) {
        effect.elapsed += static_cast<float>(dt);

        // Calculate interpolation progress (0.0 to 1.0)
        float progress = effect.elapsed / effect.duration;
        if (progress > 1.0f) {
            progress = 1.0f;
        }

        // Lerp scale from startScale to endScale
        float currentScale = effect.startScale + (effect.endScale - effect.startScale) * progress;
        drawable.scale_x = currentScale;
        drawable.scale_y = currentScale;

        // Mark for removal when animation complete
        if (effect.elapsed >= effect.duration) {
            toRemove.push_back(static_cast<GameEngine::entity_t>(entity));
        }
    });

    // Remove SpawnEffect component from completed entities
    for (auto entity : toRemove) {
        registry.removeComponent<component::SpawnEffect>(entity);
    }
}

} // namespace rtype::ecs
