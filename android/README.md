# GEAND — GoldenEye 007 Android/AArch64 port

This directory is the Android port workspace for the preserved ARM64 GoldenEye PC port.

## Target

- Android arm64-v8a
- Android 11+ initially (API 30)
- Android NDK / Clang
- SDL2 Android frontend
- OpenGL ES renderer
- User-accessible game data directory; no ROM or copyrighted game data is distributed
- GitHub Actions as the primary build environment

## Current phase: bootstrap

The existing R36S port is already AArch64 and GLES-capable, but it is a Linux executable build. Android is not just another Linux target.

Known blockers found during bootstrap:

1. The host CMake build deliberately prefers GCC.
2. The decomp uses `#define inherits struct` plus GCC/Plan-9 struct embedding semantics.
3. The Android NDK toolchain is Clang-only, so the inherited-struct source needs a portable representation before the game can compile with the NDK.
4. The Linux build disables PIE and places the executable at a fixed address. Android requires PIE/shared-library-compatible code, so that memory model must be isolated from Android.
5. SDL2 must use its Android integration rather than Ubuntu/Linux SDL development packages.
6. The final Android application should expose storage/game-data paths through Android APIs rather than assuming PortMaster/Linux paths.

## Layout

- `android/preflight.sh` — validates the Android build host and audits known portability blockers.
- `.github/workflows/build-android.yml` — reproducible NDK/arm64-v8a CI bootstrap.
- Existing game source remains under `work/goldeneye-pc-port/`.

## Planned port sequence

1. Establish reproducible NDK arm64-v8a CI.
2. Replace/transform GCC-only inherited structs so Clang produces the same layouts.
3. Split Linux fixed-address assumptions from Android memory handling.
4. Build the core as an Android native shared library.
5. Add SDL Android activity/bootstrap and GLES context creation.
6. Move ROM/data discovery to an Android user-accessible location.
7. Produce an unsigned debug APK in CI.
8. Test startup, menus, Dam, input, audio and lifecycle handling on real arm64 Android hardware.

## Legal/data policy

The repository must not contain a GoldenEye ROM or extracted copyrighted assets. Users provide their own compatible game data.
