#pragma once

#include <any>
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include "Entity.hpp"
#include "SparseArray.hpp"

namespace rtype::ecs {

class Registry {
public:
    Registry() = default;

    template <class Component>
    SparseArray<Component>& register_component() {
        auto type_index = std::type_index(typeid(Component));
        if (_components_arrays.find(type_index) == _components_arrays.end()) {
            _components_arrays[type_index] = std::make_any<SparseArray<Component>>();
            _erasers[type_index] = [this](Entity const& e) {
                get_components<Component>().erase(e);
            };
        }
        return std::any_cast<SparseArray<Component>&>(_components_arrays[type_index]);
    }

    template <class Component>
    SparseArray<Component>& get_components() {
        auto type_index = std::type_index(typeid(Component));
        return std::any_cast<SparseArray<Component>&>(_components_arrays.at(type_index));
    }

    template <class Component>
    const SparseArray<Component>& get_components() const {
        auto type_index = std::type_index(typeid(Component));
        return std::any_cast<const SparseArray<Component>&>(_components_arrays.at(type_index));
    }

    Entity spawn_entity() {
        if (!_dead_entities.empty()) {
            Entity e = _dead_entities.back();
            _dead_entities.pop_back();
            return e;
        }
        return _next_entity_id++;
    }

    Entity entity_from_index(std::size_t idx) {
        return idx;
    }

    void kill_entity(Entity const& e) {
        for (auto& [type, eraser] : _erasers) {
            eraser(e);
        }
        _dead_entities.push_back(e);
    }

    template <typename Component>
    typename SparseArray<Component>::reference_type add_component(Entity const& to, Component&& c) {
        return get_components<Component>().insert_at(to, std::forward<Component>(c));
    }

    template <typename Component, typename... Params>
    typename SparseArray<Component>::reference_type emplace_component(Entity const& to, Params&&... p) {
        return get_components<Component>().emplace_at(to, std::forward<Params>(p)...);
    }

    template <typename Component>
    void remove_component(Entity const& from) {
        get_components<Component>().erase(from);
    }

private:
    std::unordered_map<std::type_index, std::any> _components_arrays;
    std::unordered_map<std::type_index, std::function<void(Entity const&)>> _erasers;
    std::size_t _next_entity_id = 0;
    std::vector<Entity> _dead_entities;
};

} // namespace rtype::ecs
