#include "../../include/systems/ScoreSystem.hpp"
#include "../../include/components/Score.hpp"
#include "../../shared/utils/Logger.hpp"

namespace rtype::ecs {

void ScoreSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;
    auto view = registry.view<component::Score, component::ScoreEvent>();

    for (auto entity : view) {
        auto& score = view.get<component::Score>(entity);
        const auto& event = view.get<component::ScoreEvent>(entity);

        score.value += event.points;

        Logger::instance().info("Score updated for entity " + std::to_string(static_cast<std::size_t>(entity)) + ": " +
                                std::to_string(score.value) + " (+" + std::to_string(event.points) + ")");

        registry.removeComponent<component::ScoreEvent>(static_cast<std::size_t>(entity));
    }
}

} // namespace rtype::ecs
