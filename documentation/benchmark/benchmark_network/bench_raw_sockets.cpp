#include <iostream>
#include <chrono>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

int main() {
    const int NUM_PACKETS = 10000;
    const int PACKET_SIZE = 64;

    auto start = std::chrono::high_resolution_clock::now();

    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in dest_addr{};
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(4242);
    inet_pton(AF_INET, "127.0.0.1", &dest_addr.sin_addr);

    std::vector<char> buffer(PACKET_SIZE, 'X');

    for (int i = 0; i < NUM_PACKETS; ++i) {
        sendto(sock, buffer.data(), buffer.size(), 0,
               (sockaddr*)&dest_addr, sizeof(dest_addr));
    }

    close(sock);

    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Raw Sockets: " << total << " ms (" << NUM_PACKETS << " packets)\n";
    return 0;
}
