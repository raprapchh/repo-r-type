# Benchmark ECS pour R-Type

## Contexte

Le sujet R-Type (page 13) recommande l'utilisation du pattern **Entity-Component-System** pour le moteur de jeu. Ce benchmark compare deux approches : EnTT (bibliothèque optimisée) vs une implémentation simple custom.

## Comparaison

### EnTT
- **Avantages** : Header-only, très rapide (cache-friendly), facile à intégrer avec CMake
- **Inconvénients** : Courbe d'apprentissage modérée

### Custom Simple ECS
- **Avantages** : Contrôle total, éducatif, pas de dépendance
- **Inconvénients** : Performance limitée, plus de maintenance

## Résultats (10,000 entités, 100 itérations)

```
EnTT:          ~10 ms
Custom Simple: ~50 ms
```

EnTT est **~5x plus rapide** que l'implémentation simple.

## Recommandation

**✅ EnTT** est recommandé pour R-Type car :
- Performance essentielle pour jeu réseau en temps réel (60 FPS)
- Stockage contigu des composants → facilite la sérialisation réseau
- Header-only → intégration facile (Conan, vcpkg, CMake)
- Permet le découplage requis (Rendering, Physics, Network systems)

## Installation

```bash
# Conan
conan install entt/3.13.0

# vcpkg
vcpkg install entt
```

## Exemple d'utilisation R-Type

```cpp
entt::registry registry;

// Créer une entité
auto enemy = registry.create();
registry.emplace<Position>(enemy, 800.0f, 300.0f);
registry.emplace<Velocity>(enemy, -2.0f, 0.0f);
registry.emplace<Sprite>(enemy, enemyTexture);

// Système de mouvement
auto view = registry.view<Position, Velocity>();
for(auto entity : view) {
    auto [pos, vel] = view.get<Position, Velocity>(entity);
    pos.x += vel.dx;
    pos.y += vel.dy;
}
```

## Comment tester

```bash
# Compiler avec EnTT
g++ -std=c++17 -O3 -I/path/to/entt/include bench_entt.cpp -o bench_entt

# Compiler custom simple (pas de dépendance)
g++ -std=c++17 -O3 bench_custom_simple.cpp -o bench_custom_simple

# Exécuter
./bench_entt
./bench_custom_simple
```

## Références

- [EnTT GitHub](https://github.com/skypjack/entt)
- [Game Programming Patterns - Component](https://gameprogrammingpatterns.com/component.html)
