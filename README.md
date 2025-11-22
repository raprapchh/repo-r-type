# R-Type EPITECH PARIS

This is a 3rd year project, where we had to recreate the old game "R-Type" and add a multiplayer mode. We had to create a UDP server and client for the network transmission and we used SFML for the graphical client.

# Game Engine

This game runs on a custom **ECS (Entity Component System)**.
This system makes the code cleaner and faster by strictly separating **Data** from **Logic**.

It works in three simple parts:

* **Entities:** Simple ID tags representing objects (like a Player, an Enemy, or a Bullet).
* **Components:** Pure data attached to an entity (e.g., Position, Health, Speed).
* **Systems:** The logic that processes the components to make things happen (e.g., a *MovementSystem* that updates the *Position* based on the *Speed*).

![image](https://fs-prod-cdn.nintendo-europe.com/media/images/10_share_images/games_15/virtual_console_wii_u_7/H2x1_WiiUVC_RType.jpg)

# Project Architecture

The project is organized into several modules:

## Server (`server/`)

The server handles game logic, client connections, and maintains the authoritative state of the game. It communicates with clients via UDP, sending updates about entities (players, enemies, projectiles).

## Client (`client/`)

The client handles the graphical rendering (using SFML) and user input. It sends player actions to the server and renders the game state received from the server.

## ECS (`ecs/`)

The Entity Component System (ECS) library. This core module manages entities, components, and systems, separating data from behavior to improve performance and code reusability.

## Shared (`shared/`)

Contains code and resources shared between the Client and Server, such as network protocols, common data structures, and game constants.

## Config (`config/`)

Stores configuration files used to tune game parameters and settings without recompiling the code.

# How to Play

## Compilation

To compile the project, use CMake:

```bash
mkdir build
cd build
cmake ..
make
```

You will get `r-type_server` and `r-type_client`.

## Execution

1. Execute the server:

```bash
./r-type_server
```

2. Execute the client:

```bash
./r-type_client [ip] [port]
```
(Default port is 4242)

# Goal of the game

You have to play from 1 to 4 players against enemies that spawn randomly from the right part of your screen. If you kill enough enemies, they become stronger and stronger, and finally, the final boss appears and shoots a big laser ray. You start the game with 5 hp and can recuperate some hp by taking the little hearts on the map. There is no friendly fire and once you have no more hp, you can't play anymore.

# Move and shoot

- Move: `Z` `Q` `S` `D` (AZERTY Layout)
- Shoot: `Left Mouse Click`
- Menu: `Esc`

# Developer

If you are a developer, please refer to the `documentation` folder.

# Authors

Developed by:

- Raphaël Chanliongco
- Hubert Touraine
- Jean-Baptiste Boshra
- Gabin Rudigoz
- Ylan Cuvier
- Swann Grandin

<br>
<hr>
<p align="center">
  <img src="https://upload.wikimedia.org/wikipedia/commons/thumb/2/2d/Epitech.png/320px-Epitech.png" alt="Epitech Logo" width="200">
  <br>
</p>