# URPG Private-Use Asset Intake Plan

## Purpose

This intake plan is for **private/internal tool development and prototyping**.

That means the goal is not to build a clean redistributable asset pack yet. The goal is to:
- get useful assets into the repo fast,
- expand test coverage for the asset pipeline,
- improve editor/runtime realism,
- seed stronger placeholder and prototype content,
- create a practical bridge from reference-fixture mode into content-rich development.

This plan intentionally relaxes distribution-focused filtering, but it still avoids wasting time on:
- repos that are only lists and contain no actual assets,
- repos that are mostly engine wrappers with no transferable value,
- repos that are too chaotic to ingest without structure.

Program linkage: this intake plan is governed at the program level by `docs/PROGRAM_COMPLETION_STATUS.md` under `P3-03 — Private-Use Asset Intake Needs Canonical Governance` and `Phase 4 / Workstream 4.2`.

## Governance Artifacts

This plan is supported by the following canonical artifacts:

- [Asset Source Registry](docs/asset_intake/ASSET_SOURCE_REGISTRY.md)
- [Asset Promotion Guide](docs/asset_intake/ASSET_PROMOTION_GUIDE.md)
- [Asset Category Gaps](docs/asset_intake/ASSET_CATEGORY_GAPS.md)
- Staging root: [`imports/staging/asset_intake/`](imports/staging/asset_intake/)
- Normalized root: [`imports/normalized/`](imports/normalized/)
- Manifest roots: [`imports/manifests/`](imports/manifests/)
- Report root: [`imports/reports/`](imports/reports/)
- Direct-ingest asset mirrors: [`imports/raw/third_party_assets/github_assets/`](imports/raw/third_party_assets/github_assets/)

Cross-cutting external repository intake artifacts:

- [Repo Watchlist](docs/external-intake/repo-watchlist.md)
- [License Matrix](docs/external-intake/license-matrix.md)
- [Repo Audit Template](docs/external-intake/repo-audit-template.md)
- [Feature Adoption Matrix](docs/external-intake/urpg_feature_adoption_matrix.md)

---

## Intake Targets

## Tier 1 — Direct Ingest Repos

These are the highest-value repos because they contain assets or immediately usable packaged content.

### 1. AscensionGameDev/Intersect-Assets
**Role:** broad fantasy starter library

**Why it matters**
- It is the largest practical asset haul from the search.
- It includes tilesets, characters, animations, UI, sounds, music, tools, and more.
- It is the best candidate for quickly moving URPG beyond fixture-only content.

**Best uses in URPG**
- environment kit bootstrapping
- character placeholder coverage
- UI prototyping
- audio resolver testing
- animation/VFX placeholder coverage

**Do not do blindly**
- do not ingest the entire repo as one flat asset dump
- do not merge raw content directly into production roots
- do not assume naming/organization is already aligned with URPG conventions

**Private-use decision**
- ingest aggressively, but preserve source provenance by folder and manifest

### 2. GDQuest/game-sprites
**Role:** clean prototype sprite pack

**Why it matters**
- small, low-friction, and useful immediately
- better for gameplay prototyping than for final visual identity
- ideal for placeholder combat, map, and interaction loops

**Best uses in URPG**
- combat prototype actors
- inventory/item placeholders
- grid-based and interaction tests
- early map/entity validation

**Private-use decision**
- ingest early as the fastest zero-friction baseline content pack

### 3. Calinou/kenney-interface-sounds
**Role:** UI/audio micro-pack

**Why it matters**
- highly focused asset pack
- useful immediately for menu/UI/feedback polish
- quick win for editor feel and runtime usability feedback

**Best uses in URPG**
- menu confirm/cancel
- button hover/click
- panel open/close
- inventory and notification feedback
- editor shell feedback sounds

**Private-use decision**
- ingest almost immediately and normalize into the SFX library

---

## Tier 2 — Discovery / Source-Mining Repos

These are not direct asset packs. They are discovery layers and source maps.

### 4. madjin/awesome-cc0
**Role:** curated CC0 discovery index

**Why it matters**
- useful for finding next-wave assets fast
- especially useful for textures, materials, HDRIs, 3D models, and broader CC0 sources

**Best uses in URPG**
- sourcing environment materials
- finding future UI/icon/audio/3D references
- building a vetted internal source list

**Private-use decision**
- do not ingest as assets
- convert into a tracked source catalog and harvest selectively

### 5. HotpotDesign/Game-Assets-And-Resources
**Role:** broad discovery directory

**Why it matters**
- good as a hunting map when the team needs more assets quickly
- broad enough to discover category gaps

**Best uses in URPG**
- finding category-specific asset sources
- locating emergency fill-ins for missing content classes
- maintaining a future acquisition backlog

**Private-use decision**
- do not ingest directly
- mine it for targeted follow-up sources only

---

## Overall Intake Strategy

## Primary rule
Treat asset repos and directory repos differently.

### Direct-ingest repos
These go through staging, normalization, tagging, dedupe, and controlled promotion:
- Intersect-Assets
- GDQuest/game-sprites
- kenney-interface-sounds

### Discovery repos
These become tracked source catalogs and acquisition backlogs:
- awesome-cc0
- Game-Assets-And-Resources

---

## Repo Layout Additions

Create these roots if they do not already exist:

```text
third_party/
  github_assets/
    intersect-assets/
    gdquest-game-sprites/
    kenney-interface-sounds/

imports/
  staging/
    asset_intake/
      intersect-assets/
      gdquest-game-sprites/
      kenney-interface-sounds/
  normalized/
    ui_sfx/
    prototype_sprites/
    fantasy_environment/
    placeholder_characters/
    music/
    vfx/
  manifests/
    asset_sources/
    asset_bundles/
  reports/
    asset_intake/

docs/
  asset_intake/
    ASSET_SOURCE_REGISTRY.md
    ASSET_PROMOTION_GUIDE.md
    ASSET_CATEGORY_GAPS.md
  external-intake/
    repo-watchlist.md
    license-matrix.md
    repo-audit-template.md
    urpg_feature_adoption_matrix.md
```

---

## Asset Intake Phases

## Phase A — Source Capture

### Objective
Mirror or snapshot candidate sources into repo-controlled staging roots.

### Tasks
1. Clone or download the direct-ingest repos into `imports/raw/third_party_assets/github_assets/`.
2. Record source metadata for each repo:
   - repo name
   - branch/commit
   - snapshot date
   - intended use
   - source category
3. Build a small manifest per source under `imports/manifests/asset_sources/`.
4. Keep original folder structures intact during capture.

### Acceptance criteria
- all direct-ingest repos are mirrored locally
- every source has a manifest entry
- provenance is recoverable without git archaeology

---

## Phase B — Initial Audit

### Objective
Understand what was actually pulled before mixing anything into normalized content roots.

### Tasks
1. Run `tools/assets/asset_hygiene.py` against the new roots.
2. Produce reports for:
   - duplicate files
   - oversize files
   - junk artifacts
   - suspicious naming collisions
3. Manually inspect top-level structure of each pack.
4. Bucket assets into rough classes:
   - tilesets
   - characters
   - UI
   - animations/VFX
   - SFX
   - BGM
   - tools/source files

### Acceptance criteria
- hygiene report exists for each new pack
- category breakdown exists
- obvious junk and low-value noise are identified

---

## Phase C — Controlled Normalization

### Objective
Convert useful source content into URPG-friendly asset roots without losing provenance.

### Tasks
1. Normalize names and folder structure for promoted subsets.
2. Preserve source pack identity in metadata.
3. Split promoted assets into logical URPG buckets:
   - `normalized/ui_sfx/`
   - `normalized/prototype_sprites/`
   - `normalized/fantasy_environment/`
   - `normalized/placeholder_characters/`
   - `normalized/music/`
   - `normalized/vfx/`
4. Generate per-bundle manifests:
   - original source
   - original relative path
   - promoted target path
   - category
   - notes

### Acceptance criteria
- normalized roots exist with clean substructure
- promoted assets are not anonymous
- source-to-target mapping is explicit

---

## Phase D — Fast-Win Integration

### Objective
Get immediate value into URPG instead of waiting for a perfect asset system.

### Target integrations

#### D1. UI Sound Pass
Use `kenney-interface-sounds` first.

**Integrations**
- editor button click
- panel open/close
- command confirm/cancel
- notification / toast feedback
- menu navigation sounds

**Acceptance criteria**
- editor shell and one runtime menu path both use real UI sounds
- audio events are mapped through your state-driven audio layer, not ad hoc file calls

#### D2. Prototype Sprite Pass
Use `GDQuest/game-sprites` next.

**Integrations**
- placeholder actors in map scene
- simple combat entity visuals
- item/weapon placeholders
- test scenes for sprite atlas packing

**Acceptance criteria**
- prototype sprite set is visible in at least one map flow and one battle flow
- sprite pipeline can process the chosen subset cleanly

#### D3. Fantasy Environment Pass
Use `Intersect-Assets` in subsets, not whole-pack dump.

**Integrations**
- one curated environment tileset bundle
- one placeholder character family
- one UI subset if it visually fits
- one VFX/animation subset
- one audio subset for environmental realism

**Acceptance criteria**
- one representative vertical slice scene uses real environment assets
- one character/monster placeholder set is usable in-scene
- one VFX subset is loaded and tested

---

## Phase E — Source Mining Expansion

### Objective
Use the discovery repos to build the next-wave acquisition backlog.

### Tasks
1. Review `awesome-cc0` by category.
2. Pull only the highest-value candidate sources into [`docs/asset_intake/ASSET_SOURCE_REGISTRY.md`](docs/asset_intake/ASSET_SOURCE_REGISTRY.md).
3. Review `Game-Assets-And-Resources` for category gaps only.
4. Add follow-up targets by category, not by random browsing.

### Initial categories to mine
- environment textures/materials
- icon packs
- fantasy UI frames
- battle VFX sheets
- ambient/background audio
- 3D materials and references for future presentation experiments

### Acceptance criteria
- discovery repos are converted into a structured acquisition backlog
- no raw directory repo is mistaken for a direct asset pack

---

## Per-Repo Execution Plan

## Intersect-Assets
### Pass 1
- capture source
- audit structure
- identify high-value subtrees

### Pass 2
Promote only:
- selected tilesets
- selected character/monster subsets
- selected UI elements
- selected SFX/BGM subsets
- selected animation/VFX assets

### Pass 3
- generate source manifest
- route promoted assets through hygiene and dedupe
- add vertical slice scenes that prove usefulness

### Avoid
- importing every branch variant
- mixing source, compiled, and upgrade branches together blindly
- flattening all art into one folder

---

## GDQuest/game-sprites
### Pass 1
- ingest full repo
- classify by characters/grid/weapons

### Pass 2
- normalize for sprite atlas tests
- use as placeholder visuals in debug and prototype scenes

### Pass 3
- wire into sample content packs for battle/map testing

### Avoid
- treating as final art direction
- overfitting naming conventions to this one small repo

---

## kenney-interface-sounds
### Pass 1
- ingest complete pack
- convert/normalize filenames if needed

### Pass 2
- map sounds into UI semantic events

### Pass 3
- add a small sound routing table for editor/runtime reuse

### Avoid
- scattering raw audio file calls everywhere
- engine-specific folder assumptions from the source packaging

---

## awesome-cc0
### Pass 1
- extract only high-priority source candidates into source registry

### Pass 2
- pick 5–10 likely next asset providers

### Pass 3
- add them as future acquisition targets, not immediate ingestion

### Avoid
- trying to ingest the list itself
- treating every linked source as trustworthy or maintained

---

## Game-Assets-And-Resources
### Pass 1
- use only for gap analysis

### Pass 2
- identify category-specific sources to inspect manually

### Pass 3
- add only validated follow-up candidates to the backlog

### Avoid
- broad copying from a giant directory
- spending days wandering list pages without category goals

---

## Tooling Changes Required

## 1. Asset source manifest schema
Add or extend a manifest schema for external asset source tracking.

Suggested fields:
- source_id
- repo_name
- snapshot_commit
- source_type (`direct_asset_pack`, `discovery_index`, `utility_pack`)
- category_tags
- intended_use
- internal_notes

## 2. Promotion manifest schema
Track each promoted asset with:
- source_id
- original_relative_path
- promoted_relative_path
- category
- pack_name
- status (`staged`, `normalized`, `approved_for_internal_use`)

## 3. Asset bundle registry
Group promoted assets into internal-use bundles such as:
- `prototype_ui_sfx_01`
- `prototype_sprite_pack_01`
- `fantasy_environment_pack_01`
- `placeholder_character_pack_01`

## 4. Intake report generation
Add a lightweight report pass that outputs:
- source count
- promoted file count
- duplicates removed
- category coverage
- remaining gaps

---

## Category Gaps After This Intake

Even after this plan, you will still likely be missing:
- cohesive final-character portrait art
- a unified final UI skin
- a polished final VFX identity
- a final music identity
- high-end environment texture/material coherence

This intake is about **coverage and acceleration**, not final visual canon.

---

## Recommended Execution Order

1. `kenney-interface-sounds`
2. `GDQuest/game-sprites`
3. `Intersect-Assets` (curated subsets only)
4. `awesome-cc0` source mining
5. `Game-Assets-And-Resources` gap-driven mining

That order is intentional:
- fastest integration value first
- smallest risk first
- biggest/noisiest pack after the pipeline is warmed up
- discovery directories only after direct ingestion starts paying off

---

## Minimum Success Definition

This intake program is successful when all of the following are true:

1. URPG is no longer operating only with fixture/reference content.
2. The editor has real UI sound feedback.
3. Map and battle prototype scenes use real placeholder sprite content.
4. At least one vertical-slice fantasy scene uses curated environment assets from a broad pack.
5. Asset provenance is preserved even in private-use mode.
6. Discovery repos have been converted into a tracked acquisition backlog instead of random bookmarks.

---

## Immediate Next Actions

1. Create the new asset intake folder structure.
2. Pull `kenney-interface-sounds`, `GDQuest/game-sprites`, and `Intersect-Assets` into staging.
3. Run asset hygiene reports.
4. Promote one tiny subset from each direct-ingest repo.
5. Wire those subsets into one editor path and one runtime path.
6. Build the source registry from `awesome-cc0` and `Game-Assets-And-Resources`.
