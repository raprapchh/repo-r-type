#pragma once

#include "../../../shared/utils/GameRules.hpp"

namespace rtype::ecs::component {

/// @brief Component to store game rules in the ECS registry
/// Typically attached to a world entity for systems to access
struct GameRulesComponent {
    rtype::config::GameRules rules;

    GameRulesComponent() : rules() {
    }

    explicit GameRulesComponent(const rtype::config::GameRules& r) : rules(r) {
    }
};

} // namespace rtype::ecs::component
