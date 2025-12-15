#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class AnimationSystem : public ISystem {
  public:
    ~AnimationSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
