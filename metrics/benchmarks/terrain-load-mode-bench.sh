#!/usr/bin/env bash
#
# Terrain FPV-Flight load-mode benchmark (MapLibre Native, Android OpenGL).
#
# Runs the "3D Terrain (FPV flight)" activity three times - once per TerrainLoadMode
# (quality, balanced, performance) - over the SAME deterministic baked flight path, with the
# rendering-stats HUD on, and captures the machine-parseable PERF-HUD log line
#   PERF-HUD fps=.. worstFrameMs=.. jank=.. maxEncodeMs=..
# (emitted once per HUD interval; every counter resets each interval, so each line is an
# independent per-interval sample). Prints a per-mode comparison table.
#
# The flight camera animation is a fixed baked path (identical across modes); only the
# tile-build + drape-render budgets differ by mode, so the windows are directly comparable.
# The mode effect shows up mostly in worstFrameMs / jank (smoothness), less in average fps.
#
# Intended for cross-device comparison of the TerrainLoadMode budgets: run it on a high-end
# and a low-end device (same git commit, same secs-per-mode) and compare the summaries. The
# budgets trade initial-load sharpness for smoother frames, so they are expected to help most
# on weaker GPUs; on a fast GPU that snap-loads in one frame, Quality is usually smoothest.
#
# Usage:
#   metrics/benchmarks/terrain-load-mode-bench.sh <output-dir> [abi] [secs-per-mode] [build:0|1]
#     <output-dir>     REQUIRED. Any writable dir; keep the raw perf_*.log + summary.txt.
#     [abi]            device ABI for the build, default arm64-v8a (use x86_64 for emulator)
#     [secs-per-mode]  steady-state capture window seconds per mode, default 90
#     [build]          1 = build+install openglDebug first (default), 0 = use installed app
#
# Run from the repo root. Requires adb on PATH + one device connected. To compare two devices,
# both must run the SAME git commit and the SAME secs-per-mode; then diff their summary.txt
# (and bench_meta.txt records the device model / GPU / git sha for each run).

set -u
export MSYS_NO_PATHCONV=1   # keep git-bash on Windows from mangling the pkg/.Activity arg (no-op elsewhere)

PKG=org.maplibre.android.testapp
ACT=.activity.style.TerrainFlightActivity

OUT="${1:-}"; ABI="${2:-arm64-v8a}"; SECS="${3:-90}"; BUILD="${4:-1}"
WARMUP=14   # seconds to let the map load + flight start; the load spike is BEFORE the window

if [ -z "$OUT" ]; then
  echo "usage: $0 <output-dir> [abi=arm64-v8a] [secs-per-mode=90] [build=1]"; exit 1
fi
mkdir -p "$OUT" || { echo "cannot create $OUT"; exit 1; }
command -v adb >/dev/null 2>&1 || { echo "ERROR: adb not on PATH"; exit 1; }
adb get-state >/dev/null 2>&1 || { echo "ERROR: no device/emulator (adb devices)"; exit 1; }

if [ "$BUILD" = "1" ]; then
  echo ">> building + installing :MapLibreAndroidTestApp:installOpenglDebug ($ABI)"
  ( cd platform/android && ./gradlew :MapLibreAndroidTestApp:installOpenglDebug -Pmaplibre.abis="$ABI" ) \
    || { echo "ERROR: build/install failed"; exit 1; }
fi

adb logcat -G 16M >/dev/null 2>&1   # enlarge ring buffer so a full window can't drop lines

MODEL=$(adb shell getprop ro.product.model 2>/dev/null | tr -d '\r')
GPUHW=$(adb shell getprop ro.hardware.egl 2>/dev/null | tr -d '\r')
GITSHA=$(git rev-parse --short HEAD 2>/dev/null || echo "?")
{
  echo "device_model=$MODEL"
  echo "egl_hw=$GPUHW"
  echo "abi=$ABI"
  echo "secs_per_mode=$SECS"
  echo "warmup_s=$WARMUP"
  echo "git=$GITSHA"
  echo "date=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
} | tee "$OUT/bench_meta.txt"
echo

for M in quality balanced performance; do
  echo ">> mode: $M"
  adb shell am force-stop "$PKG" >/dev/null 2>&1
  # launch flight fresh (restarts the baked path at t=0); terrain on, stats on, mode from frame 1
  adb shell am start -n "$PKG/$ACT" --es mode "$M" --ez stats true --ez terrain true >/dev/null 2>&1
  echo "   warmup ${WARMUP}s (load + flight start; excluded from the window)..."
  sleep "$WARMUP"
  adb logcat -c >/dev/null 2>&1          # <-- clear AFTER warmup: window starts clean here
  echo "   capturing ${SECS}s of steady-state flight..."
  sleep "$SECS"
  adb logcat -d -v time 2>/dev/null | grep "PERF-HUD" > "$OUT/perf_$M.log"   # dump just this window
  adb shell am force-stop "$PKG" >/dev/null 2>&1
  echo "   -> $(grep -c PERF-HUD "$OUT/perf_$M.log" 2>/dev/null || echo 0) samples in perf_$M.log"
  echo
done

# The test app PERSISTS the load mode across launches, so leaving the device on the last
# mode benchmarked (performance) silently changes behaviour for any manual testing that
# follows - Performance caps drape re-renders per frame, which looks like drape "flicker"
# while zooming out. Restore the default (Quality) so the device is left as found.
echo ">> restoring load mode to quality (default)"
adb shell am force-stop "$PKG" >/dev/null 2>&1
adb shell am start -n "$PKG/$ACT" --es mode quality >/dev/null 2>&1
sleep 6
adb shell am force-stop "$PKG" >/dev/null 2>&1
echo

# ---- summary (portable: sort + awk percentiles) ----
pct() { sort -n | awk -v p="$1" '{a[++n]=$1} END{ if(n==0){print "NA"; exit} i=int(p/100*n+0.5); if(i<1)i=1; if(i>n)i=n; printf "%.1f", a[i] }'; }
meanpos() { awk '$1>0{s+=$1;n++} END{ printf "%.1f", (n? s/n : 0) }'; }

echo "==================== SUMMARY: $MODEL (git $GITSHA, ${SECS}s/mode) ===================="
printf "%-11s | %-6s | %-22s | %-22s | %-9s | %-8s\n" mode samples "fps mean/p5/min(>0)" "worstMs med/p95/max" "jank/s" "enc p95/max"
printf -- "------------+--------+------------------------+------------------------+-----------+----------\n"
{
printf "device=%s  gpu=%s  git=%s  secs/mode=%s\n" "$MODEL" "$GPUHW" "$GITSHA" "$SECS"
for M in quality balanced performance; do
  f="$OUT/perf_$M.log"
  n=$(grep -c PERF-HUD "$f" 2>/dev/null || echo 0)
  if [ "$n" -eq 0 ]; then printf "%-11s | %-6s | (no samples)\n" "$M" 0; continue; fi
  fps=$(grep -oE "fps=[0-9.]+" "$f" | cut -d= -f2)
  worst=$(grep -oE "worstFrameMs=[0-9.]+" "$f" | cut -d= -f2)
  enc=$(grep -oE "maxEncodeMs=[0-9.]+" "$f" | cut -d= -f2)
  jank=$(grep -oE "jank=[0-9]+" "$f" | cut -d= -f2)
  fmean=$(echo "$fps" | meanpos)
  fp5=$(echo "$fps" | awk '$1>0' | pct 5)
  fmin=$(echo "$fps" | awk '$1>0' | pct 0)
  wmed=$(echo "$worst" | pct 50); wp95=$(echo "$worst" | pct 95); wmax=$(echo "$worst" | pct 100)
  ep95=$(echo "$enc" | pct 95); emax=$(echo "$enc" | pct 100)
  janks=$(echo "$jank" | awk -v s="$SECS" '{t+=$1} END{ printf "%.2f", (s? t/s : 0) }')
  printf "%-11s | %-6s | %-22s | %-22s | %-9s | %-8s\n" \
    "$M" "$n" "$fmean/$fp5/$fmin" "$wmed/$wp95/$wmax" "$janks" "$ep95/$emax"
done
} | tee "$OUT/summary.txt"
printf -- "------------+--------+------------------------+------------------------+-----------+----------\n"
echo "fps: higher better. worstMs/enc: lower better (frame-time & CPU-encode spikes). jank/s: frames over"
echo "the jank threshold per second (lower = smoother). Raw logs + summary.txt in: $OUT"
