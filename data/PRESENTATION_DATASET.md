# Presentation Dataset

`presentation_events.csv` is a small, fully synthetic teaching dataset. All public-looking IPs use documentation ranges; no row represents real traffic. Timestamps are Unix seconds on 2026-10-21 UTC.

The file contains 19 rows: four routine events, 13 events in three planted incident sequences, and two deliberate decoys. `scenario_label` is ground truth for explaining this demo; the engine does not read it when correlating or scoring.

| Sequence | UTC time | Event IDs | Expected result |
|---|---|---|---|
| Credential compromise | 09:05-09:08 | `AUTH-01` to `AUTH-05` | Five-event incident with failed sign-ins, a success, privilege escalation, and file access |
| Reconnaissance | 09:20-09:21:30 | `RECON-01` to `RECON-04` | Four-event incident across several destination hosts |
| Possible exfiltration | 09:30-09:33 | `EXFIL-01` to `EXFIL-04` | Four-event incident ending in a large outbound transfer |
| Different-family decoy | 09:08:30 | `DECOY-01` | Shares an IP and host with `AUTH` but belongs to a different event family, so it stays out |
| Outside-window decoy | 09:45 | `DECOY-02` | Shares an IP, user, and host with `AUTH` but occurs too late to join |

Run from the repository root:

```bash
./build/soc_cli data/presentation_events.csv
```

Expected incident scores are 100 for possible exfiltration, 91 for credential compromise, and 41 for reconnaissance. These scores are rule-based teaching values, not probabilities or measured detection accuracy.
