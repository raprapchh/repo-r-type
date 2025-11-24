#pragma once

#include <entt/entt.hpp>
#include <stdexcept>
#include "../../shared/interfaces/ecs/IEntityRegistry.hpp"

namespace GameEngine {

class Registry : public IEntityRegistry {
  public:
    Registry() = default;
    ~Registry() override = default;

    // --- IEntityRegistry Implementation ---
    entity_t createEntity() override;
    void destroyEntity(entity_t entity) override;
    bool isValid(entity_t entity) const override;

    // Constructs component T in-place using forwarded args
    template <typename T, typename... Args> T& addComponent(entity_t entity, Args&&... args) {
        auto enttEntity = static_cast<entt::entity>(entity);
        return _registry.emplace<T>(enttEntity, std::forward<Args>(args)...);
    }

    // Returns component T (Throws if missing)
    template <typename T> T& getComponent(entity_t entity) {
        auto enttEntity = static_cast<entt::entity>(entity);
        if (!_registry.all_of<T>(enttEntity)) {
            throw std::runtime_error("Entity does not have the requested component");
        }
        return _registry.get<T>(enttEntity);
    }

    template <typename T> void removeComponent(entity_t entity) {
        _registry.remove<T>(static_cast<entt::entity>(entity));
    }

    template <typename T> bool hasComponent(entity_t entity) const {
        return _registry.all_of<T>(static_cast<entt::entity>(entity));
    }

  private:
    entt::registry _registry;
};

} // namespace GameEngine