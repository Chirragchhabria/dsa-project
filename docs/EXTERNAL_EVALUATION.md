# CIDDS-001 external-data pilot

Measured 2026-11-04 on macOS 26.6.2 arm64, using a CMake Release build and `soc_cli` defaults (`--window 300 --min-events 2`, all four families enabled). Source: [Hochschule Coburg CIDDS-001](https://www.hs-coburg.de/forschen/cidds-coburg-intrusion-detection-data-sets/), `traffic/OpenStack/CIDDS-001-internal-week1.csv` in the publisher ZIP. The university describes its simulated small-business network, attack types, anonymized public IPs, and citation terms there. The inspected ZIP response was 402,679,606 bytes with ETag `"67287077-18006736"`. It was read via HTTP ranges and no full archive was retained.

## Selection and mapping

`python3 scripts/prepare_cidds.py data/external/cidds_10000.csv` uses seed 42 reservoir sampling over the entire week-1 member: 5,000 `class=normal` flows with no AttackID and 5,000 flows with a publisher AttackID. The scan saw 7,010,897 eligible normal and 1,440,623 AttackID-labeled flows. The generated CSV SHA-256 was `821cc9094fabc615a5cd0fccd418ec5a3a6644c0298ce35dff5d4be7367a01e6` (1,258,179 bytes). It contains 4,326 DoS, 672 scan, 2 brute-force, and 5,000 normal rows. Timestamp and IPs come from the source CSV; AttackID is stored in `scenario_label` and never used by the engine. DoS maps to `DENIAL_OF_SERVICE`, scans to `PORT_SCAN`, brute force to `FAILED_LOGIN`, and normal flows to `NORMAL_ACTIVITY`.

## Results

Commands: `./build/soc_cli data/external/cidds_10000.csv --format json` and `python3 scripts/evaluate.py data/external/cidds_10000.csv`.

| Measure | Result |
| --- | ---: |
| Loaded / rejected | 10,000 / 0 |
| Correlated incidents | 61 |
| Ground-truth same-AttackID pairs | 1,393,932 |
| Predicted pairs | 1,389,929 |
| Correct predicted pairs | 1,389,929 |
| False merges / missed joins | 0 / 4,003 |
| Pairwise precision / recall / F1 | 1.000 / 0.9971 / 0.9986 |
| Labeled attack-event coverage in top 5 / 10 / 20 ranked incidents | 11.12% / 34.4% / 82.0% |
| CLI wall time / maximum resident size | 0.86 s / 192 MB (one schema-v2 Release run, `/usr/bin/time -l`) |

The enhanced evaluator writes per-AttackID `missed_join_pairs` and `mixed_label_pairs` in `by_scenario`. In this run, attack IDs 34, 14, and 7 account for the largest missed-join counts (1,217, 1,124, and 553), because their flows are split across multiple predicted incidents. Rank coverage is a triage diagnostic based on the ordering of selected incident groups; there is no analyst-labeled urgency ground truth.

The balance was imposed by sampling and is not the source traffic's class ratio. DoS accounts for 4,326 of the 5,000 attack rows, so the aggregate pairwise score is dominated by DoS and says little about brute force. These are **flow records**, not independently produced SOC alerts. AttackID identifies a simulated attack, while the engine groups by shared entities, type family, and a five-minute direct-edge window; those definitions are related but not identical. Naive source timestamps were treated as UTC; this does not change time differences within the sample. The pairwise scores apply only to this selected sample and cannot be treated as production detection accuracy. Runtime and memory are one local observation, not a throughput benchmark. The derived CSV and JSON output are ignored under `data/external/`; regenerate them from the publisher source rather than redistributing them without permission.
