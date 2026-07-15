# PFU-04 Menu Responsive Anchor Constraints

## Scope

Menu pane layouts now carry native persisted responsive anchors and minimum dimensions. The existing design-canvas rectangle remains the authored source; no browser or sidecar layout authority is introduced.

## Behavior

- A pane may anchor to its left, top, right, and bottom design-canvas edges.
- Opposite horizontal or vertical anchors preserve both margins and stretch the pane at a target canvas size, bounded by its authored minimum width or height.
- A trailing-only anchor preserves its trailing margin as the target size changes.
- `resolveMenuPaneLayoutForCanvas` provides the shared native layout resolution for runtime and preview consumers.
- The Menu Inspector exposes the anchors and minimum dimensions alongside the existing rectangle, history, runtime-apply, project-save, and recovery path.
- Menu Preview accepts a transient target canvas and renders the resolved pane/focus layout without mutating the authored canvas; direct manipulation and distribution are disabled until the preview target is reset. It reports resolved panes that overflow the target or leave native layout bounds.
- Serialization is backward-compatible: existing menu layouts load with the legacy left/top fixed behavior.

## Limits

This is a typed layout-constraint and target-size preview foundation, not responsive qualification. Controller/accessibility review, component systems, and runtime evidence remain PFU-04 work. Verification is deferred by the user's instruction not to build or test during this implementation phase.
