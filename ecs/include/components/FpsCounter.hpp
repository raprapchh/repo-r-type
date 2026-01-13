#pragma once

namespace rtype::ecs::component {

/// @brief Logic data component for FPS calculation and smoothing
struct FpsCounter {
    int frameCount = 0;
    float timeAccumulator = 0.0f;
    float updateInterval = 0.5f;
};

} // namespace rtype::ecs::component
