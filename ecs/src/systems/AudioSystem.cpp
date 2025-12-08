#include "../../include/systems/AudioSystem.hpp"
#include "../../include/components/Sound.hpp"
#include <iostream>

namespace rtype::ecs {

AudioSystem::AudioSystem() {
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
        std::cerr << "AudioSystem error: " << e.what() << std::endl;
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

} // namespace rtype::ecs
