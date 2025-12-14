#pragma once

#include <entt/entt.hpp>
#include <stdexcept>
#include "../../shared/interfaces/ecs/IEntityRegistry.hpp"

namespace GameEngine {

/// @brief Main ECS Registry for managing entities and components
class Registry : public IEntityRegistry {
  public:
    Registry() = default;
    ~Registry() override = default;

    /// @brief Creates a new entity
    entity_t createEntity() override;

    /// @brief Destroys an entity and its components
    void destroyEntity(entity_t entity) override;

    /// @brief Checks if an entity is valid
    bool isValid(entity_t entity) const override;

    /// @brief Clears all entities
    void clear() override;

    /// @brief Adds a component to an entity
    template <typename T, typename... Args> T& addComponent(entity_t entity, Args&&... args) {
        auto enttEntity = static_cast<entt::entity>(entity);
        return _registry.emplace<T>(enttEntity, std::forward<Args>(args)...);
    }

    /// @brief Gets a component from an entity
    /// @throws std::runtime_error if component is missing
    template <typename T> T& getComponent(entity_t entity) {
        auto enttEntity = static_cast<entt::entity>(entity);
        if (!_registry.all_of<T>(enttEntity)) {
            throw std::runtime_error("Entity does not have the requested component");
        }
        return _registry.get<T>(enttEntity);
    }

    /// @brief Removes a component from an entity
    template <typename T> void removeComponent(entity_t entity) {
        _registry.remove<T>(static_cast<entt::entity>(entity));
    }

    /// @brief Checks if an entity has a component
    template <typename T> bool hasComponent(entity_t entity) const {
        return _registry.all_of<T>(static_cast<entt::entity>(entity));
    }

    /// @brief Creates a view for iterating entities with specific components
    template <typename... Components> auto view() {
        return _registry.view<Components...>();
    }

  private:
    entt::registry _registry;
};

} // namespace GameEngine