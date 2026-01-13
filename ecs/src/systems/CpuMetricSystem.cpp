#include "systems/CpuMetricSystem.hpp"
#include "components/CpuStats.hpp"
#include "components/TextDrawable.hpp"
#include "components/UITag.hpp"
#include <iomanip>
#include <sstream>

namespace rtype::ecs {

void CpuMetricSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::CpuStats, component::TextDrawable, component::UITag>();

    for (auto entity : view) {
        auto& cpuStats = registry.getComponent<component::CpuStats>(entity);
        auto& textDrawable = registry.getComponent<component::TextDrawable>(entity);

        cpuStats.frameTimeMs = static_cast<float>(dt * 1000.0);

        std::stringstream ss;
        ss << "CPU: " << std::fixed << std::setprecision(2) << cpuStats.frameTimeMs << " ms";
        textDrawable.text.setString(ss.str());
    }
}

} // namespace rtype::ecs
