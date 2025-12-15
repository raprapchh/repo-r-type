#include "../../include/systems/GameRuleSystem.hpp"
#include "../../include/components/NetworkId.hpp"
#include "../../include/components/Lives.hpp"
#include "../../include/components/Health.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/Weapon.hpp"

namespace rtype::ecs {

GameRuleSystem::GameRuleSystem(uint32_t player_id, bool& game_over, bool& all_players_dead)
    : player_id_(player_id), game_over_(game_over), all_players_dead_(all_players_dead) {
}

void GameRuleSystem::update(GameEngine::Registry& registry, double dt) {
    (void)dt;

    bool player_exists = false;
    bool player_dead = false;

    auto view = registry.view<component::NetworkId>();
    for (auto entity : view) {
        auto& net_id = registry.getComponent<component::NetworkId>(static_cast<std::size_t>(entity));
        if (net_id.id == player_id_) {
            player_exists = true;
            if (registry.hasComponent<component::Lives>(static_cast<std::size_t>(entity))) {
                auto& lives = registry.getComponent<component::Lives>(static_cast<std::size_t>(entity));
                if (lives.remaining <= 0) {
                    player_dead = true;
                }
            } else if (registry.hasComponent<component::Health>(static_cast<std::size_t>(entity))) {
                auto& health = registry.getComponent<component::Health>(static_cast<std::size_t>(entity));
                if (health.hp <= 0) {
                    player_dead = true;
                }
            }
            break;
        }
    }

    if (!player_exists || player_dead) {
        if (!game_over_) {
            game_over_ = true;
            auto player_view = registry.view<component::NetworkId>();
            for (auto entity : player_view) {
                auto& net_id = registry.getComponent<component::NetworkId>(static_cast<std::size_t>(entity));
                if (net_id.id == player_id_ &&
                    registry.hasComponent<component::Drawable>(static_cast<std::size_t>(entity))) {
                    registry.removeComponent<component::Drawable>(static_cast<std::size_t>(entity));
                }
            }
        }
    }

    if (game_over_) {
        int alive_players = 0;
        auto all_players_view = registry.view<component::NetworkId, component::Weapon>();
        for (auto entity : all_players_view) {
            auto& net_id = registry.getComponent<component::NetworkId>(static_cast<std::size_t>(entity));
            if (net_id.id != player_id_) {
                bool other_player_alive = true;
                if (registry.hasComponent<component::Lives>(static_cast<std::size_t>(entity))) {
                    auto& lives = registry.getComponent<component::Lives>(static_cast<std::size_t>(entity));
                    if (lives.remaining <= 0) {
                        other_player_alive = false;
                    }
                } else if (registry.hasComponent<component::Health>(static_cast<std::size_t>(entity))) {
                    auto& health = registry.getComponent<component::Health>(static_cast<std::size_t>(entity));
                    if (health.hp <= 0) {
                        other_player_alive = false;
                    }
                }
                if (other_player_alive) {
                    alive_players++;
                }
            }
        }
        all_players_dead_ = (alive_players == 0);
    }
}

} // namespace rtype::ecs
