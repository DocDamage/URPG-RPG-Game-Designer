# Visual Golden Governance

Renderer-backed snapshot tests are executable coverage, but the current full-frame JSON
goldens are expensive to review and store. The policy is intentionally conservative:
existing large JSON baselines stay in place until a compact migration is implemented,
while future large additions must be deliberate.

The machine-readable policy lives at `content/fixtures/visual_golden_governance.json`.
Any committed `tests/snapshot/goldens/*.golden.json` file above 10 MiB must be listed
there with:

- a reason for keeping the large baseline,
- a review strategy,
- a human-reviewable crop, heatmap, or summary artifact,
- and a `maxBytes` ceiling that catches accidental churn.

Current `.gitattributes` keeps `*.json` as text. Future compact PNG or binary baseline
storage must use the existing Git LFS binary rules and preserve executable comparison,
visual diff heatmaps, and report JSON diagnostics.
