# R-Type User Guide

## 1. Introduction

### What is R-Type?

**R-Type** is a modern recreation of the famous "shoot 'em up" arcade game (horizontal scrolling shooter). Face waves of enemies in space, collect power-ups, and defeat formidable bosses, **alone or with friends** (up to 4 players online).

This project offers a retro gaming experience with modern technology: smooth graphics, optimized netcode for online multiplayer, and accessibility features.

### Who is this guide for?

This guide is intended for **all players** who want to install and play R-Type, whether they are:

- Casual players looking for a nostalgic experience.
- Multiplayer gamers wanting to challenge their friends online.
- Users concerned with accessibility (colorblind modes available).

**No programming skills are required** to play. This guide will walk you through step by step.

---

## 2. Getting Started (Quick Start)

### Prerequisites

Before starting, make sure your system has:

- **Operating System**: Linux (Fedora, Ubuntu, Debian, etc.), Windows, or macOS.
- **Internet Connection**: Required for downloading dependencies and online play.

**Required Software** (automatic installation via build script):

- A modern C++ compiler (automatically installed if missing).
- CMake and Git (usually pre-installed on Linux).

**System Dependencies** (Linux only):
On Fedora, install SFML with this command:

```bash
sudo dnf install SFML-devel
```

On Ubuntu/Debian:

```bash
sudo apt-get install libsfml-dev
```

---

### Installation

#### Step 1: Download the project

Open a terminal and run:

```bash
git clone https://github.com/EpitechProjects/B-CPP-500-rtype.git
cd B-CPP-500-rtype
```

> **Note**: Replace the URL above with the official repository URL if it differs.

---

#### Step 2: Run the automatic installation

We've created a script that handles everything (dependency download, compilation):

```bash
chmod +x ./build.sh
./build.sh
```

**This script will automatically**:

1. Download and configure the `vcpkg` package manager.
2. Install the required libraries (EnTT, ASIO, SFML).
3. Compile the game server and client.

**Estimated time**: 5 to 10 minutes (depending on your Internet connection).

---

#### Step 3: Verify the installation

If everything went well, you should see two executable files in the `./bin/linux/` folder:

- `r-type_server`: The game server.
- `r-type_client`: The client (player interface).

To find them, type:

```bash
ls -lh ./bin/linux/r-type_*
```

---

### First Use

#### Solo Mode / Local Host

If you want to test the game solo or host a game for your friends:

**Terminal 1: Start the server**

```bash
./bin/linux/r-type_server 4242
```

> **Explanation**: `4242` is the network port on which the server listens. You can choose another port if needed (between 1024 and 65535).

You should see a message like:

```
Server started on port 4242
Waiting for players...
```

**Terminal 2: Start the client (player 1)**

```bash
./bin/linux/r-type_client 127.0.0.1 4242
```

> **Explanation**:
>
> - `127.0.0.1`: Local IP address (your own computer).
> - `4242`: Port used by the server (must match).

The game opens with the main menu.

---

#### Network Multiplayer Mode

**If you are the host (the one who starts the server)**:

1. Find your public or local IP address:

   - **Local IP** (same Wi-Fi network): `ifconfig` (Linux) or `ipconfig` (Windows).
   - **Public IP** (Internet): Search "my IP" on Google.

2. Start the server:

   ```bash
   ./bin/linux/r-type_server 4242
   ```

3. Share your IP and port with your friends.

**If you are a guest player**:

Ask the host for their IP address and port, then run:

```bash
./bin/linux/r-type_client <HOST_IP> <PORT>
```

Example:

```bash
./bin/linux/r-type_client 192.168.1.42 4242
```

---

#### Basic Controls

Once in the game:

- **Movement**:

  - Arrow keys **OR** WASD (QWERTY keyboard) **OR** ZQSD (AZERTY keyboard).

- **Shoot**:

  - Press **Space**.

- **Menu**:
  - Navigate with arrows and confirm with **Enter**.

**That's it!** You're ready to play.

---

## 3. Feature Guide (Core Features)

### 3.1. Network Multiplayer (up to 4 players)

#### Why use it?

Face waves of enemies with your friends! Multiplayer mode offers a cooperative experience where up to **4 players** can play simultaneously in the same game.

#### How to use it?

1. **One of you starts the server**:

   ```bash
   ./bin/linux/r-type_server 4242
   ```

2. **Each player connects** with the client:

   ```bash
   ./bin/linux/r-type_client <SERVER_IP> 4242
   ```

3. **In the game menu**:
   - Select **Multiplayer Mode**.
   - Wait in the lobby for all players to join.
   - The first player starts the game by selecting **Start**.

**Tip**: The server handles all game logic (collisions, enemies, score). Clients only send your commands and display the result. This ensures a smooth experience even with average connections.

---

### 3.2. Accessibility Modes (Colorblindness)

#### Why use it?

R-Type includes visual modes adapted for colorblind people, so everyone can enjoy the game in the best conditions.

#### Available mode types:

- **Deuteranopia**: Difficulty perceiving green.
- **Protanopia**: Difficulty perceiving red.
- **Tritanopia**: Difficulty perceiving blue.

#### How to activate it?

1. Launch the client.
2. In the **main menu**, select **Settings**.
3. Choose the **Color Mode** option.
4. Select the mode adapted to your type of colorblindness.
5. Confirm and return to the game.

The colors of enemies, projectiles and effects will be automatically adjusted.

---

### 3.3. Weapon and Shooting System

#### Why use it?

The weapon system is at the heart of the gameplay. Shoot enemies to destroy them, collect power-ups to improve your firepower.

#### How does it work?

- **Basic shooting**: Press **Space** to fire projectiles.
- **Upgraded weapons**: Destroy special enemies to release power-ups. Collect them to get:
  - Multiple shots.
  - More powerful projectiles.
  - Special trajectory shots.

**Strategy example**:

- Stay mobile to avoid enemy fire.
- Focus your fire on the most dangerous enemies.
- Collect power-ups as soon as they appear.

---

### 3.4. Lives and Score System

#### Why is it important?

Each player starts with a limited number of **lives**. Losing all your lives means the end of the game (Game Over). The **score** measures your performance.

#### How does it work?

- **Lives**:

  - You start with **3 lives**.
  - A life is lost when your ship collides with an enemy or a projectile.
  - Earn bonus lives by reaching certain score milestones.

- **Score**:
  - Destroying an enemy earns points.
  - Bosses are worth many more points.
  - The score is displayed at the top of the screen.

**Tip**: In multiplayer mode, the score is shared among all players. Cooperate to maximize your points!

---

### 3.5. Enemies and Bosses

#### Why is it exciting?

The game offers **multiple enemy types** with varied behaviors, as well as **formidable bosses** at the end of levels.

#### Enemy types:

- **Standard enemies** (monster_0 to monster_4):

  - Predictable movements.
  - Simple shots.
  - Easy to destroy.

- **Advanced enemies**:

  - Complex trajectories.
  - Multiple shots.
  - Increased resistance.

- **Bosses** (boss_1, boss_2):
  - Appear at the end of levels.
  - Elaborate attack patterns.
  - Require strategy to be defeated.

**How to face them?**

1. Observe movement patterns.
2. Stay in constant motion.
3. Aim for weak points (often visually indicated).
4. In multiplayer, coordinate your attacks.

---

### 3.6. Visual and Sound Effects

#### Why is it immersive?

R-Type features **animated explosions**, **particle effects**, and **realistic shooting sounds** for an authentic arcade experience.

#### Features:

- **Explosions**: Detailed GIF animations when enemies are destroyed.
- **Sounds**:

  - `shoot.wav`: Shooting sound.
  - Add your own sounds to the `client/assets/audio/` folder.

- **Animated backgrounds**: Dynamic space scenery for each level.

**How to disable sound?** (if needed)
Currently, sound is enabled by default. To disable it, modify the settings in the menu (feature in development).

---

## 4. Advanced Configuration

### 4.1. Custom Network Settings

By default, the server uses **port 4242** and broadcasts the game state **20 times per second** (20 Hz). You can customize these values.

#### Change the server port:

```bash
./bin/linux/r-type_server 8080
```

> **Warning**: Clients must use the same port.

#### Optimize latency (for developers):

Network parameters are defined in `/home/jbbsh/repo-r-type/shared/GameConstants.hpp`:

```cpp
constexpr int SERVER_UPDATE_RATE = 60;  // Hz (game logic)
constexpr int STATE_BROADCAST_RATE = 20; // Hz (broadcast to clients)
```

To modify these values, recompile the project after editing.

---

### 4.2. Resolution and Fullscreen

By default, the game displays in **1920x1080** (Full HD).

#### Change the resolution:

Modify the constants in `shared/GameConstants.hpp`:

```cpp
constexpr float SCREEN_WIDTH = 1920.0f;
constexpr float SCREEN_HEIGHT = 1080.0f;
```

Then recompile:

```bash
./build.sh
```

#### Fullscreen mode:

Currently managed automatically. To switch to windowed mode, modify the `ScreenMode` option in the client code (feature in development).

---

### 4.3. Configuration Files (game.json)

A JSON configuration file is planned at the location:
`/home/jbbsh/repo-r-type/config/game.json`

**Current status**: The file is empty (placeholder). It will allow in the future to customize:

- Enemy speed.
- Starting number of lives.
- Power-up spawn rate.

**Planned format**:

```json
{
  "player": {
    "startingLives": 3,
    "speed": 5.0
  },
  "enemies": {
    "spawnRate": 2.0,
    "difficulty": "normal"
  }
}
```

> **Note**: This feature will be implemented in a future version.

---

### 4.4. Adding Your Own Assets

You can customize the game by replacing sprites and sounds.

#### Sprites:

Place your PNG/GIF files in `/home/jbbsh/repo-r-type/client/sprites/` and reference them in the code.

#### Sounds:

Add your WAV files to `/home/jbbsh/repo-r-type/client/assets/audio/`.

**Example**: Replace the shooting sound:

1. Rename your file to `shoot.wav`.
2. Replace the existing file in `client/assets/audio/shoot.wav`.
3. Restart the client.

---

## 5. FAQ & Troubleshooting

### ❓ Server won't start ("Address already in use")

**Cause**: Port 4242 is already in use by another application.

**Solution**:

1. Choose another port:

   ```bash
   ./bin/linux/r-type_server 5000
   ```

2. Or stop the process using port 4242:
   ```bash
   lsof -i :4242
   kill -9 <PID>
   ```

---

### ❓ Client won't connect to server ("Connection refused")

**Possible causes**:

1. The server is not running.
2. The IP address or port is incorrect.
3. A firewall is blocking the connection.

**Solutions**:

1. **Verify the server is running**:
   In the server terminal, you should see "Server started on port...".

2. **Check the IP and port**:

   - Local IP: `127.0.0.1`
   - Network IP: Use `ifconfig` (Linux) or `ipconfig` (Windows).

3. **Temporarily disable the firewall** (Linux):

   ```bash
   sudo ufw allow 4242
   ```

   Windows: Allow `r-type_server.exe` in firewall settings.

---

### ❓ Game is choppy / laggy

**Possible causes**:

1. Unstable network connection.
2. Overloaded server (too many players or insufficient resources).
3. Underpowered computer.

**Solutions**:

1. **Check your ping**:

   ```bash
   ping <SERVER_IP>
   ```

   A ping >100ms may cause stuttering.

2. **Close resource-heavy applications** (browser, streaming).

3. **Reduce resolution** (see section 4.2).

---

### ❓ Compilation error when running ./build.sh

**Possible causes**:

1. Missing system dependencies (SFML).
2. C++ compiler too old.

**Solutions**:

1. **Install SFML**:

   - Fedora: `sudo dnf install SFML-devel`
   - Ubuntu: `sudo apt-get install libsfml-dev`

2. **Check compiler version**:

   ```bash
   g++ --version
   ```

   You need **g++ 10+** or **clang 12+**.

   To install a recent version (Ubuntu):

   ```bash
   sudo apt-get install g++-11
   export CXX=g++-11
   ./build.sh
   ```

---

### ❓ No sound in the game

**Possible causes**:

1. Missing audio file.
2. Problem with the system's audio system.

**Solutions**:

1. **Verify the file exists**:

   ```bash
   ls client/assets/audio/shoot.wav
   ```

2. **Test system sound**:

   ```bash
   paplay /usr/share/sounds/alsa/Front_Center.wav
   ```

3. **Check permissions**:
   ```bash
   chmod +r client/assets/audio/*.wav
   ```

---

## Need Additional Help?

If you encounter a problem not covered by this guide:

1. **Consult the developer documentation**:
   [/home/jbbsh/repo-r-type/documentation/DEVELOPER_DOCUMENTATION.md](../documentation/DEVELOPER_DOCUMENTATION.md)

2. **Report a bug**:
   Open an issue on the project's GitHub repository.

3. **Community**:
   Contact the development team via official channels.

---

**Happy gaming! 🚀**

_Documentation written for R-Type - Version 1.0_
_Last updated: December 2025_
