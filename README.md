# Garden Guardians (C++ / raylib)

Garden Guardians is a standalone 2D lane-defense game inspired by the classic plant-vs-monster formula, implemented in modern C++17 with raylib and CMake.

The project is fully self-contained (source + config) and uses procedural drawing for visuals and generated tone SFX, so there are no copyrighted third-party game assets.

## Gameplay Overview

- 5-lane grid battlefield with a protected home zone on the left
- Collect sun resource and spend it on plants
- Place plants by selecting seed packets and clicking tiles
- Defend against increasingly difficult zombie waves spawning from the right
- Win by surviving all waves, lose if enemies break through

## Features

- Main menu (`Start`, `How to Play`, `Quit`) with selectable difficulty (`Easy`, `Normal`, `Hard`)
- Game states: menu, how-to, playing, paused, win, lose
- In-game pause button with clickable resume/menu actions
- Smoother UI transitions for pause/end screens and state changes
- Resource economy:
  - periodic falling suns (reduced frequency)
  - `SunBloom` resource plant (sunlight appears in front of the plant, expires in 7s if uncollected)
- 10 plant types:
  - `Peashooter` (ranged lane DPS)
  - `SunBloom` (multi-sunlight generator)
  - `WallRoot` (high-health blocker)
  - `Chomper` (one-shot swallow, then digest cooldown)
  - `FrostBurst` (area explosion + slow)
  - `Repeater` (double-shot DPS)
  - `Ice Pea` (slow support shooter)
  - `SpikeTrap` (contact damage hazard)
  - `BombBerry` (arming explosive)
  - `Magnet Vine` (armor removal support)
- 9 zombie types:
  - `Basic`
  - `Runner` (fast, low HP)
  - `Tank` (high HP, high damage)
  - `Bucket` (armor + slow resistance behavior)
  - `Conehead` (armored frontline)
  - `Imp` (very fast fragile pressure)
  - `Spitter` (ranged lane pressure)
  - `Pole-Jump` (jumps first blocker once)
  - `Summoner` (spawns Imp allies over time)
- Projectile collisions, melee contact attacks, HP-based death cleanup
- Impact feedback polish: screen shake + hit flashes on units
- Zombie spawn telegraph indicators before lane entry
- Lane mower emergency clear mechanic
- Progressive wave system with intermission, progress HUD, and final victory condition
- Procedural animated sprite sheets for plants and zombies (runtime-generated atlases)
- Persistent save stats (`wins`, `games played`, `best wave`) across sessions
- Config file (`assets/config/game.cfg`) for easy tuning
- Lightweight procedural audio with safe fallback if audio device is unavailable
- Lane-based update loops and occupancy-grid placement checks for better scaling
- Seed packet tooltips + placement error feedback + tile validity highlighting

## Controls

- Main menu:
  - click `Easy`, `Normal`, or `Hard` before `Start Game`
- Mouse Left Click:
  - select seed packet
  - place plant on tile
  - collect sun tokens
  - select `Shovel` and click a planted tile to remove a plant
- `Esc`:
  - pause / resume during gameplay
- Top-right pause icon:
  - click to pause
- Pause screen:
  - click `Resume` or `Main Menu`
- End screen:
  - `R` restart
  - `M` return to menu

## Third-Pass Feel Improvements

- **Screen shake:** projectile hits, explosions, mower activation, and mower kills add camera shake.
- **Hit flashes:** plants and zombies briefly flash white when damaged.
- **Spawn telegraphing:** warning circles and enemy labels appear on the right lane before zombies enter.
- **Wave rebalance:** smoother enemy composition ramp and steadier spawn-interval curve.
- **UI motion polish:** transition fade on state changes and animated pause/end overlays.

## Build Requirements

- CMake 3.20+
- C++17 compiler:
  - Windows: MSVC (Visual Studio 2022 recommended) or MinGW
  - macOS: AppleClang
  - Linux: GCC or Clang

By default raylib is fetched automatically. For fully local/offline setups, vendor raylib under `third_party/raylib` and configure with:

```bash
cmake -S . -B build -DUSE_LOCAL_RAYLIB=ON
```

## Build and Run

### Quick Build with Makefile (macOS / Linux)

```bash
make
make run
```

Useful shortcuts:

```bash
make debug
make release
make clean
make distclean
```

If your project path contains spaces and raylib `FetchContent` fails, use:

```bash
make DEPS_DIR=/tmp/GardenGuardians_deps run
```

### macOS / Linux

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
./build/GardenGuardians
```

### Windows (PowerShell)

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
.\build\Release\GardenGuardians.exe
```

### Windows (Visual Studio IDE)

```powershell
cmake -S . -B build
```

Then open generated solution/project from `build`, build `Release`, run `GardenGuardians`.

## Share a build (players do not need source code)

The game loads `assets/config/game.cfg` and writes saves using paths **next to the executable** when possible, so you can ship a small folder containing only the binary and `assets/`.

**macOS / Linux (Makefile)**

```bash
make dist
```

This creates `dist/GardenGuardians-portable/` with `GardenGuardians` and `assets/`. Zip that folder and share it. The recipient unpacks, opens a terminal in the folder, and runs `./GardenGuardians` (macOS/Linux may require `chmod +x GardenGuardians` after unzip).

**Any platform (CMake only)**

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j
cmake --build build --config Release --target portable_bundle
```

The playable folder is `build/GardenGuardians-portable/` (executable + `assets/`). On Windows with Visual Studio, the executable is `GardenGuardians.exe` in that folder.

**Important**

- Keep the **executable and `assets/` in the same directory** when zipping or moving files.
- Saves appear under `save/` beside the executable for portable installs.
- macOS Gatekeeper may block unsigned builds; see **Troubleshooting** below for Open / Open Anyway / `xattr`.
- Linux players may need graphics/audio libraries GLFW depends on (e.g. `libGL`, X11); the binary is otherwise self-contained with the vendored static raylib build.

## Project Structure

```text
GardenGuardians/
├── CMakeLists.txt
├── Makefile
├── README.md
├── packaging/
│   └── PLAYER_README.txt   (copied as README.txt into portable zip builds)
├── assets/
│   └── config/
│       └── game.cfg
├── include/
│   ├── Board.hpp
│   ├── Game.hpp
│   ├── GameConfig.hpp
│   ├── Plant.hpp
│   ├── ProceduralSprites.hpp
│   ├── SaveSystem.hpp
│   ├── Types.hpp
│   ├── WaveManager.hpp
│   └── Zombie.hpp
└── src/
    ├── Board.cpp
    ├── Game.cpp
    ├── GameConfig.cpp
    ├── Plant.cpp
    ├── ProceduralSprites.cpp
    ├── SaveSystem.cpp
    ├── WaveManager.cpp
    ├── Zombie.cpp
    └── main.cpp
```

## How Files Connect

- `main.cpp`: game bootstrap and main loop entry.
- `Game.*`: top-level game controller (states, input, update loop, rendering, UI/HUD, win/lose flow).
- `Board.*`: lawn geometry, tile math, lane/grid drawing, coordinate conversion.
- `Plant.*`: base plant class + 5 specialized plant behaviors and card data (cost/cooldown).
- `Zombie.*`: base zombie class + 4 zombie variants with movement/combat differences.
- `WaveManager.*`: wave progression, spawn pacing, lane/type randomization.
- `GameConfig.*` + `assets/config/game.cfg`: editable runtime constants and config parser.
- `ProceduralSprites.*`: runtime sprite atlas generation and animated frame rendering.
- `SaveSystem.*`: persistent run statistics serialization.
- `save/profile.cfg`: generated automatically at runtime on first save (not stored in repo).
- `Types.hpp`: enums and shared lightweight data types (projectiles, suns, mowers).

## Troubleshooting

- **macOS: “Apple could not verify GardenGuardians…” (Gatekeeper)**  
  This is normal for a **local build** or a **zip you downloaded** that is not signed or notarized by Apple. You are not doing anything wrong; the system is cautious about unknown developers.
  1. **Finder (easiest):** Control-click (or right-click) `GardenGuardians` → **Open** → in the dialog choose **Open** (not available on the very first alert until you use Open from the menu once).
  2. **System Settings:** Open **System Settings → Privacy & Security**, scroll down; after a failed launch you may see **GardenGuardians was blocked** — click **Open Anyway** and confirm.
  3. **Quarantine flag (zips / browsers):** If the file was downloaded, macOS may attach a quarantine attribute. From Terminal, in the folder that contains the binary:  
     `xattr -dr com.apple.quarantine GardenGuardians`  
     Then try again (still use Open from Finder the first time if needed).
  4. **Run from Terminal** (often works for builds in your own project folder):  
     `cd` to the folder that contains `GardenGuardians` and `./GardenGuardians` — this may still trigger a prompt the first time; approve if asked.

- If CMake cannot fetch raylib:
  - verify internet access for the first configure/build
  - retry configure: `cmake -S . -B build`
- If app launches with no sound:
  - game still runs correctly (audio fallback path is intentional)
- If window appears black:
  - ensure GPU/OpenGL drivers are available and up to date
- If executable path differs on Windows:
  - use `build\Release\GardenGuardians.exe` for multi-config generators

## Dependency Notes

- Only external dependency is raylib (`FetchContent` in CMake).
- No proprietary/copyrighted Plants-vs-Zombies assets are included.
- All game visuals are drawn procedurally with raylib primitives.
