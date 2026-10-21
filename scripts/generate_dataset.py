#!/usr/bin/env python3
"""Generate deterministic synthetic alert data for repeatable load experiments."""
import argparse
import csv
import random
from pathlib import Path

FIELDS = ["event_id", "timestamp", "source_ip", "destination_ip", "username", "hostname",
          "event_type", "severity", "description", "source", "scenario_label"]


def generate(path: Path, incidents: int, noise: int, seed: int) -> None:
    if incidents < 0 or noise < 0:
        raise ValueError("counts must be non-negative")
    rng = random.Random(seed)
    rows = []
    base = 1722500000
    for index in range(incidents):
        start = base + index * 1200
        source_ip = f"192.0.2.{index % 250 + 1}"
        username = f"account-{index}"
        for offset, event_type, severity in [(0, "FAILED_LOGIN", "MEDIUM"),
                                             (35, "SUCCESSFUL_LOGIN", "HIGH"),
                                             (70, "PRIVILEGE_ESCALATION", "CRITICAL")]:
            rows.append([f"I{index:06d}-{offset}", start + offset, source_ip,
                         f"10.0.{index // 250 % 250}.{index % 250 + 1}", username,
                         f"host-{index}", event_type, severity, "Synthetic incident event",
                         "generator", f"scenario-{index}"])
    for index in range(noise):
        timestamp = base + rng.randrange(max(1, incidents * 1200 + 1200))
        rows.append([f"N{index:06d}", timestamp, f"198.51.100.{index % 250 + 1}",
                     "", f"normal-{index}", f"routine-{index}", "NORMAL_ACTIVITY",
                     "INFORMATIONAL", "Synthetic routine activity", "generator", "normal"])
    rng.shuffle(rows)
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream)
        writer.writerow(FIELDS)
        writer.writerows(rows)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("output", type=Path)
    parser.add_argument("--incidents", type=int, default=100)
    parser.add_argument("--noise", type=int, default=1000)
    parser.add_argument("--seed", type=int, default=42)
    args = parser.parse_args()
    generate(args.output, args.incidents, args.noise, args.seed)


if __name__ == "__main__":
    main()
