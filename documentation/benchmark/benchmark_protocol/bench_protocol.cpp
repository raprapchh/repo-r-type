#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <numeric>
#include <thread>

namespace bench {
using Clock = std::chrono::steady_clock;
constexpr std::size_t kIterations = 2000;
constexpr std::size_t kPayloadSize = 392;
using Buffer = std::array<std::uint8_t, kPayloadSize>;

struct Stats {
    double sendNs = 0.0;
    double roundTripNs = 0.0;
    double connectNs = 0.0;
};

[[noreturn]] void fatal(const char* label) {
    std::perror(label);
    std::exit(1);
}

Buffer makePayload() {
    Buffer buf{};
    std::iota(buf.begin(), buf.end(), 0);
    return buf;
}

void runServer(int fd, int type) {
    Buffer tmp{};

    if (type == SOCK_DGRAM) {
        sockaddr_in client{};
        socklen_t len = sizeof(client);
        for (std::size_t i = 0; i < kIterations; ++i) {
            if (::recvfrom(fd, tmp.data(), tmp.size(), 0, reinterpret_cast<sockaddr*>(&client), &len) !=
                static_cast<ssize_t>(tmp.size())) {
                break;
            }
            ::sendto(fd, tmp.data(), tmp.size(), 0, reinterpret_cast<sockaddr*>(&client), len);
        }
        ::close(fd);
        return;
    }

    sockaddr_in client{};
    socklen_t len = sizeof(client);
    int conn = ::accept(fd, reinterpret_cast<sockaddr*>(&client), &len);
    if (conn < 0) {
        fatal("accept");
    }
    for (std::size_t i = 0; i < kIterations; ++i) {
        if (::recv(conn, tmp.data(), tmp.size(), MSG_WAITALL) != static_cast<ssize_t>(tmp.size())) {
            fatal("tcp recv");
        }
        if (::send(conn, tmp.data(), tmp.size(), 0) != static_cast<ssize_t>(tmp.size())) {
            fatal("tcp send");
        }
    }
    ::close(conn);
    ::close(fd);
}

template <typename SendFunc, typename RecvFunc> Stats runBenchmark(int type, SendFunc sendFunc, RecvFunc recvFunc) {
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(0);

    int server = ::socket(AF_INET, type, 0);
    if (server < 0 || ::bind(server, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        fatal("server");
    }
    if (type == SOCK_STREAM && ::listen(server, 1) < 0) {
        fatal("listen");
    }

    socklen_t len = sizeof(addr);
    if (::getsockname(server, reinterpret_cast<sockaddr*>(&addr), &len) < 0) {
        fatal("getsockname");
    }

    int client = ::socket(AF_INET, type, 0);
    if (client < 0) {
        fatal("client");
    }

    std::thread serverThread(runServer, server, type);

    Stats stats{};
    Buffer payload = makePayload();
    Buffer buffer{};

    auto connectStart = Clock::now();
    if (::connect(client, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        fatal("connect");
    }
    if (type == SOCK_STREAM) {
        stats.connectNs = std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now() - connectStart).count();
    }

    double sendTotal = 0.0;
    double roundTripTotal = 0.0;
    for (std::size_t i = 0; i < kIterations; ++i) {
        auto sendStart = Clock::now();
        sendFunc(client, payload);
        auto sendEnd = Clock::now();

        recvFunc(client, buffer);
        auto recvEnd = Clock::now();

        sendTotal += std::chrono::duration_cast<std::chrono::nanoseconds>(sendEnd - sendStart).count();
        roundTripTotal += std::chrono::duration_cast<std::chrono::nanoseconds>(recvEnd - sendStart).count();
    }

    ::close(client);
    serverThread.join();

    stats.sendNs = sendTotal / static_cast<double>(kIterations);
    stats.roundTripNs = roundTripTotal / static_cast<double>(kIterations);
    return stats;
}

void print(const char* label, const Stats& stats) {
    std::cout << label << " average send time: " << stats.sendNs << " ns per packet\n";
    std::cout << label << " average round-trip time: " << stats.roundTripNs << " ns per packet\n";
    if (stats.connectNs > 0.0) {
        std::cout << label << " connect time: " << stats.connectNs << " ns\n";
    }
    std::cout << label << " payload size: " << kPayloadSize << " bytes\n";
}
} // namespace bench

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <udp|tcp>\n";
        return 1;
    }

    auto udpSend = [](int fd, const bench::Buffer& data) {
        if (::send(fd, data.data(), data.size(), 0) != static_cast<ssize_t>(data.size())) {
            bench::fatal("udp send");
        }
    };
    auto udpRecv = [](int fd, bench::Buffer& out) {
        if (::recv(fd, out.data(), out.size(), 0) != static_cast<ssize_t>(out.size())) {
            bench::fatal("udp recv");
        }
    };
    auto tcpSend = [](int fd, const bench::Buffer& data) {
        if (::send(fd, data.data(), data.size(), 0) != static_cast<ssize_t>(data.size())) {
            bench::fatal("tcp send");
        }
    };
    auto tcpRecv = [](int fd, bench::Buffer& out) {
        if (::recv(fd, out.data(), out.size(), MSG_WAITALL) != static_cast<ssize_t>(out.size())) {
            bench::fatal("tcp recv");
        }
    };

    if (std::strcmp(argv[1], "udp") == 0) {
        bench::print("UDP", bench::runBenchmark(SOCK_DGRAM, udpSend, udpRecv));
    } else if (std::strcmp(argv[1], "tcp") == 0) {
        bench::print("TCP", bench::runBenchmark(SOCK_STREAM, tcpSend, tcpRecv));
    } else {
        std::cerr << "unknown mode\n";
        return 1;
    }
    return 0;
}
