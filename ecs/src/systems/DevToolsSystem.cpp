#include "systems/DevToolsSystem.hpp"
#include "components/TextDrawable.hpp"
#include "components/UITag.hpp"

namespace rtype::ecs {

void DevToolsSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    if (!toggle_pressed_) {
        return;
    }

    auto view = registry.view<component::UITag, component::TextDrawable>();

    for (auto entity : view) {
        auto& text_drawable = registry.getComponent<component::TextDrawable>(static_cast<GameEngine::entity_t>(entity));
        text_drawable.hidden = !text_drawable.hidden;
    }

    toggle_pressed_ = false;
}

} // namespace rtype::ecs
