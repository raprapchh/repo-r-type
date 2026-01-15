#include "systems/SpectatorSystem.hpp"
#include "components/SpectatorComponent.hpp"
#include <iostream>

namespace rtype::ecs {

void SpectatorSystem::update(GameEngine::Registry& registry, double delta_time) {
    (void)registry;
    (void)delta_time;
    // Logic for spectator updates can be added here
}

} // namespace rtype::ecs
