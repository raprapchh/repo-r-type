#include "../../include/systems/AudioSystem.hpp"
#include "../../include/components/Sound.hpp"
#include "../../include/components/AudioEvent.hpp"
#include "../../include/components/Position.hpp"
#include "../../include/components/Velocity.hpp"
#include "../../include/components/Drawable.hpp"
#include "../../include/components/Tag.hpp"
#include <iostream>

namespace rtype::ecs {

AudioSystem::AudioSystem() : current_music_id_("") {
}

void AudioSystem::update(GameEngine::Registry& registry, double) {
    active_sounds_.remove_if([](const sf::Sound& sound) { return sound.getStatus() == sf::Sound::Stopped; });

    // Process existing Sound components (backward compatibility)
    try {
        auto view = registry.view<Sound>();
        for (auto entity : view) {
            auto& sound_comp = registry.getComponent<Sound>(static_cast<GameEngine::entity_t>(entity));
            if (sound_comp.should_play) {
                playSound(sound_comp.sound_id);
                sound_comp.should_play = false;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "AudioSystem error processing Sound: " << e.what() << std::endl;
    }

    // Process AudioEvent components
    try {
        auto event_view = registry.view<component::AudioEvent>();
        std::vector<GameEngine::entity_t> entities_to_process;

        // Collect entities first
        for (auto entity : event_view) {
            entities_to_process.push_back(static_cast<GameEngine::entity_t>(entity));
        }

        // Process each entity
        for (auto entity : entities_to_process) {
            if (!registry.isValid(entity)) continue;

            auto& audio_event = registry.getComponent<component::AudioEvent>(entity);

            std::string sound_id = mapEventToSoundId(audio_event.type);
            if (!sound_id.empty()) {
                playSound(sound_id);
            }

            // Remove the component
            registry.removeComponent<component::AudioEvent>(entity);

            // If the entity only had AudioEvent component (temporary audio entity), destroy it
            // Check by trying to get common components - if none exist, it's a temp entity
            bool has_other_components =
                registry.hasComponent<component::Position>(entity) ||
                registry.hasComponent<component::Velocity>(entity) ||
                registry.hasComponent<component::Drawable>(entity) ||
                registry.hasComponent<component::Tag>(entity);

            if (!has_other_components) {
                registry.destroyEntity(entity);
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "AudioSystem error processing AudioEvent: " << e.what() << std::endl;
    }
}

bool AudioSystem::loadSoundBuffer(const std::string& id, const std::string& filepath) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(filepath)) {
        std::cerr << "Failed to load sound: " << filepath << std::endl;
        return false;
    }
    sound_buffers_[id] = std::move(buffer);
    return true;
}

bool AudioSystem::loadMusic(const std::string& id, const std::string& filepath) {
    auto music = std::make_unique<sf::Music>();
    if (!music->openFromFile(filepath)) {
        std::cerr << "Failed to load music: " << filepath << std::endl;
        return false;
    }
    music_tracks_[id] = std::move(music);
    return true;
}

void AudioSystem::playSound(const std::string& id) {
    auto it = sound_buffers_.find(id);
    if (it != sound_buffers_.end()) {
        active_sounds_.emplace_back();
        sf::Sound& sound = active_sounds_.back();
        sound.setBuffer(it->second);
        sound.play();
    } else {
        std::cerr << "Sound ID not found: " << id << std::endl;
    }
}

void AudioSystem::playMusic(const std::string& id, bool loop) {
    auto it = music_tracks_.find(id);
    if (it != music_tracks_.end()) {
        if (it->second->getStatus() != sf::Music::Playing) {
            it->second->setLoop(loop);
            it->second->play();
        }
    } else {
        std::cerr << "Music ID not found: " << id << std::endl;
    }
}

void AudioSystem::stopMusic(const std::string& id) {
    auto it = music_tracks_.find(id);
    if (it != music_tracks_.end()) {
        it->second->stop();
    }
}

std::string AudioSystem::mapEventToSoundId(component::AudioEventType type) {
    switch (type) {
        case component::AudioEventType::PLAYER_SHOOT:
            return "player_shoot";
        case component::AudioEventType::ENEMY_SHOOT:
            return "enemy_shoot";
        case component::AudioEventType::EXPLOSION:
            return "explosion";
        case component::AudioEventType::COLLISION_HIT:
            return "hit";
        case component::AudioEventType::POWERUP_COLLECT:
            return "powerup";
        case component::AudioEventType::ENEMY_DEATH:
            return "enemy_death";
        case component::AudioEventType::PLAYER_DAMAGE:
            return "player_damage";
        default:
            return "";
    }
}

void AudioSystem::initializeAudioAssets() {
    // Load sound effects (using available audio files)
    loadSoundBuffer("player_shoot", "client/assets/audio/shoot.wav");     // Player shooting
    loadSoundBuffer("enemy_shoot", "client/assets/audio/shoot.wav");      // Enemy shooting
    loadSoundBuffer("explosion", "client/assets/audio/explosion.wav");    // Explosion sound
    loadSoundBuffer("hit", "client/assets/audio/impact.wav");             // Impact/hit sound
    loadSoundBuffer("powerup", "client/assets/audio/power-up.wav");       // Power-up collection
    loadSoundBuffer("enemy_death", "client/assets/audio/explosion.wav");  // Reuse explosion for death
    loadSoundBuffer("player_damage", "client/assets/audio/impact.wav");   // Player damage sound

    // Load background music (WAV format - will work but OGG recommended for size)
    loadMusic("gameplay", "client/assets/audio/music.wav");

    std::cout << "AudioSystem: Assets initialized" << std::endl;
}

void AudioSystem::startBackgroundMusic() {
    if (current_music_id_.empty()) {
        playMusic("gameplay", true);
        current_music_id_ = "gameplay";
    }
}

void AudioSystem::stopBackgroundMusic() {
    if (!current_music_id_.empty()) {
        stopMusic(current_music_id_);
        current_music_id_.clear();
    }
}

} // namespace rtype::ecs
