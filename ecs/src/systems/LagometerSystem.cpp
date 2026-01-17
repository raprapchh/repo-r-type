#include "systems/LagometerSystem.hpp"
#include "components/LagometerComponent.hpp"
#include "components/PingStats.hpp"
#include "components/Position.hpp"
#include "components/UITag.hpp"
#include <iomanip>
#include <sstream>

namespace rtype::ecs {

void LagometerSystem::update(GameEngine::Registry& registry, double dt, sf::RenderWindow& window) {
    auto lagometer_view = registry.view<component::LagometerComponent, component::Position, component::UITag>();
    auto ping_view = registry.view<component::PingStats>();

    for (auto lag_entity : lagometer_view) {
        auto& lagometer = registry.getComponent<component::LagometerComponent>(lag_entity);

        if (!lagometer.visible) {
            continue;
        }

        for (auto ping_entity : ping_view) {
            auto& ping_stats = registry.getComponent<component::PingStats>(ping_entity);

            lagometer.update_timer += static_cast<float>(dt);
            if (lagometer.update_timer >= 0.1f) {
                lagometer.add_ping_sample(ping_stats.lastPingMs);
                lagometer.update_timer = 0.0f;
            }
        }

        render_lagometer(registry, window);
    }
}

void LagometerSystem::render_lagometer(GameEngine::Registry& registry, sf::RenderWindow& window) {
    auto view = registry.view<component::LagometerComponent, component::Position>();

    for (auto entity : view) {
        auto& lagometer = registry.getComponent<component::LagometerComponent>(entity);
        auto& pos = registry.getComponent<component::Position>(entity);

        if (!lagometer.visible) {
            continue;
        }

        lagometer.background.setPosition(pos.x, pos.y);
        window.draw(lagometer.background);

        float max_height = 80.0f;
        float bar_width = 3.0f;
        float max_ping_for_scale = 200.0f;

        for (size_t i = 0; i < lagometer.HISTORY_SIZE; ++i) {
            size_t idx = (lagometer.current_index + 1 + i) % lagometer.HISTORY_SIZE;
            float ping = lagometer.ping_history[idx];

            if (ping > 0.0f) {
                float normalized_ping = std::min(ping / max_ping_for_scale, 1.0f);
                float bar_height = normalized_ping * max_height;

                sf::Vector2f bar_pos(pos.x + 10.0f + i * bar_width, pos.y + 90.0f - bar_height);

                lagometer.bars[i].setPosition(bar_pos);
                lagometer.bars[i].setSize(sf::Vector2f(bar_width - 0.5f, bar_height));
                lagometer.bars[i].setFillColor(lagometer.get_color_for_ping(ping));

                window.draw(lagometer.bars[i]);
            }
        }

        sf::Font font;
        if (font.loadFromFile("client/fonts/Ethnocentric-Regular.otf")) {
            float avg_ping = lagometer.get_average_ping();
            float max_ping = lagometer.get_max_ping();

            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << "Avg: " << avg_ping << " ms";

            sf::Text avg_text(ss.str(), font, 14);
            avg_text.setPosition(pos.x + 10.0f, pos.y + 5.0f);
            avg_text.setFillColor(sf::Color::White);
            window.draw(avg_text);

            ss.str("");
            ss << std::fixed << std::setprecision(1) << "Max: " << max_ping << " ms";

            sf::Text max_text(ss.str(), font, 14);
            max_text.setPosition(pos.x + 10.0f, pos.y + 25.0f);
            max_text.setFillColor(sf::Color::White);
            window.draw(max_text);
        }
    }
}

} // namespace rtype::ecs
