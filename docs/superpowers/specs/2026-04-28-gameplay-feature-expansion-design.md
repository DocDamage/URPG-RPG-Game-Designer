# Gameplay Feature Expansion Design

Date: 2026-04-28
Status: Approved for implementation planning

## Goal

Add game-facing maker features that creators can use inside URPG projects, not just editor infrastructure. Each feature must satisfy URPG's WYSIWYG rule before it can be called complete:

- visual authoring surface
- live preview
- saved project data
- runtime execution
- diagnostics/export evidence
- focused verification

## Feature Set

The expansion covers ten feature lanes:

1. Quest + Objective Graph
2. Skill Tree / Class Progression
3. Relationship / Affinity System
4. Crafting + Gathering + Economy
5. Monster Capture + Party Management
6. Enemy Encounter Designer
7. Item Affix / Loot Generator
8. NPC Schedule + Daily Routine Authoring
9. Metroidvania Ability Gates
10. Cutscene / Event Timeline

## Implementation Order

Implementation starts with Quest + Objective Graph because later systems can depend on quest state, rewards, conditions, and diagnostics. The planned order is:

1. Quest + Objective Graph
2. Skill Tree / Class Progression
3. Relationship / Affinity
4. Crafting + Gathering + Economy
5. Monster Capture + Party Management
6. Enemy Encounter Designer
7. Item Affix / Loot Generator
8. NPC Schedule + Daily Routine
9. Metroidvania Ability Gates
10. Cutscene / Event Timeline

## Architecture

Each lane gets the same bounded structure:

- Runtime code under `engine/core/<feature>/`
- Editor authoring panel under `editor/<feature>/`
- JSON schema under `content/schemas/`
- Proof fixture under `content/fixtures/`
- Diagnostics/export evidence through the existing editor diagnostics or project-audit patterns
- Focused tests under `tests/unit/`

Feature systems should integrate with existing URPG domains instead of creating duplicate authorities. For example, quests use existing rewards, variables, dialogue, save, and event-command concepts where available. Skill trees integrate with ability and character progression. Crafting integrates with economy/vendor/item systems. Cutscenes integrate with dialogue, timeline, audio, and event commands.

## Data Flow

Creator-authored JSON is loaded into runtime documents, validated, edited through a visual panel, previewed live, saved back to project data, and executed by runtime services. Diagnostics snapshots expose the same authored/runtime state so export and project-audit checks can prove the shipped game data matches what the editor preview showed.

## Completion Contract

A lane is not complete when only a runtime class exists. A lane is complete only when all of the following exist:

- schema and fixture
- runtime document/model
- runtime execution path
- editor panel with preview snapshot
- save/load or project-data persistence as appropriate
- diagnostics/export snapshot
- tests covering authoring, runtime execution, saved data, and diagnostics
- readiness/docs status updated without overclaiming broader scope

## Error Handling

Validation should fail closed with structured diagnostics. Missing references, circular graphs, invalid rewards, unavailable abilities, invalid item IDs, and unreachable nodes must be reported as authored-data issues rather than causing runtime crashes. Preview panels should expose blocking diagnostics instead of silently dropping invalid content.

## Testing Strategy

Each lane gets focused unit coverage for:

- valid fixture loading
- invalid fixture diagnostics
- editor panel preview snapshot
- runtime execution
- JSON round trip
- diagnostics/export evidence

Broader integration and full release gates can follow after all feature lanes have first-class implementations.

## Scope Boundaries

This design does not include external online services, commercial marketplaces, proprietary platform SDKs, or public asset acquisition. It also does not claim every possible genre-specific permutation is complete. The target is complete first-class in-tree feature implementations for the ten listed maker/gameplay systems.
