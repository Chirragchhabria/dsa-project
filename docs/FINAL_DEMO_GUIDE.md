# Final project demonstration

Aim for a 6–8 minute walkthrough. The checked-in synthetic data and dashboard work offline.

## Prepare

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/soc_cli data/presentation_events.csv --format json > /tmp/incidents.json
python3 scripts/render_dashboard.py /tmp/incidents.json /tmp/incident-review.html
```

Open `/tmp/incident-review.html` in a browser. Keep [the architecture diagram](ARCHITECTURE.md) and [final report](FINAL_REPORT.md) ready. If network access is available and the local CIDDS sample exists, prepare `python3 scripts/evaluate.py data/external/cidds_10000.csv` too. Do not spend presentation time downloading the source ZIP.

## Walkthrough

| Time | Show | Explain |
| --- | --- | --- |
| 0:00–0:45 | Problem and architecture | Separate alerts need a shared incident timeline and an ordered review queue. |
| 0:45–2:00 | Data structures | Hash indexes locate entities; per-entity temporal buckets limit candidate comparisons; a graph stores relationships; the custom linked list holds timelines; a max heap orders incidents. The public graph API also demonstrates BFS and recursive DFS. |
| 2:00–3:15 | CLI and dashboard | Show 19 loaded events, zero rejected, three incidents. Select the highest-ranked case, inspect its four-event timeline and score reasons, then search for `RECON-01`. |
| 3:15–4:15 | Why events link | Run `./build/soc_cli data/presentation_events.csv --explain-pair AUTH-01 AUTH-02`, then compare `AUTH-01 RECON-01`. Explain the window, shared entity, and enabled-family checks. |
| 4:15–5:15 | Validation | Show the three passing CTest targets. The 10,000-flow CIDDS pilot had 61 incident groups, pairwise precision 1.000, recall 0.9971, and F1 0.9986 against publisher AttackIDs. |
| 5:15–6:15 | Performance and limits | The 65,000-event synthetic Release run had median 0.454 s wall time and 110.1 MB peak resident memory on an Apple M1. CIDDS rows are network flows, not native SOC alerts; the score is a heuristic, not an attack probability. |

If asked about the high CIDDS score, explain that the sample intentionally balances normal and attack flows and that 4,326 of the 5,000 attack rows are DoS. AttackIDs identify simulated attack membership, while the engine's five-minute grouping rule has a different definition. Per-AttackID missed joins and rank coverage are in [the evaluation note](EXTERNAL_EVALUATION.md).
