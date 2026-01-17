#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

/// @brief System to calculate FPS and update the FpsCounter text display
class FpsSystem : public ISystem {
  public:
    FpsSystem() = default;
    ~FpsSystem() override = default;

    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
