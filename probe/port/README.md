# R36S Hardware Probe

Launch this from the PortMaster/EmulationStation Ports menu. It is designed to be read-only with respect to the operating system and writes all output beneath `r36sprobe/reports/<timestamp>/`.

The native probe directly opens DRM nodes and tests KMS resources, DRM capabilities, GBM buffer allocation, DMA-BUF export, EGL over GBM, GLES2/GLES3 context creation, offscreen GPU clear/readback, shader compilation/linking, GLES compute-shader compilation, desktop OpenGL through EGL, and Vulkan loader/physical-device/logical-device creation. The launcher also captures CPU, RAM, cpufreq, thermal, device-tree, loaded-module, graphics-library, DRM sysfs and filtered kernel-log information.

The most useful files to return after a run are `summary.txt`, `report.json`, `active-probe.txt`, `dmesg-gpu.txt`, `drm-sysfs.txt`, `device-tree-summary.txt`, `cpufreq.txt`, `devfreq-thermal.txt`, and `graphics-libraries.txt`.
