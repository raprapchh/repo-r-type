#include "systems/AudioSystem.hpp"
#include "components/Sound.hpp"
#include "components/AudioEvent.hpp"
#include "components/Position.hpp"
#include "components/Velocity.hpp"
#include "components/Drawable.hpp"
#include "components/Tag.hpp"
#include <iostream>
#include <thread>
#include <chrono>

namespace rtype::ecs {

AudioSystem::AudioSystem() : current_music_id_("") {
}

AudioSystem::~AudioSystem() {
    cleanup();
}

void AudioSystem::cleanup() {
    // First, stop all music (streaming audio uses threads)
    for (auto& pair : music_tracks_) {
        if (pair.second) {
            pair.second->stop();
        }
    }

    // Stop all active sounds
    for (auto& sound : active_sounds_) {
        sound.stop();
    }

    // Small delay to let audio threads finish
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Clear sounds first (they reference buffers)
    active_sounds_.clear();

    // Explicitly reset each unique_ptr before clearing the map
    for (auto& pair : music_tracks_) {
        pair.second.reset();
    }
    music_tracks_.clear();

    // Clear sound buffers last
    sound_buffers_.clear();

    current_music_id_.clear();
}

void AudioSystem::update(GameEngine::Registry& registry, double) {
    active_sounds_.remove_if([](const sf::Sound& sound) { return sound.getStatus() == sf::Sound::Stopped; });

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

    try {
        auto event_view = registry.view<component::AudioEvent>();
        std::vector<GameEngine::entity_t> entities_to_process;

        for (auto entity : event_view) {
            entities_to_process.push_back(static_cast<GameEngine::entity_t>(entity));
        }

        for (auto entity : entities_to_process) {
            if (!registry.isValid(entity))
                continue;

            auto& audio_event = registry.getComponent<component::AudioEvent>(entity);

            // Handle special music events
            if (audio_event.type == component::AudioEventType::BOSS_MUSIC_START) {
                switchToBossMusic();
            } else {
                std::string sound_id = mapEventToSoundId(audio_event.type);
                if (!sound_id.empty()) {
                    playSound(sound_id);
                }
            }

            registry.removeComponent<component::AudioEvent>(entity);

            bool has_other_components = registry.hasComponent<component::Position>(entity) ||
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
        sound.setVolume(100.0f);
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
            it->second->setVolume(100.0f);
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
    case component::AudioEventType::PLAYER_MISSILE:
        return "player_missile";
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
    case component::AudioEventType::BOSS_ROAR:
        return "boss_roar";
    case component::AudioEventType::BOSS_MUSIC_START:
        return ""; // Handled separately in update()
    default:
        return "";
    }
}

void AudioSystem::initializeAudioAssets() {
    // Load sound effects
    loadSoundBuffer("player_shoot", "client/assets/audio/impact.wav");       // Player shooting (shoot.wav is empty)
    loadSoundBuffer("player_missile", "client/assets/audio/game_shoot.wav"); // Player missile (charged shot)
    loadSoundBuffer("enemy_shoot", "client/assets/audio/impact.wav");        // Enemy shooting (shoot.wav is empty)
    loadSoundBuffer("explosion", "client/assets/audio/explosion.wav");       // Explosion sound
    loadSoundBuffer("hit", "client/assets/audio/impact.wav");                // Impact/hit sound
    loadSoundBuffer("powerup", "client/assets/audio/power-up.wav");          // Power-up collection
    loadSoundBuffer("enemy_death", "client/assets/audio/explosion.wav");     // Reuse explosion for death
    loadSoundBuffer("player_damage", "client/assets/audio/impact.wav");      // Player damage sound
    loadSoundBuffer("boss_roar", "client/assets/audio/boss_apparition.wav"); // Boss roar/apparition

    // Load background music
    loadMusic("lobby_music", "client/assets/audio/lobby_music.mp3"); // Lobby background music
    loadMusic("gameplay", "client/assets/audio/music.wav");          // Gameplay background music
    loadMusic("boss_music", "client/assets/audio/music_boss.wav");   // Boss battle music

    std::cout << "AudioSystem: Assets initialized" << std::endl;
}

void AudioSystem::startBackgroundMusic() {
    if (current_music_id_.empty()) {
        playMusic("gameplay", true);
        current_music_id_ = "gameplay";
    }
}

void AudioSystem::startLobbyMusic() {
    // Stop current music if any
    if (!current_music_id_.empty()) {
        stopMusic(current_music_id_);
    }
    // Start lobby music
    playMusic("lobby_music", true);
    current_music_id_ = "lobby_music";
}

void AudioSystem::stopBackgroundMusic() {
    if (!current_music_id_.empty()) {
        stopMusic(current_music_id_);
        current_music_id_.clear();
    }
}

void AudioSystem::switchToBossMusic() {
    // Stop current music
    if (!current_music_id_.empty()) {
        stopMusic(current_music_id_);
    }
    // Start boss music (will use gameplay music for now, add boss music file later)
    playMusic("boss_music", true);
    current_music_id_ = "boss_music";
}

void AudioSystem::switchToGameplayMusic() {
    // Stop current music
    if (!current_music_id_.empty()) {
        stopMusic(current_music_id_);
    }
    // Start gameplay music
    playMusic("gameplay", true);
    current_music_id_ = "gameplay";
}

} // namespace rtype::ecs
