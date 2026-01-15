#include "systems/UIRenderSystem.hpp"
#include "components/TextDrawable.hpp"
#include "components/UITag.hpp"
#include "components/Position.hpp"
#include "components/SpectatorComponent.hpp"
#include "components/Controllable.hpp"

namespace rtype::ecs {

UIRenderSystem::UIRenderSystem(sf::RenderWindow* window, sf::Font* font) : window_(window), font_(font) {
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

    // Display "SPECTATOR MODE" if local player is spectating
    auto spectator_check_view = registry.view<component::Controllable, component::SpectatorComponent>();
    for (auto entity : spectator_check_view) {
        auto& ctrl = registry.getComponent<component::Controllable>(static_cast<GameEngine::entity_t>(entity));

        if (ctrl.is_local_player) {
            // Found local player with spectator component - display text
            sf::Text spectator_text;
            spectator_text.setString("SPECTATOR MODE");
            spectator_text.setCharacterSize(30);
            spectator_text.setFillColor(sf::Color::Yellow);
            spectator_text.setStyle(sf::Text::Bold);

            // Center text at top of screen
            sf::FloatRect textBounds = spectator_text.getLocalBounds();
            float x = (window_->getSize().x - textBounds.width) / 2.0f;
            float y = 20.0f;
            spectator_text.setPosition(x, y);

            window_->draw(spectator_text);
            break;
        }
    }

    // Restore original view
    window_->setView(currentView);
}

} // namespace rtype::ecs
