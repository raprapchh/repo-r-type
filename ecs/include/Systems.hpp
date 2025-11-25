#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"

namespace rtype::ecs {

class MovementSystem : public ISystem {
  public:
    ~MovementSystem() override = default;
    void update(GameEngine::Registry& registry) override;
};

} // namespace rtype::ecs
