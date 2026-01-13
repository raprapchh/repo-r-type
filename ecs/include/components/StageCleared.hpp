#pragma once

namespace rtype::ecs::component {

/**
 * @brief Component that marks the game stage as cleared (boss defeated)
 *
 * This is added to the world entity when a boss is defeated.
 * The BroadcastSystem detects this and sends StageCleared to all clients.
 */
struct StageCleared {
    uint8_t stage_number;
    bool broadcasted;

    StageCleared() : stage_number(1), broadcasted(false) {
    }
    explicit StageCleared(uint8_t stage) : stage_number(stage), broadcasted(false) {
    }
};

} // namespace rtype::ecs::component
