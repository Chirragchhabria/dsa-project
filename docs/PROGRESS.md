# Implementation Progress

Last verified: 2026-10-21

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
- Versioned JSON CLI output, configurable window/minimum incident size, and black-box CLI integration checks.
- Pairwise grouping evaluator on labeled synthetic data and deterministic dataset/benchmark scripts.
- Cross-platform CI workflow, contribution guide, and security policy.
- CMake 4.4.4 configuration and build, followed by a passing CTest run on Apple Clang 21.0.0.

## Verification evidence

On 2026-10-21, CMake 4.4.4 configured and built the project with Apple Clang 21.0.0. CTest passed its registered `soc_tests` target (1/1); the target runs seven behavioral test groups. A separate direct Clang build with AddressSanitizer and UndefinedBehaviorSanitizer also passed all seven groups.

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

- Larger public or safely replayed datasets.
- Published, repeatable throughput and memory measurements across meaningful dataset sizes and environments.
- Evaluation on larger independently labeled datasets and ranking-quality measurements. The current pairwise precision/recall result is only on the controlled presentation dataset.
- Configurable/composable rule objects beyond the initial three event families.
- Attack-chain reconstruction beyond chronological incident timelines.
- Dedicated `RiskScorer`, metrics, and benchmark modules.
- Dashboard or other visualization.

No performance or accuracy claims have been made because those measurements do not yet exist.

## Next recommended increment

Confirm the professor's midterm rubric and rehearse the CLI demonstration. For the final-project increment, add a benchmark harness and a deterministic larger dataset generator. Capture measurements before adding any performance figures to reports or resume bullets.


## Release preparation added after the midterm snapshot

The repository now includes JSON CLI output, command-line correlation settings, an integration test, a deterministic synthetic dataset generator, a pairwise grouping evaluator, a wall-time benchmark harness, and CI configuration. On the 19-row presentation sample, the evaluator reports 22/22 correct event pairs (pairwise precision, recall, and F1 of 1.0). This is a result on planted synthetic labels only. No real-world accuracy claim follows from it. The historical verification figures above describe the original midterm snapshot; run the current commands in the README for a fresh check.
