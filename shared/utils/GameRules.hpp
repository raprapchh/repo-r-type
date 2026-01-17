#pragma once

#include <cstdint>

namespace rtype::config {

/// @brief Game mode defining player interaction rules
enum class GameMode : uint8_t {
    COOP = 0, ///< Cooperative mode (no friendly fire)
    PVP = 1,  ///< Player vs Player (friendly fire enabled)
    PVE = 2   ///< Player vs Enemies (reserved for future use)
};

/// @brief Difficulty level affecting enemy stats and spawn rates
enum class Difficulty : uint8_t {
    EASY = 0,   ///< Easier gameplay (reduced enemy stats)
    NORMAL = 1, ///< Standard gameplay (default values)
    HARD = 2    ///< Harder gameplay (increased enemy stats)
};

/// @brief Game rules configuration for a game session
struct GameRules {
    GameMode mode;
    Difficulty difficulty;
    bool friendly_fire_enabled;
    float enemy_hp_multiplier;
    float enemy_speed_multiplier;
    float enemy_fire_rate_multiplier;
    float player_damage_multiplier;
    uint8_t initial_lives;

    GameRules()
        : mode(GameMode::COOP), difficulty(Difficulty::NORMAL), friendly_fire_enabled(false), enemy_hp_multiplier(1.0f),
          enemy_speed_multiplier(1.0f), enemy_fire_rate_multiplier(1.0f), player_damage_multiplier(1.0f),
          initial_lives(3) {
    }

    GameRules(GameMode m, Difficulty d, uint8_t lives = 3)
        : mode(m), difficulty(d), friendly_fire_enabled(m == GameMode::PVP), enemy_hp_multiplier(1.0f),
          enemy_speed_multiplier(1.0f), enemy_fire_rate_multiplier(1.0f), player_damage_multiplier(1.0f),
          initial_lives(lives) {
        apply_difficulty_multipliers();
    }

    void apply_difficulty_multipliers() {
        switch (difficulty) {
        case Difficulty::EASY:
            enemy_hp_multiplier = 0.7f;
            enemy_speed_multiplier = 0.8f;
            enemy_fire_rate_multiplier = 0.8f;
            player_damage_multiplier = 1.3f;
            break;
        case Difficulty::NORMAL:
            enemy_hp_multiplier = 1.0f;
            enemy_speed_multiplier = 1.0f;
            enemy_fire_rate_multiplier = 1.0f;
            player_damage_multiplier = 1.0f;
            break;
        case Difficulty::HARD:
            enemy_hp_multiplier = 1.5f;
            enemy_speed_multiplier = 1.3f;
            enemy_fire_rate_multiplier = 1.5f;
            player_damage_multiplier = 0.8f;
            break;
        }

        if (mode == GameMode::PVP) {
            friendly_fire_enabled = true;
        }
    }
};

} // namespace rtype::config
