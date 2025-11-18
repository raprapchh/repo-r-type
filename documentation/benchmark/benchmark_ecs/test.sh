#!/bin/bash

echo "=== Compilation du benchmark ECS ==="
echo

# Custom simple (pas de dépendance)
echo "[1/2] Compilation Custom Simple ECS..."
g++ -std=c++17 -O3 bench_custom_simple.cpp -o bench_custom_simple
if [ $? -eq 0 ]; then
    echo "  ✓ Custom Simple compilé"
else
    echo "  ✗ Erreur compilation Custom Simple"
    exit 1
fi

# EnTT - télécharge si pas présent
echo "[2/2] Compilation EnTT ECS..."
if [ ! -d "entt" ]; then
    echo "  → Téléchargement d'EnTT..."
    git clone --depth 1 https://github.com/skypjack/entt.git
fi

g++ -std=c++17 -O3 -I./entt/single_include bench_entt.cpp -o bench_entt
if [ $? -eq 0 ]; then
    echo "  ✓ EnTT compilé"
    ENTT_COMPILED=0
else
    echo "  ✗ Erreur compilation EnTT"
    ENTT_COMPILED=1
fi

echo
echo "=== Exécution des benchmarks ==="
echo

if [ $ENTT_COMPILED -eq 0 ]; then
    echo "EnTT:"
    ./bench_entt
    echo
fi

echo "Custom Simple:"
./bench_custom_simple
echo

echo "=== Fin ==="
echo "Voir bilan.md pour l'analyse complète"
