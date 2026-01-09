#include "Server.hpp"
#include "../../shared/GameConstants.hpp"
#include "../../ecs/include/components/PlayerName.hpp"
#include "../shared/net/Protocol.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../include/BroadcastSystem.hpp"
#include "../../ecs/include/systems/MovementSystem.hpp"
#include "../../ecs/include/systems/BoundarySystem.hpp"
#include "../../ecs/include/systems/CollisionSystem.hpp"
#include "../../ecs/include/systems/WeaponSystem.hpp"
#include "../../ecs/include/systems/SpawnSystem.hpp"
#include "../../ecs/include/systems/MobSystem.hpp"
#include "../../ecs/include/systems/ForcePodSystem.hpp"
#include "../../ecs/include/systems/SpawnEffectSystem.hpp"
#include "../../ecs/include/components/Position.hpp"
#include "../../ecs/include/components/Velocity.hpp"
#include "../../ecs/include/components/Weapon.hpp"
#include "../../ecs/include/components/HitBox.hpp"
#include "../../ecs/include/components/Health.hpp"
#include "../../ecs/include/components/NetworkId.hpp"
#include "../../ecs/include/components/EnemySpawner.hpp"
#include "../../ecs/include/components/Score.hpp"
#include "../../ecs/include/components/Tag.hpp"
#include "../../ecs/include/components/Lives.hpp"
#include "../../ecs/include/systems/ScoreSystem.hpp"
#include "../../ecs/include/systems/LivesSystem.hpp"
#include "../../ecs/include/systems/ProjectileSystem.hpp"
#include "../../ecs/include/components/Projectile.hpp"
#include "../../ecs/include/components/MapBounds.hpp"
#include "../../ecs/include/components/CollisionLayer.hpp"
#include "../../shared/utils/Logger.hpp"
#include "../../shared/utils/GameConfig.hpp"
#include <unordered_set>
#include "Server.hpp"

#include "../shared/net/Protocol.hpp"
#include "../shared/net/ProtocolAdapter.hpp"
#include "../shared/net/MessageSerializer.hpp"
#include "../../shared/utils/Logger.hpp"
#include <chrono>
#include <iostream>
#include <vector>

namespace rtype::server {

Server::Server(uint16_t port) : port_(port), running_(false), next_session_id_(1) {
    io_context_ = std::make_unique<asio::io_context>();
    udp_server_ = std::make_unique<UdpServer>(*io_context_, port_);
    protocol_adapter_ = std::make_unique<rtype::net::ProtocolAdapter>();
    message_serializer_ = std::make_unique<rtype::net::MessageSerializer>();
}

Server::~Server() {
    stop();
}

void Server::start() {
    if (running_.load())
        return;

    running_ = true;
    udp_server_->set_message_handler([this](const std::string& ip, uint16_t port, const std::vector<uint8_t>& data) {
        handle_client_message(ip, port, data);
    });

    udp_server_->start();
    work_guard_.emplace(asio::make_work_guard(*io_context_));
    network_thread_ = std::thread(&Server::network_loop, this);

    Logger::instance().info("Server started on port " + std::to_string(port_));
}

void Server::stop() {
    if (!running_.load())
        return;

    running_ = false;
    Logger::instance().info("Server stopping...");

    std::vector<GameSession*> active_sessions;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        for (auto& [id, session] : sessions_) {
            if (session) {
                session->set_session_empty_callback(nullptr);
                active_sessions.push_back(session.get());
            }
        }
    }

    for (auto* session : active_sessions) {
        if (session)
            session->stop();
    }

    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        sessions_.clear();
        client_session_map_.clear();
    }

    if (udp_server_)
        udp_server_->stop();
    if (work_guard_.has_value()) {
        work_guard_->reset();
        work_guard_.reset();
    }
    if (io_context_)
        io_context_->stop();
    if (network_thread_.joinable())
        network_thread_.join();
}

void Server::run() {
    start();
    if (network_thread_.joinable())
        network_thread_.join();
}

void Server::network_loop() {
    if (io_context_) {
        io_context_->run();
    }
}

uint32_t Server::allocate_session_id() {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    while (sessions_.count(next_session_id_) > 0) {
        ++next_session_id_;
    }
    return next_session_id_++;
}

GameSession* Server::get_or_create_session(uint32_t session_id) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    auto it = sessions_.find(session_id);
    if (it != sessions_.end())
        return it->second.get();

    auto session = std::make_unique<GameSession>(session_id, *udp_server_, *protocol_adapter_, *message_serializer_);
    session->set_client_unmap_callback([this](const std::string& key) { unmap_client(key); });
    session->set_session_empty_callback([this](uint32_t id) { remove_session(id); });
    session->start();
    auto* raw_ptr = session.get();
    sessions_.emplace(session_id, std::move(session));
    Logger::instance().info("Created session " + std::to_string(session_id));
    return raw_ptr;
}

void Server::unmap_client(const std::string& client_key) {
    std::lock_guard<std::mutex> lock(sessions_mutex_);
    client_session_map_.erase(client_key);
}

void Server::remove_session(uint32_t session_id) {
    std::unique_ptr<GameSession> to_delete;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = sessions_.find(session_id);
        if (it != sessions_.end()) {
            if (it->second)
                it->second->set_session_empty_callback(nullptr);
            to_delete = std::move(it->second);
            sessions_.erase(it);
        }
        for (auto iter = client_session_map_.begin(); iter != client_session_map_.end();) {
            if (iter->second == session_id)
                iter = client_session_map_.erase(iter);
            else
                ++iter;
        }
    }

    if (to_delete)
        to_delete->stop();

    Logger::instance().info("Removed session " + std::to_string(session_id));
}

void Server::handle_client_message(const std::string& client_ip, uint16_t client_port,
                                   const std::vector<uint8_t>& data) {
    if (!protocol_adapter_ || !protocol_adapter_->validate(data)) {
        if (protocol_adapter_)
            Logger::instance().warn("Invalid packet from " + client_ip + ":" + std::to_string(client_port));
        return;
    }

    rtype::net::Packet packet = protocol_adapter_->deserialize(data);
    std::string client_key = client_ip + ":" + std::to_string(client_port);
    auto msg_type = static_cast<rtype::net::MessageType>(packet.header.message_type);

    if (msg_type == rtype::net::MessageType::ListRooms) {
        std::vector<rtype::net::RoomInfoData> rooms;
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            for (const auto& [id, session] : sessions_) {
                if (session) {
                    std::string room_name = "Room " + std::to_string(id);
                    auto it = session_names_.find(id);
                    if (it != session_names_.end() && !it->second.empty())
                        room_name = it->second;
                    rooms.emplace_back(id, static_cast<uint8_t>(session->client_count()), 4,
                                       session->is_running() ? 0 : 1, room_name);
                }
            }
        }
        for (const auto& room : rooms) {
            udp_server_->send(client_ip, client_port,
                              protocol_adapter_->serialize(message_serializer_->serialize_room_info(room)));
        }
        return;
    }

    if (msg_type == rtype::net::MessageType::CreateRoom) {
        auto create_data = message_serializer_->deserialize_create_room(packet);
        uint32_t new_session_id = allocate_session_id();
        std::string room_name(create_data.room_name);
        if (room_name.empty())
            room_name = "Room " + std::to_string(new_session_id);
        {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            session_names_[new_session_id] = room_name;
        }
        rtype::net::RoomInfoData room_info(new_session_id, 0, create_data.max_players, 0, room_name);
        udp_server_->send(client_ip, client_port,
                          protocol_adapter_->serialize(message_serializer_->serialize_room_info(room_info)));
        Logger::instance().info("Created room '" + room_name + "' with id " + std::to_string(new_session_id));
        return;
    }

    if (msg_type == rtype::net::MessageType::PlayerJoin) {
        uint32_t target_session = 0;
        if (packet.header.payload_size == packet.body.size()) {
            auto join = message_serializer_->deserialize_player_join(packet);
            target_session = join.session_id;
        }
        if (target_session == 0)
            target_session = allocate_session_id();

        GameSession* session = get_or_create_session(target_session);
        if (!session)
            return;
        bool accepted = session->handle_player_join(client_ip, client_port, packet);
        if (accepted) {
            std::lock_guard<std::mutex> lock(sessions_mutex_);
            client_session_map_[client_key] = target_session;
        }
        return;
    }

    uint32_t session_id = 0;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = client_session_map_.find(client_key);
        if (it != client_session_map_.end())
            session_id = it->second;
    }

    if (session_id == 0) {
        Logger::instance().warn("Dropping message from " + client_key + " with no session mapping");
        return;
    }

    GameSession* session = nullptr;
    {
        std::lock_guard<std::mutex> lock(sessions_mutex_);
        auto it = sessions_.find(session_id);
        if (it != sessions_.end())
            session = it->second.get();
    }

    if (!session) {
        Logger::instance().warn("Session " + std::to_string(session_id) + " not found for client " + client_key);
        unmap_client(client_key);
        return;
    }

    session->handle_packet(client_ip, client_port, packet);
}

} // namespace rtype::server
