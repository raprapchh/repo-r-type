#pragma once

namespace rtype::ecs::component {

struct PingStats {
    float lastPingMs;
    float timer;

    bool pingRequested;

    PingStats() : lastPingMs(0.0f), timer(0.0f), pingRequested(false) {
    }
};

} // namespace rtype::ecs::component
