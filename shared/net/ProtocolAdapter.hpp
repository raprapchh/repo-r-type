#pragma once

#include <iostream>
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
            std::cerr << "Packet too short: " << data.size() << " bytes, expected header " << sizeof(PacketHeader)
                      << std::endl;
            return false;
        }

        PacketHeader header;
        std::memcpy(&header, data.data(), sizeof(PacketHeader));

        uint16_t expected_size = sizeof(PacketHeader) + header.payload_size;

        if (data.size() != expected_size) {
            std::cerr << "Invalid packet size: " << data.size() << " expected " << expected_size << " for type "
                      << header.message_type << std::endl;
            return false;
        }

        if (header.message_type == 0 || header.message_type > 30) {
            std::cerr << "Invalid message type: " << header.message_type << std::endl;
            return false;
        }

        if (header.payload_size > MAX_PAYLOAD_SIZE) {
            std::cerr << "Payload too large: " << header.payload_size << std::endl;
            return false;
        }

        return true;
    }

  private:
    static constexpr size_t MAX_PAYLOAD_SIZE = 1024 - sizeof(PacketHeader);
};

} // namespace rtype::net
