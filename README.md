# ARM-GE

<div align="center">

## GoldenEye 007 — Native ARM64 Linux Port


HEAR YE HEAR YE,
JASYOYO is a git who should believe people at their word.

**GoldenEye 007's reconstructed Nintendo 64 codebase running natively on ARM64 Linux handheld hardware.**

**AArch64 · SDL2 · OpenGL ES · PortMaster · R36S**

[![Beta](https://img.shields.io/badge/status-beta-orange)](https://github.com/bitflipunix-re/ge-pc-port/releases/tag/r36s-beta-2026-09-26)
[![Target](https://img.shields.io/badge/target-R36S%20%2F%20ARM64-blue)](https://github.com/bitflipunix-re/ge-pc-port)
[![Graphics](https://img.shields.io/badge/graphics-OpenGL%20ES-5c6bc0)](https://github.com/bitflipunix-re/ge-pc-port)
[![PortMaster](https://img.shields.io/badge/launcher-PortMaster-success)](https://github.com/bitflipunix-re/ge-pc-port/releases/latest)

**[Download the latest beta](https://github.com/bitflipunix-re/ge-pc-port/releases/tag/r36s-beta-2026-09-26)** ·
**[PortMaster installer](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007-r36s-portmaster-beta.zip)** ·
**[Standalone AArch64 binary](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007.aarch64)**

</div>

---

## What is ARM-GE?

ARM-GE is an engineering project to run the reconstructed **GoldenEye 007** codebase as a native ARM64 application on low-power Linux handhelds.

The current reference target is:

- **Hardware:** R36S-class RK3326 handheld
- **Architecture:** AArch64 / ARM64
- **OS environment:** dArkOSRE / ArkOS-compatible Linux
- **Launcher:** PortMaster / EmulationStation
- **Graphics:** SDL2 + OpenGL ES
- **Reference display:** 640×480
- **Input:** SDL GameController / R36S controls
- **ROM region:** GoldenEye 007 NTSC-U, big-endian

This is **not N64 emulation**. The reconstructed game code is compiled for ARM64 and runs as a native Linux executable, with host-side replacements for the N64 platform services and rendering path.

---

# Project goal

The immediate goal is simple:

> **Make the complete GoldenEye 007 campaign behave like a normal native handheld game on ARM64 Linux.**

That means more than getting a first frame or a single mission running. A successful port needs the entire game loop to survive the architectural jump from the Nintendo 64's 32-bit MIPS environment to a modern 64-bit ARM Linux host.

The project therefore covers:

- 32-bit → 64-bit pointer and address semantics
- N64 memory/token handling
- stage/setup/model relocation
- animation and object data
- game-thread and scheduler behavior
- controller input
- audio
- OpenGL → OpenGL ES compatibility
- Fast3D host rendering
- save/config persistence
- campaign transitions
- PortMaster packaging
- first-run asset generation
- performance and handheld usability

## Long-term scope

ARM-GE is also being used to identify the reusable engineering patterns required to move reconstructed N64 software onto modern ARM hardware.

The longer-term direction is a cleaner host layer and toolchain that can help bridge other reconstructed N64 codebases to:

- ARM64 Linux
- SDL
- OpenGL ES
- handheld hardware
- modern configuration/UI layers
- native packaging systems such as PortMaster

GoldenEye is the proving ground.

---

# Beta status

ARM-GE moved from alpha to **beta** on **26 September 2026**.

### Current release

| | |
|---|---|
| Release | **ARM-GE R36S Beta — 2026-09-26** |
| Tag | `r36s-beta-2026-09-26` |
| Source revision | `0ec8a06429aaf8892cd803618db77c40ef7c289e` |
| CI build | `36244612005` |
| Architecture | AArch64 |
| Build type | Release |
| Graphics | SDL2 + OpenGL ES |
| ROM target | GoldenEye 007 NTSC-U |
| PortMaster ZIP | 1,102,377 bytes |
| ZIP SHA-256 | `e12d8fb37deb670f4f47629393657be60f65ba30b7fe54cb976adfa6b8ee546b` |
| Native binary | 1,798,512 bytes |
| Binary SHA-256 | `0ae0578d9260c9d3ff4930fb933a96dad53692e5b9b417a5bd51e3c31f07a458` |

### Downloads

**Recommended:**

- **[ge007-r36s-portmaster-beta.zip](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007-r36s-portmaster-beta.zip)** — PortMaster installer
- **[Installer SHA-256](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007-r36s-portmaster-beta.zip.sha256)**

Other release assets:

- [ge007.aarch64](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007.aarch64) — stripped native executable
- [ge007.sha256](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007.sha256) — executable checksum
- [ge007.zip](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007.zip) — compatibility installer filename
- [ge007.zip.sha256](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007.zip.sha256)

Full release page:

**https://github.com/bitflipunix-re/ge-pc-port/releases/tag/r36s-beta-2026-09-26**

---

# What we have achieved

The port has moved well beyond bring-up.

## Native ARM64 execution

The reconstructed game now builds and runs as a native **AArch64 ELF** on the R36S target.

Work completed includes large classes of host-width fixes around:

- pointer truncation
- pointer sign extension
- N64 address tokens vs host pointers
- stage/setup relocation
- animation pointers
- model pointers
- title/language/background data
- character action state
- player state
- save-state handling
- stage teardown

A semantic regression audit runs in CI before the reference ARM64 build to catch known pointer-width/address-class mistakes.

## Rendering

The game runs through a native handheld graphics stack using:

- SDL2
- OpenGL ES
- the port's Fast3D host renderer
- ARM64-specific renderer optimisations

Working features include:

- menus
- intro
- in-game 3D rendering
- missions
- HUD
- framebuffer effects
- resolution scaling
- MSAA
- texture filtering
- anisotropic filtering
- adjustable FOV
- draw-distance controls
- model LOD controls
- optional temporal accumulation / TAA-lite

The renderer includes an **R36S Performance** preset designed for the 640×480 target.

## Gameplay

The project has demonstrated:

- game boot
- front end
- intro sequence
- mission loading
- player spawning
- gameplay
- weapons
- controller input
- mission state
- direct mission loading during development
- campaign transition fixes
- save handling improvements
- death/player teardown hardening
- scripted camera protection
- crosshair/aim stabilisation

## Input

The port integrates with PortMaster's controller mapping and SDL GameController.

Implemented handheld behavior includes:

- R36S controls
- analogue input
- controller navigation
- mouse support on desktop builds
- direct/absolute mouse-look work
- **Start + Select** clean exit back to EmulationStation

## Audio

The native audio path is integrated and functional.

Work has also been done around:

- mixer lifecycle
- stage-boundary SFX cleanup
- audio stability
- buffer/latency controls in Port Control

## ARM-GE Glass Control Deck

ARM-GE includes a custom resolution-scalable in-game control interface.

It exposes:

### Video

- fullscreen/output resolution
- 50–200% internal render scale
- graphics presets
- VSync
- frame cap
- MSAA
- TAA-lite
- texture filtering
- mip filtering
- anisotropic filtering
- framebuffer effects
- FOV
- draw distance
- LOD distance

### System

- live FPS
- CPU usage
- process RAM
- available memory
- CPU governor
- GPU governor
- VM swappiness
- CPU profile
- GPU profile
- RAM profile
- allocator trim

### Input / gameplay / audio

The overlay also exposes controller, mouse, gameplay and audio configuration.

System tuning is deliberately handled by the PortMaster launcher rather than by running the game as root. Original kernel values are restored on exit.

## PortMaster integration

The release behaves like a normal handheld port:

1. install the ZIP;
2. provide the legal retail ROM;
3. first launch generates the required host-side data;
4. subsequent launches boot directly into the game.

The game can be launched from the normal **Ports** section in EmulationStation.

---

# What is left on the table

Beta does **not** mean the port is finished.

The remaining work is now mostly real-game correctness, compatibility and polish rather than basic ARM64 bring-up.

## Campaign validation

Every mission and transition still needs systematic real-device completion testing.

Priority areas include:

- mission start state
- spawn position/state
- mission completion
- level-to-level transitions
- title/front-end handoff
- death/restart paths
- save progression
- special mission scripts
- end-game sequences

## Stage / object semantics

Some of the hardest remaining bugs are likely to be in systems where N64-era data structures mix:

- pointers
- segmented addresses
- ROM offsets
- runtime handles
- packed setup data

The main remaining classes are:

- props
- objectives
- doors
- scripted objects
- collision edge cases
- navigation/AI edge cases
- mission-specific setup data

## Graphics polish

Remaining graphics work includes:

- stage-specific rendering defects
- framebuffer-effect validation
- UI scaling edge cases
- texture/filtering edge cases
- GLES driver-specific behavior
- performance tuning on constrained handheld GPUs

## Audio polish

The audio stack works, but longer sessions and stage transitions still need wider validation for:

- degradation over time
- stale SFX state
- mixer lifecycle issues
- buffer behavior across firmware/runtime combinations

## Wider device support

The R36S is the reference target.

Other ARM64 Linux handhelds may work, but wider support requires testing across:

- different Mali GPUs
- different Mesa/Panfrost versions
- different PortMaster runtimes
- different controllers
- different display modes
- different firmware families

---

# The beta build

The public beta is deliberately cleaner than the development environment used to create it.

The release is compiled with:

```text
CMAKE_BUILD_TYPE=Release
GE_BETA_RELEASE=ON
GE_DEV_PROBES=OFF
```

The binary is stripped with:

```bash
aarch64-linux-gnu-strip --strip-unneeded ge007.aarch64
```

## Removed from the player build

The beta excludes development-only instrumentation such as:

- automated benchmark code
- benchmark PortMaster launcher
- DAM-lab debug HUD
- render presentation probes
- D-series developer probes
- frame-dump/debug capture paths
- benchmark CLI strings
- debug symbols

## Retained in the player build

The beta keeps:

- full game code
- ARM-GE Glass Control Deck
- normal runtime logging
- crash handling
- crash screen
- CPU/FPS/RAM telemetry used by Port Control
- controller support
- graphics settings
- audio settings
- performance profiles
- ROM verification
- sidecar generation
- save/config support
- Start + Select exit

---

# What is included in the release?

The recommended release file is:

```text
ge007-r36s-portmaster-beta.zip
```

Its installed structure is approximately:

```text
ports/
├── GoldenEye 007.sh
└── ge007/
    ├── ge007.aarch64
    ├── build-info.txt
    ├── data/
    │   ├── ge007.ini
    │   └── .place-user-rom-and-sidecars-here
    └── prepare-assets/
        ├── ge007-convert
        ├── prepare-assets.py
        ├── d43_emit.py
        ├── d69_emit.py
        ├── d88_emit.py
        ├── d88_propdefs.py
        └── vendor/
```

The package also contains PortMaster metadata:

- `port.json`
- `gameinfo.xml`
- release README

## Not included

The project does **not** distribute:

- GoldenEye ROM images
- `*.z64`
- `*.n64`
- `*.v64`
- generated `pcmodels.bin`
- generated `pccg.bin`
- extracted copyrighted game data

Those are deliberately rejected by the packager and CI release checks.

---

# ROM requirement

You must provide your own legally obtained **GoldenEye 007 US NTSC big-endian ROM**.

Expected filename:

```text
ge007.ntsc-final.z64
```

Expected SHA-1:

```text
abe01e4aeb033b6c0836819f549c791b26cfde83
```

The launcher verifies the SHA-1 when `sha1sum` is available and refuses known-wrong ROMs.

---

# Install on R36S

## Requirements

You need:

- an ARM64 R36S-class handheld
- dArkOSRE / compatible ArkOS environment
- working PortMaster support files
- Python 3 on the handheld for first-run conversion
- the ARM-GE beta ZIP
- your own verified GoldenEye 007 NTSC-U ROM

## Method 1 — PortMaster autoinstall

Download:

**[ge007-r36s-portmaster-beta.zip](https://github.com/bitflipunix-re/ge-pc-port/releases/download/r36s-beta-2026-09-26/ge007-r36s-portmaster-beta.zip)**

Copy it to the PortMaster autoinstall directory.

Typical first-ROM-volume path:

```text
/roms/tools/PortMaster/autoinstall/
```

If your firmware uses the second ROM volume, use the corresponding `roms2` PortMaster path.

Then:

1. start PortMaster;
2. allow it to process the ZIP;
3. exit back to EmulationStation;
4. locate the installed `ge007/` directory;
5. copy your ROM to:

```text
/roms/ports/ge007/data/ge007.ntsc-final.z64
```

6. launch **GoldenEye 007** from the Ports system.

## Method 2 — Direct EmulationStation install

If PortMaster support is already installed, you can extract the ZIP directly into the active ROM volume's `ports/` directory.

After extraction you should have:

```text
/roms/ports/GoldenEye 007.sh
/roms/ports/ge007/
```

Then place the ROM at:

```text
/roms/ports/ge007/data/ge007.ntsc-final.z64
```

Refresh or restart EmulationStation and launch **GoldenEye 007**.

The PortMaster application itself does not need to remain open. The launcher uses PortMaster's installed control/device helpers.

---

# First launch

The first launch performs the complete setup path automatically.

The launcher:

1. loads the PortMaster environment;
2. loads controller mappings;
3. verifies `ge007.aarch64`;
4. checks the ROM;
5. verifies the ROM SHA-1 when possible;
6. checks whether host-format sidecars already exist;
7. runs the bundled Python converter if required;
8. generates the ROM-derived sidecars locally;
9. verifies the generated output;
10. applies any selected temporary performance profile;
11. launches the game.

Generated files include:

```text
ge007/data/pcmodels-ntsc-final/pcmodels.bin
ge007/data/pccg-ntsc-final/pccg.bin
```

These stay on the user's device.

Subsequent launches skip conversion when the required files are already present.

---

# Runtime files

A normal installed game directory looks like:

```text
ge007/
├── ge007.aarch64
├── build-info.txt
├── conf/
├── data/
│   ├── ge007.ntsc-final.z64
│   ├── ge007.ini
│   ├── pcmodels-ntsc-final/
│   └── pccg-ntsc-final/
├── prepare-assets/
└── log.txt
```

The first file to collect when reporting a startup failure or crash is:

```text
ge007/log.txt
```

---

# Controls

PortMaster supplies the SDL controller mapping for the handheld.

Important port controls:

| Action | Control |
|---|---|
| Open Port Control | Select / Back |
| Keyboard Port Control | F10 |
| Exit to EmulationStation | Start + Select |
| Navigate Port Control | D-pad / left stick |
| Change Port Control page | LB / RB |
| Increase / activate | A / X |
| Decrease / back | B / Y |
| Close Port Control | Start |

The original GoldenEye in-game controller configuration remains available inside the game.

---

# Build from source

The reference build is performed on **Ubuntu 24.04 x86_64** and cross-compiled to **AArch64 Linux**.

The canonical CI definition is:

```text
.github/workflows/build-r36s.yml
```

If this README and CI ever disagree, the workflow is the source of truth.

## Clone

```bash
git clone https://github.com/bitflipunix-re/ge-pc-port.git
cd ge-pc-port
```

## Install build dependencies

Enable ARM64 packages:

```bash
sudo dpkg --add-architecture arm64
sudo apt-get update
```

Install the required tools and libraries:

```bash
sudo apt-get install -y \
  cmake make ccache pkg-config unzip file python3 git \
  gcc-aarch64-linux-gnu g++-aarch64-linux-gnu \
  binutils-aarch64-linux-gnu libc6-dev-arm64-cross \
  libsdl2-dev libgles2-mesa-dev libegl1-mesa-dev zlib1g-dev \
  libsdl2-2.0-0:arm64 libgles2:arm64 libegl1:arm64 zlib1g:arm64
```

The reference workflow uses separate Ubuntu archive definitions for amd64 and ARM64 packages. If `apt` cannot locate ARM64 dependencies, mirror the source configuration in `.github/workflows/build-r36s.yml`.

## Extract ARM64 SDL2 development headers

```bash
mkdir -p /tmp/sdl2-arm64-dev
cd /tmp
apt-get download libsdl2-dev:arm64
dpkg-deb -x libsdl2-dev_*_arm64.deb /tmp/sdl2-arm64-dev
cd -
```

Ensure target linker names exist:

```bash
sudo ln -sf libSDL2-2.0.so.0 /usr/lib/aarch64-linux-gnu/libSDL2.so
sudo ln -sf libGLESv2.so.2 /usr/lib/aarch64-linux-gnu/libGLESv2.so
sudo ln -sf libEGL.so.1 /usr/lib/aarch64-linux-gnu/libEGL.so
sudo ln -sf libz.so.1 /usr/lib/aarch64-linux-gnu/libz.so
```

## Run the ARM64 semantic audit

Before compiling:

```bash
python3 work/goldeneye-pc-port/tools_pc/arm64_semantic_audit.py
```

This checks known classes of accidental host-pointer truncation and address/token confusion.

## Configure the beta-equivalent build

```bash
export PKG_CONFIG_LIBDIR=/usr/lib/aarch64-linux-gnu/pkgconfig:/usr/share/pkgconfig
export PKG_CONFIG_SYSROOT_DIR=/

cmake -S work/goldeneye-pc-port -B build/arm64 \
  -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
  -DCMAKE_ASM_COMPILER=aarch64-linux-gnu-gcc \
  -DCMAKE_C_COMPILER_TARGET=aarch64 \
  -DCMAKE_CXX_COMPILER_TARGET=aarch64 \
  -DCMAKE_SYSTEM_NAME=Linux \
  -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
  -DROMID=ntsc-final \
  -DCMAKE_BUILD_TYPE=Release \
  -DGE_BETA_RELEASE=ON \
  -DGE_DEV_PROBES=OFF \
  -DSDL2_INCLUDE_DIR=/usr/include/SDL2 \
  -DSDL2_LIBRARY=/usr/lib/aarch64-linux-gnu/libSDL2.so \
  -DZLIB_INCLUDE_DIR=/usr/include \
  -DZLIB_LIBRARY=/usr/lib/aarch64-linux-gnu/libz.so \
  -DGL_LIBRARY=/usr/lib/aarch64-linux-gnu/libGLESv2.so \
  -DCMAKE_C_FLAGS="-DUSE_GLES=1 -I/tmp/sdl2-arm64-dev/usr/include/aarch64-linux-gnu" \
  -DCMAKE_CXX_FLAGS="-DUSE_GLES=1 -I/tmp/sdl2-arm64-dev/usr/include/aarch64-linux-gnu"
```

A correct configure reports:

```text
Target arch: aarch64
Output binary: ge007.aarch64
ARM-GE beta/player build: diagnostics stripped
Fast3D developer probes: disabled
```

## Compile

```bash
cmake --build build/arm64 -j"$(nproc)"
```

Output:

```text
build/arm64/ge007.aarch64
```

## Strip the player binary

```bash
aarch64-linux-gnu-strip --strip-unneeded build/arm64/ge007.aarch64
```

## Verify the binary

```bash
file build/arm64/ge007.aarch64
aarch64-linux-gnu-readelf -h build/arm64/ge007.aarch64
sha256sum build/arm64/ge007.aarch64
```

The ELF header must report:

```text
Machine: AArch64
```

---

# Build the PortMaster installer

From the repository root:

```bash
python3 package.py \
  --game-bin build/arm64/ge007.aarch64 \
  --out dist
```

Output:

```text
dist/ge007.zip
```

Validate it:

```bash
unzip -t dist/ge007.zip
unzip -l dist/ge007.zip
unzip -p dist/ge007.zip port.json | python3 -m json.tool
sha256sum dist/ge007.zip
```

The packager refuses to ship ROM images and generated ROM-derived sidecars.

---

# Development builds

The beta profile intentionally removes development instrumentation.

For renderer/port investigation, configure without `GE_BETA_RELEASE` and enable only the diagnostics you need.

For example:

```bash
-DGE_DEV_PROBES=ON
```

Development builds may include:

- Fast3D diagnostic probes
- frame dumping
- benchmark hooks
- DAM-lab instrumentation
- additional logging

Those tools are useful for engineering work but are intentionally absent from the public player package.

---

# Repository layout

```text
.
├── .github/workflows/          CI / release automation
├── bundle/prepare-assets/      first-run conversion bundle
├── docs/                       engineering documentation
├── port/                       PortMaster metadata and launcher
├── work/goldeneye-pc-port/     GoldenEye host-port source tree
├── build-arm.sh                ARM build helper
├── package.py                  PortMaster packager
└── README.md
```

Important port-layer areas inside `work/goldeneye-pc-port/` include:

```text
port/src/       host platform implementation
port/include/   host interfaces
port/fast3d/    rendering backend
tools_pc/       development/audit tooling
src/            reconstructed game code
```

---

# Architecture notes

One of the central problems in this port is that not every 32-bit-looking value in GoldenEye means the same thing.

The port must distinguish between:

1. **native host pointers**
2. **N64 / ROM / segmented address tokens**
3. **ordinary integer game state**

Blindly widening every value breaks the game just as easily as leaving a host pointer at 32 bits.

Much of the ARM64 work has therefore been done by semantic class rather than crash-by-crash patching.

That includes:

- sign-extension fixes
- zero-extension fixes
- token preservation
- host-pointer widening
- setup/stage rebasing
- model/animation address handling
- lifetime/teardown fixes
- host-memory reservation
- renderer address translation

---

# Performance philosophy

The port aims to improve host efficiency without changing GoldenEye's simulation semantics.

Current work includes:

- larger same-state Fast3D triangle batches
- optimized Fast3D/RSP translation code
- configurable internal resolution
- model LOD controls
- R36S performance preset
- optional CPU/GPU/RAM launch profiles

Performance profiles do **not** permanently alter system settings.

The launcher records supported original values and restores them when the game exits.

---

# Reporting problems

When reporting a real-device issue, include:

- device model
- firmware
- PortMaster version if known
- mission / menu / exact reproduction path
- whether the issue occurs from a fresh save
- relevant graphics preset/settings
- `ge007/log.txt`
- crash screen values if shown

For stage-specific bugs, include the exact stage and what happened immediately before the failure.

---

# Legal / project boundaries

This repository does not distribute the GoldenEye 007 retail ROM or generated ROM-derived game data.

Users must supply their own legally obtained compatible ROM.

The packaging and release workflow explicitly rejects ROM images and generated sidecar binaries.

ARM-GE is an independent engineering project built around the reconstructed GoldenEye codebase and the surrounding open-source work that made that reconstruction possible.

See the repository notices and source files for applicable attribution and licensing information.

---

# Credits

ARM64/R36S port work and PortMaster integration:

- **bitflipunix**
- **Tomobobo710**

This project also depends on years of work by the GoldenEye decompilation/reconstruction community and PC-port contributors.

---

<div align="center">

## ARM-GE Beta

**Native GoldenEye 007 on ARM64 Linux handheld hardware.**

**[Download the current R36S beta](https://github.com/bitflipunix-re/ge-pc-port/releases/tag/r36s-beta-2026-09-26)**

</div>
