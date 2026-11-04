# Product

<!-- impeccable:product-schema 1 -->

## Platform

web

## Stack

Inferred from the user's request to finish this existing C++ course project: a standalone HTML investigation view generated from the CLI's JSON, with no web runtime dependency.

## Users

Inferred from the repository: a student demonstrating the DSA project and a reviewer examining how alerts were grouped and ranked.

## Product Purpose

The project turns validated security event records into explainable incident groups and a prioritized review queue. Success means a reviewer can follow an incident's timeline, see why it ranked where it did, and reproduce the result locally.

## Operating Context

The C++17 CLI and synthetic presentation CSV are the offline demonstration. A 10,000-flow CIDDS-001 sample is local-only and has limited evaluation claims.

## Capabilities and Constraints

- The CLI owns correlation and scoring; the web view only reads its JSON output.
- The view must work offline from one generated HTML file.
- Dataset labels are for evaluation and must not influence incident grouping.
- Source flows are not native SOC alerts. The interface must not imply production detection certainty.

## Evidence on Hand

- `data/presentation_events.csv`: checked-in 19-row synthetic demonstration.
- `docs/EXTERNAL_EVALUATION.md`: measured CIDDS-001 sample results and caveats.
- No brand assets or UI design system existed before this view.

## Product Principles

- Show decisions and their evidence together.
- Keep exact event values and score reasons inspectable.
- Make the classroom demo understandable without a network connection.
