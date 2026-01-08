#pragma once

namespace rtype::ecs::component {

/// @brief Temporary component for spawn pop-in animation
struct SpawnEffect {
    float elapsed = 0.0f;
    float duration = 0.5f;
    float startScale = 0.1f;
    float endScale = 1.0f;

    SpawnEffect() = default;
    SpawnEffect(float dur, float start = 0.1f, float end = 1.0f) : duration(dur), startScale(start), endScale(end) {
    }
};

} // namespace rtype::ecs::component
