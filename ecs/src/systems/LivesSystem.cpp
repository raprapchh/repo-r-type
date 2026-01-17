#include "systems/LivesSystem.hpp"
#include "components/Health.hpp"
#include "components/Lives.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "components/CollisionLayer.hpp"
#include "components/InvincibilityTimer.hpp"
#include "components/SpectatorComponent.hpp"
#include "components/Controllable.hpp"
#include "components/GameModeComponent.hpp"
#include "utils/Logger.hpp"
#include <vector>

namespace rtype::ecs {

void LivesSystem::update(GameEngine::Registry& registry, double dt) {
    auto view = registry.view<component::Health, component::Lives, component::Position>();
    std::vector<GameEngine::entity_t> to_destroy;

    for (auto entity : view) {
        auto& health = view.get<component::Health>(entity);
        auto& lives = view.get<component::Lives>(entity);

        if (health.hp <= 0) {
            lives.remaining--;
            Logger::instance().info("Player " + std::to_string(static_cast<std::size_t>(entity)) +
                                    " lost a life. Remaining: " + std::to_string(lives.remaining));

            if (lives.remaining > 0) {
                health.hp = health.max_hp;

                if (registry.hasComponent<component::Velocity>(static_cast<std::size_t>(entity))) {
                    auto& velocity = registry.getComponent<component::Velocity>(static_cast<std::size_t>(entity));
                    velocity.vx = 0.0f;
                    velocity.vy = 0.0f;
                }

                if (registry.hasComponent<component::Collidable>(static_cast<std::size_t>(entity))) {
                    auto& collidable = registry.getComponent<component::Collidable>(static_cast<std::size_t>(entity));
                    collidable.is_active = false;
                }

                if (!registry.hasComponent<component::InvincibilityTimer>(static_cast<std::size_t>(entity))) {
                    registry.addComponent<component::InvincibilityTimer>(static_cast<std::size_t>(entity), 2.0f);
                } else {
                    auto& invincibility =
                        registry.getComponent<component::InvincibilityTimer>(static_cast<std::size_t>(entity));
                    invincibility.timeRemaining = 2.0f;
                }

                auto& position = registry.getComponent<component::Position>(static_cast<std::size_t>(entity));
                Logger::instance().info("Player respawned at (" + std::to_string(position.x) + ", " +
                                        std::to_string(position.y) + ")");
            } else {
                // Player is dead (lives <= 0)
                Logger::instance().info("Game Over for player " + std::to_string(static_cast<std::size_t>(entity)));

                // Check if this is multiplayer mode
                bool is_multiplayer = false;
                auto mode_view = registry.view<component::GameModeComponent>();
                for (auto mode_entity : mode_view) {
                    auto& mode =
                        registry.getComponent<component::GameModeComponent>(static_cast<std::size_t>(mode_entity));
                    is_multiplayer = mode.is_multiplayer;
                    break;
                }

                // Check if this is the local player
                bool is_local_player = false;
                if (registry.hasComponent<component::Controllable>(static_cast<std::size_t>(entity))) {
                    auto& ctrl = registry.getComponent<component::Controllable>(static_cast<std::size_t>(entity));
                    is_local_player = ctrl.is_local_player;
                }

                // In multiplayer: local player becomes spectator
                // In solo or for remote players: destroy entity
                if (is_multiplayer && is_local_player) {
                    Logger::instance().info("Local player entering spectator mode");
                    if (!registry.hasComponent<component::SpectatorComponent>(static_cast<std::size_t>(entity))) {
                        registry.addComponent<component::SpectatorComponent>(static_cast<std::size_t>(entity),
                                                                             component::SpectatorComponent{0});
                    }
                } else {
                    to_destroy.push_back(static_cast<GameEngine::entity_t>(entity));
                }
            }
        }
    }

    for (auto entity : to_destroy) {
        registry.destroyEntity(entity);
    }

    auto invincibility_view = registry.view<component::InvincibilityTimer>();
    std::vector<GameEngine::entity_t> to_remove_invincibility;

    for (auto entity : invincibility_view) {
        auto& invincibility = invincibility_view.get<component::InvincibilityTimer>(entity);
        invincibility.timeRemaining -= static_cast<float>(dt);

        if (invincibility.timeRemaining <= 0.0f) {
            if (registry.hasComponent<component::Collidable>(static_cast<std::size_t>(entity))) {
                auto& collidable = registry.getComponent<component::Collidable>(static_cast<std::size_t>(entity));
                collidable.is_active = true;
            }
            to_remove_invincibility.push_back(static_cast<GameEngine::entity_t>(entity));
        }
    }

    for (auto entity : to_remove_invincibility) {
        registry.removeComponent<component::InvincibilityTimer>(entity);
    }
}

} // namespace rtype::ecs
