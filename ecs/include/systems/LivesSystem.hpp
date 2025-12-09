#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class LivesSystem : public ISystem {
  public:
    ~LivesSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;
};

} // namespace rtype::ecs
