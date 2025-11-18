#include <iostream>
#include <chrono>
#include <enet/enet.h>

int main() {
    const int NUM_PACKETS = 10000;
    const int PACKET_SIZE = 64;

    if (enet_initialize() != 0) {
        std::cerr << "ENet init failed\n";
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

    ENetHost* client = enet_host_create(nullptr, 1, 2, 0, 0);

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 4242;

    ENetPeer* peer = enet_host_connect(client, &address, 2, 0);

    std::vector<char> buffer(PACKET_SIZE, 'X');

    for (int i = 0; i < NUM_PACKETS; ++i) {
        ENetPacket* packet = enet_packet_create(buffer.data(), buffer.size(), 0);
        enet_peer_send(peer, 0, packet);
    }

    enet_host_flush(client);
    enet_host_destroy(client);
    enet_deinitialize();

    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "ENet: " << total << " ms (" << NUM_PACKETS << " packets)\n";
    return 0;
}
