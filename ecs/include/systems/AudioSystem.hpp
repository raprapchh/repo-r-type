#pragma once

#include "../../shared/interfaces/ecs/ISystem.hpp"
#include "../Registry.hpp"
#include <SFML/Audio.hpp>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

namespace rtype::ecs {

class AudioSystem : public ISystem {
  public:
    AudioSystem();
    ~AudioSystem() override = default;

    void update(GameEngine::Registry& registry, double dt) override;

    bool loadSoundBuffer(const std::string& id, const std::string& filepath);
    bool loadMusic(const std::string& id, const std::string& filepath);

    void playSound(const std::string& id);
    void playMusic(const std::string& id, bool loop = true);
    void stopMusic(const std::string& id);

  private:
    std::unordered_map<std::string, sf::SoundBuffer> sound_buffers_;
    std::unordered_map<std::string, std::unique_ptr<sf::Music>> music_tracks_;
    std::list<sf::Sound> active_sounds_;
};

} // namespace rtype::ecs
