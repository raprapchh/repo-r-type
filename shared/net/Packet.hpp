#pragma once

#include <cstdint>
#include <cstring>
#include <vector>

namespace rtype::net {

#pragma pack(push, 1)
struct PacketHeader {
    uint16_t message_type;
    uint16_t payload_size;
};
#pragma pack(pop)

struct Packet {
    PacketHeader header;
    std::vector<uint8_t> body;

    Packet() : header{0, 0} {
    }

    Packet(uint16_t msg_type, const std::vector<uint8_t>& data)
        : header{msg_type, static_cast<uint16_t>(data.size())}, body(data) {
    }

    std::vector<uint8_t> serialize() const {
        std::vector<uint8_t> result;
        result.reserve(sizeof(PacketHeader) + body.size());

        uint8_t* header_bytes = reinterpret_cast<uint8_t*>(const_cast<PacketHeader*>(&header));
        result.insert(result.end(), header_bytes, header_bytes + sizeof(PacketHeader));
        result.insert(result.end(), body.begin(), body.end());

        return result;
    }

    static Packet deserialize(const std::vector<uint8_t>& data) {
        Packet pkt;
        if (data.size() >= sizeof(PacketHeader)) {
            std::memcpy(&pkt.header, data.data(), sizeof(PacketHeader));
            if (pkt.header.payload_size > 0 && data.size() >= sizeof(PacketHeader) + pkt.header.payload_size) {
                pkt.body.assign(data.begin() + sizeof(PacketHeader),
                                data.begin() + sizeof(PacketHeader) + pkt.header.payload_size);
            }
        }
        return pkt;
    }
};

} // namespace rtype::net
