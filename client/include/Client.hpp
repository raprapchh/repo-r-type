#pragma once

#include <asio.hpp>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "../shared/net/Protocol.hpp"
#include "NetworkSystem.hpp"
#include "UdpClient.hpp"
#include "Renderer.hpp"
#include "../../ecs/include/systems/AudioSystem.hpp"

namespace rtype::client {

class Client {
  public:
    using GameStartCallback = std::function<void()>;
    using PlayerJoinCallback = std::function<void(uint32_t)>;

    Client(const std::string& host, uint16_t port, Renderer& renderer);
    ~Client();

    void connect();
    void disconnect();
    void run();
    void update(double dt);

    void send_move(float vx, float vy);
    void send_shoot(int32_t x, int32_t y);
    void send_game_start_request();
    void send_map_resize(float width, float height);

    void set_game_start_callback(GameStartCallback callback);
    void set_player_join_callback(PlayerJoinCallback callback);
    bool is_connected() const {
        return connected_.load();
    }
    uint32_t get_player_id() const {
        return player_id_;
    }
    GameEngine::Registry& get_registry() {
        return registry_;
    }
    std::mutex& get_registry_mutex() {
        return registry_mutex_;
    }

  private:
    void handle_udp_receive(const asio::error_code& error, std::size_t bytes_transferred,
                            const std::vector<uint8_t>& data);
    void handle_server_message(const std::vector<uint8_t>& data);

    std::string host_;
    uint16_t port_;
    std::unique_ptr<asio::io_context> io_context_;
    std::unique_ptr<std::thread> network_thread_;
    std::unique_ptr<UdpClient> udp_client_;
    std::atomic<bool> connected_;
    uint32_t player_id_;

    Renderer& renderer_;

    GameStartCallback game_start_callback_;
    PlayerJoinCallback player_join_callback_;

    GameEngine::Registry registry_;
    NetworkSystem network_system_;
    rtype::ecs::AudioSystem audio_system_;
    std::mutex registry_mutex_;
};

} // namespace rtype::client
