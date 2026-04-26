# URPG FacebookResearch Tooling Integration Plan

## Executive decision

Do **not** turn URPG into a research-lab codebase.

The correct move is to add a **small, isolated offline tooling layer** around the project and keep the actual game/runtime clean.

That means:

- the **game** stays game-focused
- the **ML/research repos** stay in `tools/` or separate helper environments
- the **runtime consumes static outputs only**: JSON, PNG, WAV/OGG, manifests, vector indexes, metadata
- no giant PyTorch stack inside the shipped player build unless there is a hard, proven reason

## What to adopt

### Tier 1 — worth adding now

These are the best fits from the Facebook Research org for a URPG pipeline.

| Repo | Use in URPG | Decision |
|---|---|---|
| `faiss` | semantic search over lore, dialogue, NPC memory, quests, items, imported game metadata | Adopt |
| `segment-anything` | sprite/portrait/UI extraction and cutout tooling | Adopt |
| `sam2` | improved segmentation/video-aware workflows for asset prep | Adopt |
| `demucs` | stem separation and cleanup for music/audio assets | Adopt |
| `encodec` | audio compression / asset-pipeline experimentation | Adopt |
| `audiocraft` | music/SFX prototyping for ideation and temp assets | Adopt, but tooling-only |

### Tier 2 — useful later if needed

| Repo | Use in URPG | Decision |
|---|---|---|
| `detectron2` | automatic asset tagging / QA / object detection in art batches | Optional |
| `pytorch3d` | mesh cleanup, previews, conversions, procedural 3D tooling | Optional |
| `iopath` / `fvcore` / `hydra` / `xformers` | support libraries for the offline ML toolchain | Optional support only |

### Tier 3 — skip for now

These are not bad repos. They are just the wrong thing to wire into the main project right now.

- `habitat-sim`
- `habitat-lab`
- most RL repos
- most benchmark repos
- most language-model training repos
- most vision research baselines
- most archived repositories

## Non-negotiable rules

1. **No direct research dependency in the runtime unless proven necessary.**
2. **No submoduling half of Facebook Research into the main game repo.**
3. **All heavy ML code runs offline or in editor-time helper services.**
4. **Outputs must be stable, inspectable, and versionable.**
5. **Every pipeline stage must be restartable and safe to rerun.**
6. **Every adopted repo must pass license review before shipping.**
7. **Anything experimental must stay behind a tooling boundary.**

## Recommended architecture

### Suggested repo layout

```text
project_root/
├─ docs/
│  ├─ roadmaps/
│  │  └─ URPG_facebookresearch_tooling_integration_plan.md
│  ├─ decisions/
│  │  └─ ADR_001_ml_tooling_boundary.md
│  └─ pipeline/
│     ├─ asset_pipeline.md
│     ├─ audio_pipeline.md
│     └─ retrieval_pipeline.md
│
├─ tools/
│  ├─ ml_env/
│  │  ├─ requirements.txt
│  │  ├─ environment.yml
│  │  └─ README.md
│  ├─ retrieval/
│  │  ├─ faiss_index_builder/
│  │  ├─ embedding_jobs/
│  │  └─ query_debugger/
│  ├─ vision/
│  │  ├─ sam_segment/
│  │  ├─ sam2_batch_segment/
│  │  ├─ asset_classifier/
│  │  └─ manifest_builder/
│  ├─ audio/
│  │  ├─ demucs_separate/
│  │  ├─ encodec_compress/
│  │  ├─ audiocraft_prototype/
│  │  └─ audio_manifest_builder/
│  ├─ importers/
│  │  ├─ rpg_maker/
│  │  ├─ pixel_game_maker_mv/
│  │  └─ generic_asset_import/
│  └─ shared/
│     ├─ logging/
│     ├─ config/
│     ├─ cache/
│     └─ job_runner/
│
├─ data/
│  ├─ raw/
│  ├─ staged/
│  ├─ processed/
│  ├─ indexes/
│  └─ manifests/
│
├─ content/
│  ├─ art/
│  ├─ audio/
│  ├─ lore/
│  └─ gameplay/
│
├─ runtime/
│  ├─ asset_loaders/
│  ├─ search/
│  └─ content_db/
│
└─ scripts/
   ├─ build_indexes.*
   ├─ import_assets.*
   └─ refresh_manifests.*
```

### Boundary model

#### Runtime side

The runtime should only read:

- exported PNG/WebP cutouts
- cleaned/encoded WAV or OGG files
- JSON manifests
- vector index files or precomputed retrieval data
- tagged asset metadata

#### Tooling side

The tooling layer handles:

- segmentation
- asset classification
- search indexing
- audio stem separation
- compression tests
- music/SFX prototyping
- batch import QA

## Planned implementation phases

## Phase 0 — lay down the boundary first

### Goal

Create the structure that prevents future mess.

### Work

- create `docs/roadmaps/`, `docs/decisions/`, and `tools/`
- add one ADR documenting the runtime/tooling split
- add a dedicated ML/tooling environment file
- add a shared job runner pattern for offline jobs
- define manifest formats before building tools

### Deliverables

- folder scaffolding exists
- ADR for tooling boundary exists
- base manifest schemas exist
- one command to run offline jobs exists

### Acceptance criteria

- no runtime package imports PyTorch-heavy tooling
- all tooling runs from `tools/` or a separate environment
- build scripts can regenerate outputs without manual patching

## Phase 1 — semantic retrieval with FAISS

### Why this matters

This is the highest-value systems addition.

It gives URPG a project-wide searchable brain for:

- lore lookups
- NPC memory support
- quest reference retrieval
- asset metadata search
- dialogue authoring assistance
- imported RPG Maker / Pixel Game Maker MV project knowledge

### Scope

- embed lore docs, quest text, item descriptions, enemy notes, dialogue banks, imported project metadata
- build a FAISS index
- build a local query tool for authors and debugging
- optionally export a lightweight runtime-friendly retrieval bundle

### Suggested outputs

```text
/data/indexes/
  lore.faiss
  dialogue.faiss
  assets.faiss
/data/manifests/
  lore_chunks.json
  dialogue_chunks.json
  assets_chunks.json
```

### Acceptance criteria

- query returns relevant lore/dialogue chunks in under a practical local threshold
- index rebuild is scriptable
- results can be inspected with chunk IDs and source paths
- imported external game metadata can be searched from the same tool

## Phase 2 — asset segmentation with SAM / SAM2

### Why this matters

This is the best art-pipeline win.

You can use it for:

- portraits
- character cutouts
- UI pieces
- object extraction
- tileset cleanup
- sprite sheet prep
- quick masking for manual repaint or recomposition

### Scope

- batch segmentation for image folders
- export masks and cutouts
- generate per-asset JSON metadata
- allow manual override/cleanup after auto segmentation
- support imported assets from RPG Maker and Pixel Game Maker MV sources

### Suggested outputs

```text
/data/staged/segmentation/
  <asset_name>/
    source.png
    mask.png
    cutout.png
    manifest.json
```

### Acceptance criteria

- segmented outputs can be batch-generated
- each output includes source path + dimensions + mask metadata
- manual edits can override auto outputs without getting wiped unintentionally
- cutout results save real time on asset prep

## Phase 3 — audio pipeline with Demucs + Encodec + AudioCraft

### Why this matters

This gives you a real audio pipeline instead of random one-off asset handling.

### Demucs use

Use for:

- separating stems from songs
- isolating vocals/instrumentals for editing
- cleaning source material
- remix prep

### Encodec use

Use for:

- compression experiments
- batch testing audio footprint tradeoffs
- pipeline-side encoding research before final export decisions

### AudioCraft use

Use for:

- prototype music
- temp ambient tracks
- temp SFX ideas
- moodboarding

Do **not** treat it as the only final-audio path.

### Scope

- batch stem separation tool
- batch audio analysis + manifest generation
- compression experiment harness
- temp track generation pipeline for prototyping only

### Acceptance criteria

- stems can be generated in batch
- processed audio is tracked by manifests
- pipeline can compare before/after size and quality
- generated prototype audio is clearly labeled as temp, not final

## Phase 4 — asset tagging with Detectron2

### Why this matters

This is optional, but useful once asset count gets large.

### Use cases

- detect likely portraits vs sprites vs props vs UI
- auto-tag batches for review
- identify repeated structures across imports
- catch mislabeled or malformed asset folders

### Acceptance criteria

- tags improve sorting speed
- false positives are tolerable or easy to correct
- tagging never becomes a runtime dependency

## Phase 5 — optional 3D/mesh tooling with PyTorch3D

### Only do this if one of these becomes true

- URPG adds 3D content
- URPG adds 2.5D with real mesh processing
- you need automated preview/render/cleanup tools for imported models

### Use cases

- mesh conversion helpers
- preview renders
- normal/tangent checking
- asset QA
- procedural 3D data transforms

### Acceptance criteria

- improves the 3D content pipeline enough to justify maintenance
- stays out of the player build unless necessary

## What not to do

### Do not

- import research repos directly into the runtime
- let experimental Python code leak into shipping game logic
- depend on cloud-only inference for core workflows
- treat generated outputs as trustworthy without manifests and QA
- assume archived repos are production-ready
- adopt everything because it looks impressive

### Specifically skip for now

- embodied-agent stacks
- robotics/sim repos
- large RL frameworks
- benchmark suites
- training frameworks that do not solve a direct asset or content problem

## First 30-day rollout plan

## Week 1

- create docs and tooling folder structure
- add ADR for boundaries
- define manifest schemas
- scaffold FAISS index builder

## Week 2

- build first lore/dialogue retrieval index
- create local query/debug CLI
- test on existing project text and imported metadata

## Week 3

- build SAM batch segmentation tool
- export masks, cutouts, and manifests
- test on portraits, UI, and sprite assets

## Week 4

- add Demucs-based stem separation
- add audio manifests
- add Encodec test harness
- optionally add AudioCraft temp-generation workflow

## Backlog after that

- Detectron2 tagging
- PyTorch3D helpers if 3D pipeline becomes real
- editor integration / dashboards
- importer-specific adapters for RPG Maker and Pixel Game Maker MV

## Suggested task breakdown

### Task URPG-ML-001 — repository scaffolding

- create `docs/roadmaps`
- create `docs/decisions`
- create `tools/retrieval`
- create `tools/vision`
- create `tools/audio`
- create `tools/importers`
- create `data/raw`, `data/staged`, `data/processed`, `data/indexes`, `data/manifests`

### Task URPG-ML-002 — manifest schema definition

Create schemas for:

- asset manifests
- audio manifests
- retrieval chunk manifests
- job run metadata

### Task URPG-ML-003 — FAISS indexer

Build:

- text chunker
- embedding adapter
- FAISS writer
- query debugger
- manifest exporter

### Task URPG-ML-004 — SAM segmentation job

Build:

- folder batch runner
- mask exporter
- cutout exporter
- per-asset metadata exporter
- manual override support

### Task URPG-ML-005 — audio pipeline

Build:

- Demucs separation runner
- Encodec benchmark runner
- AudioCraft prototype runner
- audio manifest builder

### Task URPG-ML-006 — importer integration

Build adapters for:

- RPG Maker asset layouts
- Pixel Game Maker MV asset layouts
- generic imported project folders

## Recommended prompts for your IDE agent

### Prompt 1 — scaffold the boundary

```text
Create the initial URPG tooling boundary.

Requirements:
- Do not modify runtime gameplay systems.
- Add docs/roadmaps, docs/decisions, tools/retrieval, tools/vision, tools/audio, tools/importers, and data/* directories.
- Add one ADR describing that heavy ML/research tooling stays outside the shipped runtime.
- Add placeholder README files explaining purpose of each tooling directory.
- Keep changes small, explicit, and easy to review.
```

### Prompt 2 — build the FAISS path first

```text
Implement the first usable URPG retrieval toolchain.

Requirements:
- Build an offline FAISS indexing pipeline for lore, dialogue, quest text, and imported project metadata.
- Export chunk manifests with source file paths and chunk IDs.
- Add a local query CLI for debugging.
- Do not add any FAISS or PyTorch dependency to the runtime build.
- Keep all code under tools/retrieval.
- Add brief documentation for rebuilding the index.
```

### Prompt 3 — build the art pipeline next

```text
Implement the first URPG asset segmentation pipeline.

Requirements:
- Create a batch segmentation job under tools/vision using a SAM/SAM2-compatible workflow.
- Input: folders of images.
- Output: mask, cutout, and manifest.json for each asset.
- Preserve source path metadata.
- Support reruns without deleting manual overrides.
- Do not wire any of this into runtime code yet.
```

### Prompt 4 — add audio tooling without contaminating the game

```text
Implement the URPG audio tooling layer.

Requirements:
- Add batch stem separation tooling under tools/audio.
- Add manifest generation for processed audio outputs.
- Add a compression experiment harness.
- Treat generated/prototype music as temporary assets only.
- Do not introduce heavyweight audio ML dependencies into the runtime build.
```

## Final recommendation

If you only do **three** things from this whole plan, do these first:

1. **FAISS retrieval layer**
2. **SAM/SAM2 asset segmentation pipeline**
3. **Demucs/Encodec audio pipeline**

That gives URPG the biggest real-world gain for the least architectural damage.

Everything else is secondary.

## Bottom line

The right plan is **not** “add Facebook Research to the project.”

The right plan is:

- keep URPG clean
- build a separate offline toolchain
- use a few of the best repos as focused pipeline components
- ship stable outputs, not research experiments
