#include "../include/Server.hpp"
#include "../../ecs/include/components/PlayerName.hpp"
#include "../../shared/utils/Logger.hpp"

namespace rtype::server {

void Server::handle_player_name(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet) {
    if (!running_.load())
        return;

    std::string client_key = client_ip + ":" + std::to_string(client_port);
    std::string new_name;
    uint32_t player_id = 0;

    if (message_serializer_) {
        auto name_data = message_serializer_->deserialize_player_name(packet);
        if (name_data.player_name[0] != '\0') {
            new_name = std::string(name_data.player_name);
            player_id = name_data.player_id;
        }
    }

    if (new_name.empty() || player_id == 0) {
        return;
    }

    {
        std::lock_guard<std::mutex> clients_lock(clients_mutex_);
        if (clients_.find(client_key) != clients_.end()) {
            clients_[client_key].player_name = new_name;
            Logger::instance().info("Player " + std::to_string(player_id) + " changed name to " + new_name);
        } else {
            return;
        }
    }

    // Broadcast the name change to all clients (including sender)
    if (protocol_adapter_ && message_serializer_ && udp_server_) {
        rtype::net::PlayerNameData name_data(player_id, new_name);
        rtype::net::Packet response = message_serializer_->serialize_player_name(name_data);
        auto response_data = protocol_adapter_->serialize(response);
        broadcast_message(response_data); // Send to ALL clients
    }
}
} // namespace rtype::server
