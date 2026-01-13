#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class TextureAnimationSystem : public ISystem {
  public:
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
