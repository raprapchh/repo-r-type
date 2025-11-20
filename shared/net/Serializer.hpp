#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

namespace rtype::net {

class Serializer
{
public:
    Serializer() = default;

    template <typename T>
    typename std::enable_if<std::is_pod_v<T>, void>::type write(const T& value)
    {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
        data.insert(data.end(), ptr, ptr + sizeof(T));
    }

    void write(const std::vector<uint8_t>& bytes)
    {
        data.insert(data.end(), bytes.begin(), bytes.end());
    }

    void write(const std::string& str)
    {
        uint32_t size = str.size();
        write(size);
        data.insert(data.end(), str.begin(), str.end());
    }

    const std::vector<uint8_t>& get_data() const
    {
        return data;
    }

    std::vector<uint8_t> get_data_and_clear()
    {
        std::vector<uint8_t> result = std::move(data);
        data.clear();
        return result;
    }

    void clear()
    {
        data.clear();
    }

private:
    std::vector<uint8_t> data;
};

} // namespace rtype::net
