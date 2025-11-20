#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

namespace rtype::net {

class Deserializer
{
public:
    Deserializer(const std::vector<uint8_t>& buffer) : data(buffer), offset(0)
    {
    }

    template <typename T>
    typename std::enable_if<std::is_pod_v<T>, T>::type read()
    {
        if (offset + sizeof(T) > data.size()) {
            throw std::runtime_error("Deserializer: not enough data");
        }
        T value;
        std::memcpy(&value, data.data() + offset, sizeof(T));
        offset += sizeof(T);
        return value;
    }

    std::vector<uint8_t> read_bytes(size_t count)
    {
        if (offset + count > data.size()) {
            throw std::runtime_error("Deserializer: not enough data");
        }
        std::vector<uint8_t> result(data.begin() + offset, data.begin() + offset + count);
        offset += count;
        return result;
    }

    std::string read_string()
    {
        uint32_t size = read<uint32_t>();
        if (offset + size > data.size()) {
            throw std::runtime_error("Deserializer: not enough data for string");
        }
        std::string result(reinterpret_cast<const char*>(data.data() + offset), size);
        offset += size;
        return result;
    }

    bool has_data(size_t count = 1) const
    {
        return offset + count <= data.size();
    }

    size_t get_offset() const
    {
        return offset;
    }

    void reset()
    {
        offset = 0;
    }

private:
    const std::vector<uint8_t>& data;
    size_t offset;
};

} // namespace rtype::net
