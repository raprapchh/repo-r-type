#pragma once

namespace rtype::ecs::component {

/**
 * @brief Component for visual hit feedback (flash effect when damaged)
 */
struct HitFlash {
    float duration = 0.1f; // Total flash duration in seconds
    float timer = 0.0f;    // Current countdown timer
    bool active = false;   // Whether flash is currently active
};

} // namespace rtype::ecs::component
