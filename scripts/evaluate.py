#!/usr/bin/env python3
"""Evaluate pairwise incident grouping against CSV scenario labels."""
import argparse
import csv
import json
import subprocess
from collections import Counter
from math import comb


NEGATIVE = {"", "normal", "unrelated"}


def measure_groups(labels: dict, groups: list) -> dict:
    positive = lambda label: label not in NEGATIVE
    truth_counts = Counter(label for label in labels.values() if positive(label))
    truth_pairs = sum(comb(count, 2) for count in truth_counts.values() if count >= 2)
    predicted_pairs = sum(comb(len(group), 2) for group in groups)
    true_positive = sum(comb(count, 2) for group in groups
                        for label, count in Counter(labels[event_id] for event_id in group).items()
                        if positive(label) and count >= 2)
    false_positive = predicted_pairs - true_positive
    false_negative = truth_pairs - true_positive
    precision = true_positive / predicted_pairs if predicted_pairs else 0.0
    recall = true_positive / truth_pairs if truth_pairs else 0.0
    f1 = 2 * precision * recall / (precision + recall) if precision + recall else 0.0

    details = []
    for label, total in sorted(truth_counts.items()):
        memberships = [sum(labels[event_id] == label for event_id in group) for group in groups]
        found_pairs = sum(comb(count, 2) for count in memberships if count >= 2)
        mixed_pairs = sum(count * (len(group) - count) for count, group in zip(memberships, groups))
        details.append({"label": label, "events": total,
                        "predicted_incidents": sum(count > 0 for count in memberships),
                        "missed_join_pairs": comb(total, 2) - found_pairs,
                        "mixed_label_pairs": mixed_pairs})

    total_attack_events = sum(truth_counts.values())
    coverage = {}
    for limit in (5, 10, 20):
        found = sum(positive(labels[event_id]) for group in groups[:limit] for event_id in group)
        coverage[str(limit)] = found / total_attack_events if total_attack_events else 0.0

    return {"events": len(labels), "incidents": len(groups), "true_pairs": truth_pairs,
            "predicted_pairs": predicted_pairs, "true_positive": true_positive,
            "false_positive": false_positive, "false_negative": false_negative,
            "pairwise_precision": precision, "pairwise_recall": recall, "pairwise_f1": f1,
            "attack_event_coverage_at_rank": coverage, "by_scenario": details}
from pathlib import Path


def evaluate(csv_path: Path, binary: Path, window: int) -> dict:
    with csv_path.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    labels = {row["event_id"]: row.get("scenario_label", "").strip() for row in rows}
    if len(labels) != len(rows):
        raise ValueError("CSV contains duplicate event IDs")
    if not any(label not in NEGATIVE for label in labels.values()):
        raise ValueError("CSV requires non-normal scenario_label values")
    result = subprocess.run(
        [str(binary), str(csv_path), "--format", "json", "--window", str(window)],
        capture_output=True, text=True, check=True,
    )
    output = json.loads(result.stdout)
    if output["summary"]["events_rejected"]:
        raise ValueError("evaluation CSV contains rejected events: " + repr(output["errors"]))
    groups = [set(event["id"] for event in incident["events"]) for incident in output["incidents"]]
    if any(event_id not in labels for group in groups for event_id in group):
        raise ValueError("output references an unknown event")
    return measure_groups(labels, groups)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/soc_cli"))
    parser.add_argument("--window", type=int, default=300)
    args = parser.parse_args()
    print(json.dumps(evaluate(args.csv, args.binary.resolve(), args.window), indent=2))


if __name__ == "__main__":
    main()
