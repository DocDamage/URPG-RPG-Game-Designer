# URPG Maker Shell Visual Language

Status date: 2026-05-05

This document defines the visual and interaction guardrails for the URPG Maker Shell. It keeps the product direction clear without copying another product's protected look.

## Intent

URPG Maker should feel fast, visual, cohesive, and creator-first. The shell should make common creation loops obvious:

- choose or create a project
- pick a template
- browse assets
- place parts on the canvas
- preview and playtest immediately
- fix blockers from diagnostics
- package when ready

## IP-Safe Rule

Maker tools can inspire interaction architecture. They cannot be visual source material.

Do not copy:

- Nintendo characters
- Nintendo names or trademarks
- exact icon silhouettes
- exact layout measurements
- sound effects
- animation timing
- color identity
- trade dress

URPG must use original RPG-focused terminology, iconography, sounds, and styling.

## UI Personality

The editor should be:

- canvas-first
- dense enough for real production
- readable at a glance
- playful only where it improves creation speed
- diagnostic-rich
- predictable
- stable under keyboard, mouse, and future controller input

Avoid:

- oversized landing-page composition
- decorative cards inside cards
- hidden controls
- UI that only explains itself with text
- one-note palettes
- dark empty space without useful affordances

## First-Screen Direction

The first screen is a working main menu, not a marketing page.

Required areas:

- Continue Last Project
- New Project
- Open Project
- Recent Projects
- Pinned Projects
- Settings
- Missing project remediation when a recent project path no longer exists

## Onboarding Direction

Onboarding should be robust, helpful, and skippable from settings.

It should ask fewer or more questions based on game type. The first shipped game types are:

- JRPG
- action RPG
- tactical RPG
- visual novel hybrid
- cozy/life sim
- monster collector
- platform/adventure

Each path should recommend defaults with reasons, while allowing override.

## Maker Shell Layout

Default layout:

```text
Top: project actions, part belt, search
Left: collapsible asset browser and folders/categories
Center: Level Builder canvas
Right: inspector, diagnostics, package, All Tools
Bottom: playtest, save, blockers, coordinates, zoom
```

The left-side asset browser is the default because it matches the user's requested FL Studio-like browsing model while preserving the top belt for common placement parts.

## Asset Browser

The browser must support:

- folders/categories
- search
- source/pack filters
- template scope
- project scope
- full-library opt-in
- preview drawer
- pinned favorites
- recent assets
- missing/deleted project remediation

The browser must not load the full generated asset library during default startup.

## Preview Rules

Preview should handle:

- image files
- spritesheets
- UI theme pieces
- audio metadata
- font metadata
- JSON/manifests
- unknown file fallback

Preview failures should show diagnostics and must not crash the editor.

## Level Builder Interaction

Required creation loop:

- select part
- preview ghost on grid
- click to paint
- drag to paint
- right-click or secondary action to erase/context
- undo/redo
- save
- playtest
- return to edit

The Level Builder must keep `GridPartDocument` as the source of truth.

## Surface Cards

All Tools/Search cards must show truth:

- ReleaseTopLevel
- Nested
- DevOnly
- Deferred
- Showcase
- Template
- OwnerOnly

Disabled cards must explain why they are disabled and what promotes them.

## Theme Policy

In-game UI themes and editor themes are separate.

- Game UI themes can be offered to generated games when `gameUiReady` passes.
- Editor themes require stricter completeness, including semantic icon aliases, font pairing, nine-slice rules, scale policy, color tokens, and all required editor-control states.
- Complete UI Essential is currently game-UI-ready.
- Wenrexa is currently candidate-only.

## Accessibility

Every visible command needs:

- label
- role
- tooltip or description when needed
- enabled/disabled state
- disabled reason
- keyboard/focus reachability

Disabled state cannot rely on color alone.

## Feature Flag Policy

The Maker Shell should be feature-flagged until Level Builder parity is proven.

Legacy ImGui surfaces can remain while migrating. Only surfaces marked as document-backed should be held to the no-direct-ImGui boundary outside the backend adapter.
