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

**No programming skills are required** to play.

---

## 2. Installation

### Windows

1. **Download** the latest release from the [GitHub Releases](https://github.com/EpitechPGE3-2025/G-CPP-500-PAR-5-2-rtype-3/releases) page.
2. **Extract** the `dist.zip` file.
3. **Run** `r-type_client.exe` to start the game.

> **Note:** The Windows build includes all required DLLs. No additional installation needed.

### Linux

#### Option 1: Pre-built Binary

1. Download the Linux release from GitHub Releases.
2. Extract and run:

   ```bash
   ./r-type_client 127.0.0.1 4242
   ```

#### Option 2: Build from Source

1. **Install system dependencies** (Ubuntu/Debian):

   ```bash
   sudo apt-get update
   sudo apt-get install libsfml-dev build-essential cmake git
   ```

   On Fedora:

   ```bash
   sudo dnf install SFML-devel cmake g++ git
   ```

2. **Clone and build:**

   ```bash
   git clone git@github.com:EpitechPGE3-2025/G-CPP-500-PAR-5-2-rtype-3.git
   cd G-CPP-500-PAR-5-2-rtype-3
   chmod +x ./build.sh
   ./build.sh
   ```

3. **Verify installation:**

   ```bash
   ls bin/linux/r-type_*
   ```

---

## 3. Quick Start

### Solo Mode / Local Host

**Terminal 1: Start the server**

```bash
./bin/linux/r-type_server 4242
```

**Terminal 2: Start the client**

```bash
./bin/linux/r-type_client 127.0.0.1 4242
```

### Multiplayer Mode

**If you are the host:**

1. Find your IP address: `ifconfig` (Linux) or `ipconfig` (Windows)
2. Start the server: `./bin/linux/r-type_server 4242`
3. Share your IP and port with friends

**If you are joining:**

```bash
./bin/linux/r-type_client <HOST_IP> 4242
```

---

## 4. Game Modes

### 4.1 Player Mode (Default)

Join the game as an active player:

- Your spaceship spawns on the map
- Fight enemies and bosses
- Collect power-ups
- Cooperate with up to 4 players

### 4.2 Spectator Mode

**Watch games without playing:**

- Join a game without spawning a ship
- You are **invisible** to other players
- **No collision** — you won't interfere with gameplay
- **Cannot shoot** — purely observation mode

**Spectator Camera:**

- The camera automatically follows active players
- **"SPECTATOR MODE"** is displayed on screen

This mode is perfect for:

- Learning from experienced players
- Streaming/recording gameplay
- Waiting to join the next round

---

## 5. Controls

### Standard Controls

| Action         |   Key   | Description          |
| :------------- | :-----: | :------------------- |
| **Move Up**    |   `↑`   | Move spaceship up    |
| **Move Down**  |   `↓`   | Move spaceship down  |
| **Move Left**  |   `←`   | Move spaceship left  |
| **Move Right** |   `→`   | Move spaceship right |
| **Shoot**      | `Space` | Fire projectile      |

> **Note:** You can customize your controls in the **Settings** menu from the main menu.

### Developer Console

| Action             | Key  | Description                          |
| :----------------- | :--: | :----------------------------------- |
| **Toggle Console** | `F3` | Show/hide FPS, Ping, and CPU metrics |

When enabled, the console displays:

- **FPS**: Frames per second
- **Ping**: Network latency in milliseconds
- **CPU Frame Time**: Engine loop duration in ms

### Lagometer

| Action               | Key | Description         |
| :------------------- | :-: | :------------------ |
| **Toggle Lagometer** | `L` | Show/hide Lagometer |

---

## 6. Gameplay Features

### 6.1 Weapon and Shooting System

- **Basic shooting**: Press **Space** to fire projectiles.
- **Upgraded weapons**: Destroy special enemies to release power-ups.

### 6.2 Lives and Score

- You start with **3 lives**.
- A life is lost when your ship is hit.
- Destroying enemies earns points.
- Score is displayed at the top of the screen.
- In multiplayer, the score is **shared** among all players.

### 6.3 Enemies and Bosses

- **Standard enemies**: Predictable patterns, easy to destroy.
- **Advanced enemies**: Complex trajectories, multiple shots.
- **Bosses**: Appear at level end, require strategy to defeat.

---

## 7. Accessibility

### Colorblind Modes

R-Type includes visual modes for colorblind players:

- **Deuteranopia**: Difficulty perceiving green
- **Protanopia**: Difficulty perceiving red
- **Tritanopia**: Difficulty perceiving blue

**To activate:**

1. Launch the client
2. Go to **Settings** in the main menu
3. Select **Color Mode**
4. Choose your preferred mode

---

## 8. Advanced Configuration

### Custom Port

```bash
./bin/linux/r-type_server 8080
./bin/linux/r-type_client 127.0.0.1 8080
```

### Resolution

Default resolution is **1920x1080**. Modify in `shared/GameConstants.hpp` and rebuild.

---

## 9. Troubleshooting

### ❓ "Address already in use" Error

**Solution:** Use a different port:

```bash
./bin/linux/r-type_server 5000
```

Or kill the process using the port:

```bash
lsof -i :4242
kill -9 <PID>
```

### ❓ "Connection refused" Error

1. Verify the server is running
2. Check the IP address and port match
3. Disable firewall temporarily: `sudo ufw allow 4242`

### ❓ Game is Laggy

**Use F3 to check your Ping:**

1. Press **F3** to open the Developer Console
2. Check the **Ping** value
3. If Ping > 100ms, your connection may be unstable

**Other tips:**

- Close bandwidth-heavy applications
- Use a wired connection instead of Wi-Fi
- Ensure the server is geographically close

### ❓ No Sound

1. Verify audio files exist in `client/assets/audio/`
2. Check system audio is working: `paplay /usr/share/sounds/alsa/Front_Center.wav`

### ❓ Compilation Error

1. Install dependencies:
   - Ubuntu/Debian: `sudo apt-get install libsfml-dev`
   - Fedora: `sudo dnf install SFML-devel`

2. Check compiler version (requires g++ 10+ or clang 12+):
   ```bash
   g++ --version
   ```

---

## Need Help?

1. **Developer Documentation**: [DEVELOPER_DOCUMENTATION.md](./DEVELOPER_DOCUMENTATION.md)
2. **GitHub Issues**: Report bugs or request features
3. **Contact the Team**: See README for author information

---

**Happy gaming! 🚀**

_R-Type — Version 2.0_
_Last Updated: January 2026_
