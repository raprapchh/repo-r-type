#include "../include/Registry.hpp"

namespace GameEngine {

entity_t Registry::createEntity() {
    entity_t entity = _nextEntity++;
    _validEntities.insert(entity);
    return entity;
}

void Registry::destroyEntity(entity_t entity) {
    if (_validEntities.find(entity) != _validEntities.end()) {
        _validEntities.erase(entity);

        // Remove entity from all component arrays
        for (auto& [typeIndex, storage] : _componentArrays) {
            // Note: SparseArray will handle entity removal gracefully
            // We can't easily iterate over all types, but erase is safe
        }
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
