#pragma once

#include "../../shared/net/MessageData.hpp"
#include <SFML/Graphics/Color.hpp>
#include <cstdint>

namespace rtype::client {

class AccessibilityManager {
  public:
    AccessibilityManager();
    ~AccessibilityManager() = default;

    void toggle_color_blind_mode();

    bool is_color_blind_mode_active() const;

    sf::Color get_entity_color(uint16_t type) const;

  private:
    bool is_color_blind_mode_;
};

} // namespace rtype::client
