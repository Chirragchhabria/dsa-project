# Incident review visual system

The Vercel root now serves `web/overview.html`, an aggregate CIDDS study view using the same palette and type as the offline dashboard. Its masthead and four measured totals lead to a priority breakdown and a searchable 61-row incident table. It contains scores, counts, and activity families but no source flow records, addresses, or event timelines. The existing `web/demo.html` remains the detailed 19-event guided example and local JSON viewer.

This records the shipped offline dashboard in `web/template.html` and its generated example, `web/demo.html`. The view presents the CLI's incident JSON for a classroom review; the score is a review heuristic, not a production detection claim.

## Color and type

- Warm paper `#f3f0e9` supports the main case; near-white `#fffdf8` marks selected cards and controls. The queue uses pale sage `#eaece6`, and score evidence uses `#e9ede7`.
- Deep blue-black `#15242b` is both the masthead background and primary text. Secondary text is `#526168`; rules are `#c7cec9`. Critical badges use rust text `#7e2e12` on `#f8e8df`; high uses ochre `#70400d` on `#f5ead8`; medium uses green `#345248` on `#e5ebe3`. Neutral priorities use `#304239` on `#e2e8e4`.
- Georgia supplies the editorial page, section, case, and score headings, usually at regular weight. System sans serif supplies body text, labels, and controls at 15px/1.5. Entity chips use system monospace at 11px. Numeric metrics, scores, and times use tabular figures.

## Structure and spacing

- A full-width dark masthead carries the small uppercase wordmark, circular `S`, outlined JSON file action, page title, explanatory line, and three status metrics. The status rule separates context from counts.
- Below it, desktop uses a 290–355px queue beside a flexible case pane. The queue contains search, priority filter, and ranked incident buttons. The selected button gains a light surface, visible border, and subtle shadow.
- The case pane has a centered maximum width of 1080px. Its header pairs incident ID and priority badge with a large right-aligned score. A ruled divider leads to a wider event timeline and narrower tinted score-evidence panel. Timeline dots and connecting lines express sequence; subdued monospace chips preserve exact entity values.
- Space comes mainly from panel padding, 25–50px column gaps, and thin rules. Rounded corners are reserved for controls, selected queue cards, badges, and the evidence panel.

## States and interaction

- The bundled JSON opens by default. “Open analysis JSON” replaces it with a local schema-version-2 file; parsing or schema errors appear in the masthead alert. Rejected-row details use a disclosure below the metrics when present.
- Search matches incident IDs, priority, and event IDs, types, descriptions, IPs, users, and hosts. Priority filtering and search immediately rebuild the queue. If the selected incident disappears, the first match is selected. No matches show plain dashed empty messages in both queue and case.
- Queue buttons show ID, score, priority, event count, and start time. Selecting one updates the case and moves keyboard focus to its main region. The case shows UTC timestamps, the event timeline, engine-provided score reasons, and the active correlation-window policy. The wording keeps score certainty appropriately limited.
- Hover lightly fills queue items and the file action. Selected state is also identified with `aria-current="true"`; it does not depend on color alone. Queue background and shadow transitions run only when reduced motion is not requested.

## Responsive and accessibility conventions

- At 850px and below, the queue moves above the case. Its incident list becomes a horizontally scrollable row of 205px cards with a match count and sideways-scroll hint. At 620px and below, timeline and score evidence form one column, header padding tightens, and the score shrinks from 50px to 40px. Local desktop and mobile review captures live in ignored `.impeccable/review/`.
- Keep semantic header, aside, main, sections, ordered timeline, buttons, labels, and native details. Search, priority, and file input have accessible names. Status counts use a polite live region; file errors use an alert. Focus-visible controls use a 3px blue `#1b657e` outline with offset; the file action has a light blue focus-within outline on dark ground.
- Preserve wrapping for long event descriptions and entity values, readable text labels on priority badges, explicit UTC times, and text-rendered JSON values. The dashboard is a single offline HTML file with no remote font, image, or runtime dependency.
