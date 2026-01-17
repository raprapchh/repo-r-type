#include "Registry.hpp"

namespace GameEngine {

entity_t Registry::createEntity() {
    entity_t entity = _nextEntity++;
    _validEntities.insert(entity);
    return entity;
}

void Registry::destroyEntity(entity_t entity) {
    if (_validEntities.find(entity) != _validEntities.end()) {
        _validEntities.erase(entity);
    }
}

bool Registry::isValid(entity_t entity) const {
    return _validEntities.find(entity) != _validEntities.end();
}

void Registry::clear() {
    _validEntities.clear();
    _componentArrays.clear();
    _nextEntity = 0;
}

} // namespace GameEngine
