#pragma once

#include "Registry.hpp"

namespace rtype::ecs {

class SpectatorSystem {
  public:
    void update(GameEngine::Registry& registry, double delta_time);
};

} // namespace rtype::ecs
