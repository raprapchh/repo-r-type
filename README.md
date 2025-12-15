# R-Type - Epitech Project

<p align="center">
  <img src="https://fs-prod-cdn.nintendo-europe.com/media/images/10_share_images/games_15/virtual_console_wii_u_7/H2x1_WiiUVC_RType.jpg" alt="R-Type Banner" width="100%">
</p>

## 🚀 Introduction

This project is a recreation of the classic **R-Type** arcade game, developed as part of the **Advanced C++ Knowledge (B-CPP-500)** module at Epitech.
It features a custom game engine built on an **Entity Component System (ECS)** architecture and a networked multiplayer mode using **UDP** for real-time communication. The client is powered by **SFML**.

<br>

## ✨ Features

- **Custom ECS Engine**: A high-performance, data-oriented architecture separating Entities, Components, and Systems.
- **Networked Multiplayer**: Real-time gameplay supporting multiple players via a custom UDP protocol.
- **Game States**: Includes Menu, Lobby, and Gameplay states.
- **Configuration**: Data-driven design with configuration files.

<br>

## ♿ Accessibility

We believe games are for everyone. This project includes specific features to ensure inclusivity:

- **Colorblind Modes**: Support for Deuteranopia, Protanopia, and Tritanopia (toggle available in Settings).

<br>

## 🛠️ Prerequisites

Before building the project, ensure you have the following installed:

- **C++ Compiler** (supporting C++20)
- **CMake** (v3.20 or newer)
- **Git**
- **SFML 2.6.x** (system library)

### Installing SFML (Fedora)

```bash
sudo dnf install SFML-devel
```

> **Note:** This project uses **vcpkg** as a package manager for dependencies (ASIO, EnTT, Catch2). These are installed automatically during the build process. SFML uses the system library as it depends on platform-specific components (OpenGL, X11).

<br>

## 🏗️ Installation & Compilation

1. **Clone the repository:**

   ```bash
   git clone git@github.com:EpitechPGE3-2025/G-CPP-500-PAR-5-2-rtype-3.git
   cd G-CPP-500-PAR-5-2-rtype-3
   ```

2. **Build the project:**
   We provide a helper script to automate the build process.
   ```bash
   chmod +x ./build.sh
   ./build.sh
   ```

   _Alternatively, you can build manually:_

   ```bash
   cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
   cmake --build build --parallel
   ```

3. **Clean the project:**

   ```bash
   chmod +x ./clean.sh
   ./clean.sh
   ```

<br>

## 🎮 Usage

### 1. Start the Server

The server manages the game state and synchronization. Run it first.

```bash
./r-type_server [port]
```

_(Default port is 4242)_

### 2. Start the Client

Launch the client to connect to the server.

```bash
./r-type_client [ip] [port]
```

- **ip**: The IP address of the server (default: `127.0.0.1` or `localhost`).
- **port**: The port the server is listening on (default: `4242`).

<br>

## 🕹️ Controls

| Action         | Key (AZERTY) | Key (QWERTY) | Key (Arrows) | description              |
| :------------- | :----------: | :----------: | :----------: | :----------------------- |
| **Move Up**    |     `Z`      |     `W`      |     `↑`      | Move spaceship up        |
| **Move Down**  |     `S`      |     `S`      |     `↓`      | Move spaceship down      |
| **Move Left**  |     `Q`      |     `A`      |     `←`      | Move spaceship left      |
| **Move Right** |     `D`      |     `D`      |     `→`      | Move spaceship right     |
| **Shoot**      |   `Space`    |   `Space`    |   `Space`    | Fire standard projectile |

<br>

## 📂 Project Architecture

The codebase is modularized for clarity and scalability:

- **`server/`**: Handles game logic, authoritative state, and UDP communication.
- **`client/`**: manages rendering (SFML), input handling, and interpolation.
- **`ecs/`**: The core Entity Component System library (Entities, Components, Systems).
- **`shared/`**: Common resources, network protocols, and data structures.
- **`config/`**: JSON/YAML configuration files for game balancing.
- **`documentation/`**: Developer guides and technical documentation.

<br>

## 👥 Authors

**Epitech Paris**

- **Raphaël Chanliongco**
- **Hubert Touraine**
- **Jean-Baptiste Boshra**
- **Gabin Rudigoz**
- **Ylan Cuvier**
- **Swann Grandin**

<br>
<hr>
<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/2/2d/Epitech.png/320px-Epitech.png" alt="Epitech Logo" width="200">
</p>
