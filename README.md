# SOC Alert Correlation and Incident Prioritization Engine

A modern C++ event-processing engine that consolidates related security alerts into explainable incidents and ranks them for analyst review. This repository is a Data Structures Using C++ mini-project, but the architecture models a genuine SOC workflow rather than a generic log viewer.

Authors:

- Harish M — 230932300
- Chirrag Chhabria — 230932288

## What the project includes

The implementation provides:

- validated CSV ingestion and hash-map indexes;
- a custom node-based linked list for chronological incident timelines;
- a queue-backed sliding-window component and per-entity temporal buckets in the correlation engine;
- an adjacency-list entity/event graph;
- iterative BFS and recursive DFS;
- conservative, explainable temporal correlation rules;
- incident construction and transparent risk scoring;
- heap-based incident prioritization;
- a command-line demonstration, controlled synthetic dataset, and CIDDS-001 adapter;
- direct-pair explanations and selectable activity families; and
- a standalone offline incident-review dashboard;
- behavioral tests for each core subsystem.

See [docs/PROGRESS.md](docs/PROGRESS.md) for the verified history and remaining work.

The [final project report](docs/FINAL_REPORT.md) summarizes the design, verification, external-data evaluation, performance measurements, and limits.
An [editable Word version](docs/SOC_Alert_Correlation_Final_Report.docx) is included for submission; `scripts/create_final_report.py` rebuilds it from the Markdown report.
Use the [final demo guide](docs/FINAL_DEMO_GUIDE.md) for a short classroom walkthrough.

For Vercel, import this repository as an **Other** framework project. `vercel.json` serves only the static `web/` directory and opens a public aggregate overview of the 10,000-flow CIDDS run at `/`. It lists all 61 incidents by score, priority, size, and activity without publishing individual source records. The 19-event guided dashboard remains at `/demo.html`. On this machine, choose **Open analysis JSON** there and select `data/external/cidds_result.json` to inspect the full local result. Rebuild the public overview with `python3 scripts/render_public_overview.py data/external/cidds_result.json data/external/cidds_evaluation.json web/overview.html`.

The rendered and visually verified midterm submission is available as [SOC Alert Correlation Midterm Report](docs/SOC_Alert_Correlation_Midterm_Report.docx). Its reproducible generator is checked in under `scripts/create_midterm_report.py`.

## Build

Requires a C++17 compiler and CMake 3.20 or newer.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the demonstration from the repository root:

```bash
./build/soc_cli data/synthetic_events.csv
```

Pass another CSV path as the first argument to analyze a different event file.

For a shorter classroom walkthrough, run `./build/soc_cli data/presentation_events.csv`. That separate 19-event synthetic file includes three easy-to-follow scenarios and two decoys that show why a shared entity alone does not create an incident. See [data/PRESENTATION_DATASET.md](data/PRESENTATION_DATASET.md) for the expected timeline. The CLI prints readable UTC times and descriptions for each incident event.

## CSV schema

Required columns are `event_id`, `timestamp`, `event_type`, and `severity`. Timestamp values are Unix seconds. An event must also have at least one correlatable IP, user, or host entity.

```text
event_id,timestamp,source_ip,destination_ip,username,hostname,event_type,severity,description,source,scenario_label
```

Malformed rows are rejected individually and reported; a bad row does not discard valid rows. `scenario_label` is ground truth for future evaluation and is not used by the correlation algorithm.

## Correlation policy (version 1)

Two events receive a direct correlation edge only when all of these hold:

1. They occur within the configured time window (five minutes by default).
2. They share an IP address, username, or host.
3. Their typed event categories belong to the same supported activity family: credential compromise, reconnaissance, possible exfiltration, or denial of service.

Connected components of direct event-to-event edges become incidents. Entity nodes remain in the graph for exploration, but correlation components deliberately do not traverse entity nodes; this prevents one frequently used account or server from merging unrelated activity across time.

Risk scores are deterministic, capped at 100, and explained in CLI output. They are prioritization heuristics—not probabilities.

## Repository layout

```text
include/soc/     Public engine interfaces
src/             Engine implementations and CLI
tests/           Behavioral test executable
data/            Defensive synthetic event data
docs/            Architecture and verified progress
benchmarks/      Reproducible and measured benchmarks
scripts/         Repeatable project utilities
```

## Safety and scope

This is a defensive alert-analysis simulator. It does not scan systems, exploit vulnerabilities, or execute offensive actions.

## CLI and JSON output

```bash
./build/soc_cli data/presentation_events.csv --window 300 --min-events 2
./build/soc_cli data/presentation_events.csv --format json > incidents.json
./build/soc_cli data/presentation_events.csv --families credential,recon
./build/soc_cli data/presentation_events.csv --explain-pair AUTH-01 AUTH-02
./build/soc_cli --help
```

`--window` sets the maximum separation for a *direct pairwise edge* in seconds. `--min-events` sets the minimum connected-component size reported as an incident. `--families` accepts a comma-separated subset of `credential,recon,exfil,dos`; all are enabled by default. `--explain-pair ID1 ID2` reports the three direct-edge checks without changing data. JSON output uses `schema_version: 2` and includes enabled families and event entities. See [the JSON contract](docs/JSON_SCHEMA.md). Diagnostics go to standard error; JSON goes to standard output.

## Offline investigation view

The checked-in [demo dashboard](web/demo.html) opens directly in a browser. To render a standalone dashboard for another CSV:

```bash
./build/soc_cli data/presentation_events.csv --format json > /tmp/incidents.json
python3 scripts/render_dashboard.py /tmp/incidents.json /tmp/incident-review.html
```

Open `/tmp/incident-review.html` in a browser. The page embeds the CLI result, works offline, and lets you search events/entities, filter priorities, inspect timelines and score reasons, or open another schema-v2 JSON file. It does not run correlation in the browser.

## Evaluation and performance

```bash
python3 scripts/evaluate.py data/presentation_events.csv
python3 scripts/benchmark.py --incidents 100 --noise 1000 --seed 42 --runs 5
```

The evaluator measures pairwise incident grouping against synthetic `scenario_label` values. Labels `normal` and `unrelated` are negative examples. This metric does not establish detection quality on real traffic. The benchmark measures end-to-end CLI wall time on deterministic synthetic data; report the machine and build mode with any result. See [scripts/README.md](scripts/README.md) and [benchmarks/README.md](benchmarks/README.md).

## External 10,000-flow evaluation

The [CIDDS-001 publisher](https://www.hs-coburg.de/forschen/cidds-coburg-intrusion-detection-data-sets/) provides flow records with IP addresses, timestamps, and attack IDs. Create a reproducible sample of 5,000 normal and 5,000 attack flows from internal week 1, then run the engine:

```bash
python3 scripts/prepare_cidds.py data/external/cidds_10000.csv
./build/soc_cli data/external/cidds_10000.csv --format json > data/external/cidds_result.json
python3 scripts/evaluate.py data/external/cidds_10000.csv
```

The preparation script reads the needed ZIP member using HTTP ranges; pass `--zip /path/to/CIDDS-001.zip` to use a downloaded archive. It samples deterministically with seed 42 and keeps publisher AttackIDs as evaluation labels only. Port scans, brute force, and DoS map to supported event types; other attacks map to `UNKNOWN`. These are **network flows, not native SOC alerts**. The sample and results stay in ignored `data/external/` because the publisher's page does not explicitly grant redistribution of a derived CSV. See [external evaluation](docs/EXTERNAL_EVALUATION.md) for measured results, provenance, and limitations.

Contributions are welcome; see [CONTRIBUTING.md](CONTRIBUTING.md) and [SECURITY.md](SECURITY.md).

See [ROADMAP.md](ROADMAP.md) for the path from the current prototype to a more robust public release.
