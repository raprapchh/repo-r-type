#pragma once

#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <vector>
#include "SparseArray.hpp"
#include "interfaces/ecs/IEntityRegistry.hpp"

namespace GameEngine {

/// @brief Custom ECS Registry using SparseArray for component storage
class Registry : public IEntityRegistry {
  public:
    Registry() : _nextEntity(0) {
    }
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
        auto& storage = getOrCreateStorage<T>();
        storage.emplace_at(entity, std::forward<Args>(args)...);
        return *storage[entity];
    }

    /// @brief Gets a component from an entity
    /// @throws std::runtime_error if component is missing
    template <typename T> T& getComponent(entity_t entity) {
        auto& storage = getOrCreateStorage<T>();
        if (!storage[entity].has_value()) {
            throw std::runtime_error("Entity does not have the requested component");
        }
        return *storage[entity];
    }

    /// @brief Removes a component from an entity
    template <typename T> void removeComponent(entity_t entity) {
        auto& storage = getOrCreateStorage<T>();
        storage.erase(entity);
    }

    /// @brief Checks if an entity has a component
    template <typename T> bool hasComponent(entity_t entity) const {
        auto it = _componentArrays.find(std::type_index(typeid(T)));
        if (it == _componentArrays.end()) {
            return false;
        }
        auto storage = std::static_pointer_cast<rtype::ecs::SparseArray<T>>(it->second);
        return storage->operator[](entity).has_value();
    }

    /// @brief Creates a view for iterating entities with specific components
    template <typename... Components> class View {
      public:
        View(Registry& registry) : _registry(registry) {
            for (entity_t entity : _registry._validEntities) {
                if (_registry.hasAllComponents<Components...>(entity)) {
                    _entities.push_back(entity);
                }
            }
        }

        /// @brief Iterate over entities with callback (passes entity ID first, then components)
        template <typename Func> void each(Func&& func) {
            for (entity_t entity : _entities) {
                func(entity, _registry.getComponent<Components>(entity)...);
            }
        }

        /// @brief Get a specific component from an entity in this view
        template <typename T> T& get(entity_t entity) {
            return _registry.getComponent<T>(entity);
        }

        /// @brief Get list of entities
        const std::vector<entity_t>& entities() const {
            return _entities;
        }

        /// @brief Begin iterator for range-based for loop
        auto begin() {
            return _entities.begin();
        }

        /// @brief End iterator for range-based for loop
        auto end() {
            return _entities.end();
        }

        /// @brief Const begin iterator
        auto begin() const {
            return _entities.begin();
        }

        /// @brief Const end iterator
        auto end() const {
            return _entities.end();
        }

      private:
        Registry& _registry;
        std::vector<entity_t> _entities;
    };

    /// @brief Creates a view for iterating entities with specific components
    template <typename... Components> View<Components...> view() {
        return View<Components...>(*this);
    }

  private:
    entity_t _nextEntity;
    std::unordered_set<entity_t> _validEntities;
    std::unordered_map<std::type_index, std::shared_ptr<void>> _componentArrays;

    /// @brief Gets or creates component storage for type T
    template <typename T> rtype::ecs::SparseArray<T>& getOrCreateStorage() {
        auto typeIndex = std::type_index(typeid(T));
        auto it = _componentArrays.find(typeIndex);

        if (it == _componentArrays.end()) {
            auto storage = std::make_shared<rtype::ecs::SparseArray<T>>();
            _componentArrays[typeIndex] = storage;
            return *storage;
        }

        return *std::static_pointer_cast<rtype::ecs::SparseArray<T>>(it->second);
    }

    /// @brief Helper to check if entity has all components
    template <typename... Components> bool hasAllComponents(entity_t entity) const {
        return (hasComponent<Components>(entity) && ...);
    }
};

} // namespace GameEngine
