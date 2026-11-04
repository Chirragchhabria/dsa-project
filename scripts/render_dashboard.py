#!/usr/bin/env python3
"""Render a standalone, offline incident-review page from soc_cli JSON."""
import argparse
import json
from pathlib import Path

TEMPLATE = Path(__file__).resolve().parents[1] / "web" / "template.html"


def render(source: Path, output: Path) -> None:
    if source.resolve() == output.resolve():
        raise ValueError("input JSON and output HTML must be different files")
    data = json.loads(source.read_text(encoding="utf-8"))
    if data.get("schema_version") != 2 or not isinstance(data.get("incidents"), list):
        raise ValueError("expected soc_cli JSON schema version 2")
    payload = json.dumps(data, ensure_ascii=False, separators=(",", ":"))
    payload = payload.replace("<", "\\u003c").replace(">", "\\u003e").replace("&", "\\u0026")
    template = TEMPLATE.read_text(encoding="utf-8")
    if template.count("__INCIDENT_JSON__") != 1:
        raise ValueError("dashboard template marker is missing or duplicated")
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(template.replace("__INCIDENT_JSON__", payload), encoding="utf-8")


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("json", type=Path, help="soc_cli --format json output")
    parser.add_argument("html", type=Path, help="standalone output HTML")
    args = parser.parse_args()
    render(args.json, args.html)
    print(args.html)


if __name__ == "__main__":
    main()
