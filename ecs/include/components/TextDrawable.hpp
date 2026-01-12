#pragma once

#include <SFML/Graphics.hpp>
#include <memory>

namespace rtype::ecs::component {

/// @brief Text rendering component with visibility control for UI elements
struct TextDrawable {
    sf::Text text;
    std::shared_ptr<sf::Font> font;
    bool hidden = false; // Toggle visibility via F3

    TextDrawable() : font(nullptr), hidden(false) {
    }

    TextDrawable(std::shared_ptr<sf::Font> f, const std::string& str, unsigned int size, sf::Color color)
        : font(f), hidden(false) {
        if (font) {
            text.setFont(*font);
        }
        text.setString(str);
        text.setCharacterSize(size);
        text.setFillColor(color);
    }
};

} // namespace rtype::ecs::component
