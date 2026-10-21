#!/usr/bin/env python3
"""Evaluate pairwise incident grouping against CSV scenario labels."""
import argparse
import csv
import json
import subprocess
from pathlib import Path


def evaluate(csv_path: Path, binary: Path, window: int) -> dict:
    with csv_path.open(newline="", encoding="utf-8") as stream:
        rows = list(csv.DictReader(stream))
    labels = {row["event_id"]: row.get("scenario_label", "").strip() for row in rows}
    if len(labels) != len(rows):
        raise ValueError("CSV contains duplicate event IDs")
    if not any(label and label != ("normal", "unrelated") for label in labels.values()):
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
    predicted_pairs = {tuple(sorted((left, right))) for group in groups
                       for left in group for right in group if left < right}
    truth_pairs = {tuple(sorted((left, right))) for left in labels for right in labels
                   if left < right and labels[left] == labels[right]
                   and labels[left] not in ("", "normal", "unrelated")}
    true_positive = len(predicted_pairs & truth_pairs)
    false_positive = len(predicted_pairs - truth_pairs)
    false_negative = len(truth_pairs - predicted_pairs)
    precision = true_positive / (true_positive + false_positive) if predicted_pairs else 0.0
    recall = true_positive / (true_positive + false_negative) if truth_pairs else 0.0
    f1 = 2 * precision * recall / (precision + recall) if precision + recall else 0.0
    return {"events": len(rows), "incidents": len(groups), "true_pairs": len(truth_pairs),
            "predicted_pairs": len(predicted_pairs), "true_positive": true_positive,
            "false_positive": false_positive, "false_negative": false_negative,
            "pairwise_precision": precision, "pairwise_recall": recall, "pairwise_f1": f1}


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", type=Path)
    parser.add_argument("--binary", type=Path, default=Path("build/soc_cli"))
    parser.add_argument("--window", type=int, default=300)
    args = parser.parse_args()
    print(json.dumps(evaluate(args.csv, args.binary.resolve(), args.window), indent=2))


if __name__ == "__main__":
    main()
