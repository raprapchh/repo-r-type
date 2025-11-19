# Network Benchmark for R-Type

## Context

The R-Type subject requires a **high-performance networking system** to manage multiplayer in real-time. This benchmark compares four solutions: ASIO, raw sockets, ENet, and SDL_net.

## Comparison

### ASIO (Boost.Asio / standalone)
- **Advantages**: Powerful async I/O, cross-platform, header-only (standalone), widely used (industry)
- **Disadvantages**: Complex API initially, verbose for simple cases

### Raw Sockets (POSIX)
- **Advantages**: Maximum performance, total control, zero dependencies
- **Disadvantages**: Not cross-platform, lots of boilerplate code, manual error handling

### ENet
- **Advantages**: Reliable UDP (reliable + sequencing), simple API, optimized for games
- **Disadvantages**: Less flexible, overhead for reliable UDP, additional dependency

### SDL_net
- **Advantages**: Simple, integrated with SDL, good for prototypes
- **Disadvantages**: No native async, limited performance, less control

## Results (10,000 UDP packets 64 bytes, localhost)

```
ASIO:        ~15 ms
Raw Sockets: ~12 ms
ENet:        ~20 ms
SDL_net:     ~25 ms
```

Raw sockets is the fastest, but ASIO offers the best ratio **performance/maintainability/features**.

## Recommendation

**✅ ASIO** is recommended for R-Type because:
- **Async I/O**: handles multiple clients without blocking (essential for server)
- **Cross-platform**: Windows (IOCP), Linux (epoll), macOS (kqueue)
- **Header-only** (standalone): no linking, easy to integrate
- **Scalability**: can handle hundreds of simultaneous connections
- **UDP + TCP**: flexibility for game state (UDP) and lobby/chat (TCP)
- **Production-ready**: used by many real games and servers

## Installation

```bash
# Standalone ASIO (recommended, no need for Boost)
git clone https://github.com/chriskohlhoff/asio.git
# Header-only: asio/asio/include/

# Or via vcpkg
vcpkg install asio

# Or via conan
conan install asio/1.28.0
```

## R-Type Usage Example

```cpp
#include <asio.hpp>
using asio::ip::udp;

// Async UDP server
class UdpServer {
    asio::io_context& io_context_;
    udp::socket socket_;
    udp::endpoint remote_endpoint_;
    std::array<char, 1024> recv_buffer_;

public:
    UdpServer(asio::io_context& io_context, short port)
        : io_context_(io_context),
          socket_(io_context, udp::endpoint(udp::v4(), port)) {
        start_receive();
    }

    void start_receive() {
        socket_.async_receive_from(
            asio::buffer(recv_buffer_), remote_endpoint_,
            [this](std::error_code ec, std::size_t bytes) {
                if (!ec) {
                    // Process packet (player input, etc.)
                    handle_packet(recv_buffer_.data(), bytes);
                    start_receive(); // Continue receiving
                }
            });
    }

    void handle_packet(const char* data, size_t size) {
        // Parse and process player inputs
    }
};

int main() {
    asio::io_context io_context;
    UdpServer server(io_context, 4242);
    io_context.run(); // Event loop
}
```

## How to Test

```bash
chmod +x test.sh
./test.sh
```

The script compiles and executes all benchmarks. Raw sockets always works, the others require their respective libraries.

## References

- [ASIO Documentation](https://think-async.com/Asio/)
- [ASIO GitHub](https://github.com/chriskohlhoff/asio)
- [Game Networking Resources](https://gafferongames.com/)
