#!/usr/bin/env python3
"""Black-box checks for the CLI contract."""
import json
import subprocess
import sys
import tempfile
from pathlib import Path

binary = Path(sys.argv[1]).resolve()
fixture = Path(sys.argv[2]).resolve()
result = subprocess.run([str(binary), str(fixture), "--format", "json"],
                        check=True, text=True, capture_output=True)
assert not result.stderr
payload = json.loads(result.stdout)
assert payload["schema_version"] == 1
assert payload["summary"]["events_loaded"] == 19
assert payload["summary"]["incidents"] == 3
assert [item["risk_score"] for item in payload["incidents"]] == [100, 91, 41]
assert [item["id"] for item in payload["incidents"]] == ["INC-0003", "INC-0001", "INC-0002"]
assert all(item["events"] and item["explanations"] for item in payload["incidents"])

changed = subprocess.run([str(binary), str(fixture), "--format", "json", "--min-events", "5"],
                         check=True, text=True, capture_output=True)
assert json.loads(changed.stdout)["summary"]["incidents"] == 1

invalid = subprocess.run([str(binary), "--window", "abc"], text=True, capture_output=True)
assert invalid.returncode != 0 and "positive integer" in invalid.stderr

with tempfile.TemporaryDirectory() as directory:
    file = Path(directory) / "bad.csv"
    file.write_text('event_id,timestamp,source_ip,event_type,severity,description\n'
                    'E1,100,192.0.2.1,FAILED_LOGIN,MEDIUM,"quote ""inside"" field"\n'
                    'E2,bad,192.0.2.1,FAILED_LOGIN,MEDIUM,bad\n', encoding="utf-8")
    output = subprocess.run([str(binary), str(file), "--format", "json"],
                            check=True, text=True, capture_output=True)
    data = json.loads(output.stdout)
    assert data["summary"]["events_rejected"] == 1
    assert len(data["errors"]) == 1
print("CLI integration checks passed")
