# Scripts

- `create_midterm_report.py` rebuilds the historical editable Word midterm report.
- `generate_dataset.py` writes reproducible labeled synthetic alerts. Example: `python3 scripts/generate_dataset.py /tmp/soc-events.csv --incidents 100 --noise 1000 --seed 42`.
- `evaluate.py` runs the CLI on a labeled CSV and reports pairwise incident precision, recall, and F1. Example: `python3 scripts/evaluate.py data/presentation_events.csv`. Labels `normal` and `unrelated` are treated as negative examples; other matching labels define true incident pairs. This evaluates grouping only, not severity ranking or real-world detection quality.
- `benchmark.py` generates temporary data and measures end-to-end CLI wall time. See [benchmarks/README.md](../benchmarks/README.md).
