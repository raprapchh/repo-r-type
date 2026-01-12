#include "../../include/systems/PingSystem.hpp"
#include "../../include/components/PingStats.hpp"
#include "../../../ecs/include/components/TextDrawable.hpp"
#include "../../../ecs/include/components/UITag.hpp"
#include <iomanip>
#include <sstream>
#include <chrono>

namespace rtype::ecs {

// Update method now depends only on Registry and time
void PingSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::PingStats, component::TextDrawable, component::UITag>();

    for (auto entity : view) {
        auto& pingStats = registry.getComponent<component::PingStats>(entity);
        auto& textDrawable = registry.getComponent<component::TextDrawable>(entity);

        pingStats.timer += static_cast<float>(dt);

        // Request PING every 1.0 second
        if (pingStats.timer >= 1.0f) {
            pingStats.timer = 0.0f;
            pingStats.pingRequested = true;
        }

        // Update Text
        std::stringstream ss;
        ss << "PING: " << std::fixed << std::setprecision(2) << pingStats.lastPingMs << " ms";
        textDrawable.text.setString(ss.str());
    }
}

} // namespace rtype::ecs
