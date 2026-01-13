#pragma once

#include "../../../ecs/include/Registry.hpp"

namespace rtype::ecs {

class PingSystem {
  public:
    void update(GameEngine::Registry& registry, double dt);
};

} // namespace rtype::ecs
