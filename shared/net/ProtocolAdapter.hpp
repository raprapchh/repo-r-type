#pragma once

#include "../interfaces/network/IProtocolAdapter.hpp"
#include "Packet.hpp"
#include <cstdint>
#include <cstring>
#include <vector>

namespace rtype::net {

class ProtocolAdapter : public IProtocolAdapter {
  public:
    ProtocolAdapter() = default;
    ~ProtocolAdapter() override = default;

    Packet deserialize(const std::vector<uint8_t>& data) override {
        return Packet::deserialize(data);
    }

    std::vector<uint8_t> serialize(const Packet& packet) override {
        return packet.serialize();
    }

    bool validate(const std::vector<uint8_t>& data) const override {
        if (data.empty()) {
            return false;
        }

        if (data.size() < sizeof(PacketHeader)) {
            return false;
        }

        PacketHeader header;
        std::memcpy(&header, data.data(), sizeof(PacketHeader));

        uint16_t expected_size = sizeof(PacketHeader) + header.payload_size;

        if (data.size() != expected_size) {
            return false;
        }

        if (header.message_type == 0 || header.message_type > 20) {
            return false;
        }

        if (header.payload_size > MAX_PAYLOAD_SIZE) {
            return false;
        }

        return true;
    }

  private:
    static constexpr size_t MAX_PAYLOAD_SIZE = 1024 - sizeof(PacketHeader);
};

} // namespace rtype::net
