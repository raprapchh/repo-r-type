#pragma once

#include "../../include/Registry.hpp"
#include "../../shared/interfaces/ecs/ISystem.hpp"

namespace rtype::ecs {

class MobSystem : public ISystem {
  public:
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
