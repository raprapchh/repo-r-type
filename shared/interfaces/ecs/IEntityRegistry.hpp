#pragma once

#include <cstddef>

namespace GameEngine {

/// @brief Entity identifier type
using entity_t = std::size_t;

/// @brief Abstract interface for ECS registry implementations
class IEntityRegistry {
  public:
    virtual ~IEntityRegistry() = default;

    /// @brief Creates a new entity
    virtual entity_t createEntity() = 0;

    /// @brief Destroys an entity
    virtual void destroyEntity(entity_t entity) = 0;

    /// @brief Checks if an entity is valid
    virtual bool isValid(entity_t entity) const = 0;

    /// @brief Clears all entities
    virtual void clear() = 0;
};

} // namespace GameEngine
