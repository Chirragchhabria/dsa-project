# SOC Alert Correlation and Incident Prioritization Engine

A modern C++ event-processing engine that consolidates related security alerts into explainable incidents and ranks them for analyst review. This repository is a Data Structures Using C++ mini-project, but the architecture models a genuine SOC workflow rather than a generic log viewer.

Authors:

- Harish M — 230932300
- Chirrag Chhabria — 230932288

## Current milestone

The midterm implementation currently provides:

- validated CSV ingestion and hash-map indexes;
- a custom node-based linked list for chronological incident timelines;
- queue-backed sliding-window processing;
- an adjacency-list entity/event graph;
- iterative BFS and recursive DFS;
- conservative, explainable temporal correlation rules;
- incident construction and transparent risk scoring;
- heap-based incident prioritization;
- a command-line demonstration and controlled synthetic dataset; and
- behavioral tests for each core subsystem.

See [docs/PROGRESS.md](docs/PROGRESS.md) for the source of truth on completed and future work.

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
3. Their typed event categories belong to the same supported activity family: credential compromise, reconnaissance, or possible exfiltration.

Connected components of direct event-to-event edges become incidents. Entity nodes remain in the graph for exploration, but correlation components deliberately do not traverse entity nodes; this prevents one frequently used account or server from merging unrelated activity across time.

Risk scores are deterministic, capped at 100, and explained in CLI output. They are prioritization heuristics—not probabilities.

## Repository layout

```text
include/soc/     Public engine interfaces
src/             Engine implementations and CLI
tests/           Behavioral test executable
data/            Defensive synthetic event data
docs/            Architecture and verified progress
benchmarks/      Reserved for measured benchmarks
scripts/         Reserved for repeatable project utilities
```

## Safety and scope

This is a defensive alert-analysis simulator. It does not scan systems, exploit vulnerabilities, or execute offensive actions.

