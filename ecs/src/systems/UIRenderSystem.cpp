#include "../../include/systems/UIRenderSystem.hpp"
#include "../../include/components/TextDrawable.hpp"
#include "../../include/components/UITag.hpp"
#include "../../include/components/Position.hpp"

namespace rtype::ecs {

UIRenderSystem::UIRenderSystem(sf::RenderWindow* window) : window_(window) {
}

void UIRenderSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    if (!window_) {
        return;
    }

    // Save current view
    sf::View currentView = window_->getView();

    // Set default view for UI rendering (screen-space)
    window_->setView(window_->getDefaultView());

    auto view = registry.view<component::UITag, component::TextDrawable>();

    for (auto entity : view) {
        auto& text_drawable = registry.getComponent<component::TextDrawable>(static_cast<GameEngine::entity_t>(entity));

        if (!text_drawable.hidden) {
            // Update position if entity has Position component
            if (registry.hasComponent<component::Position>(static_cast<GameEngine::entity_t>(entity))) {
                auto& pos = registry.getComponent<component::Position>(static_cast<GameEngine::entity_t>(entity));
                text_drawable.text.setPosition(pos.x, pos.y);
            }

            window_->draw(text_drawable.text);
        }
    }

    // Restore original view
    window_->setView(currentView);
}

} // namespace rtype::ecs
