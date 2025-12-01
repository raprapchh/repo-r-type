#include <iostream>
#include <chrono>
#include <vector>
#include <SDL2/SDL_net.h>

int main() {
    const int NUM_PACKETS = 10000;
    const int PACKET_SIZE = 64;

    if (SDLNet_Init() < 0) {
        std::cerr << "SDL_net init failed\n";
        return 1;
    }

    auto start = std::chrono::high_resolution_clock::now();

    UDPsocket sock = SDLNet_UDP_Open(0);

    IPaddress dest_addr;
    SDLNet_ResolveHost(&dest_addr, "127.0.0.1", 4242);

    UDPpacket* packet = SDLNet_AllocPacket(PACKET_SIZE);
    packet->address = dest_addr;
    packet->len = PACKET_SIZE;

    std::vector<char> buffer(PACKET_SIZE, 'X');

    for (int i = 0; i < NUM_PACKETS; ++i) {
        std::memcpy(packet->data, buffer.data(), PACKET_SIZE);
        SDLNet_UDP_Send(sock, -1, packet);
    }

    SDLNet_FreePacket(packet);
    SDLNet_UDP_Close(sock);
    SDLNet_Quit();

    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "SDL_net: " << total << " ms (" << NUM_PACKETS << " packets)\n";
    return 0;
}
