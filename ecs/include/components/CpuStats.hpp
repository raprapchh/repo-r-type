#pragma once

namespace rtype::ecs::component {

struct CpuStats {
    float frameTimeMs;

    CpuStats() : frameTimeMs(0.0f) {
    }
};

} // namespace rtype::ecs::component
