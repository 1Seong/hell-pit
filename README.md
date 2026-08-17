# HELL PIT

**HELL PIT** is a tiny Win32/GDI vertical hook-action game built for a **1.44 MB game jam / contest** target.

The project is designed around the contest rule that the final uncompressed submission should stay within **1,474,560 bytes**. It uses a single native Windows executable, software rendering, code-driven pixel art, and compact procedural audio/visual effects.

## Concept

You are trapped in a demonic vertical pit. Fire a hook, slash through enemies, chain jumps, abuse orbs and bullets as movement anchors, and climb before the monster below swallows the stage.

The game focuses on:

- fast vertical hook movement
- short hit-stop and slash-dash impact
- compact procedural stages
- demonic CRT horror atmosphere
- active items, perks, achievements, checkpoints, and infinite mode

## Controls

- `A` / `D`: move left / right
- `SPACE`: jump
- `Left Click`: fire hook
- `Q`: use active item
- `R`: restart
- `ESC`: pause / options

## Main Features

- Four 2000m chapters: `CHAMBER`, `MARCH`, `DREAM`, and `DREAD`
- Chapter checkpoints and stage-clear transitions
- Infinite mode unlock after escaping
- Hook targets: platforms, flying enemies, bullets, shop portals, orbs, and sawblades
- Super-dash orbs with invulnerability and enemy clearing
- Three-heart health system
- Combo and bloody coin reward system
- Demon shop with active items
- Random stage-start perks from stage 2 onward
- Steam-like achievement popup system
- Main menu, pause/options menu, codex/help pages, credits, death screen, and ending
- CRT, fog, flame particles, background perspective grid, and tentacle devourer effects
- Procedural ambient BGM and compact SFX

## Active Items

- **Dynamite**: explosive jump boost
- **Mini Orb**: portable super-dash
- **Blood Chalice**: heal one heart
- **Hourglass**: stop time briefly
- **Sawblade**: throw a hookable blade that kills enemies and destroys bullets

## Perks

- **Cross Necklace**: one-time revive with automatic super-dash
- **Parachute**: hold jump after spending jump to slow falling
- **Devil Heart**: stronger jump burst
- **Angel Skin**: half-heart damage
- **Blood Battery**: double jump
- **Contract**: +1 active item use

## Build

Open a **Visual Studio Developer Command Prompt** and run:

```bat
build_msvc.bat
```

The executable is generated at:

```text
build\HELL PIT.exe
```

## Project Structure

```text
src\main.cpp        Main game source
src\resources.rc    Windows icon/version resource
scripts\make_icon.ps1
build_msvc.bat      MSVC build script
```

Generated object files, resource files, local save data, and build outputs are ignored by git.

## Contest Note

This project is intended as an entry for a **1.44 MB contest**. The current architecture intentionally avoids large external assets and engines so that gameplay, UI, effects, audio, and executable/runtime size can stay within the contest limit.

## Developer

Developed by **CHOI SEONG WON**.
