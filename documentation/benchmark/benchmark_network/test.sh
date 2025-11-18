#!/bin/bash

echo "=== Compilation du benchmark Réseau ==="
echo

# Raw sockets (toujours disponible sur Linux)
echo "[1/4] Compilation Raw Sockets..."
g++ -std=c++17 -O3 bench_raw_sockets.cpp -o bench_raw_sockets
if [ $? -eq 0 ]; then
    echo "  ✓ Raw Sockets compilé"
    RAW_COMPILED=0
else
    echo "  ✗ Erreur compilation Raw Sockets"
    RAW_COMPILED=1
fi

# ASIO - télécharge si pas présent
echo "[2/4] Compilation ASIO..."
if [ ! -d "asio" ]; then
    echo "  → Téléchargement d'ASIO standalone..."
    git clone --depth 1 https://github.com/chriskohlhoff/asio.git
fi

g++ -std=c++17 -O3 -I./asio/asio/include -pthread bench_asio.cpp -o bench_asio
if [ $? -eq 0 ]; then
    echo "  ✓ ASIO compilé"
    ASIO_COMPILED=0
else
    echo "  ✗ Erreur compilation ASIO"
    ASIO_COMPILED=1
fi

# ENet
echo "[3/4] Compilation ENet..."
g++ -std=c++17 -O3 bench_enet.cpp -o bench_enet -lenet
if [ $? -eq 0 ]; then
    echo "  ✓ ENet compilé"
    ENET_COMPILED=0
else
    echo "  ✗ ENet non disponible (sudo apt install libenet-dev)"
    ENET_COMPILED=1
fi

# SDL_net
echo "[4/4] Compilation SDL_net..."
g++ -std=c++17 -O3 bench_sdl_net.cpp -o bench_sdl_net -lSDL2_net
if [ $? -eq 0 ]; then
    echo "  ✓ SDL_net compilé"
    SDL_NET_COMPILED=0
else
    echo "  ✗ SDL_net non disponible (sudo apt install libsdl2-net-dev)"
    SDL_NET_COMPILED=1
fi

echo
echo "=== Exécution des benchmarks ==="
echo

if [ $ASIO_COMPILED -eq 0 ]; then
    echo "ASIO:"
    ./bench_asio 2>/dev/null || echo "  (Erreur runtime - port occupé?)"
    echo
fi

if [ $RAW_COMPILED -eq 0 ]; then
    echo "Raw Sockets:"
    ./bench_raw_sockets 2>/dev/null || echo "  (Erreur runtime - port occupé?)"
    echo
fi

if [ $ENET_COMPILED -eq 0 ]; then
    echo "ENet:"
    ./bench_enet 2>/dev/null || echo "  (Erreur runtime - connexion échouée)"
    echo
fi

if [ $SDL_NET_COMPILED -eq 0 ]; then
    echo "SDL_net:"
    ./bench_sdl_net 2>/dev/null || echo "  (Erreur runtime)"
    echo
fi

echo "=== Fin ==="
echo "Voir bilan.md pour l'analyse complète"
