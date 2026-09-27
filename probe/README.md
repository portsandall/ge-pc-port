# R36S deep hardware qualification probe

This branch adds a PortMaster-delivered hardware/driver qualification suite for the R36S/RK3326/Mali-G31. The purpose is to measure the real device stack instead of inferring capability from SDL fallbacks or extension strings.

The native AArch64 probe performs active tests against DRM/KMS, GBM, DMA-BUF, EGL, GLES2, GLES3, desktop OpenGL and Vulkan. Rendering contexts are exercised with an offscreen clear/readback check and shader compilation/linking. GLES3 additionally attempts a 3.1 compute-shader compile. Vulkan is loaded dynamically so a missing loader or ICD is reported rather than preventing the probe from starting.

The PortMaster launcher augments those tests with CPU/RAM/cpufreq, thermals, device tree, loaded modules, DRM sysfs, GPU devfreq/OPP exposure, installed graphics libraries/packages, optional `modetest`/`eglinfo`/`vulkaninfo`, debugfs DRM state when already readable, and filtered GPU/kernel messages.

## Build

The `R36S hardware probe` GitHub Actions workflow cross-compiles the AArch64 probe and produces `r36s-hardware-probe.zip` as an artifact. No ROM or game data is involved.

## Device test

Install the zip with PortMaster, launch **R36S Hardware Probe** from Ports, then retrieve the newest directory under:

`/roms/ports/r36sprobe/reports/`

At minimum return `summary.txt`, `report.json`, `active-probe.txt`, `dmesg-gpu.txt`, `drm-sysfs.txt`, `device-tree-summary.txt`, `cpufreq.txt`, `devfreq-thermal.txt`, and `graphics-libraries.txt`.

The results are intended to decide whether the current firmware limitation is userspace Mesa/EGL, kernel Panfrost/DRM, board device-tree/OPP configuration, or the application itself, and to compare the stock stack against the CFW target (Linux 6.12 LTS + Mesa 26.2.3 Panfrost).
