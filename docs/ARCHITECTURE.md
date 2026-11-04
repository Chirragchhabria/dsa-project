# Architecture

## Processing flow

```text
CSV input
   |
   v
EventManager ----> hash indexes (event ID / IP / user / host)
   |
   v
sorted SecurityEvent references
   |
   +----> per-entity temporal buckets (recent events only)
   |
   v
CorrelationEngine ----> EntityGraph (adjacency list)
                              |
                    BFS / recursive DFS
                              |
                    event connected components
                              |
                              v
                  Incident + EventLinkedList
                              |
                        transparent score
                              |
                              v
                 IncidentPrioritizer (max heap)
```

## Ownership

- `EventManager` owns all ingested `SecurityEvent` values.
- Hash indexes store vector positions, not owning pointers.
- The per-entity temporal buckets hold non-owning event pointers only during a correlation run; the manager outlives that run. The standalone `SlidingWindow` remains available for teaching and tests.
- `EventLinkedList` owns its nodes through `std::unique_ptr` and stores event snapshots. Copy operations perform a deep copy.
- `IncidentPrioritizer` stores non-owning incident pointers; the caller must keep the incident vector alive while the queue is used.

These rules keep ownership explicit and prevent graph/index structures from duplicating the primary event store.

## Graph design decision

The graph contains typed node keys such as `event:C001`, `ip:203.0.113.45`, and `user:alex`. Event-to-entity edges support analyst exploration with BFS/DFS. Separately, the correlation engine creates direct event-to-event edges only when its temporal, entity, and event-family rules all pass.

Incident connected components traverse direct event neighbors only. Traversing through entity nodes would cause transitive over-correlation—for example, every event involving a shared server could collapse into one incident even when events are hours apart.

## Complexity

Let `n` be the number of events, `w` the maximum events simultaneously in the temporal window, `c` the number of recent events sharing an entity with the current event, and `V`/`E` graph nodes/edges.

| Operation | Time | Additional space |
|---|---:|---:|
| CSV ingestion and indexing | average `O(n)` | `O(n)` |
| Hash lookup | average `O(1 + k)` | `O(k)` result |
| Linked-list chronological insert | `O(m)` | `O(1)` node |
| Sliding-window add/expire | amortized `O(1)` | `O(w)` |
| Candidate correlation | average `O(n + Σc)` after sorting | `O(w)` active index |
| Graph BFS | `O(V + E)` | `O(V)` |
| Recursive graph DFS | `O(V + E)` | `O(V)` including call stack |
| Heap push/pop | `O(log i)` | `O(i)` |

The engine looks up only recent events sharing an IP, user, or host; a heavily shared entity can still make `c` approach `w`. The graph itself can require `O(E)` space when many events correlate. Event-component discovery uses an explicit stack so long chains do not overflow the C++ call stack; the public graph `dfs` method remains recursive for the data-structures demonstration.

## Extensibility boundaries

- Correlation policy lives behind `CorrelationEngine` and can later become composable rules.
- The CLI exposes the four built-in family switches and a direct-pair explanation. Both use the same family and entity checks as incident construction.
- Risk calculation is currently on `Incident`; a future `RiskScorer` can extract it without changing event ingestion.
- Ground-truth labels are stored but never used to form incidents, preserving their use for later precision/recall evaluation.

