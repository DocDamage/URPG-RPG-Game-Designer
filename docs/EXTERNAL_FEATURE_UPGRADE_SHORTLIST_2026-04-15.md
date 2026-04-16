# External Feature Upgrade Shortlist (GitHub + Codeberg Scan)

Date: 2026-04-15  
Scope: actionable upgrades for URPG based on live external repository scan

## Best Features To Add

1. Native pathfinding/nav stack (`A*` now, navmesh lane next)
- Why this is good: immediately improves map AI, event movement reliability, and tactical/autobattle behaviors.
- Evidence sources:
  - `comuns-rpgmaker/schach-pathfinding`
  - `recastnavigation/recastnavigation`
- URPG fit:
  - Lane 3.3 `Modular Level Assembly`
  - movement authority/runtime battle positioning
- Recommended implementation:
  - Phase A: deterministic tile-graph pathfinding kernel + editor debug overlay
  - Phase B: optional navmesh baking + Detour-like query adapter for larger/open levels

2. Dialogue/event script compiler pipeline (text <-> event commands)
- Why this is good: massively improves writer workflow and reduces fragile hand-authored event trees.
- Evidence sources:
  - `yktsr/Text2Frame-MV`
  - `YarnSpinnerTool/YarnSpinner`
  - `inkle/ink`
- URPG fit:
  - Lane 2 `Message/Text Core`
  - importer/exporter/migration pipeline
- Recommended implementation:
  - Phase A: `txt/yarn-like` input -> `dialogue_sequences` + event command IR
  - Phase B: reverse export from native event IR back to source text for diff-friendly workflows

3. Behavior-tree AI runtime for combat/event agents
- Why this is good: cleaner control of async/reactive behavior than large hardcoded state machines.
- Evidence source:
  - `BehaviorTree/BehaviorTree.CPP`
- URPG fit:
  - Lane 3.1 `Gameplay Ability Framework`
  - Battle AI and map NPC behavior orchestration
- Recommended implementation:
  - Phase A: minimal BT node set (`Selector`, `Sequence`, `Condition`, `Action`, `Cooldown`)
  - Phase B: authoring panel + runtime profiler/replay hooks

4. Build/package automation lane for exports
- Why this is good: lowers release friction and improves reproducibility across targets.
- Evidence source:
  - `erri120/rpgmpacker`
- URPG fit:
  - Lane 4 `Productization and release hardening`
- Recommended implementation:
  - Add deterministic packaging CLI for desktop/web artifacts
  - Add unused-asset pruning and encryption toggle in CI presets

5. Localization operations toolkit (extraction, linting, batch)
- Why this is good: scales narrative-heavy projects and prevents late-stage localization regressions.
- Evidence sources:
  - `RPG-Maker-Translation-Tools/rvpacker-txt-rs-lib`
  - `RPG-Maker-Translation-Tools/rpgmtranslate-qt`
  - `MizaGBF/RPGMTL`
- URPG fit:
  - Lane 2 text ownership + Lane 4 release readiness
- Recommended implementation:
  - canonical extraction format + roundtrip writer
  - automated text lint checks in CI

6. Telemetry/error reporting integration for runtime and editor
- Why this is good: shortens crash triage and increases confidence during compat exit.
- Evidence source:
  - `SourceZoneDev/rgss-telemetry` (Codeberg)
- URPG fit:
  - Lane 1 compat diagnostics and Lane 4 observability
- Recommended implementation:
  - structured crash envelopes, version-aware update checks, optional OTLP sink

## Secondary (Good But Not Immediate)

1. Mobile wrapper strategy for web runtime (`yoimerdr/ludens`)
2. Linux runtime helper patterns (`bakustarver/rpgmakermlinux-cicpoffs`)
3. Rollback netcode R&D (`pond3r/ggpo`) as future extension lane

## Do-Now Backlog (Suggested Order)

1. Add deterministic pathfinding kernel + inspector visualization.
2. Ship text-to-event compiler MVP for Message/Text subsystem.
3. Add BT micro-runtime for NPC/combat decision logic.
4. Add packaging CLI + CI job for repeatable export artifacts.
5. Add localization extraction/writeback + lint gate.

