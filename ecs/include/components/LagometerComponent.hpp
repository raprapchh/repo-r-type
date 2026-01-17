#pragma once
#include <array>
#include <SFML/Graphics.hpp>

namespace rtype::ecs::component {

struct LagometerComponent {
    static constexpr size_t HISTORY_SIZE = 60;

    std::array<float, HISTORY_SIZE> ping_history;
    size_t current_index;
    bool visible;
    float update_timer;

    sf::RectangleShape background;
    std::array<sf::RectangleShape, HISTORY_SIZE> bars;

    LagometerComponent() : ping_history{}, current_index(0), visible(false), update_timer(0.0f), background(), bars() {
        ping_history.fill(0.0f);

        background.setSize(sf::Vector2f(200.0f, 120.0f));
        background.setFillColor(sf::Color(0, 0, 0, 180));
        background.setOutlineColor(sf::Color::White);
        background.setOutlineThickness(2.0f);

        for (auto& bar : bars) {
            bar.setSize(sf::Vector2f(3.0f, 1.0f));
        }
    }

    void add_ping_sample(float ping_ms) {
        current_index = (current_index + 1) % HISTORY_SIZE;
        ping_history[current_index] = ping_ms;
    }

    float get_average_ping() const {
        float sum = 0.0f;
        int count = 0;
        for (float ping : ping_history) {
            if (ping > 0.0f) {
                sum += ping;
                count++;
            }
        }
        return count > 0 ? sum / count : 0.0f;
    }

    float get_max_ping() const {
        float max_ping = 0.0f;
        for (float ping : ping_history) {
            if (ping > max_ping) {
                max_ping = ping;
            }
        }
        return max_ping;
    }

    sf::Color get_color_for_ping(float ping_ms) const {
        if (ping_ms < 50.0f) {
            return sf::Color::Green;
        } else if (ping_ms < 100.0f) {
            return sf::Color::Yellow;
        } else if (ping_ms < 200.0f) {
            return sf::Color(255, 165, 0);
        } else {
            return sf::Color::Red;
        }
    }
};

} // namespace rtype::ecs::component
