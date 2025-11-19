# Graphics Benchmark for R-Type

## Context

R-Type requires a **high-performance graphics engine** to handle 2D real-time rendering with sprites, animations, and visual effects. This benchmark compares three popular solutions: SFML, Raylib, and SDL2.

## Comparison

### SFML (Simple and Fast Multimedia Library)
- **Advantages**:
  - Modern and intuitive C++ API (object-oriented)
  - Optimized for 2D with OpenGL backend
  - Native sprite, texture, and animation management
  - Integrated audio (Music, Sound)
  - Integrated networking (TCP/UDP) - synergy with R-Type architecture
  - Cross-platform (Windows, Linux, macOS)
  - Excellent documentation and active community
- **Disadvantages**:
  - Heavier than Raylib
  - Primarily 2D-focused

### Raylib
- **Advantages**:
  - Simple and clean C API
  - Very lightweight and easy to learn
  - Native 2D and 3D
  - No external dependencies
- **Disadvantages**:
  - C API (not object-oriented)
  - Fewer advanced features
  - Smaller community
  - No integrated networking

### SDL2 (Simple DirectMedia Layer)
- **Advantages**:
  - Industry standard (used by Steam, etc.)
  - Excellent raw performance
  - Low-level control
  - Excellent multi-platform support
- **Disadvantages**:
  - Low-level C API (verbose)
  - Much boilerplate for 2D
  - No native sprite/animation management
  - Requires extensions (SDL_image, SDL_mixer, etc.)

## Results (Simple UI menu: Title + 3 buttons)

```
SFML:   ~90 lines of code (Button class C++)
Raylib: ~100 lines of code (Button struct + C functions)
SDL2:   ~160 lines of code (Manual + SDL_ttf required)
```

**SFML** provides the most concise and elegant API for creating user interfaces, while **SDL2** requires the most boilerplate code.

## Performance Analysis

The results show that **SFML** offers the best development experience for creating user interfaces. All frameworks can create the same menu, but with very different complexities:

- **SFML**: Natural object-oriented API, integrated Button classes
- **Raylib**: Simple C API but requires more manual code
- **SDL2**: Very verbose, requires SDL_ttf for text, complete manual management

## Recommendation

**✅ SFML** is recommended for R-Type because:

### UI Development Simplicity
- **90 lines** for a complete menu with hover effects
- Object-oriented C++ API (sf::Button, sf::Text, sf::RectangleShape)
- Automatic resource management with RAII
- Readable and maintainable code

### Perfect API for R-Type
- **sf::Sprite**: native visual entity management
- **sf::RenderWindow**: intuitive game loop
- **sf::View**: easy camera and scrolling
- **sf::Clock/Time**: precise timing for 60 FPS

### Architecture Integration
- **SFML Networking**: can complement ASIO for certain uses
- **Object-oriented C++**: integrates perfectly with ECS/EnTT
- **RAII**: automatic resource management

### Ecosystem
- **CMake/Conan**: simple integration
- **Cross-platform**: Windows, Linux, macOS
- **Documentation**: excellent tutorials and R-Type examples

## Installation

```bash
# Ubuntu/Debian
sudo apt install libsfml-dev

# Conan
conan install sfml/2.6.1

# vcpkg
vcpkg install sfml

# Arch Linux
sudo pacman -S sfml
```

## R-Type Usage Example

```cpp
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type");
    window.setFramerateLimit(60);

    // Load resources
    sf::Texture playerTexture, enemyTexture;
    playerTexture.loadFromFile("assets/player.png");
    enemyTexture.loadFromFile("assets/enemy.png");

    sf::Music backgroundMusic;
    backgroundMusic.openFromFile("assets/music.ogg");
    backgroundMusic.play();

    // Game loop
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Time deltaTime = clock.restart();

        // Events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        // Update (ECS systems here)
        updateSystems(deltaTime);

        // Render
        window.clear();
        renderSystems(window); // Uses sf::Sprite from components
        window.display();
    }

    return 0;
}
```

## Synergy with R-Type Architecture

```cpp
// SFML Component for ECS
struct SpriteComponent {
    sf::Sprite sprite;
    sf::Texture* texture;
    int layer = 0;
};

// Render System
class RenderSystem {
    sf::RenderWindow& window;

public:
    void update(entt::registry& registry) {
        auto view = registry.view<SpriteComponent, Position>();

        // Sort by layer for Z-order
        std::vector<entt::entity> entities(view.begin(), view.end());
        std::sort(entities.begin(), entities.end(), [&](auto a, auto b) {
            return view.get<SpriteComponent>(a).layer <
                   view.get<SpriteComponent>(b).layer;
        });

        // Optimized rendering
        for (auto entity : entities) {
            auto [sprite, pos] = view.get<SpriteComponent, Position>(entity);
            sprite.sprite.setPosition(pos.x, pos.y);
            window.draw(sprite.sprite);
        }
    }
};
```

## How to Test

```bash
# Compile with SFML
g++ -std=c++17 -O3 bench_sfml.cpp -lsfml-graphics -lsfml-window -lsfml-system -o bench_sfml

# Compile with Raylib
g++ -std=c++17 -O3 bench_raylib.cpp -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -o bench_raylib

# Compile with SDL2
g++ -std=c++17 -O3 bench_sdl2.cpp -lSDL2 -lSDL2_ttf -o bench_sdl2

# Or run the complete test
bash test.sh
```
