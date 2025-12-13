#include "../include/AccessibilityManager.hpp"

namespace rtype::client {

AccessibilityManager::AccessibilityManager() : is_color_blind_mode_(false) {
}

void AccessibilityManager::toggle_color_blind_mode() {
    is_color_blind_mode_ = !is_color_blind_mode_;
}

bool AccessibilityManager::is_color_blind_mode_active() const {
    return is_color_blind_mode_;
}

sf::Color AccessibilityManager::get_entity_color(uint16_t type) const {
    if (!is_color_blind_mode_) {
        return sf::Color::White;
    } else {
        if (type == rtype::net::EntityType::PLAYER) {
            return sf::Color::Blue;
        } else if (type == rtype::net::EntityType::ENEMY) {
            return sf::Color(255, 165, 0);
        }
    }
    return sf::Color::White;
}

} // namespace rtype::client
