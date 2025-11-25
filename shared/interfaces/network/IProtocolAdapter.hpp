#pragma once

#include "../../net/Packet.hpp"
#include <vector>
#include <cstdint>

namespace rtype::net {

class IProtocolAdapter {
  public:
    virtual ~IProtocolAdapter() = default;

    virtual Packet deserialize(const std::vector<uint8_t>& data) = 0;

    virtual std::vector<uint8_t> serialize(const Packet& packet) = 0;

    virtual bool validate(const std::vector<uint8_t>& data) const = 0;
};

} // namespace rtype::net

