#!/bin/bash

# Change to the script's directory
cd "$(dirname "$0")"

echo "=== Benchmark Graphique R-Type ==="
echo "Comparaison SFML vs Raylib vs SDL2"
echo "Répertoire: $(pwd)"
echo ""

# Check dependencies
echo "Vérification des dépendances..."

# SFML
if pkg-config --exists sfml-graphics; then
    echo "✅ SFML trouvé"
    SFML_AVAILABLE=true
else
    echo "❌ SFML non trouvé (sudo apt install libsfml-dev)"
    SFML_AVAILABLE=false
fi

# Raylib
if command -v raylib >/dev/null 2>&1 || ldconfig -p | grep -q raylib; then
    echo "✅ Raylib trouvé"
    RAYLIB_AVAILABLE=true
else
    echo "❌ Raylib non trouvé (sudo apt install libraylib-dev)"
    RAYLIB_AVAILABLE=false
fi

# SDL2
if pkg-config --exists sdl2; then
    echo "✅ SDL2 trouvé"
    # Check for SDL2_ttf separately
    if pkg-config --exists SDL2_ttf; then
        SDL2_LIBS="$(pkg-config --cflags --libs sdl2 SDL2_ttf)"
        echo "  ✅ SDL2_ttf trouvé"
    else
        SDL2_LIBS="$(pkg-config --cflags --libs sdl2)"
        echo "  ⚠️  SDL2_ttf non trouvé - demo avec limitations texte"
    fi
    SDL2_AVAILABLE=true
else
    echo "❌ SDL2 non trouvé (sudo apt install libsdl2-dev libsdl2-ttf-dev)"
    SDL2_AVAILABLE=false
fi

echo ""

# Compile and run benchmarks
if [ "$SFML_AVAILABLE" = true ]; then
    echo "=== Compilation SFML ==="
    g++ -std=c++17 -O3 bench_sfml.cpp $(pkg-config --cflags --libs sfml-graphics sfml-window sfml-system) -o bench_sfml
    if [ $? -eq 0 ]; then
        echo "✅ Compilation réussie"
        echo ""
        echo "=== Test SFML ==="
        ./bench_sfml
        echo ""
    else
        echo "❌ Erreur de compilation SFML"
    fi
fi

if [ "$RAYLIB_AVAILABLE" = true ]; then
    echo "=== Compilation Raylib ==="
    g++ -std=c++17 -O3 bench_raylib.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o bench_raylib 2>/dev/null
    if [ $? -eq 0 ]; then
        echo "✅ Compilation réussie"
        echo ""
        echo "=== Test Raylib ==="
        timeout 5s ./bench_raylib
        echo ""
    else
        echo "❌ Erreur de compilation Raylib (bibliothèque non installée)"
    fi
fi

if [ "$SDL2_AVAILABLE" = true ]; then
    echo "=== Compilation SDL2 ==="
    g++ -std=c++17 -O3 bench_sdl2.cpp -lSDL2 -o bench_sdl2
    if [ $? -eq 0 ]; then
        echo "✅ Compilation réussie"
        echo ""
        echo "=== Test SDL2 ==="
        timeout 5s ./bench_sdl2
        echo ""
    else
        echo "❌ Erreur de compilation SDL2"
    fi
fi

echo "=== UI Demo terminé ==="
echo ""
echo "Résumé des recommandations :"
echo "✅ SFML recommandé pour R-Type"
echo "   - API C++ moderne et intuitive (90 lignes)"
echo "   - Classes nationales pour UI"
echo "   - Gestion automatique des ressources"
echo ""
echo "⚠️  Raylib: Simple mais verbeux (100 lignes C)"
echo "   - Structs manuels pour boutons"
echo "   - Collision detection manuelle"
echo ""
echo "❌ SDL2: Très verbeux (160+ lignes)"
echo "   - Gestion manuelle de tout"
echo "   - Dépendances supplémentaires (SDL_ttf)"
echo ""
echo "Voir bilan.md pour l'analyse détaillée."
