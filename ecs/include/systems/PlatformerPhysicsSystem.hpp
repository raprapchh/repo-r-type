#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class PlatformerPhysicsSystem : public ISystem {
  public:
    ~PlatformerPhysicsSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
