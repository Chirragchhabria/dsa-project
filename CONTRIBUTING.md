# Contributing

Thanks for helping improve this defensive security teaching project.

## Start here

1. Open an issue describing the bug or proposed improvement. Small fixes can go directly to a pull request.
2. Build with CMake and run `ctest --test-dir build --output-on-failure`.
3. Add a behavioral test when changing parsing, correlation, graph traversal, or scoring.
4. Keep datasets synthetic or explicitly licensed for redistribution. Never include real credentials, private IP logs, or personal data.
5. Document changes to CLI output or evaluation rules and include reproducible commands for performance claims.

The core library uses C++17 without external runtime dependencies. Python 3 is used only for evaluation and benchmark tooling. Run `python3 scripts/evaluate.py data/presentation_events.csv` to check the labeled sample.

Use focused pull requests with a description of the behavior changed, how it was verified, and any limitations. See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) before changing incident grouping: entity edges support exploration, while only direct event edges form incident components.
