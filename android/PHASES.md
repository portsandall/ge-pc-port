# GEAND Android port phases

This file is the source of truth for the Android port. A phase is complete only when its CI gate is green.

## P0 — Toolchain bootstrap
**Gate:** GitHub Actions installs the pinned Android NDK and compiles an arm64-v8a smoke object.

Status: **complete**

## P1 — NDK/Clang source compatibility
**Gate:** GoldenEye host/core sources configure and compile with NDK Clang for arm64-v8a without GCC-only compiler extensions.

Work:
- Resolve the decomp's inherited-struct syntax.
- Separate GNU/Linux-only flags and host-header assumptions.
- Add Android platform definitions.
- Keep compile-time layout assertions for sensitive structs.

Status: **in progress**

## P2 — Android-safe address model
**Gate:** Android native target links successfully with PIE/shared-library semantics and no Linux `-no-pie` / fixed executable text address.

Work:
- Keep cart/DRAM address translation explicit.
- Audit fixed `mmap` reservations on Android.
- Remove assumptions that the ELF image itself occupies a specific range.

Status: pending

## P3 — Native Android game library
**Gate:** CI emits an arm64-v8a `libge007.so` with all required game/port symbols resolved.

Status: pending

## P4 — SDL Android shell + GLES
**Gate:** CI builds an installable debug APK containing `libge007.so`, SDL Android glue and GLES renderer.

Status: pending

## P5 — Runtime boot
**Gate:** Real Android device reaches the GoldenEye startup/intro and emits a clean startup log.

Status: pending

## P6 — User-accessible data and saves
**Gate:** ROM, generated sidecars, config and saves operate from Android-accessible storage with no root/ADB requirement.

Status: pending

## P7 — Input
**Gate:** Physical controller works end-to-end; touchscreen overlay can provide the required N64 controls.

Status: pending

## P8 — Audio + lifecycle
**Gate:** SDL audio works and pause/resume/background/foreground transitions do not hang or corrupt the scheduler.

Status: pending

## P9 — APK CI artifact
**Gate:** Every green GEAND build uploads an unsigned/debug arm64 APK plus checksums and a concise build report.

Status: pending

## P10 — Device validation
**Gate:** Menu, Dam, multiple missions, saves, audio, input and lifecycle are validated on real arm64 Android hardware.

Status: pending
