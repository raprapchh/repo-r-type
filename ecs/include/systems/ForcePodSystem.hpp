#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

/// @brief System handling Force Pod companion behavior
class ForcePodSystem : public ISystem {
  public:
    ~ForcePodSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
