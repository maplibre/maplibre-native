#!/usr/bin/env python3
"""Run alternating built-in/plugin trials with an identical Vulkan environment."""
import argparse
import json
import os
import platform
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("executable", type=Path)
parser.add_argument("--baseline-plugin", type=Path, help="Compare two plugin builds instead of builtin versus plugin")
parser.add_argument("--side", type=int, default=100)
parser.add_argument("--frames", type=int, default=40)
parser.add_argument("--trials", type=int, default=3)
parser.add_argument("--output", type=Path, required=True)
args = parser.parse_args()
if min(args.side, args.frames, args.trials) < 1:
    parser.error("sample sizes must be positive")
env = dict(os.environ)
env.setdefault("LP_NUM_THREADS", "4")
results = []
for trial in range(1, args.trials + 1):
    baseline = "baseline" if args.baseline_plugin else "builtin"
    modes = (baseline, "plugin") if trial % 2 else ("plugin", baseline)
    for mode in modes:
        print(f"Trial {trial}/{args.trials}: {mode}", flush=True)
        executable = args.baseline_plugin if mode == "baseline" else args.executable
        implementation = "plugin" if mode == "baseline" else mode
        run = subprocess.run(
            [str(executable.resolve()), implementation, str(args.side), str(args.frames)],
            env=env, capture_output=True, text=True, check=True, timeout=600,
        )
        for line in run.stdout.splitlines():
            results.append({"trial": trial, **json.loads(line), "implementation": mode})
args.output.write_text(json.dumps({
    "platform": platform.platform(),
    "executables": {"plugin": str(args.executable), "baseline": str(args.baseline_plugin) if args.baseline_plugin else None},
    "environment": {key: env.get(key) for key in ("LP_NUM_THREADS", "VK_DRIVER_FILES", "VK_ICD_FILENAMES")},
    "results": results,
}, indent=2) + "\n")
