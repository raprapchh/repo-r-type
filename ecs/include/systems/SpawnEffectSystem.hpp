#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

/// @brief System handling spawn pop-in animation effect
class SpawnEffectSystem : public ISystem {
  public:
    ~SpawnEffectSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
