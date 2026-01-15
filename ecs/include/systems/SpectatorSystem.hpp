#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

/// @brief System to handle spectator target switching
class SpectatorSystem : public ISystem {
  public:
    SpectatorSystem(bool left_pressed, bool right_pressed, float cooldown = 0.3f);
    ~SpectatorSystem() override = default;
    void update(GameEngine::Registry& registry, double dt) override;

  private:
    bool left_pressed_;
    bool right_pressed_;
    float cooldown_duration_;
};

} // namespace rtype::ecs
