#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>

namespace rtype::client {

enum class ColorBlindMode { None, Deuteranopia, Protanopia, Tritanopia };

class AccessibilityManager {
  public:
    AccessibilityManager();
    ~AccessibilityManager() = default;

    void cycle_mode();
    ColorBlindMode get_current_mode() const;
    std::string get_mode_name() const;
    sf::Color get_entity_color(uint16_t type) const;

  private:
    ColorBlindMode current_mode_;
};

} // namespace rtype::client
