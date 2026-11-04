# Roadmap

The goal is a trustworthy, approachable open-source alert correlation engine. Popularity is an outcome of usefulness, documentation, and community work, not a testable engineering milestone.

## Current release foundation

- C++17 library and CLI with text and versioned JSON output.
- Explainable rule-based grouping and scoring.
- Synthetic teaching datasets, unit and CLI integration checks.
- Deterministic generator, labeled pairwise evaluator, and repeatable wall-time benchmark.
- Build and test workflow on macOS, Linux, and Windows (CI runs after publication).

## Completed for the final mini-project

- Added a deterministic 10,000-flow CIDDS-001 pilot using publisher AttackIDs; the derived data stays local because redistribution permission is unclear.
- Reported false merges and missed joins by AttackID, plus attack-event coverage by incident rank. Analyst-labeled urgency is unavailable, so ranking quality remains unmeasured.
- Added regression checks for time boundaries, transitive chains, duplicate and malformed input, indexed pair-rule equivalence, and a large connected component.
- Advanced JSON to `schema_version: 2` for enabled families and event entities, with key-level compatibility checks.
- Added per-entity temporal indexing and three-size Release timing and memory measurements.
- Added selectable built-in activity families, a direct-pair explanation mode, and an offline incident-review view.

## Future work outside the course mini-project

- Evaluate native SOC alert datasets and analyst-labeled incident urgency.
- Consider user-authored rule definitions when a concrete new policy is needed. The current CLI selects the four built-in families and explains direct-pair decisions.
- Consider richer live investigation workflows after validating analyst needs. The current offline view supports search, priority filtering, and evidence review.

## Release and community

- Publish tagged releases with changelogs and downloadable binaries only after platform CI passes.
- Add clear examples, architecture diagrams, and a short recorded walkthrough.
- Triage issues, document contribution-sized tasks, and respond to external users' bug reports.

The project remains a defensive analysis prototype. Do not claim production detection accuracy from synthetic fixtures.
