#!/usr/bin/env python3
"""Measure wall time of a reproducible synthetic CLI run."""
import argparse
import json
import platform
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from generate_dataset import generate


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", type=Path, default=Path("build/soc_cli"))
    parser.add_argument("--incidents", type=int, default=100)
    parser.add_argument("--noise", type=int, default=1000)
    parser.add_argument("--seed", type=int, default=42)
    parser.add_argument("--runs", type=int, default=5)
    args = parser.parse_args()
    if args.runs < 1:
        parser.error("--runs must be positive")
    with tempfile.TemporaryDirectory() as directory:
        dataset = Path(directory) / "events.csv"
        generate(dataset, args.incidents, args.noise, args.seed)
        durations = []
        for _ in range(args.runs):
            start = time.perf_counter()
            subprocess.run([str(args.binary.resolve()), str(dataset), "--format", "json"],
                           check=True, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
            durations.append(time.perf_counter() - start)
    print(json.dumps({"platform": platform.platform(), "python": sys.version.split()[0],
                      "binary": str(args.binary.resolve()), "incidents_generated": args.incidents,
                      "noise_events": args.noise, "total_events": args.incidents * 3 + args.noise,
                      "seed": args.seed, "runs": args.runs,
                      "wall_seconds": durations, "median_wall_seconds": statistics.median(durations)},
                     indent=2))


if __name__ == "__main__":
    main()
