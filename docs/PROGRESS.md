# Implementation Progress

Last verified: 2026-09-17

This file is the source of truth for midterm reporting. A capability is listed as completed only when its implementation compiles and its relevant checks pass.

## Completed and verified

- C++17 project structure and CMake target definitions.
- `SecurityEvent` with typed event/severity enums.
- CSV ingestion with quoted-field parsing, required-column checks, row-level rejection, duplicate-ID prevention, and entity validation.
- Hash-map indexes for event ID, source IP, destination IP, username, and hostname.
- Manually implemented singly linked list with owned nodes, append, chronological insert, lookup, removal, deep copy, and clear.
- Queue/deque sliding window with timestamp-order validation and expiration.
- Adjacency-list graph with entity/event nodes and undirected edges.
- Iterative BFS.
- Recursive DFS and recursive event-component discovery.
- Explainable version-1 correlation using time, shared entities, and event-family compatibility.
- Incident creation with chronological event chains and involved-entity sets.
- Transparent risk scoring and priority labels.
- Priority-queue (max-heap) incident ordering.
- Defensive synthetic dataset with normal activity and three labeled multi-event scenarios.
- CLI reporting data-derived counts, graph size, incident timelines, scores, and explanations.
- Behavioral test executable covering CSV parsing, malformed rows, indexes, linked-list operations, sliding-window expiration, graph construction, BFS, recursive DFS, correlation boundaries, incident creation, and heap ordering.

## Verification evidence

The machine used for this verification has Apple Clang 21.0.0 but does not currently have CMake installed. Therefore the checked-in CMake configuration exists but was not falsely reported as executed. Equivalent C++17 compile/link commands were run directly with strict warnings.

Observed test result:

```text
Tests passed: 7, failed: 0
```

Observed demonstration result from `data/synthetic_events.csv`:

```text
Events loaded: 23
Events rejected: 0
Unique source IPs: 14
Unique users: 12
Unique hosts: 13
Graph nodes: 71
Graph edges: 110
Correlated incidents: 3
```

Observed prioritization order and scores:

```text
INC-0003  possible-exfiltration sequence  CRITICAL  100/100
INC-0001  credential-compromise sequence CRITICAL   91/100
INC-0002  reconnaissance sequence         MEDIUM     41/100
```

The scenario names above come from the controlled dataset design. The engine itself does not read `scenario_label` when correlating or scoring.

## Not yet completed

- CMake/CTest execution on a machine with CMake installed.
- Larger public or safely replayed datasets.
- Dedicated benchmark executable and measured throughput, latency, or memory results.
- Precision/recall and ranking-quality evaluation against ground truth.
- Configurable/composable rule objects beyond the initial three event families.
- Attack-chain reconstruction beyond chronological incident timelines.
- Dedicated `RiskScorer`, metrics, and benchmark modules.
- Dashboard or other visualization.

No performance or accuracy claims have been made because those measurements do not yet exist.

## Next recommended increment

Install CMake, reproduce the build with CTest, then add a benchmark harness and a deterministic larger dataset generator. Capture measurements before adding any performance figures to reports or resume bullets.

