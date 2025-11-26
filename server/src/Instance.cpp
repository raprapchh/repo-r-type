#include "../../server/include/Instance.hpp"
#include "../../shared/utils/Logger.hpp"

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
            // ignore
        }
    }

    players_.clear();
    Logger::instance().debug("Instance destroyed: " + name_);
}

const std::string& Instance::name() const noexcept {
    return name_;
}

GameEngine::Registry& Instance::registry() noexcept {
    return registry_;
}

} // namespace rtype::server
