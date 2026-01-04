#pragma once

#include <vector>
#include <memory>
#include "../../shared/interfaces/ecs/ISystem.hpp"

namespace GameEngine {

class Registry;

class SystemManager {
  public:
    SystemManager() = default;
    ~SystemManager() = default;

    /**
     * @brief Adds a system to the manager.
     * @tparam T The system type.
     * @tparam Args Constructor arguments for the system.
     */
    template <typename T, typename... Args> void addSystem(Args&&... args) {
        _systems.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    /**
     * @brief Updates all registered systems.
     * @param registry The entity registry.
     * @param dt The delta time.
     */
    void update(Registry& registry, double dt) {
        for (auto& system : _systems) {
            system->update(registry, dt);
        }
    }

    /**
     * @brief Clears all systems.
     */
    void clear() {
        _systems.clear();
    }

  private:
    std::vector<std::unique_ptr<rtype::ecs::ISystem>> _systems;
};

} // namespace GameEngine
