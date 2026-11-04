#!/usr/bin/env python3
"""Publish aggregate CIDDS results without embedding source flow records."""
import argparse
import json
from collections import Counter
from pathlib import Path

TEMPLATE = Path(__file__).resolve().parents[1] / "web" / "overview_template.html"
ACTIVITY = {
    "DENIAL_OF_SERVICE": "DoS",
    "PORT_SCAN": "Reconnaissance",
    "FAILED_LOGIN": "Credential",
}


def render(result: Path, evaluation: Path, output: Path) -> None:
    data = json.loads(result.read_text(encoding="utf-8"))
    measured = json.loads(evaluation.read_text(encoding="utf-8"))
    if data.get("schema_version") != 2 or data["summary"]["events_loaded"] != measured["events"]:
        raise ValueError("result and evaluation must describe the same schema-v2 run")
    incidents = data["incidents"]
    if len(incidents) != measured["incidents"]:
        raise ValueError("incident counts disagree")
    overview = {
        "loaded": data["summary"]["events_loaded"],
        "rejected": data["summary"]["events_rejected"],
        "incidents": len(incidents),
        "clustered": sum(len(incident["events"]) for incident in incidents),
        "priorities": dict(Counter(incident["priority"] for incident in incidents)),
        "precision": measured["pairwise_precision"],
        "recall": measured["pairwise_recall"],
        "rows": [
            {
                "id": incident["id"],
                "score": incident["risk_score"],
                "priority": incident["priority"],
                "events": len(incident["events"]),
                "activity": ", ".join(sorted({ACTIVITY.get(event["type"], "Other")
                                             for event in incident["events"]})),
            }
            for incident in incidents
        ],
    }
    payload = json.dumps(overview, separators=(",", ":"))
    payload = payload.replace("<", "\\u003c").replace(">", "\\u003e").replace("&", "\\u0026")
    template = TEMPLATE.read_text(encoding="utf-8")
    if template.count("__OVERVIEW_JSON__") != 1:
        raise ValueError("overview template marker is missing or duplicated")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(template.replace("__OVERVIEW_JSON__", payload), encoding="utf-8")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("result", type=Path)
    parser.add_argument("evaluation", type=Path)
    parser.add_argument("output", type=Path)
    args = parser.parse_args()
    render(args.result, args.evaluation, args.output)
