#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

class InputSystem : public ISystem {
  public:
    InputSystem(bool up, bool down, bool left, bool right, float speed);
    ~InputSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    bool up_pressed_;
    bool down_pressed_;
    bool left_pressed_;
    bool right_pressed_;
    float speed_;
};

} // namespace rtype::ecs

