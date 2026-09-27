#!/bin/bash
# R36S deep hardware/driver qualification probe for PortMaster.
# Read-only except for creating its own report directory.

XDG_DATA_HOME=${XDG_DATA_HOME:-$HOME/.local/share}
if [ -d "/opt/system/Tools/PortMaster/" ]; then
  controlfolder="/opt/system/Tools/PortMaster"
elif [ -d "/opt/tools/PortMaster/" ]; then
  controlfolder="/opt/tools/PortMaster"
elif [ -d "$XDG_DATA_HOME/PortMaster/" ]; then
  controlfolder="$XDG_DATA_HOME/PortMaster"
else
  controlfolder="/roms/ports/PortMaster"
fi

if [ -f "$controlfolder/control.txt" ]; then
  source "$controlfolder/control.txt"
  [ -f "${controlfolder}/mod_${CFW_NAME}.txt" ] && source "${controlfolder}/mod_${CFW_NAME}.txt"
  type get_controls >/dev/null 2>&1 && get_controls
fi

DIRECTORY_ROOT="${directory:-roms}"
GAMEDIR="/$DIRECTORY_ROOT/ports/r36sprobe"
BINARY="$GAMEDIR/r36s-hwprobe.aarch64"
STAMP="$(date +%Y%m%d-%H%M%S 2>/dev/null || echo boot)"
REPORT="$GAMEDIR/reports/$STAMP"
mkdir -p "$REPORT"
cd "$GAMEDIR" || exit 1

# Show the live probe log on the same virtual console PortMaster uses.
# This deliberately avoids SDL/EGL so display feedback does not depend on
# the graphics stack we are trying to diagnose.
PROBE_TTY="${CUR_TTY:-/dev/tty0}"
export TERM=linux
if [ -c "$PROBE_TTY" ]; then
  if [ -n "${ESUDO:-}" ]; then
    $ESUDO chmod 666 "$PROBE_TTY" >/dev/null 2>&1 || true
  else
    chmod 666 "$PROBE_TTY" >/dev/null 2>&1 || true
  fi
fi

if [ -w "$PROBE_TTY" ]; then
  printf '\033c\033[2J\033[H' > "$PROBE_TTY" 2>/dev/null || true
  printf 'R36S HARDWARE PROBE\n-------------------\nStarting...\n\n' > "$PROBE_TTY" 2>/dev/null || true
  exec > >(tee "$REPORT/full.log" "$PROBE_TTY") 2>&1
else
  exec > >(tee "$REPORT/full.log") 2>&1
fi

echo "=== R36S Hardware Qualification Probe ==="
echo "timestamp=$STAMP"
echo "gamedir=$GAMEDIR"
echo "portmaster.controlfolder=${controlfolder:-unavailable}"
echo "portmaster.cfw=${CFW_NAME:-unknown}"
echo "portmaster.device=${DEVICE:-unknown}"
echo "portmaster.param_device=${param_device:-unknown}"
echo

section() { echo; echo "===== $* ====="; }
copy_readable() {
  src="$1"; dst="$2"
  [ -r "$src" ] && cat "$src" > "$REPORT/$dst" 2>/dev/null || true
}

section "SYSTEM"
uname -a 2>&1 | tee "$REPORT/uname.txt"
[ -r /etc/os-release ] && cat /etc/os-release | tee "$REPORT/os-release.txt"
[ -r /proc/version ] && cat /proc/version | tee "$REPORT/proc-version.txt"
command -v ldd >/dev/null 2>&1 && ldd --version 2>&1 | head -n 2 | tee "$REPORT/libc.txt"

section "CPU"
cat /proc/cpuinfo 2>/dev/null | tee "$REPORT/cpuinfo.txt"
command -v lscpu >/dev/null 2>&1 && lscpu 2>&1 | tee "$REPORT/lscpu.txt"
{
  for p in /sys/devices/system/cpu/cpufreq/policy*; do
    [ -d "$p" ] || continue
    echo "[$p]"
    for f in affected_cpus cpuinfo_cur_freq cpuinfo_max_freq cpuinfo_min_freq scaling_available_frequencies scaling_available_governors scaling_cur_freq scaling_driver scaling_governor scaling_max_freq scaling_min_freq; do
      [ -r "$p/$f" ] && printf '%s=' "$f" && cat "$p/$f"
    done
  done
} | tee "$REPORT/cpufreq.txt"

section "MEMORY"
cat /proc/meminfo 2>/dev/null | tee "$REPORT/meminfo.txt"
command -v free >/dev/null 2>&1 && free -h 2>&1 | tee "$REPORT/free.txt"
copy_readable /proc/iomem iomem.txt
copy_readable /proc/zoneinfo zoneinfo.txt

section "BOARD_DEVICE_TREE"
{
  for f in /proc/device-tree/model /sys/firmware/devicetree/base/model; do
    [ -r "$f" ] && printf '%s=' "$f" && tr '\000' ' ' < "$f" && echo
  done
  for f in /proc/device-tree/compatible /sys/firmware/devicetree/base/compatible; do
    [ -r "$f" ] && printf '%s=' "$f" && tr '\000' '\n' < "$f"
  done
} | tee "$REPORT/device-tree-summary.txt"
if [ -d /sys/firmware/devicetree/base ]; then
  find /sys/firmware/devicetree/base -maxdepth 4 -type f \( -name compatible -o -name status -o -name clock-frequency -o -name operating-points-v2 -o -name interrupts -o -name reg \) -print 2>/dev/null | sort > "$REPORT/device-tree-files.txt"
fi

section "KERNEL_MODULES"
cat /proc/modules 2>/dev/null | tee "$REPORT/modules.txt"
for m in panfrost rockchipdrm drm gpu_sched; do
  command -v modinfo >/dev/null 2>&1 && modinfo "$m" 2>&1 | sed "s/^/[$m] /"
done | tee "$REPORT/modinfo-gpu.txt"

section "DRM_SYSFS"
ls -la /dev/dri 2>&1 | tee "$REPORT/dev-dri.txt"
{
  find /sys/class/drm -maxdepth 2 -type f \( -name status -o -name modes -o -name enabled -o -name uevent -o -name edid \) -print 2>/dev/null | sort
  for d in /sys/class/drm/card* /sys/class/drm/renderD*; do
    [ -e "$d" ] || continue
    echo "--- $d ---"
    readlink -f "$d/device/driver" 2>/dev/null || true
    readlink -f "$d/device/driver/module" 2>/dev/null || true
    [ -r "$d/device/uevent" ] && cat "$d/device/uevent"
  done
} | tee "$REPORT/drm-sysfs.txt"

section "GPU_KERNEL_DEVICE"
{
  echo "[device nodes]"
  ls -la /dev/mali* /dev/dri/* 2>&1 || true
  echo "[platform drivers]"
  for d in /sys/bus/platform/drivers/*mali* /sys/bus/platform/drivers/*gpu* /sys/bus/platform/drivers/panfrost*; do
    [ -e "$d" ] || continue
    echo "$d"
    ls -la "$d" 2>/dev/null || true
  done
  echo "[GPU device tree]"
  for d in /sys/firmware/devicetree/base/*gpu* /sys/firmware/devicetree/base/gpu@*; do
    [ -d "$d" ] || continue
    echo "--- $d ---"
    for f in compatible status clock-names operating-points-v2; do
      if [ -r "$d/$f" ]; then
        printf '%s=' "$f"
        tr '\\000' ' ' < "$d/$f" 2>/dev/null || true
        echo
      fi
    done
  done
} | tee "$REPORT/gpu-kernel-device.txt"

section "ACTIVE_NATIVE_PROBE"
chmod +x "$BINARY" 2>/dev/null || true
if [ -x "$BINARY" ]; then
  {
    echo "[binary]"
    file "$BINARY" 2>/dev/null || true
    echo "[runtime dependencies]"
    ldd "$BINARY" 2>&1 || true
  } | tee "$REPORT/native-binary-runtime.txt"
  "$BINARY" 2>&1 | tee "$REPORT/active-probe.txt"
  echo "active_probe_rc=${PIPESTATUS[0]}"
else
  echo "MISSING $BINARY" | tee "$REPORT/active-probe.txt"
fi

section "GPU_DEVFREQ_THERMAL"
{
  for d in /sys/class/devfreq/* /sys/devices/platform/*gpu*/devfreq/*; do
    [ -d "$d" ] || continue
    case "$d" in *gpu*|*ff9a0000*|*mali*) ;; *) continue ;; esac
    echo "[$d]"
    for f in name governor available_governors available_frequencies cur_freq min_freq max_freq trans_stat; do
      [ -r "$d/$f" ] && printf '%s=' "$f" && cat "$d/$f"
    done
  done
  for z in /sys/class/thermal/thermal_zone*; do
    [ -d "$z" ] || continue
    printf '%s type=' "$z"; cat "$z/type" 2>/dev/null || true
    printf '%s temp=' "$z"; cat "$z/temp" 2>/dev/null || true
  done
} | tee "$REPORT/devfreq-thermal.txt"

section "GRAPHICS_USERSPACE"
{
  echo "[ldconfig]"
  command -v ldconfig >/dev/null 2>&1 && ldconfig -p 2>/dev/null | grep -Ei 'lib(mali|EGL|GLES|GLX|OpenGL|gbm|drm|vulkan)' || true
  echo "[known graphics directories]"
  for d in /usr/lib/aarch64-linux-gnu/dri /usr/lib/dri /usr/lib/aarch64-linux-gnu /lib/aarch64-linux-gnu /usr/share/vulkan/icd.d /etc/vulkan/icd.d; do
    [ -d "$d" ] || continue
    echo "--- $d ---"
    ls -la "$d" 2>/dev/null | grep -Ei 'mali|pan|dri|EGL|GLES|GL\.so|gbm|drm|vulkan|icd' || true
  done
  echo "[specific files]"
  for p in \
    /usr/lib*/libMali.so* /usr/lib*/libmali.so* /lib*/libMali.so* /lib*/libmali.so* \
    /usr/lib/*/libMali.so* /usr/lib/*/libmali.so* /lib/*/libMali.so* /lib/*/libmali.so* \
    /usr/lib/*/dri/panfrost_dri.so /usr/lib/*/dri/*pan*_dri.so; do
    [ -e "$p" ] && ls -l "$p"
  done
} | tee "$REPORT/graphics-libraries.txt"

if command -v dpkg-query >/dev/null 2>&1; then
  dpkg-query -W 2>/dev/null | grep -Ei 'mesa|libdrm|vulkan|panfrost|mali|linux-image' | tee "$REPORT/packages-gpu.txt" || true
fi

section "OPTIONAL_SYSTEM_TOOLS"
command -v modetest >/dev/null 2>&1 && modetest -c -p -f 2>&1 | tee "$REPORT/modetest.txt" || true
command -v eglinfo >/dev/null 2>&1 && eglinfo -B 2>&1 | tee "$REPORT/eglinfo.txt" || true
command -v glxinfo >/dev/null 2>&1 && glxinfo -B 2>&1 | tee "$REPORT/glxinfo.txt" || true
command -v vulkaninfo >/dev/null 2>&1 && vulkaninfo --summary 2>&1 | tee "$REPORT/vulkaninfo.txt" || true

section "KERNEL_GPU_LOG"
if command -v dmesg >/dev/null 2>&1; then
  if dmesg >/dev/null 2>&1; then
    dmesg 2>&1 | grep -Ei 'panfrost|panthor|mali|gpu|drm|rockchip|vop|display|devfreq|opp|iommu|clk|regulator|firmware|failed|error|timeout|fault' | tee "$REPORT/dmesg-gpu.txt" || true
  elif [ -n "${ESUDO:-}" ]; then
    $ESUDO dmesg 2>&1 | grep -Ei 'panfrost|panthor|mali|gpu|drm|rockchip|vop|display|devfreq|opp|iommu|clk|regulator|firmware|failed|error|timeout|fault' | tee "$REPORT/dmesg-gpu.txt" || true
  fi
fi

section "DEBUGFS_DRM"
if [ -d /sys/kernel/debug/dri ]; then
  find /sys/kernel/debug/dri -maxdepth 2 -type f -readable -print 2>/dev/null | sort | tee "$REPORT/debugfs-dri-files.txt"
  for f in /sys/kernel/debug/dri/*/name /sys/kernel/debug/dri/*/clients /sys/kernel/debug/dri/*/state; do
    [ -r "$f" ] && { echo "--- $f ---"; cat "$f"; }
  done | tee "$REPORT/debugfs-dri-summary.txt"
else
  echo "debugfs DRM not mounted/readable" | tee "$REPORT/debugfs-dri-summary.txt"
fi

section "SUMMARY"
{
  echo "report_dir=$REPORT"
  grep -E '^(system|cpu|memory|kms|drm|gbm|egl|gles|opengl|vulkan|probe)\.' "$REPORT/active-probe.txt" 2>/dev/null || true
} | tee "$REPORT/summary.txt"

if command -v python3 >/dev/null 2>&1; then
  python3 - "$REPORT" <<'PY'
import json, pathlib, sys
p = pathlib.Path(sys.argv[1])
summary = {}
active = p / "active-probe.txt"
if active.exists():
    for line in active.read_text(errors="replace").splitlines():
        if "=" in line and not line.startswith("["):
            k, v = line.split("=", 1)
            if k and all(c.isalnum() or c in "._-" for c in k):
                summary[k] = v
meta = {
    "schema": 1,
    "summary": summary,
    "files": sorted(x.name for x in p.iterdir() if x.is_file()),
}
(p / "report.json").write_text(json.dumps(meta, indent=2, sort_keys=True) + "\n")
PY
fi

echo
echo "Probe complete: $REPORT"
echo "Returning to EmulationStation..."
sleep 2
type pm_message >/dev/null 2>&1 && pm_message "R36S hardware probe complete. Report saved under r36sprobe/reports/$STAMP"
type pm_finish >/dev/null 2>&1 && pm_finish
exit 0
