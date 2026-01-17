#include "Instance.hpp"
#include "utils/Logger.hpp"

namespace rtype::server {

Instance::Instance(std::string name) : name_(std::move(name)), registry_() {
    Logger::instance().debug("Instance created: " + name_);
}

Instance::~Instance() {
    std::lock_guard<std::mutex> lock(players_mutex_);
    for (auto& kv : players_) {
        try {
            registry_.destroyEntity(kv.second.entity);
        } catch (...) {
            Logger::instance().error("Failed to destroy player entity");
        }
    }

    players_.clear();
    Logger::instance().debug("Instance destroyed: " + name_);
}

const std::string& Instance::name() const noexcept {
    return name_;
}

std::optional<GameEngine::entity_t> Instance::addPlayer(uint32_t playerId, const std::string& ip, uint16_t port) {
    std::lock_guard<std::mutex> lock(players_mutex_);
    if (players_.find(playerId) != players_.end()) {
        return std::nullopt;
    }

    GameEngine::entity_t ent = registry_.createEntity();
    RoomPlayer p;
    p.player_id = playerId;
    p.ip = ip;
    p.port = port;
    p.entity = ent;
    p.is_connected = true;

    players_.emplace(playerId, std::move(p));
    Logger::instance().info("Player added to instance '" + name_ + "': " + std::to_string(playerId));
    return ent;
}

bool Instance::removePlayer(uint32_t playerId) {
    std::lock_guard<std::mutex> lock(players_mutex_);
    auto it = players_.find(playerId);
    if (it == players_.end()) {
        return false;
    }

    try {
        registry_.destroyEntity(it->second.entity);
    } catch (...) {
        Logger::instance().error("Failed to destroy player entity");
    }

    players_.erase(it);
    Logger::instance().info("Player removed from instance '" + name_ + "': " + std::to_string(playerId));
    return true;
}

bool Instance::hasPlayer(uint32_t playerId) const {
    std::lock_guard<std::mutex> lock(players_mutex_);
    return players_.find(playerId) != players_.end();
}

std::optional<RoomPlayer> Instance::getPlayer(uint32_t playerId) const {
    std::lock_guard<std::mutex> lock(players_mutex_);
    auto it = players_.find(playerId);
    if (it == players_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<RoomPlayer> Instance::listPlayers() const {
    std::lock_guard<std::mutex> lock(players_mutex_);
    std::vector<RoomPlayer> out;
    out.reserve(players_.size());
    for (const auto& kv : players_) {
        out.push_back(kv.second);
    }
    return out;
}

GameEngine::entity_t Instance::createEntity() {
    return registry_.createEntity();
}

void Instance::destroyEntity(GameEngine::entity_t entity) {
    registry_.destroyEntity(entity);
}

GameEngine::Registry& Instance::registry() noexcept {
    return registry_;
}

} // namespace rtype::server
