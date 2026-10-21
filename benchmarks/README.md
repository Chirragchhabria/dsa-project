# Reproducible benchmarks

Build a Release binary first, then run:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
python3 scripts/benchmark.py --binary build/soc_cli --incidents 100 --noise 1000 --seed 42 --runs 5
```

The script generates a deterministic temporary CSV, runs the CLI repeatedly, and prints individual and median wall times with platform, binary, seed, and data size. Wall time includes process startup, CSV parsing, correlation, scoring, and JSON serialization. It does not measure memory or isolate correlation latency. Do not compare results across machines or build modes without reporting both.
