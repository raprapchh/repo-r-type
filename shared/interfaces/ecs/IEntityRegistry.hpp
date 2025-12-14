#pragma once

#include <cstddef>

namespace GameEngine {

using entity_t = std::size_t;

class IEntityRegistry {
  public:
    virtual ~IEntityRegistry() = default;

    virtual entity_t createEntity() = 0;

    virtual void destroyEntity(entity_t entity) = 0;

    virtual bool isValid(entity_t entity) const = 0;

    virtual void clear() = 0;
};

} // namespace GameEngine
