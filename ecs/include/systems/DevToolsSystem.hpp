#pragma once

#include "interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"

namespace rtype::ecs {

/// @brief System to handle F3 toggle for dev tools visibility
class DevToolsSystem : public ISystem {
  public:
    DevToolsSystem() = default;
    ~DevToolsSystem() override = default;

    void update(GameEngine::Registry& registry, double dt) override;

    /// @brief Set whether F3 was pressed this frame
    void setTogglePressed(bool pressed) {
        toggle_pressed_ = pressed;
    }

  private:
    bool toggle_pressed_ = false;
};

} // namespace rtype::ecs
