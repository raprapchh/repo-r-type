#include "systems/SpectatorSystem.hpp"
#include "components/SpectatorComponent.hpp"
#include "components/Position.hpp"
#include "components/Tag.hpp"
#include <vector>
#include <algorithm>

namespace rtype::ecs {

SpectatorSystem::SpectatorSystem(bool left_pressed, bool right_pressed, float cooldown)
    : left_pressed_(left_pressed), right_pressed_(right_pressed), cooldown_duration_(cooldown) {
}

void SpectatorSystem::update(GameEngine::Registry& registry, double dt) {
    auto spectator_view = registry.view<component::SpectatorComponent>();

    for (auto entity : spectator_view) {
        auto& spectator = registry.getComponent<component::SpectatorComponent>(static_cast<std::size_t>(entity));

        // Update cooldown timer
        if (spectator.switchTimer > 0.0f) {
            spectator.switchTimer -= static_cast<float>(dt);
            if (spectator.switchTimer < 0.0f) {
                spectator.switchTimer = 0.0f;
            }
        }

        // Collect all valid player targets
        std::vector<std::size_t> valid_targets;
        auto player_view = registry.view<component::Position, component::Tag>();

        for (auto player_entity : player_view) {
            std::size_t player_id = static_cast<std::size_t>(player_entity);
            auto& tag = registry.getComponent<component::Tag>(player_id);

            if (tag.name == "Player") {
                valid_targets.push_back(player_id);
            }
        }

        // Sort targets for consistent ordering
        std::sort(valid_targets.begin(), valid_targets.end());

        if (valid_targets.empty()) {
            // No valid targets, reset to 0
            spectator.targetEntityId = 0;
            continue;
        }

        // Find current target index
        auto current_it = std::find(valid_targets.begin(), valid_targets.end(), spectator.targetEntityId);

        // If current target is invalid, switch to first available
        if (current_it == valid_targets.end()) {
            spectator.targetEntityId = valid_targets[0];
            continue;
        }

        // Handle input if cooldown expired
        if (spectator.switchTimer <= 0.0f) {
            std::size_t current_index = std::distance(valid_targets.begin(), current_it);
            bool should_switch = false;

            if (right_pressed_ && valid_targets.size() > 1) {
                // Switch to next target
                current_index = (current_index + 1) % valid_targets.size();
                should_switch = true;
            } else if (left_pressed_ && valid_targets.size() > 1) {
                // Switch to previous target
                current_index = (current_index == 0) ? (valid_targets.size() - 1) : (current_index - 1);
                should_switch = true;
            }

            if (should_switch) {
                spectator.targetEntityId = valid_targets[current_index];
                spectator.switchTimer = cooldown_duration_;
            }
        }
    }
}

} // namespace rtype::ecs
