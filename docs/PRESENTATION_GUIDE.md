# Midterm Presentation Guide

Target length: 6-7 minutes, followed by questions. Adjust to the professor's allotted time and rubric.

## Before entering the room

From the repository root, verify the build and keep the terminal ready:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
./build/soc_cli data/presentation_events.csv
```

Keep the terminal large enough for the incident output to be readable. The presentation dataset is explained in `data/PRESENTATION_DATASET.md`. The checked-in report is `docs/SOC_Alert_Correlation_Midterm_Report.docx` and documents the earlier 23-event sample, so identify the 19-event presentation file by name when discussing its output. Do not rely on network access for the demo.

### Presenting from VS Code

Open the **project folder**, not an individual `.cpp` file, in VS Code. Use **Terminal > Run Task** and choose `SOC: Demo`. That task configures and builds the project before running the sample. Choose `SOC: Test` to show CTest plus the seven named test groups. Both tasks use the integrated terminal and need no VS Code extension.

Keep these files ready in editor tabs, in this order:

1. `docs/ARCHITECTURE.md` for the processing diagram and complexity table.
2. `src/CorrelationEngine.cpp` for the five-minute window and pairwise correlation rule.
3. `src/EventLinkedList.cpp` for the custom node operations.
4. `src/EntityGraph.cpp` for BFS and recursive DFS.
5. `src/IncidentPrioritizer.cpp` for heap ordering.

Before the presentation, increase the integrated-terminal font size and collapse the Explorer if the screen is small. Run `SOC: Demo` once, then leave the resulting terminal output visible. If the task menu does not appear, use the four commands above in VS Code's integrated terminal from the project root.

## Talk track

| Time | Show | Say |
|---|---|---|
| 0:00-0:40 | Title and problem | "A SOC receives many isolated alerts. Our C++ project groups related alerts into incidents and ranks the incidents for analyst review." |
| 0:40-1:20 | Processing diagram in the report or `docs/ARCHITECTURE.md` | "CSV events are validated and indexed, compared inside a time window, connected in a graph, collected into incidents, then placed in a priority queue." |
| 1:20-2:50 | Data-structure table | Explain the role of the hash maps, deque window, custom linked list, adjacency-list graph, BFS/recursive DFS, and max heap. Say which structures are custom and which use the standard library. |
| 2:50-3:35 | Correlation rule | "An event pair is linked only when it is within five minutes, shares an IP/user/host, and belongs to the same supported activity family. Event-only connected components become incidents." |
| 3:35-5:15 | Terminal demo | Run the CLI on `presentation_events.csv`. Point out 19 loaded events, zero rejected rows, three incidents, and their ranking: 100, 91, 41. Show why the two decoy rows are excluded. |
| 5:15-6:05 | Test output | Run `ctest --test-dir build --output-on-failure`, then `./build/soc_tests` if the professor wants to see the seven named test groups. |
| 6:05-6:40 | Limits and next step | "This is a midterm prototype on controlled synthetic data. Next we will measure speed and detection quality on a larger labeled dataset, then improve the rules." |

## The key explanation

The three incident IDs reflect chronological creation order; the displayed order reflects priority. `INC-0003` ranks first because the four-event possible-exfiltration sequence has high severity plus privilege, sensitive-access, chain-length, and temporal-density contributions. The final score is capped at 100. The score is a deterministic heuristic, not a probability of an attack.

The graph has event and entity nodes for exploration. Incident components traverse direct event-to-event edges only. This avoids merging unrelated activity merely because events touch the same common server or account.

## Likely viva questions

| Question | Short answer |
|---|---|
| Why use a sliding window? | It limits pair comparisons to recent events. The current candidate-comparison cost is `O(nw)`, where `w` is the maximum number of events in the window. |
| Why a linked list? | The custom node chain stores each incident's chronological event timeline and demonstrates insertion, removal, deep copy, and ownership. In this version insertion is `O(m)` for an incident with `m` events. |
| Why BFS and DFS? | BFS explores graph relationships iteratively. Recursive DFS explores reachable nodes and finds connected event components. |
| Why not connect through entity nodes when making incidents? | A shared account or server could otherwise join unrelated events into one incident. |
| Why a heap? | It returns the highest risk incident first; push and pop are `O(log i)` for `i` incidents. |
| Is a score of 100 a certainty? | No. It is a capped prioritization score based on fixed, printed factors. |
| What does the evaluation prove? | The seven test groups and controlled demo verify expected program behavior. Accuracy and production performance have not yet been measured. |
| What is custom? | The linked list is implemented with owned nodes. The deque, hash maps, adjacency sets, and priority queue use C++ standard-library containers. |

## Scope to state honestly

The presentation demo uses 19 synthetic events, including 13 events in three planted scenarios and two decoys. The policy has three fixed activity families. Connected components can span more than five minutes overall through a chain of valid pairwise links; the five-minute limit applies to each direct edge. The current project has no precision/recall or throughput claims.
