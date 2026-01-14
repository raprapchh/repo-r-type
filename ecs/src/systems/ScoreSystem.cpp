#include "systems/ScoreSystem.hpp"
#include "components/Score.hpp"
#include "utils/Logger.hpp"

namespace rtype::ecs {

void ScoreSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Score, component::ScoreEvent>();
    std::vector<GameEngine::entity_t> to_remove_event;

    for (auto entity : view) {
        auto& score = view.get<component::Score>(entity);
        const auto& event = view.get<component::ScoreEvent>(entity);

        std::cout << "[ScoreSystem] Adding " << event.points << " points to entity " << static_cast<std::size_t>(entity)
                  << std::endl;

        score.value += event.points;

        Logger::instance().info("Score updated for entity " + std::to_string(static_cast<std::size_t>(entity)) + ": " +
                                std::to_string(score.value) + " (+" + std::to_string(event.points) + ")");

        to_remove_event.push_back(static_cast<GameEngine::entity_t>(entity));
    }

    for (auto entity : to_remove_event) {
        registry.removeComponent<component::ScoreEvent>(entity);
    }
}

} // namespace rtype::ecs
