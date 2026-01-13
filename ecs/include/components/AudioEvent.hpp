#pragma once

namespace rtype::ecs::component {

enum class AudioEventType {
    PLAYER_SHOOT,
    PLAYER_MISSILE,
    ENEMY_SHOOT,
    EXPLOSION,
    COLLISION_HIT,
    POWERUP_COLLECT,
    ENEMY_DEATH,
    PLAYER_DAMAGE,
    BOSS_MUSIC_START,
    BOSS_ROAR
};

struct AudioEvent {
    AudioEventType type;

    AudioEvent() : type(AudioEventType::PLAYER_SHOOT) {
    }
    explicit AudioEvent(AudioEventType t) : type(t) {
    }
};

} // namespace rtype::ecs::component
