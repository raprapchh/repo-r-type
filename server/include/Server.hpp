#pragma once

#include "UdpServer.hpp"
#include "../../ecs/include/Registry.hpp"
#include "../../ecs/include/SystemManager.hpp"
#include "../../shared/interfaces/network/IProtocolAdapter.hpp"
#include "../../shared/interfaces/network/IMessageSerializer.hpp"
#include "../../shared/net/Packet.hpp"
#include <atomic>
#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_set>

namespace rtype::server {

/// @brief Connected client information
struct ClientInfo {
    std::string ip;
    uint16_t port;
    uint32_t player_id;
    std::string player_name; // Custom player name
    bool is_connected;
    GameEngine::entity_t entity_id;
    std::chrono::steady_clock::time_point last_seen;
};

/// @brief Authoritative game server for R-Type multiplayer
class Server {
  public:
    Server(GameEngine::Registry& registry, uint16_t port = 4242);
    ~Server();

    void start();
    void stop();
    void run();

  private:
    void handle_client_message(const std::string& client_ip, uint16_t client_port, const std::vector<uint8_t>& data);
    void handle_player_join(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_player_name(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_player_move(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_player_shoot(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_game_start(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void handle_map_resize(const std::string& client_ip, uint16_t client_port, const rtype::net::Packet& packet);
    void broadcast_message(const std::vector<uint8_t>& data, const std::string& exclude_ip = "",
                           uint16_t exclude_port = 0);
    void game_loop();
    void network_loop();

    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<UdpServer> udp_server_;
    std::unique_ptr<rtype::net::IProtocolAdapter> protocol_adapter_;
    std::unique_ptr<rtype::net::IMessageSerializer> message_serializer_;
    std::map<std::string, ClientInfo> clients_;
    std::mutex clients_mutex_;
    std::mutex registry_mutex_;
    uint32_t next_player_id_;
    std::atomic<bool> running_;
    std::atomic<bool> game_started_;
    std::atomic<bool> game_over_;
    std::thread game_thread_;
    std::thread network_thread_;
    std::optional<asio::executor_work_guard<asio::io_context::executor_type>> work_guard_;
    static constexpr double TARGET_TICK_RATE = 60.0;
    static constexpr std::chrono::milliseconds TICK_DURATION =
        std::chrono::milliseconds(static_cast<long>(1000.0 / TARGET_TICK_RATE));
    static constexpr std::chrono::seconds CLIENT_TIMEOUT_DURATION = std::chrono::seconds(10);

    GameEngine::Registry& registry_;
    GameEngine::SystemManager system_manager_;

    void check_client_timeouts();
    void disconnect_client(const std::string& client_key, const ClientInfo& client);

    // Network sync helpers
    void broadcast_game_state(std::chrono::milliseconds elapsed);
    void broadcast_entity_positions();
    void broadcast_enemy_spawns();
    void broadcast_to_all_clients(const std::vector<uint8_t>& data);
    void broadcast_entity_destroy(uint32_t entity_id, uint8_t reason);
    std::unordered_set<uint32_t> collect_non_player_network_ids();
    void broadcast_projectile_spawns();
    void send_existing_entities_to_client(const std::string& client_ip, uint16_t client_port);
    void send_entity_spawn(const std::string& ip, uint16_t port, uint32_t entity_id, uint16_t entity_type,
                           uint16_t sub_type, float x, float y, float vx, float vy);
    uint16_t get_monster_subtype(const std::string& tag_name);
    uint16_t get_projectile_subtype(const std::string& tag_name);
    GameEngine::entity_t create_player_entity(uint32_t player_id, const std::string& player_name);
};

} // namespace rtype::server
