# Accessibility Guidelines

At R-Type, we strive to make our game accessible to as many players as possible. This document outlines the features and design choices we have implemented to support players with various needs.

## Physical Disabilities

### Customizable Controls

- **Engine Support for Remapping**: The game engine's `Renderer` system supports dynamic key bindings. While a full UI for remapping is in development, the underlying architecture (`Renderer::set_key_binding`) allows for control customization.
- **Simple Control Scheme**: The game is designed with a minimal control scheme to reduce physical strain:
  - **Arrow Keys**: Movement (Up, Down, Left, Right)
  - **Space**: Shoot (Tap) / Charge Beam (Hold)
  - **Enter/Escape**: Menu navigation
  - This limited input set makes the game compatible with various adaptive controllers that map to standard keyboard inputs.

## Visual & Audio Disabilities

### Colorblind Modes

We support three specific colorblind modes to ensure gameplay elements are distinguishable. These modes adjust the colors of players, enemies, and UI elements.

- **Deuteranopia** (Green-blind)
- **Protanopia** (Red-blind)
- **Tritanopia** (Blue-blind)

_Modes can be cycled in the settings or via availability in the `AccessibilityManager`._

### Visual Clarity

- **High Contrast Design**: The game features bright, colorful entities against a dark space background to maximize visibility.
- **Hit Feedback**: Entities flash white when hit, providing a clear visual cue for damage registration independent of color perception.
- **Charge Indicators**: A visual charge bar and particle effects indicate the weapon charge level.

### Audio Cues

Distinct sound effects are used for all critical game events, helping players with visual impairments understand the game state:

- **Combat Sounds**: Unique sounds for shooting, missile charging, and explosions.
- **Feedback Sounds**: Distinct sounds for taking damage, collecting power-ups, and hitting enemies.
- **Boss Cues**: Specific audio cues (e.g., roars) signal boss appearances.

## Cognitive Disabilities

### Simplified Navigation

- **Linear Menus**: The menu system is designed to be flat and linear, avoiding deep nested structures.
- **Clear Typography**: Key information uses large, legible fonts (e.g., "SOLO SETTINGS", "GAME OVER").
- **Consistent UI**: Buttons and interactable elements have consistent states (Normal, Hover, Selected) with clear color changes.

### Gameplay Aids

- **Difficulty Settings**: Players can adjust the difficulty (Easy, Normal, Hard) to match their processing speed and reaction time.
- **Configurable Lives**: The number of starting lives can be adjusted, allowing for a more forgiving experience while learning the game mechanics.
