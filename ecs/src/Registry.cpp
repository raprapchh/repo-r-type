#include "Registry.hpp"

namespace GameEngine {

entity_t Registry::createEntity() {
    auto entity = _registry.create();
    return static_cast<entity_t>(entity);
}

void Registry::destroyEntity(entity_t entity) {
    auto enttEntity = static_cast<entt::entity>(entity);
    if (_registry.valid(enttEntity)) {
        _registry.destroy(enttEntity);
    }
}

bool Registry::isValid(entity_t entity) const {
    auto enttEntity = static_cast<entt::entity>(entity);
    return _registry.valid(enttEntity);
}

void Registry::clear() {
    _registry.clear();
}

} // namespace GameEngine
