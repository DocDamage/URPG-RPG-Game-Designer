# Future Feature Upgrade Plans

Status Date: 2026-04-24

This document records optional future feature and product-upgrade candidates for URPG after the current release-readiness gates are closed. These items are not current `READY` requirements and do not change the status of any subsystem in `content/readiness/readiness_status.json`.

Implementation roadmap: `docs/FUTURE_FEATURE_ACTIONABLE_SPRINT_PLAN.md`.

## Planning Guardrails

- Close the existing human-review gates before promoting new release-facing scope.
- Prefer upgrades that make URPG easier to author, inspect, validate, and ship.
- Avoid expanding into multiplayer, cloud sync, marketplace, full 3D, or live provider integrations until the existing `PARTIAL` lanes are closed or deliberately re-scoped.
- Treat any new `READY` claim as evidence-gated through the canonical readiness matrix, tests, docs, and project audit.

## Candidate Upgrades

| Priority | Candidate | Why It Matters | Suggested First Slice |
|---|---|---|---|
| 1 | Project Health / Readiness Dashboard | Turns audit, readiness, schema, asset, and release-blocker signals into an editor-facing product feature. | Add an editor panel that summarizes `urpg_project_audit` output, release blockers, export blockers, and links to exact remediation docs. |
| 2 | Real Asset Library + Asset Intake UX | Makes license status, duplicate detection, missing references, import normalization, and export safety visible to creators. | Build an asset browser that reads existing asset hygiene/intake reports and labels assets as usable, risky, duplicate, oversized, or missing license evidence. |
| 3 | Visual Event Authoring | Eventing is the heart of RPG authoring; URPG needs a strong no-code command/page workflow. | Create a visual event-page editor for conditions, command lists, switches, variables, movement, message, battle, transfer, and common-event calls. |
| 4 | Plugin Compatibility Inspector | Converts compat/sandbox/plugin truth into a creator-facing workflow instead of hidden diagnostics. | Add a plugin inspector showing manifest status, dependencies, load order, permissions, unsupported calls, fallback behavior, and sandbox warnings. |
| 5 | Battle Presentation Authoring | Battle logic is strong, but creators need visual control over battlebacks, HUD, cues, popups, icons, and preview playback. | Add a battle presentation panel for battleback assignment, HUD layout, state icons, damage popup style, and deterministic preview playback. |
| 6 | Tilemap / Terrain / Layer Upgrade | Map authoring needs stronger layers, terrain sets, collision/navigation metadata, overpasses, and runtime-preview parity. | Extend the spatial editor with terrain-set painting, layer visibility/edit locking, collision metadata, and overpass validation. |
| 7 | Export Runtime Hardening | Shipping confidence depends on runtime-side bundle validation, not only validator-time checks. | Implement runtime/load-time `data.pck` signature enforcement, atomic bundle publication, and clearer export artifact reporting. |
| 8 | Starter Project Templates | Polished templates make the engine immediately usable and prove vertical-slice completeness. | Ship JRPG, Visual Novel, and Turn-Based RPG starter projects with maps, dialogue, battle, save/load, menus, export profiles, and readiness evidence. |
| 9 | Save/Load Debugger | Save systems are hard to trust without inspecting slots, recovery tiers, migration history, and corrupted states. | Add an editor debugger for slot metadata, recovery fallback tier, migration notes, corrupted payload diagnostics, and subsystem state previews. |
| 10 | One-Click Dev Room Test Harness | A generated test scene helps creators validate events, battle, save/load, plugins, audio, input, and export warnings quickly. | Generate a dev-room map/scene that exercises core systems and produces a project-health report after scripted playthrough. |

## Recommended Sequencing

1. Close `battle_core` and `save_data_core` human-review gates.
2. Finish mandatory release hardening already tracked by current readiness docs, especially export runtime signature enforcement.
3. Build the Project Health / Readiness Dashboard and Asset Library UX together, because both can consume existing governance outputs.
4. Build Visual Event Authoring as the next major creator-facing workflow.
5. Add battle presentation, tilemap, starter-template, save-debugger, and dev-room upgrades as vertical slices with their own evidence gates.

## Additional Beneficial Upgrades

These are broader product-depth candidates to consider after the first 10 upgrades are scoped. They are intentionally recorded as future backlog, not current completion criteria.

| Priority | Candidate | Why It Matters | Suggested First Slice |
|---|---|---|---|
| A1 | Guided New Project Wizard | Helps creators start from a working structure instead of an empty engine project. | Generate a selected template with starter maps, menu setup, save config, export profile, and an initial project-health report. |
| A2 | Quest / Objective System | RPG projects need durable goals, journal entries, rewards, flags, and progress tracking. | Add a native quest registry with objective states, reward records, save/load persistence, and a simple quest-inspector panel. |
| A3 | Dialogue Graph Editor | Branching conversations are a core authoring workflow beyond linear message rendering. | Create a node/graph editor for dialogue choices, conditions, variables, speaker metadata, and preview playback through the message runtime. |
| A4 | Common Event Library | Reusable event recipes reduce friction for common RPG interactions. | Ship governed templates for doors, chests, shops, inns, cutscenes, NPC patrols, save points, treasure, boss gates, and tutorials. |
| A5 | Database Editor Parity Pass | RPG creators expect polished editing for actors, classes, skills, items, enemies, troops, states, equipment, animations, and formulas. | Build a unified database workspace over existing schemas/runtime models, starting with actors, skills, items, enemies, and troops. |
| A6 | Cutscene / Timeline Sequencer | Camera movement, fades, dialogue, waits, audio cues, and character movement need a visual timeline. | Add a deterministic timeline asset with preview support for message, movement, audio, screen tint, fade, and wait commands. |
| A7 | In-Editor Playtest With Time Travel | Deterministic replay and rewind would make debugging RPG state much faster. | Record input/state checkpoints during playtest and expose step, rewind-to-checkpoint, replay, and state-diff controls. |
| A8 | Localization Workspace | Translation completeness, font coverage, and layout overflow need creator-facing workflows. | Build a localization panel for missing keys, per-language preview, string-length warnings, font coverage, and export readiness. |
| A9 | Accessibility Authoring Assistant | Audits are useful, but creators need guided fixes and previews. | Extend accessibility diagnostics with suggested repairs for labels, contrast, focus order, input alternatives, and text-speed/readability settings. |
| A10 | Formula / Rule Debugger | Designers need to understand damage, scaling, variance, guard/crit, conditions, and unsupported compat formulas. | Add a formula sandbox that evaluates sample battlers/items, displays parse status, and explains fallback reasons. |
| A11 | Economy Balancer | RPG balance depends on XP curves, prices, gold gain, drop rates, item rarity, and reward pacing. | Add simulation tools for encounter rewards, shop pricing, level curves, and projected progression over sample play paths. |
| A12 | Encounter Table Editor | Encounter design needs region weights, troop previews, level ranges, conditions, and simulation. | Add an encounter-table asset with map-region binding, weighted troop selection, preview simulation, and validation warnings. |
| A13 | World Map / Travel System | Many RPGs need overworld travel, fast travel, vehicles, portals, and route gating. | Add a world-map graph asset with unlock states, route requirements, travel events, and save persistence. |
| A14 | 2D Lighting / Weather Authoring | Modern 2D RPGs benefit from previewable ambience, weather, tint, fog, and local lights. | Add map-scoped lighting/weather profiles with day/night tint, fog/rain/snow parameters, local light markers, and runtime preview. |
| A15 | Sprite / Animation Import Pipeline | Character, enemy, VFX, and UI animation import should be reliable and creator-friendly. | Add import support for sliced sprite sheets with frame tags, preview playback, collision/source rectangles, and atlas-generation metadata. |
| A16 | Controller + Keyboard Remap UX | Runtime input governance exists, but players and creators need a polished remap screen. | Add an in-game remap screen plus editor preview for keyboard/controller bindings, conflicts, reset defaults, and accessibility alternatives. |
| A17 | Crash / Diagnostics Bundle Export | Support and debugging are easier when creators can package the right evidence in one click. | Export logs, project audit output, asset reports, config, save metadata, system info, and recent diagnostics snapshots into a single zip. |
| A18 | Mod SDK Documentation + Sample Mod | A bounded SDK story clarifies extension points before live mod loading is production-grade. | Publish a sample manifest-only mod with validation docs, permission examples, and expected editor diagnostics. |
| A19 | Cloud-Free Backup / Project Snapshotting | Local restore points protect creators before migrations and risky edits. | Add versioned local project snapshots, restore points, migration-before backups, and diff summaries. |
| A20 | Tutorial Project + Interactive Lessons | Approachability is a major RPG-maker differentiator. | Ship an interactive lesson project that walks through map creation, eventing, dialogue, battle, save/load, and export validation. |

## Second-Wave Recommended Sequencing

1. Guided New Project Wizard.
2. Quest / Objective System.
3. Dialogue Graph Editor.
4. Common Event Library.
5. In-Editor Playtest With Time Travel.
6. Database Editor Parity Pass.
7. Cutscene / Timeline Sequencer.
8. Localization Workspace and Accessibility Authoring Assistant.
9. Formula / Rule Debugger, Economy Balancer, and Encounter Table Editor.
10. World Map / Travel System, 2D Lighting / Weather Authoring, Sprite / Animation Import Pipeline, input-remap UX, diagnostics bundle export, Mod SDK sample, local project snapshots, and tutorial lessons.

## Third-Wave "Magic Layer" Upgrades

These ideas are aspirational product differentiators: features that would make URPG feel not only complete, but unusually powerful for RPG creators. They should still land as narrow, evidence-gated vertical slices.

| Priority | Candidate | Why It Matters | Suggested First Slice |
|---|---|---|---|
| M1 | Procedural Dungeon / Map Generator | Gives creators fast seeded drafts for dungeons, towns, caves, forests, and overworlds while preserving hand-editable output. | Add a deterministic dungeon generator that outputs normal map data, collision metadata, encounters, and validation warnings. |
| M2 | Flag / Switch / Variable Dependency Graph | RPG projects become hard to debug when flags, switches, variables, events, quests, and saves interact invisibly. | Build a graph viewer showing which events read/write each switch, variable, quest flag, and save field, with orphan and cycle warnings. |
| M3 | Narrative Continuity Checker | Helps catch impossible branches, orphaned dialogue, missing quest resolutions, and NPC lines that reference unreachable events. | Scan dialogue/event/quest data for broken references, unreachable branches, unresolved choices, and contradictory condition requirements. |
| M4 | Event Macro Recorder | Lets creators perform actions in playtest and convert them into event commands or timeline steps. | Record player/editor actions during a capture session and generate a draft event-command list with editable parameters. |
| M5 | Local AI Design Assistant | Gives creators optional assistance for drafts and balance while keeping runtime scope clean and provider-independent. | Add an offline or bring-your-own-provider assistant for item text, enemy names, quest outlines, dialogue drafts, and balance suggestions. |
| M6 | Smart Autofill Database Tools | Speeds up repetitive database authoring without hiding the generated values from designers. | Generate draft enemy stats, item prices, skill costs, XP curves, shop inventories, and encounter rewards from a selected design profile. |
| M7 | Visual Diff For Game Data | JSON diffs are hard to read; creators need semantic change review. | Show changes like "Skill Fireball damage changed 80 -> 95" or "Quest intro now requires switch 12" for project data edits. |
| M8 | Authoring Heatmaps | Local playtest telemetry can show where players walk, die, save, trigger events, open menus, or abandon battles. | Store local-only playtest samples and render map/battle heatmaps in the editor without remote upload. |
| M9 | Deterministic Replay Gallery | Replays are ideal for bug repros, boss-fight review, cutscene validation, and regression evidence. | Save input/state replay artifacts with labels, thumbnails, seed, project version, and one-click replay. |
| M10 | Boss Fight Designer | Bosses need specialized phase, threshold, music, dialogue, summon, reward, and preview tooling. | Add a boss profile editor for HP thresholds, phase transitions, scripted barks, battle BGM/ME cues, summons, and reward preview. |
| M11 | State Machine Visualizer | A deterministic engine can expose live runtime flow better than most RPG tools. | Render live graphs for battle phases, scene transitions, event pages, AI states, and ability tasks with current-state highlighting. |
| M12 | Content Completeness Score | Gives creators a clear sense of how close a project is to being shippable. | Extend project audit output with a weighted score covering missing icons, empty descriptions, broken events, untranslated strings, asset gaps, and export blockers. |
| M13 | In-Editor Screenshot / Trailer Capture | Store-page media and progress sharing are major creator workflows. | Capture deterministic screenshots, GIFs, short clips, thumbnails, and scene flythroughs from editor/playtest states. |
| M14 | Theme / UI Skin Builder | RPGs often need custom menu identity, fonts, windows, cursors, and button/audio style. | Add a skin workspace for window frames, fonts, cursor styles, button states, menu sounds, and cross-screen preview. |
| M15 | Relationship / Reputation System | Factions, affection, trust, morality, and NPC memory unlock richer RPG consequence systems. | Add a native reputation/relationship registry with save persistence, event conditions, UI summaries, and validation. |
| M16 | Crafting / Recipe System | Crafting connects items, economy, exploration, shops, drops, and progression. | Add recipe data, ingredient source lookup, unlock conditions, crafting preview, economy checks, and save-backed discovery state. |
| M17 | Bestiary / Codex System | Collection-style lore and enemy records are common RPG engagement loops. | Add enemy entries, drops, weaknesses, lore unlocks, encounter records, scanned-state persistence, and completion tracking. |
| M18 | Photo Mode / Diorama Mode | Helps creators capture shareable scenes and inspect visual composition. | Add pause-and-compose controls for camera, time/weather override, UI hide, character pose presets, and screenshot export. |
| M19 | Patch / DLC Builder | Mature projects need incremental updates, compatibility checks, and artifact manifests. | Generate patch manifests, detect changed assets/data, validate schema compatibility, and package incremental update bundles. |
| M20 | Creator Marketplace-Ready Packaging | Even before a live marketplace, creators benefit from validated distribution bundles for templates, plugins, and asset packs. | Package templates, plugins, asset packs, and sample projects with metadata, license evidence, compatibility targets, and validation reports. |

## Third-Wave Recommended Sequencing

1. Flag / Switch / Variable Dependency Graph.
2. Narrative Continuity Checker.
3. Deterministic Replay Gallery.
4. Procedural Dungeon / Map Generator.
5. Event Macro Recorder.
6. Visual Diff For Game Data.
7. State Machine Visualizer.
8. Boss Fight Designer.
9. Authoring Heatmaps and Content Completeness Score.
10. Theme / UI Skin Builder, relationship/reputation, crafting, bestiary/codex, screenshot/trailer capture, patch/DLC builder, creator packaging, photo mode, smart autofill, and optional local AI assistance.

## Fourth-Wave Feature Candidates

This pass focuses on deeper creator workflows, debugging power, and genre-specific RPG systems that can compound the value of the earlier backlog.

| Priority | Candidate | Why It Matters | Suggested First Slice |
|---|---|---|---|
| F1 | Enemy AI Behavior Designer | Designers need predictable enemy behavior without writing code. | Add a behavior-profile editor for target selection, skill weights, HP thresholds, cooldowns, and fallback actions. |
| F2 | Party Tactics / Auto-Battle Planner | Party AI, auto-battle, and companion behavior are common RPG needs. | Add party tactic profiles such as conserve MP, focus weakest, heal below threshold, exploit weakness, and defend when low HP. |
| F3 | Job / Class Progression Designer | Many RPGs rely on classes, job unlocks, skill trees, and progression gates. | Add a class progression graph with level gates, prerequisites, learned skills, stat curves, and validation. |
| F4 | Skill Combo / Synergy System | Combo rules create depth beyond single isolated skills. | Add skill tags and combo triggers that can unlock follow-up effects, bonus damage, chain reactions, or party synergies. |
| F5 | Loot Affix / Rarity Generator | Randomized loot and affixes support roguelite, dungeon, and collection loops. | Add item rarity tiers, affix pools, roll ranges, deterministic seeds, and economy validation. |
| F6 | Tactical Grid / Range Preview Toolkit | Tactics and turn-based projects need grids, ranges, line-of-sight, and path previews. | Add grid overlay, move/attack range preview, blocked-cell diagnostics, and deterministic path query tests. |
| F7 | Puzzle / Lock-Key System | RPG dungeons often need reusable puzzle state and dependency validation. | Add puzzle assets for locks, keys, switches, ordered triggers, reset rules, and completion rewards. |
| F8 | Shop / Vendor Designer | Shops are central to RPG economy and pacing. | Add vendor inventories, price rules, stock refresh conditions, buy/sell modifiers, and progression gates. |
| F9 | Inn / Rest / Recovery System | Resting, recovery, time advance, and revive rules deserve first-class data. | Add rest-point definitions with cost, recovery rules, time passage, event hooks, and save integration. |
| F10 | Calendar / Time-of-Day System | Life-sim, cozy RPG, and schedule-driven RPGs need time and calendar state. | Add calendar ticks, time blocks, schedules, event conditions, lighting hooks, and save persistence. |
| F11 | NPC Schedule / Routine Designer | Worlds feel alive when NPCs move, work, sleep, and react to time or flags. | Add schedule tracks for map, position, animation, dialogue state, and fallback behavior. |
| F12 | Reputation-Gated Content Browser | Designers need to see what content unlocks at each relationship, faction, or morality state. | Add a browser that lists content gated by reputation, relationship, flags, and quest state. |
| F13 | Map Region Rules Editor | Regions often drive encounters, music, weather, passability, hazards, and events. | Add map-region metadata for encounter pools, ambient audio, weather profile, damage floors, and movement rules. |
| F14 | Spawn / Respawn System | Enemies, pickups, and resource nodes need reusable spawn logic. | Add spawn tables with conditions, cooldowns, persistence flags, and save/load behavior. |
| F15 | Runtime Tutorial / Hint System | Shipped games need contextual tutorials and hint logic, not only editor lessons. | Add hint triggers, once-only flags, dismissal state, localization keys, and accessibility settings. |
| F16 | Player Choice Consequence Tracker | Designers need visibility into how choices affect flags, quests, relationships, and endings. | Add consequence records that link choices to changed state and expose impact summaries. |
| F17 | Ending / Route Manager | Multi-ending RPGs need route conditions and final-state validation. | Add ending definitions with prerequisites, priority ordering, conflict warnings, and route coverage reports. |
| F18 | Save Compatibility / Migration Previewer | Before shipping updates, creators need to know whether old saves still load. | Add a tool that runs old save fixtures through migrations and displays semantic before/after diffs. |
| F19 | Device / Platform Preview Profiles | Export readiness improves when creators can preview target constraints early. | Add profiles for desktop, web, handheld, and low-end devices with budget warnings and export notes. |
| F20 | Local Co-Author Review Workflow | Teams need reviewable changes without requiring live cloud collaboration. | Add local project change summaries, comments, review checklists, and handoff bundles backed by files/Git. |

## Feature Upgrade Pass

These are upgrade paths for already-recorded backlog items. Each should be treated as a follow-on slice after the base feature exists.

| Area | Upgrade | Why It Matters | Suggested Follow-On Slice |
|---|---|---|---|
| Project Health / Readiness Dashboard | Guided remediation workflows | Turn findings into actionable fix queues instead of passive reports. | Add "Fix next" cards with linked docs, owning files, validation commands, and completion criteria. |
| Asset Library + Asset Intake UX | Auto-cleanup and provenance packets | Asset hygiene should help users resolve issues, not just report them. | Add duplicate-resolution suggestions, license packet export, broken-reference repair, and safe-delete previews. |
| Visual Event Authoring | Event debugger with breakpoints | Event authoring becomes much safer when creators can step through conditions and commands. | Add breakpoints, watch variables, single-step, current-page highlight, and event stack view during playtest. |
| Plugin Compatibility Inspector | Compatibility scoring and shim hints | Creators need to know which plugins are safe, risky, or unsupported. | Add per-plugin score, unsupported API list, fallback notes, dependency graph, and suggested native replacement hints. |
| Battle Presentation Authoring | Phase timeline and cue choreography | Battle UX depends on timing across animation, camera, audio, and text. | Add timeline tracks for phase cues, VFX, camera shake, music/ME, dialogue barks, and HUD transitions. |
| Tilemap / Terrain / Layer Upgrade | Advanced brushes and chunked validation | Modern map editors need terrain transitions, layer locking, and fast collision/nav checks. | Add autotile brushes, random/weighted brushes, stamp brushes, collision chunk validation, and navigation overlay. |
| Export Runtime Hardening | Release candidate comparison | Creators need confidence that a new export did not regress. | Compare two export artifacts for content changes, schema version, asset manifest changes, and runtime signature status. |
| Starter Project Templates | Template certification suites | Templates should prove their own shippable loops. | Add per-template validation commands, guided playthrough scripts, and template-specific project audit expectations. |
| Save/Load Debugger | Corruption lab and recovery simulation | Recovery behavior should be testable before a real user loses progress. | Add tools to intentionally corrupt test saves, run recovery tiers, and export diagnostics. |
| Dev Room Test Harness | Auto-generated regression route | Dev rooms become powerful when they can run unattended. | Generate scripted routes through dev-room stations and record pass/fail evidence plus replay artifacts. |
| Quest / Objective System | Quest graph and route coverage | Quest bugs usually hide in branching dependencies. | Add graph view, objective dependency warnings, route coverage report, and save-state fixture generation. |
| Dialogue Graph Editor | Text pipeline integration | Narrative tools need localization, voice, notes, and branch previews. | Add string ID extraction, localization table sync, voice-line placeholders, speaker notes, and branch preview reports. |
| Common Event Library | Parametric recipes | Reusable events should be configurable without copy/paste drift. | Add parameters, preview fixtures, validation, versioned recipes, and one-click insertion into projects. |
| Database Editor Parity Pass | Bulk edit and validation lanes | Large RPG databases need efficient editing. | Add table bulk edit, CSV import/export, validation filters, semantic diff, and change review. |
| Cutscene / Timeline Sequencer | Runtime capture and replay | Cutscenes need reproducible previews and regression checks. | Add deterministic preview capture, timeline diff, event hooks, and golden replay output. |
| Playtest With Time Travel | Bug repro export | Rewind is strongest when it can produce shareable evidence. | Export compact repro bundles with seed, input log, state checkpoints, project version, and diagnostics. |
| Localization Workspace | Translation memory and glossary | Larger games need consistent terms across languages. | Add glossary enforcement, translation memory suggestions, missing-key fixes, and per-language layout snapshots. |
| Accessibility Authoring Assistant | Accessibility preview modes | Creators need to see issues as players experience them. | Add contrast simulation, colorblind palettes, readable-text preview, focus-order playback, and controller-only navigation test. |
| Formula / Rule Debugger | Batch balance probes | Single formula checks are useful; full dataset sweeps are better. | Run formula/rule simulations across all skills, enemies, levels, and equipment to flag outliers. |
| Economy Balancer | Playthrough economy simulation | Balance depends on whole routes, not isolated prices. | Simulate expected gold, XP, item flow, shop affordability, and encounter reward pacing over scripted routes. |
| Encounter Table Editor | Difficulty curve visualization | Encounter design needs pacing visibility. | Add region-by-region expected difficulty, reward, escape risk, and resource-drain charts. |
| Procedural Map Generator | Generator profiles and constraints | Procedural output needs creative control. | Add profiles for cave, dungeon, town, forest, and overworld with required rooms, locks, keys, shops, bosses, and encounter budgets. |
| Dependency Graph | Auto-fix and impact preview | Graphs are strongest when they help resolve problems. | Add rename/update impact previews, orphan cleanup, and "what changes if this switch is removed?" analysis. |
| Narrative Continuity Checker | Route proof reports | Writers need evidence that every route resolves. | Generate route proof reports for endings, unresolved choices, unreachable dialogue, and contradictory flags. |
| Deterministic Replay Gallery | Golden replay CI lane | Replays can become regression tests. | Add selected replay artifacts to CI and fail when deterministic state hashes diverge. |
| Local AI Design Assistant | Guardrailed source attribution and review | AI suggestions need reviewability and source boundaries. | Add opt-in provider config, prompt templates, generated-content review state, and no-runtime-dependency enforcement. |

## Fourth-Wave Recommended Sequencing

1. Enemy AI Behavior Designer and Party Tactics / Auto-Battle Planner.
2. Job / Class Progression Designer, Skill Combo / Synergy System, and Formula batch probes.
3. Shop / Vendor Designer, Loot Affix / Rarity Generator, Economy simulation, and Encounter difficulty visualization.
4. Tactical Grid / Range Preview Toolkit and Map Region Rules Editor.
5. NPC Schedule / Routine Designer, Calendar / Time-of-Day System, and Runtime Tutorial / Hint System.
6. Choice Consequence Tracker, Ending / Route Manager, and narrative route proof reports.
7. Save Compatibility / Migration Previewer and Release Candidate comparison.
8. Device / Platform Preview Profiles and Local Co-Author Review Workflow.
9. Upgrade the dashboard, asset, event, plugin, battle, tilemap, localization, accessibility, replay, and AI-assistant lanes after their base slices are landed.

## Non-Goals For This Planning Set

- No automatic `READY` promotion.
- No new release commitments without explicit scope, owner, tests, docs, and readiness-record changes.
- No live cloud, AI provider, marketplace, multiplayer, or full 3D commitments from this list.
