# JSON output contract

The CLI emits schema version **2** with `--format json`. Version 2 adds `config.enabled_families` and the entity fields on each incident event. Consumers that required exact version-1 keys must update. The CLI keeps labels out of the output and out of correlation logic.

Top-level keys are `schema_version`, `input`, `config`, `summary`, `errors`, and `incidents`.

| Path | Meaning |
| --- | --- |
| `config.window_seconds` | Maximum gap for a direct edge. |
| `config.minimum_incident_events` | Minimum reported component size. |
| `config.enabled_families` | Enabled values from `credential`, `recon`, `exfil`, `dos`. |
| `summary.events_loaded` / `events_rejected` | Valid and invalid input rows. |
| `summary.graph_nodes` / `graph_edges` | Entity and event graph size. |
| `incidents[]` | Priority-ordered incidents, each with ID, score, priority, start/end timestamps, ordered events, and score explanations. |
| `incidents[].events[]` | Event ID, Unix timestamp, type, severity, source/destination IP, username, hostname, and description. |

`--explain-pair ID1 ID2 --format json` emits a separate object with `schema_version`, `input`, and `pair`. The pair contains IDs, `within_window`, `shared_entity`, `same_enabled_family`, and `directly_linked`; the last value is true only when all three checks pass. A direct edge can be absent between two events that belong to the same incident through a chain of other direct edges.

Timestamps are Unix seconds. Scores are capped heuristic points, not attack probabilities. A schema-v2 consumer should ignore unknown additive keys while checking `schema_version` and the fields it requires.
