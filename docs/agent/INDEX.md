# Agent Knowledge Index

This directory is the agent-facing map for URPG. It points to canonical files rather than duplicating their content.

## First Reads

| Need | Start Here |
| --- | --- |
| Repository layout and ownership | `docs/agent/ARCHITECTURE_MAP.md` |
| Which verification command to run | `docs/agent/QUALITY_GATES.md` |
| How to execute release plans | `docs/agent/EXECUTION_WORKFLOW.md` |
| Current release gaps and debt | `docs/agent/KNOWN_DEBT.md` |
| Current release execution tasks | `docs/release/AAA_RELEASE_EXECUTION_PLAN.md` |
| Current release-surface audit tasks | `docs/superpowers/plans/2026-04-30-release-surface-audit-execution-plan.md` |

## Canonical Status

- Program status: `docs/PROGRAM_COMPLETION_STATUS.md`
- Technical debt: `docs/PROGRAM_COMPLETION_STATUS.md`
- Native feature roadmap: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- Release readiness matrix: `docs/release/RELEASE_READINESS_MATRIX.md`
- Editor control inventory: `docs/release/EDITOR_CONTROL_INVENTORY.md`
- Native map editor status: `README.md`, `docs/release/EDITOR_CONTROL_INVENTORY.md`, and `tests/unit/test_grid_part_editor.cpp`
- Release authoring persistence status: `docs/release/EDITOR_CONTROL_INVENTORY.md`,
  `docs/superpowers/plans/2026-04-30-release-surface-audit-execution-plan.md`, and
  `tests/unit/test_diagnostics_workspace.cpp`

## Domain Docs

- Architecture decisions: `docs/adr/`
- Presentation runtime: `docs/presentation/`
- Native Level Builder and grid-part authoring: `editor/spatial/level_builder_workspace.*`, `engine/core/map/grid_part_*`, `content/schemas/grid_part_*.schema.json`
- Export and packaging: `docs/release/RELEASE_PACKAGING.md`
- Asset intake and promotion: `docs/asset_intake/`
- Template governance: `docs/templates/`, `docs/governance/`

## Agent Hygiene

Keep this directory small and navigational. If a file becomes a subsystem manual, move that detail to the subsystem docs and link to it from here.
