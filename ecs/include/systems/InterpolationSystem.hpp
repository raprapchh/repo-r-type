#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class InterpolationSystem : public ISystem {
  public:
    ~InterpolationSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
