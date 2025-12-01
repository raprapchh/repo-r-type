#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class BoundarySystem : public ISystem {
  public:
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
