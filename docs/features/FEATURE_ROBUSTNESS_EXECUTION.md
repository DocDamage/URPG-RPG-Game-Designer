# Feature Robustness Execution Tracker

Status date: 2026-05-24

This document turns `docs/features/FEATURE_ROBUSTNESS_PLAN.md` from prose backlog into governed execution work. The canonical machine-readable tracker is `content/readiness/feature_robustness_lanes.json`.

## Boundary

These ten lanes are URPG product-depth work. They are not bounded `v0.1.0` blockers unless a future scope explicitly promotes a lane into mandatory exit criteria.

## Execution Rule

A lane cannot be marked `ready` until all of these are true:

1. Native/runtime behavior is implemented or intentionally fenced behind a named unsupported/sandbox diagnostic.
2. Editor/WYSIWYG state exposes creator-visible controls, previews, disabled reasons, and blocked reasons.
3. Diagnostics explain failure paths without silent fallback.
4. Focused tests cover success and failure behavior.
5. Status docs name the exact completed scope and do not overclaim.

## First Implementation Slice

This branch starts `FRL-07` by adding `tools/ai/collect_project_knowledge.py`. It performs bounded filesystem walking and emits `filesystem_documents` records in the same shape already accepted by `ProjectKnowledgeIndex`.

Verification:

```powershell
python tools/ci/check_feature_robustness_lanes.py
python -m unittest tools.ai.tests.test_collect_project_knowledge
```

## FRL-01 Follow-Up Slice

This branch completes the governed `FRL-01` scope by adding native imported/plugin-style battle feedback fixture parsing through `BattleRuleResolver::importFeedbackPolicyFixture()`. The importer accepts plugin parameter-style keys and string values for chip damage, chip healing, zero-damage presentation, custom buff caps, and troop-position reuse; it emits success/failure diagnostics plus deterministic fixture coverage rows. `BattlePresentationProfileFromJson()` now ingests `feedback_policy`, `BattlePresentationProfileToJson()` writes the normalized schema-versioned policy back out, and `BattlePresentationPanelSnapshot` exposes feedback fixture coverage counts, policy diagnostic counts, and editor-facing coverage row labels. Focused unit, profile/panel, and compat fixture tests now cover the completed lane.

Verification:

```powershell
ctest --preset dev-all -R "BattleRuleResolver|battle presentation|Compat fixture import: battle feedback" --output-on-failure
python tools\ci\check_feature_robustness_lanes.py
```

## Lane Order

| Lane | System | Remaining Depth |
| --- | --- | --- |
| `FRL-01` | Battle feedback | Complete for the governed FRL-01 fixture-depth scope. |
| `FRL-02` | State/message/picture | Broader fixture import coverage. |
| `FRL-03` | Progression | Richer visual controls. |
| `FRL-04` | Gameplay abilities | Full task-graph runtime sequencing and arbitrary scripting policy. |
| `FRL-05` | Asset browser/runtime library | Broader curated payload promotion and richer generated preview assets. |
| `FRL-06` | AI editor workflow | Richer painted diff rendering. |
| `FRL-07` | Project knowledge indexing | Broader automatic filesystem walking. |
| `FRL-08` | Concrete AI tools | Broader subsystem validator invocation. |
| `FRL-09` | Live chat providers | True socket-level live chunk delivery. |
| `FRL-10` | Export/release UX | Full native signing/notarization and launched multi-platform smoke. |
