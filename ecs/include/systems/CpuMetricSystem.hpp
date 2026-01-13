#pragma once

#include "Registry.hpp"

namespace rtype::ecs {

class CpuMetricSystem {
  public:
    void update(GameEngine::Registry& registry, double dt);
};

} // namespace rtype::ecs
