#include "../include/AccessibilityManager.hpp"
#include "../../shared/net/MessageData.hpp"

namespace rtype::client {

AccessibilityManager::AccessibilityManager() : current_mode_(ColorBlindMode::None) {
}

void AccessibilityManager::cycle_mode() {
    switch (current_mode_) {
    case ColorBlindMode::None:
        current_mode_ = ColorBlindMode::Deuteranopia;
        break;
    case ColorBlindMode::Deuteranopia:
        current_mode_ = ColorBlindMode::Protanopia;
        break;
    case ColorBlindMode::Protanopia:
        current_mode_ = ColorBlindMode::Tritanopia;
        break;
    case ColorBlindMode::Tritanopia:
        current_mode_ = ColorBlindMode::None;
        break;
    }
}

ColorBlindMode AccessibilityManager::get_current_mode() const {
    return current_mode_;
}

std::string AccessibilityManager::get_mode_name() const {
    switch (current_mode_) {
    case ColorBlindMode::None:
        return "None";
    case ColorBlindMode::Deuteranopia:
        return "Deuteranopia";
    case ColorBlindMode::Protanopia:
        return "Protanopia";
    case ColorBlindMode::Tritanopia:
        return "Tritanopia";
    default:
        return "Unknown";
    }
}

sf::Color AccessibilityManager::get_entity_color(uint16_t type) const {
    switch (current_mode_) {
    case ColorBlindMode::None:
        if (type == rtype::net::EntityType::PLAYER) {
            return sf::Color(255, 0, 0);
        } else if (type == rtype::net::EntityType::ENEMY) {
            return sf::Color(0, 255, 0);
        }
        break;

    case ColorBlindMode::Deuteranopia:
        if (type == rtype::net::EntityType::PLAYER) {
            return sf::Color(0, 0, 255);
        } else if (type == rtype::net::EntityType::ENEMY) {
            return sf::Color(255, 165, 0);
        }
        break;

    case ColorBlindMode::Protanopia:
        if (type == rtype::net::EntityType::PLAYER) {
            return sf::Color(0, 100, 255);
        } else if (type == rtype::net::EntityType::ENEMY) {
            return sf::Color(255, 255, 0);
        }
        break;

    case ColorBlindMode::Tritanopia:
        if (type == rtype::net::EntityType::PLAYER) {
            return sf::Color(255, 0, 0);
        } else if (type == rtype::net::EntityType::ENEMY) {
            return sf::Color(0, 200, 200);
        }
        break;
    }

    return sf::Color::White;
}

} // namespace rtype::client
