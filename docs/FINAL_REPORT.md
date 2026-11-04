# SOC Alert Correlation and Incident Prioritization Engine

**Final project report · 2026-11-04**

Harish M (230932300) and Chirrag Chhabria (230932288)

## Abstract

Security teams receive many individual alerts and must decide which related activity to investigate first. This C++17 mini-project validates event CSVs, joins temporally and semantically related events into incidents, and ranks those incidents with an explained heuristic. It demonstrates hash maps, a custom linked list, a queue-backed sliding window, an adjacency-list graph, BFS, DFS, and a max heap. A command-line interface emits readable text or versioned JSON. The system is a defensive teaching prototype, not a production intrusion detector.

## Objectives and implementation

1. **Load and index events.** `EventManager` validates required fields, timestamps, duplicate IDs, typed categories, severity, and correlatable entities. Hash indexes allow lookup by event ID, IP, user, and host. Malformed rows are reported without discarding valid rows.
2. **Find related activity.** `CorrelationEngine` sorts events chronologically and retains recent candidates in per-entity temporal buckets. Two events receive a direct edge only if they share an IP, user, or host; are within the configured window (default 300 seconds); and belong to the same activity family. Supported families are credential compromise, reconnaissance, possible exfiltration, and denial of service.
3. **Build incidents and rank them.** Event-only connected components become incidents when they meet the configured minimum size. The custom linked list stores each timeline. Risk scores use severity, chain size, privileged activity, sensitive access, host diversity, and temporal density. A max heap orders incidents by score. Each score includes its contributing reasons.
4. **Expose and investigate results.** The CLI supports text and JSON output plus `--window`, `--min-events`, selectable built-in families, and direct-pair explanations. The JSON contract has `schema_version: 2` and integration checks for its keys. A standalone offline HTML view lets reviewers search the incident queue, filter priorities, and inspect timelines, entities, and score evidence.

See [the architecture note](ARCHITECTURE.md) for ownership, the processing diagram, and complexity.

## Verification

The CMake Release build and all three CTest targets pass. A separate AddressSanitizer/UndefinedBehaviorSanitizer build passes the same suite and a 10,000-flow CIDDS run. Behavioral tests cover CSV validation, duplicate rejection, indexes, linked-list ownership, temporal expiration, graph traversal, event-family separation, randomized equivalence of indexed matching to the pair rule, a 20,001-node event component, time boundaries, transitive incident chains, and pair explanations. The CLI integration test checks JSON shape and the 19-event classroom fixture. The presentation fixture forms three incidents ranked 100, 91, and 41; its planted-label pairwise precision and recall are both 1.0. Those figures verify this controlled fixture only.

## External-data evaluation

The [CIDDS-001 publisher](https://www.hs-coburg.de/forschen/cidds-coburg-intrusion-detection-data-sets/) supplies labeled flow records with timestamps, IPs, attack types, and AttackIDs. The repository's deterministic adapter samples 5,000 normal and 5,000 AttackID-labeled flows from internal week 1. All 10,000 rows loaded; the engine formed 61 incidents. Relative to publisher AttackIDs, pairwise precision was 1.000, recall 0.9971, and F1 0.9986. There were no false merged pairs and 4,003 missed joined pairs. The priority queue's top 20 incidents contained 82% of labeled attack events in the selected sample. [The full evaluation](EXTERNAL_EVALUATION.md) reports mapping, provenance, per-scenario error analysis, and limitations.

The sample is dominated by denial-of-service flows, and CIDDS records are network flows rather than native SOC alerts. AttackID membership and the engine's five-minute incident rule are different definitions. These measurements do not establish operational detection or ranking quality.

## Performance

On an Apple M1 with a Release build, median end-to-end wall time was 0.009 seconds for 1,300 synthetic events, 0.078 seconds for 13,000, and 0.454 seconds for 65,000. Peak resident memory at 65,000 events was 110.1 MB. [Benchmark details](../benchmarks/RESULTS.md) include run counts and environment. A single heavily shared entity can still produce many candidate comparisons and graph edges.

## Reproduce the result

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/soc_cli data/presentation_events.csv
./build/soc_cli data/presentation_events.csv --explain-pair AUTH-01 AUTH-02
./build/soc_cli data/presentation_events.csv --format json > /tmp/incidents.json
python3 scripts/render_dashboard.py /tmp/incidents.json /tmp/incident-review.html
python3 scripts/prepare_cidds.py data/external/cidds_10000.csv
python3 scripts/evaluate.py data/external/cidds_10000.csv
```

The final two CIDDS commands require network access to the publisher unless a local ZIP is supplied with `--zip`. The derived sample remains under ignored `data/external/` because the publisher does not explicitly grant redistribution rights for a repackaged CSV. The classroom demonstration and checked-in [demo dashboard](../web/demo.html) work offline.

## Limits and future work

The engine groups events it is given; it does not detect attacks from raw packets or verify the severity of incoming alerts. Its scoring formula is a transparent heuristic without analyst-labeled urgency ground truth. Future work can evaluate native SOC alert datasets, test analyst ranking quality, and add user-defined correlation policies when those use cases are defined. Publishing releases and binaries requires repository hosting and cross-platform CI results that are not available in this local checkout.
