# Implementation Progress

Last verified: 2026-10-21

The sections below retain the historical midterm snapshot. For current final-project status, see [FINAL_REPORT.md](FINAL_REPORT.md) and the dated updates at the end of this file.

## Completed and verified

- C++17 project structure and CMake target definitions.
- `SecurityEvent` with typed event/severity enums.
- CSV ingestion with quoted-field parsing, required-column checks, row-level rejection, duplicate-ID prevention, and entity validation.
- Hash-map indexes for event ID, source IP, destination IP, username, and hostname.
- Manually implemented singly linked list with owned nodes, append, chronological insert, lookup, removal, deep copy, and clear.
- Queue/deque sliding window with timestamp-order validation and expiration.
- Adjacency-list graph with entity/event nodes and undirected edges.
- Iterative BFS.
- Recursive graph DFS and event-component discovery (the latter now uses an explicit stack for large components).
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

- Larger multi-source alert datasets beyond the 10,000-flow CIDDS pilot below.
- Published, repeatable throughput and memory measurements across meaningful dataset sizes and environments.
- Ranking-quality measurements and evaluation on independently labeled, native SOC alert datasets. The CIDDS pilot below measures flow grouping against publisher AttackIDs.
- User-defined rule objects beyond the four built-in event families.
- Attack-chain reconstruction beyond chronological incident timelines.
- Dedicated `RiskScorer`, metrics, and benchmark modules.
- Dashboard or other visualization.

The list above describes the historical midterm gap. Later measured results appear in [EXTERNAL_EVALUATION.md](EXTERNAL_EVALUATION.md) and [benchmarks/RESULTS.md](../benchmarks/RESULTS.md).

## Next recommended increment

Confirm the professor's midterm rubric and rehearse the CLI demonstration. For the final-project increment, add a benchmark harness and a deterministic larger dataset generator. Capture measurements before adding any performance figures to reports or resume bullets.


## Release preparation added after the midterm snapshot

The repository now includes JSON CLI output, command-line correlation settings, an integration test, a deterministic synthetic dataset generator, a pairwise grouping evaluator, a wall-time benchmark harness, and CI configuration. On the 19-row presentation sample, the evaluator reports 22/22 correct event pairs (pairwise precision, recall, and F1 of 1.0). This is a result on planted synthetic labels only. No real-world accuracy claim follows from it. The historical verification figures above describe the original midterm snapshot; run the current commands in the README for a fresh check.

## External-data pilot added 2026-11-04

The engine now supports a distinct denial-of-service event family. A deterministic, local-only 10,000-flow CIDDS-001 sample and evaluation are documented in [EXTERNAL_EVALUATION.md](EXTERNAL_EVALUATION.md). It loaded without rejected rows and produced 61 incidents. Pairwise precision was 1.000 and recall 0.9971 against the publisher's AttackIDs on that selected sample. This is a limited flow-grouping result; it is not a real-world SOC detection claim. The source archive and derived sample are not checked in.

## Final engineering increment, 2026-11-04

The engine now uses per-entity temporal buckets instead of scanning every event in the active time window. Event-component discovery uses an explicit stack so large connected components do not consume the C++ call stack. Regression checks compare indexed edges to the direct pair rule on randomized data and cover a 20,001-node component, a time-boundary chain, and the JSON schema keys. The evaluator reports per-AttackID split/merge counts and labeled attack-event coverage at ranks 5, 10, and 20. Three-size Release timing and peak-memory measurements are recorded in [benchmarks/RESULTS.md](../benchmarks/RESULTS.md). All current CTest checks pass.

## Investigation increment, 2026-11-04

The CLI accepts a selected subset of the four built-in activity families and can explain the three direct-edge checks for any two loaded event IDs. JSON schema version 2 records the enabled families and event entities. A standalone offline incident-review dashboard is generated from CLI JSON; the checked-in synthetic demo is [web/demo.html](../web/demo.html). It supports search, priority filtering, timelines, entities, score reasons, file import, and rejected-row details. Desktop and mobile layouts were inspected in a browser, and malformed JSON preserves the previous analysis.
