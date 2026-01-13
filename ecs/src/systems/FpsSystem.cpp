#include "systems/FpsSystem.hpp"
#include "components/FpsCounter.hpp"
#include "components/TextDrawable.hpp"
#include "components/UITag.hpp"
#include <sstream>
#include <iomanip>

namespace rtype::ecs {

void FpsSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::FpsCounter, component::TextDrawable, component::UITag>();

    for (auto entity : view) {
        auto& fps_counter = registry.getComponent<component::FpsCounter>(static_cast<GameEngine::entity_t>(entity));
        auto& text_drawable = registry.getComponent<component::TextDrawable>(static_cast<GameEngine::entity_t>(entity));

        fps_counter.frameCount++;
        fps_counter.timeAccumulator += static_cast<float>(dt);

        if (fps_counter.timeAccumulator >= fps_counter.updateInterval) {
            float fps = static_cast<float>(fps_counter.frameCount) / fps_counter.timeAccumulator;

            std::ostringstream ss;
            ss << "FPS: " << std::fixed << std::setprecision(0) << fps;
            text_drawable.text.setString(ss.str());

            fps_counter.frameCount = 0;
            fps_counter.timeAccumulator = 0.0f;
        }
    }
}

} // namespace rtype::ecs
