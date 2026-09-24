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
    modes = ("builtin", "plugin") if trial % 2 else ("plugin", "builtin")
    for mode in modes:
        print(f"Trial {trial}/{args.trials}: {mode}", flush=True)
        run = subprocess.run(
            [str(args.executable.resolve()), mode, str(args.side), str(args.frames)],
            env=env, capture_output=True, text=True, check=True, timeout=600,
        )
        for line in run.stdout.splitlines():
            results.append({"trial": trial, **json.loads(line)})
args.output.write_text(json.dumps({
    "platform": platform.platform(),
    "environment": {key: env.get(key) for key in ("LP_NUM_THREADS", "VK_DRIVER_FILES", "VK_ICD_FILENAMES")},
    "results": results,
}, indent=2) + "\n")
