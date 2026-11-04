# Release benchmark, 2026-11-04

Apple M1, macOS 26.6.2 arm64, Apple Clang 21.0.0, CMake `Release` build, JSON schema version 2. `scripts/generate_dataset.py` seed 42; each size contains three alert events per generated incident plus normal noise. Wall time is the median of five runs from `scripts/benchmark.py` and includes startup, parsing, correlation, and serialization. Peak resident size is the largest of three separate `/usr/bin/time -l ./build/soc_cli INPUT --format json` runs with JSON output discarded.

| Events | Generated incidents | Noise | Median wall time (s) | Peak resident memory |
| ---: | ---: | ---: | ---: | ---: |
| 1,300 | 100 | 1,000 | 0.009 | 4.6 MB |
| 13,000 | 1,000 | 10,000 | 0.078 | 23.7 MB |
| 65,000 | 5,000 | 50,000 | 0.454 | 110.1 MB |

The workload is deterministic synthetic traffic, so these figures describe this machine and workload only. In particular, many events sharing one IP in a short window can create far more graph edges and use substantially more time and memory. The 10,000-flow CIDDS-001 measurement is reported separately in [the external-data evaluation](../docs/EXTERNAL_EVALUATION.md).
