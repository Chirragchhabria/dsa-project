# Roadmap

The goal is a trustworthy, approachable open-source alert correlation engine. Popularity is an outcome of usefulness, documentation, and community work, not a testable engineering milestone.

## Current release foundation

- C++17 library and CLI with text and versioned JSON output.
- Explainable rule-based grouping and scoring.
- Synthetic teaching datasets, unit and CLI integration checks.
- Deterministic generator, labeled pairwise evaluator, and repeatable wall-time benchmark.
- Build and test workflow on macOS, Linux, and Windows (CI runs after publication).

## Next: validate correctness beyond planted examples

- Add an independently labeled, redistribution-safe dataset with a documented labeling protocol.
- Report false merges and missed joins by scenario, plus ranking quality for analyst triage.
- Add regression fixtures for time-boundary, transitive-chain, duplicate, malformed-input, and high-volume cases.
- Version the JSON schema with compatibility tests before changing field meanings.

## Then: scale and usability

- Use per-entity temporal indexes to reduce candidate comparisons while preserving the present rules.
- Measure throughput and peak memory across increasing event counts and document hardware/build settings.
- Add configurable rule definitions and a dry-run explanation of why candidate events did or did not link.
- Provide an interactive investigation interface after the engine and data contracts stabilize.

## Release and community

- Publish tagged releases with changelogs and downloadable binaries only after platform CI passes.
- Add clear examples, architecture diagrams, and a short recorded walkthrough.
- Triage issues, document contribution-sized tasks, and respond to external users' bug reports.

The project remains a defensive analysis prototype. Do not claim production detection accuracy from synthetic fixtures.
