#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "Registry.hpp"

namespace rtype::server {

struct RoomPlayer {
    uint32_t player_id;
    std::string ip;
    uint16_t port;
    GameEngine::entity_t entity;
    bool is_connected{true};
};

class Instance {
  public:
    explicit Instance(std::string name = "default_room");
    ~Instance();
    const std::string& name() const noexcept;
    std::optional<GameEngine::entity_t> addPlayer(uint32_t playerId, const std::string& ip, uint16_t port);
    bool removePlayer(uint32_t playerId);
    bool hasPlayer(uint32_t playerId) const;
    std::optional<RoomPlayer> getPlayer(uint32_t playerId) const;
    std::vector<RoomPlayer> listPlayers() const;
    GameEngine::entity_t createEntity();
    void destroyEntity(GameEngine::entity_t entity);
    GameEngine::Registry& registry() noexcept;

  private:
    std::string name_;
    mutable std::mutex players_mutex_;
    std::map<uint32_t, RoomPlayer> players_;
    GameEngine::Registry registry_;
};

} // namespace rtype::server
