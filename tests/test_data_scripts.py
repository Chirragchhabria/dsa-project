#!/usr/bin/env python3
"""Small behavior checks for the dataset adapter and evaluator."""
import csv
import sys
import tempfile
import zipfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "scripts"))
from evaluate import measure_groups
from prepare_cidds import MEMBER, sample
from render_dashboard import render


with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    archive = root / "source.zip"
    output = root / "sample.csv"
    rows = ["Date first seen,Src IP Addr,Dst IP Addr,class,attackType,attackID"]
    rows += [f"2017-03-15 00:00:{i:02d}.000,192.0.2.1,192.0.2.2,normal,---,---"
             for i in range(5)]
    rows += [f"2017-03-15 00:01:{i:02d}.000,192.0.2.1,192.0.2.2,attacker,dos,7"
             for i in range(5)]
    with zipfile.ZipFile(archive, "w") as source:
        source.writestr(MEMBER, "\n".join(rows))
    assert sample(archive, output, per_class=3) == {"normal": 5, "attack": 5}
    with output.open(newline="") as stream:
        converted = list(csv.DictReader(stream))
    assert len(converted) == 6
    assert {row["scenario_label"] for row in converted} == {"normal", "attack-7"}
    assert {row["event_type"] for row in converted} == {"NORMAL_ACTIVITY", "DENIAL_OF_SERVICE"}

labels = {"a": "attack-1", "b": "attack-1", "c": "attack-1",
          "d": "attack-2", "e": "normal"}
result = measure_groups(labels, [{"a", "b", "e"}, {"c", "d"}])
assert (result["true_pairs"], result["predicted_pairs"], result["true_positive"],
        result["false_positive"], result["false_negative"]) == (3, 4, 1, 3, 2)
assert result["by_scenario"][0]["missed_join_pairs"] == 2
assert result["by_scenario"][0]["mixed_label_pairs"] == 3

with tempfile.TemporaryDirectory() as directory:
    root = Path(directory)
    source = root / "analysis.json"
    output = root / "dashboard.html"
    source.write_text('{"schema_version":2,"incidents":[],"summary":{},"config":{},'
                      '"note":"</script><script>alert(1)</script>"}', encoding="utf-8")
    render(source, output)
    html = output.read_text(encoding="utf-8")
    assert "\\u003c/script\\u003e" in html
    assert "</script><script>alert(1)</script>" not in html
    try:
        render(source, source)
        raise AssertionError("overwriting input should fail")
    except ValueError:
        pass
print("data script checks passed")
