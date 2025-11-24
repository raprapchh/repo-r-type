#pragma once

#include <vector>
#include <optional>
#include <algorithm>
#include "Entity.hpp"

namespace rtype::ecs {

template <typename Component>
class SparseArray {
public:
    using value_type = std::optional<Component>;
    using reference_type = value_type&;
    using const_reference_type = const value_type&;
    using container_t = std::vector<value_type>;
    using size_type = typename container_t::size_type;
    using iterator = typename container_t::iterator;
    using const_iterator = typename container_t::const_iterator;

    SparseArray() = default;
    SparseArray(const SparseArray&) = default;
    SparseArray(SparseArray&&) noexcept = default;
    ~SparseArray() = default;

    SparseArray& operator=(const SparseArray&) = default;
    SparseArray& operator=(SparseArray&&) noexcept = default;

    reference_type operator[](size_t idx) {
        if (idx >= _data.size()) {
            _data.resize(idx + 1);
        }
        return _data[idx];
    }

    const_reference_type operator[](size_t idx) const {
        if (idx >= _data.size()) {
            return _null_opt;
        }
        return _data[idx];
    }

    iterator begin() { return _data.begin(); }
    const_iterator begin() const { return _data.begin(); }
    const_iterator cbegin() const { return _data.cbegin(); }

    iterator end() { return _data.end(); }
    const_iterator end() const { return _data.end(); }
    const_iterator cend() const { return _data.cend(); }

    size_type size() const { return _data.size(); }

    reference_type insert_at(size_type pos, const Component& component) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos] = component;
        return _data[pos];
    }

    reference_type insert_at(size_type pos, Component&& component) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos] = std::move(component);
        return _data[pos];
    }

    template <class... Params>
    reference_type emplace_at(size_type pos, Params&&... params) {
        if (pos >= _data.size()) {
            _data.resize(pos + 1);
        }
        _data[pos].emplace(std::forward<Params>(params)...);
        return _data[pos];
    }

    void erase(size_type pos) {
        if (pos < _data.size()) {
            _data[pos].reset();
        }
    }

    size_type get_index(const value_type& val) const {
        return &val - &_data[0];
    }

private:
    container_t _data;
    static const value_type _null_opt;
};

template <typename Component>
const typename SparseArray<Component>::value_type SparseArray<Component>::_null_opt = std::nullopt;

} // namespace rtype::ecs
