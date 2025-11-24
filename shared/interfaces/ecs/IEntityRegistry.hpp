#pragma once

#include <cstddef>

namespace GameEngine {

using entity_t = std::size_t;

class IEntityRegistry {
    public:
        virtual ~IEntityRegistry() = default;

        /**
         * @brief Creates a new entity.
         * @return The ID of the created entity.
        */
        virtual entity_t createEntity() = 0;

        /**
         * @brief Destroys an entity.
         * @param entity The ID of the entity to destroy.
        */
        virtual void destroyEntity(entity_t entity) = 0;

        /**
         * @brief Checks if an entity is valid.
         * @param entity The ID of the entity to check.
         * @return True if the entity is valid, false otherwise.
        */
        virtual bool isValid(entity_t entity) const = 0;
};

} // namespace GameEngine
