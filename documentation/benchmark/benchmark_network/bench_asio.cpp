#include <iostream>
#include <chrono>
#include <asio.hpp>

using asio::ip::udp;

int main() {
    const int NUM_PACKETS = 10000;
    const int PACKET_SIZE = 64; // Taille typique pour R-Type (position, velocity)

    auto start = std::chrono::high_resolution_clock::now();

    asio::io_context io_context;
    udp::socket socket(io_context, udp::endpoint(udp::v4(), 0));
    udp::endpoint destination(asio::ip::address::from_string("127.0.0.1"), 4242);

    std::vector<char> buffer(PACKET_SIZE, 'X');

    for (int i = 0; i < NUM_PACKETS; ++i) {
        socket.send_to(asio::buffer(buffer), destination);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto total = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "ASIO: " << total << " ms (" << NUM_PACKETS << " packets)\n";
    return 0;
}
