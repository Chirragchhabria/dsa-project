# Scripts

- `create_midterm_report.py` rebuilds the historical editable Word midterm report.
- `generate_dataset.py` writes reproducible labeled synthetic alerts. Example: `python3 scripts/generate_dataset.py /tmp/soc-events.csv --incidents 100 --noise 1000 --seed 42`.
- `evaluate.py` runs the CLI on a labeled CSV and reports pairwise incident precision, recall, F1, per-scenario missed joins and mixed-label pairs, and labeled attack-event coverage at ranks 5/10/20. Example: `python3 scripts/evaluate.py data/presentation_events.csv`. Labels `normal` and `unrelated` are treated as negative examples; other matching labels define true incident pairs. Rank coverage is a triage diagnostic, not a severity-ranking accuracy measure.
- `prepare_cidds.py` samples 5,000 normal and 5,000 AttackID-labeled flows from the publisher's CIDDS-001 internal week 1. It reads the ZIP with HTTP ranges or accepts a local archive using `--zip`. Example: `python3 scripts/prepare_cidds.py data/external/cidds_10000.csv`.
- `render_dashboard.py` embeds schema-v2 CLI JSON in a standalone offline incident-review HTML file. Example: `python3 scripts/render_dashboard.py /tmp/incidents.json /tmp/incident-review.html`.
- `benchmark.py` generates temporary data and measures end-to-end CLI wall time. See [benchmarks/README.md](../benchmarks/README.md).
