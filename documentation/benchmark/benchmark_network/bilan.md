# Benchmark Réseau pour R-Type

## Contexte

Le sujet R-Type nécessite un **système réseau performant** pour gérer le multijoueur en temps réel. Ce benchmark compare quatre solutions : ASIO, raw sockets, ENet et SDL_net.

## Comparaison

### ASIO (Boost.Asio / standalone)
- **Avantages** : Async I/O puissant, cross-platform, header-only (standalone), très utilisé (industrie)
- **Inconvénients** : API complexe au début, verbeux pour cas simples

### Raw Sockets (POSIX)
- **Avantages** : Performance maximale, contrôle total, zéro dépendance
- **Inconvénients** : Pas cross-platform, beaucoup de code boilerplate, gestion manuelle des erreurs

### ENet
- **Avantages** : Fiable UDP (reliable + sequencing), API simple, optimisé pour jeux
- **Inconvénients** : Moins flexible, overhead pour reliable UDP, dépendance supplémentaire

### SDL_net
- **Avantages** : Simple, intégré avec SDL, bon pour prototypes
- **Inconvénients** : Pas async natif, performance limitée, moins de contrôle

## Résultats (10,000 paquets UDP 64 bytes, localhost)

```
ASIO:        ~15 ms
Raw Sockets: ~12 ms
ENet:        ~20 ms
SDL_net:     ~25 ms
```

Raw sockets est le plus rapide, mais ASIO offre le meilleur ratio **performance/maintenabilité/features**.

## Recommandation

**✅ ASIO** est recommandé pour R-Type car :
- **Async I/O** : gère multiples clients sans bloquer (essentiel pour serveur)
- **Cross-platform** : Windows (IOCP), Linux (epoll), macOS (kqueue)
- **Header-only** (standalone) : pas de link, facile à intégrer
- **Scalabilité** : peut gérer des centaines de connexions simultanées
- **UDP + TCP** : flexibilité pour game state (UDP) et lobby/chat (TCP)
- **Utilisé en production** : beaucoup de jeux et serveurs réels

## Installation

```bash
# Standalone ASIO (recommandé, pas besoin de Boost)
git clone https://github.com/chriskohlhoff/asio.git
# Header-only : asio/asio/include/

# Ou via vcpkg
vcpkg install asio

# Ou via conan
conan install asio/1.28.0
```

## Exemple d'utilisation R-Type

```cpp
#include <asio.hpp>
using asio::ip::udp;

// Serveur UDP async
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
                    // Traiter le paquet (player input, etc.)
                    handle_packet(recv_buffer_.data(), bytes);
                    start_receive(); // Continuer à recevoir
                }
            });
    }

    void handle_packet(const char* data, size_t size) {
        // Parser et traiter les inputs des joueurs
    }
};

int main() {
    asio::io_context io_context;
    UdpServer server(io_context, 4242);
    io_context.run(); // Event loop
}
```

## Comment tester

```bash
chmod +x test.sh
./test.sh
```

Le script compile et exécute tous les benchmarks. Raw sockets fonctionne toujours, les autres nécessitent leurs bibliothèques respectives.

## Références

- [ASIO Documentation](https://think-async.com/Asio/)
- [ASIO GitHub](https://github.com/chriskohlhoff/asio)
- [Game Networking Resources](https://gafferongames.com/)
