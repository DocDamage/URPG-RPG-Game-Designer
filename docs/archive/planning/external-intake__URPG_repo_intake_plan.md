# URPG External Repository Intake Plan

## Title
URPG Repository Intake Program — RPG Maker / EasyRPG / Tooling Ecosystem

## Purpose
This plan defines how to evaluate, ingest, quarantine, mine, and operationalize twelve external repositories that may accelerate URPG across the following lanes:

- compat-layer surface coverage
- importer and migration tooling
- asset/reference intake
- export and build automation
- editor workflow and authoring UX
- localization bootstrap
- regression corpus expansion

This is **not** a blind vendoring plan. The point is to extract leverage without contaminating URPG’s architecture, license posture, determinism guarantees, or runtime quality.

Program linkage: this intake plan is governed at the program level by `docs/PROGRAM_COMPLETION_STATUS.md` under `P3-02 — External Repository Intake Needs Canonical Governance` and `Phase 4 / Workstream 4.1`.

## Governance Artifacts

This plan is supported by the following canonical artifacts:

- [Repo Watchlist](docs/external-intake/repo-watchlist.md)
- [License Matrix](docs/external-intake/license-matrix.md)
- [Repo Audit Template](docs/external-intake/repo-audit-template.md)
- [Feature Adoption Matrix](docs/external-intake/urpg_feature_adoption_matrix.md)

Cross-cutting private-use asset intake artifacts:

- [Asset Source Registry](docs/asset_intake/ASSET_SOURCE_REGISTRY.md)
- [Asset Promotion Guide](docs/asset_intake/ASSET_PROMOTION_GUIDE.md)
- [Asset Category Gaps](docs/asset_intake/ASSET_CATEGORY_GAPS.md)
- Staging root: [`imports/staging/asset_intake/`](imports/staging/asset_intake/)
- Normalized root: [`imports/normalized/`](imports/normalized/)
- Manifest roots: [`imports/manifests/`](imports/manifests/)
- Report root: [`imports/reports/`](imports/reports/)
- Direct-ingest asset mirrors: [`third_party/github_assets/`](third_party/github_assets/)

---

## Intake Set

### Tier A — High Value / Directly Actionable
1. `triacontane/RPGMakerMV`
2. `EasyRPG/Tools`
3. `erri120/rpgmpacker`
4. `EasyRPG/liblcf`

### Tier B — Important but Narrower / Conditional
5. `davide97l/rpgmaker-mv-translator`
6. `EasyRPG/RTP`
7. `EasyRPG/TestGame`
8. `mjshi/RPGMakerRepo`
9. `biud436/MV`
10. `DrillUp/drill_plugins`

### Tier C — Reference / Inspiration / Quarantined Use
11. `SnowSzn/rgss-script-editor`
12. `AsPJT/AsLib`

---

## Core Rules

### Rule 1 — Do not absorb architecture blindly
External code is reference first. Nothing enters runtime ownership until it passes:

- license review
- quality review
- determinism review
- dependency review
- portability review
- maintenance review

### Rule 2 — Separate reference, fixture, and production code
Every repo must land in exactly one or more of these buckets:

- **Reference**: read and mine for ideas; do not ship
- **Fixture corpus**: use as test/plugin/import sample data
- **Production candidate**: eligible for wrapped integration after review
- **Asset/reference pack**: usable for internal testing or optional content lanes

### Rule 3 — Preserve URPG invariants
Nothing from external repos may weaken:

- deterministic runtime behavior
- source-of-truth contracts
- schema versioning discipline
- license clarity
- sandbox/permission boundaries
- CI reproducibility

### Rule 4 — Prefer adapters over direct embedding
If a repo is useful, wrap it behind URPG-owned interfaces. Do not let third-party assumptions leak into engine core.

---

## Repository Role Matrix

| Repo | Primary Role | Secondary Role | Integration Stance |
|---|---|---|---|
| triacontane/RPGMakerMV | Compat fixture corpus | Feature-surface discovery | Mine + ingest fixtures, do not absorb wholesale |
| EasyRPG/Tools | Import/conversion ideas | Preview/inspection tooling | Reimplement concepts or invoke tools in isolated workflows |
| erri120/rpgmpacker | Export/build automation reference | Deployment validation | Reuse ideas aggressively; evaluate partial direct use |
| EasyRPG/liblcf | Data import foundation | Legacy format parsing | Strong production candidate behind adapters |
| davide97l/rpgmaker-mv-translator | Text extraction / localization bootstrap | Dialog segmentation heuristics | Mine concepts; do not depend on online translator flow |
| EasyRPG/RTP | Retro-compatible reference assets | Asset pipeline test material | Use only in clearly separated asset/reference lane |
| EasyRPG/TestGame | Regression corpus | Import/behavior validation | Ingest as test fixture project |
| mjshi/RPGMakerRepo | Compat fixture corpus | Plugin behavior discovery | Mine and sample selectively |
| biud436/MV | Compat fixture corpus | UI/battle/plugin stress cases | Mine and sample selectively |
| DrillUp/drill_plugins | Compat fixture corpus | Large plugin edge-case coverage | Mine and sample selectively |
| SnowSzn/rgss-script-editor | Editor workflow reference | Script extraction UX ideas | Read only; no direct integration |
| AsPJT/AsLib | Map-authoring/editor UX reference | Procedural terrain inspiration | Read only; prototype ideas internally |

---

## License and Legal Intake Gate

Before any technical work begins, create a repo-by-repo compliance sheet with the following fields:

- repo URL
- owner / org
- current license
- whether license is permissive, reciprocal, asset-specific, or ambiguous
- whether redistribution is allowed
- whether commercial use is allowed
- whether attribution is required
- whether source code and assets have different licenses
- whether code is acceptable for vendoring, wrapping, or reference only

### Preliminary expected treatment

| Repo | Expected License Treatment | Planned Policy |
|---|---|---|
| triacontane/RPGMakerMV | MIT for author-created plugins, verify per file | Accept as fixture corpus; copy only clearly licensed pieces |
| EasyRPG/Tools | Review repo license before code reuse | Prefer idea extraction or isolated utility use |
| erri120/rpgmpacker | MIT-friendly candidate | Strong candidate for reference or partial tooling integration |
| EasyRPG/liblcf | MIT | Production candidate behind clean URPG facade |
| davide97l/rpgmaker-mv-translator | Verify repo license and API constraints | Reference concepts only unless legal and offline path are acceptable |
| EasyRPG/RTP | CC-BY asset lane | Separate attribution and asset manifests required |
| EasyRPG/TestGame | Verify project license | Fixture/test only |
| mjshi/RPGMakerRepo | Credit-based custom terms | Fixture/reference only until exact license handling is documented |
| biud436/MV | Verify repo license | Fixture/reference only until cleared |
| DrillUp/drill_plugins | Verify repo license and per-plugin terms | Fixture/reference only until cleared |
| SnowSzn/rgss-script-editor | GPL-family expectation | Read only; no code absorption |
| AsPJT/AsLib | Verify repo license | Read only until cleared |

### Deliverable
[`docs/external-intake/license-matrix.md`](docs/external-intake/license-matrix.md)

Acceptance criteria:
- every repo has a recorded legal disposition
- every repo is explicitly tagged as `reference_only`, `fixture_only`, `production_candidate`, or `asset_reference`
- no code is copied into URPG before this matrix is approved

---

## Proposed Folder Layout in URPG

```text
third_party/
  external-repos/
    triacontane-rpgmakermv/
    easyrpg-tools/
    rpgmpacker/
    liblcf/
    rpgmaker-mv-translator/
    easyrpg-rtp/
    easyrpg-testgame/
    mjshi-rpgmakerrepo/
    biud436-mv/
    drillup-drill-plugins/
    rgss-script-editor/
    aslib/
  github_assets/
    intersect-assets/
    gdquest-game-sprites/
    kenney-interface-sounds/

imports/
  fixtures/
    compat/
    legacy-projects/
    localization/
    exporter/
    assets/
  extracted/
    plugin-metadata/
    command-signatures/
    text-corpora/
    map-previews/
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

reports/
  external-repo-audit/
    license/
    code-quality/
    dependency/
    fixture-catalog/
    recommendations/

docs/
  external-intake/
    repo-watchlist.md
    license-matrix.md
    repo-audit-template.md
    urpg_feature_adoption_matrix.md
    fixture-corpus-plan.md
    implementation-notes/
  asset_intake/
    ASSET_SOURCE_REGISTRY.md
    ASSET_PROMOTION_GUIDE.md
    ASSET_CATEGORY_GAPS.md
```

---

## Execution Phases

# Phase 0 — Intake Governance and Inventory

### Goals
- lock repo list
- record legal posture
- define intake folders
- define audit templates
- prevent random copy-paste ingestion

### Tasks
1. Create [`docs/external-intake/repo-watchlist.md`](docs/external-intake/repo-watchlist.md)
2. Create [`docs/external-intake/license-matrix.md`](docs/external-intake/license-matrix.md)
3. Create [`docs/external-intake/repo-audit-template.md`](docs/external-intake/repo-audit-template.md)
4. Create `third_party/external-repos/` structure
5. Create `.gitignore`/LFS rules for heavy fixture and asset lanes
6. Define asset attribution manifest schema
7. Define plugin fixture metadata schema
8. Define reference-notes template for mined ideas

### Deliverables
- repo watchlist
- legal matrix
- intake folder scaffold
- audit template pack

### Acceptance criteria
- all 12 repos listed
- each repo assigned an owner, status, and next action
- no untracked manual downloads sitting in the repo

---

# Phase 1 — Shallow Clone / Snapshot / Metadata Audit

### Goals
- get a frozen snapshot of each repo
- extract metadata without yet integrating anything

### Tasks for every repo
1. record default branch, latest commit, tags/releases
2. capture README and license
3. count files and rough language composition
4. record maintenance health:
   - last update date
   - issue count trend
   - active/inactive status
5. record dependencies
6. record platform assumptions
7. flag whether repo is code, assets, data, sample project, or utility

### Output reports
- `reports/external-repo-audit/<repo>/summary.json`
- `reports/external-repo-audit/<repo>/dependency-notes.md`
- `reports/external-repo-audit/<repo>/usefulness-score.md`

### Acceptance criteria
- every repo has a metadata snapshot
- every repo has a preliminary usefulness score
- every repo has a risk level

---

# Phase 2 — Repo-Specific Deep Audit

## 2A. triacontane/RPGMakerMV

### Why it matters
This is the highest-value compat corpus. It spans many plugin categories and likely covers a huge percentage of real-world MV/MZ extension behavior.

### Objectives
- harvest plugin headers and parameters
- catalog command registrations and plugin dependencies
- identify top 50 plugins that stress Window, Battle, Message, Menu, Data, Audio, Input, and Save systems
- convert selected plugins into URPG compat fixtures

### Tasks
1. crawl all plugin files
2. parse metadata headers
3. build plugin capability index:
   - UI/window
   - battle
   - message/text
   - menu
   - map
   - save/data
   - audio
   - input
   - rendering/effects
4. extract plugin commands and parameters
5. flag likely hard cases:
   - direct prototype patching
   - timing overrides
   - bitmap/text rendering assumptions
   - savefile mutations
6. select first fixture cohort:
   - 10 easy
   - 10 medium
   - 10 hard
7. generate per-plugin execution notes for compat lane

### Deliverables
- `imports/extracted/plugin-metadata/triacontane_index.json`
- `docs/external-intake/triacontane_fixture_selection.md`
- `tests/compat/fixtures/triacontane/`

### Acceptance criteria
- at least 30 plugins classified
- at least 10 turned into concrete compat regression fixtures
- at least 3 new URPG compat gaps identified and tracked

## 2B. EasyRPG/Tools

### Why it matters
Provides real conversion and preview tool ideas for legacy formats.

### Objectives
- understand tool surface
- identify preview/conversion workflows worth re-creating in URPG

### Tasks
1. catalog each tool and supported file types
2. test how LMU-to-image preview works conceptually
3. examine file conversion assumptions
4. note reusable UX ideas:
   - batch conversion
   - one-shot inspection
   - preview generation
5. decide whether to wrap any tool externally or reimplement internally

### Deliverables
- `docs/external-intake/easyrpg_tools_mapping.md`
- preview/conversion idea backlog entries

### Acceptance criteria
- at least 3 concrete tool workflows mapped to URPG features

## 2C. erri120/rpgmpacker

### Why it matters
Probably the cleanest exporter/deployer reference in the set.

### Objectives
- mine deployment logic, especially exclude-unused behavior and multi-platform packaging assumptions
- map useful pieces into URPG export packager and CI

### Tasks
1. audit CLI UX
2. audit platform matrix behavior
3. study unused-file exclusion logic
4. study encryption/obfuscation handling
5. study hardlink optimization workflow
6. map to URPG export packager contract
7. identify reimplementation candidates vs direct utility use

### Deliverables
- `docs/external-intake/rpgmpacker_gap_map.md`
- `docs/export/urpg_exporter_feature_alignment.md`

### Acceptance criteria
- explicit list of exporter features to adopt, ignore, or redesign
- at least 1 concrete exporter improvement ticket created for URPG

## 2D. EasyRPG/liblcf

### Why it matters
Strongest actual code-level import foundation in the list.

### Objectives
- evaluate as a wrapped import backend for RM2K/RM2K3 legacy support
- isolate what parts are safe and valuable for URPG

### Tasks
1. audit supported formats
2. build proof-of-concept parser wrapper behind a URPG-owned interface
3. define import boundary:
   - read-only parse
   - conversion to URPG canonical model
   - no format leakage into runtime
4. test text extraction, map structure extraction, database extraction
5. record encoding and conversion edge cases
6. design `LegacyProjectImporter` facade in URPG

### Deliverables
- `engine/import/legacy/legacy_project_importer.*`
- spike branch or design document for wrapped liblcf usage
- format mapping table

### Acceptance criteria
- successful parse of at least 1 sample legacy project
- zero liblcf types leaked into URPG high-level engine contracts

## 2E. davide97l/rpgmaker-mv-translator

### Why it matters
Useful for file coverage and dialogue extraction heuristics, not for its online translation dependency.

### Objectives
- mine JSON coverage map
- harvest text segmentation strategy
- ignore network-translation dependency for core URPG design

### Tasks
1. inventory covered JSON files
2. document text-bearing fields
3. inspect line-wrapping / print-neatly strategy
4. define URPG text extraction pipeline for:
   - object data
   - common events
   - map events
5. map extracted text into localization key workflow

### Deliverables
- `docs/localization/rpgmaker_text_extraction_map.md`
- `tools/localization/extract_rpgmaker_text_spike.*`

### Acceptance criteria
- URPG gets a file-coverage map for MV/MZ text extraction
- no dependency on online translation services is introduced into core plan

## 2F. EasyRPG/RTP

### Why it matters
Real assets, but old-style and attribution-bound.

### Objectives
- treat as reference/test asset bundle
- keep asset and license bookkeeping clean

### Tasks
1. inventory asset categories:
   - graphics
   - music
   - sound
2. define attribution manifest format
3. test asset hygiene and dedupe pipeline on this pack
4. tag usable subsets for internal preview/testing
5. explicitly separate from URPG visual identity assets

### Deliverables
- `imports/fixtures/assets/easyrpg_rtp_manifest.json`
- attribution records
- sample asset browser tags

### Acceptance criteria
- asset pack is cataloged and attributed
- no accidental commingling with first-party visual identity assets

## 2G. EasyRPG/TestGame

### Why it matters
It is a structured regression/test project rather than arbitrary game junk.

### Objectives
- use as importer and behavior validation project

### Tasks
1. ingest as legacy test corpus
2. parse project structure
3. record edge cases it covers
4. plug it into importer regression suite

### Deliverables
- `tests/fixtures/legacy/easyrpg_testgame/`
- `docs/external-intake/testgame_coverage.md`

### Acceptance criteria
- at least 1 importer regression run uses this project

## 2H. mjshi / biud436 / DrillUp plugin corpora

### Why they matter
They broaden compat coverage beyond one author/style.

### Objectives
- expand plugin surface corpus
- improve confidence that URPG compat isn’t overfit to one plugin author

### Tasks
1. sample each corpus
2. tag plugin categories
3. identify overlap vs unique coverage relative to triacontane
4. select high-divergence fixtures only
5. build cross-corpus compat scorecard

### Deliverables
- `docs/external-intake/plugin_corpus_comparison.md`
- `tests/compat/fixtures/community/`

### Acceptance criteria
- at least 5 unique non-triacontane stress cases captured

## 2I. SnowSzn/rgss-script-editor

### Why it matters
Interesting editor/script UX reference, but quarantined by license and scope.

### Objectives
- mine workflow ideas only

### Tasks
1. document extraction-to-file workflow
2. document load-order management approach
3. document run/debug ergonomics
4. translate relevant concepts into URPG editor workflow tickets

### Deliverables
- `docs/editor/external_script_workflow_notes.md`

### Acceptance criteria
- at least 3 editor UX ideas captured without code reuse

## 2J. AsPJT/AsLib

### Why it matters
Potentially useful for map authoring and procedural/editor inspiration.

### Objectives
- inspect map paint/procedural affordances
- convert interesting ideas into URPG-owned editor concepts

### Tasks
1. document tool affordances
2. map useful ideas into elevation/prop/map authoring backlog
3. ignore any architecture that does not fit URPG

### Deliverables
- `docs/editor/map_authoring_reference_notes.md`

### Acceptance criteria
- at least 2 concrete editor backlog items created

---

# Phase 3 — Fixture Corpus Program

### Goal
Turn external repos into structured test inputs instead of vague bookmarks.

### Fixture Buckets
- compat plugin fixtures
- importer sample projects
- localization text corpora
- exporter validation inputs
- asset/reference packs

### Tasks
1. define fixture manifest schema
2. define fixture ownership tags
3. define refresh/update process
4. define CI-safe small fixture subset
5. define large/manual fixture subset
6. build fixture catalog generator

### Deliverables
- `tools/fixtures/generate_fixture_catalog.*`
- `imports/fixtures/catalog.json`

### Acceptance criteria
- every external intake item that matters has a fixture path or an explicit reason it does not

---

# Phase 4 — URPG Feature Mapping

### Goal
Convert raw repo intake into product changes.

### Feature Buckets
1. compat layer
2. importer/migration
3. localization tooling
4. exporter/packager
5. asset browser / asset hygiene
6. editor workflow

### Required output format for each bucket
- adopted idea
- source repo
- why it matters
- implementation complexity
- runtime risk
- license risk
- whether it is direct integration, wrapper, or clean-room reimplementation

### Deliverable
[`docs/external-intake/urpg_feature_adoption_matrix.md`](docs/external-intake/urpg_feature_adoption_matrix.md)

### Acceptance criteria
- no repo remains “interesting” without a concrete adopt/defer/ignore decision

---

# Phase 5 — Implementation Program

## Workstream A — Compat Expansion
**Source repos**: triacontane, mjshi, biud436, DrillUp

### Tasks
1. extend plugin validator to ingest external metadata
2. generate plugin command/parameter signatures
3. build fixture execution harness for selected plugins
4. expand compat report categories based on real plugin behavior
5. add window/message/menu/battle/save stress suites

### Acceptance criteria
- at least 25 new compat fixtures running
- at least 10 new known gaps surfaced or closed

## Workstream B — Legacy Import Foundation
**Source repos**: liblcf, EasyRPG/Tools, TestGame

### Tasks
1. create legacy import facade
2. parse sample RM2K/RM2K3 projects
3. generate previews / metadata reports
4. convert selected data into URPG canonical structures
5. document unsupported cases

### Acceptance criteria
- 1 end-to-end legacy project parse report
- importer produces useful structured outputs even before full conversion support

## Workstream C — Export Pipeline Upgrade
**Source repo**: rpgmpacker

### Tasks
1. map packager feature parity targets
2. prototype unused-file exclusion scan
3. prototype hardlink-based output optimization for local builds
4. document platform-specific deployment assumptions

### Acceptance criteria
- at least 1 exporter feature lands in URPG or is formally deferred with reason

## Workstream D — Localization Bootstrap
**Source repo**: rpgmaker-mv-translator

### Tasks
1. define canonical text extraction coverage
2. build file scanners for events/maps/object data
3. map extracted strings to locale keys
4. add writer-review workflow instead of blind auto-translation

### Acceptance criteria
- text extraction prototype runs on at least one sample project

## Workstream E — Asset / Reference Pack Intake
**Source repo**: EasyRPG/RTP

### Tasks
1. ingest into asset hygiene pipeline
2. build attribution report
3. tag assets for browser categories
4. test duplicate detection, hashing, and manifest generation

### Acceptance criteria
- asset pack is cleanly indexed and attributable
- no license ambiguity remains for internal use of this pack

## Workstream F — Editor UX Reference Mining
**Source repos**: SnowSzn/rgss-script-editor, AsLib

### Tasks
1. write reference notes
2. create backlog tickets for script extraction, load-order visualization, map authoring affordances
3. explicitly reject any non-fitting architecture

### Acceptance criteria
- 5+ concrete editor UX backlog tickets created from these references

---

## Repo-by-Repo Final Disposition Targets

| Repo | Target End State |
|---|---|
| triacontane/RPGMakerMV | Parsed, indexed, sampled, and partially fixture-ingested |
| EasyRPG/Tools | Tool ideas mapped; selected workflows reimplemented or wrapped |
| erri120/rpgmpacker | Exporter alignment doc complete; selective feature adoption planned |
| EasyRPG/liblcf | Wrapped spike completed; legacy importer path defined |
| davide97l/rpgmaker-mv-translator | Text extraction map completed; translation dependency excluded from core |
| EasyRPG/RTP | Cataloged, attributed, indexed as test/reference asset pack |
| EasyRPG/TestGame | Ingested as regression fixture |
| mjshi/RPGMakerRepo | Sampled and mined for fixture diversity |
| biud436/MV | Sampled and mined for fixture diversity |
| DrillUp/drill_plugins | Sampled and mined for fixture diversity |
| SnowSzn/rgss-script-editor | Reference notes only |
| AsPJT/AsLib | Reference notes only |

---

## Prioritization Order

### First 4 to execute
1. `EasyRPG/liblcf`
2. `triacontane/RPGMakerMV`
3. `erri120/rpgmpacker`
4. `EasyRPG/Tools`

### Next 4
5. `EasyRPG/TestGame`
6. `davide97l/rpgmaker-mv-translator`
7. `biud436/MV`
8. `DrillUp/drill_plugins`

### Last 4
9. `EasyRPG/RTP`
10. `mjshi/RPGMakerRepo`
11. `SnowSzn/rgss-script-editor`
12. `AsPJT/AsLib`

Rationale:
- front-load code and data leverage
- then expand test coverage
- then handle optional asset/reference and workflow inspiration lanes

---

## Suggested Milestones

### Milestone M1 — Governance Locked
- legal matrix done
- watchlist done
- repo folders scaffolded
- audit templates done

### Milestone M2 — High-Value Intake Complete
- liblcf audited and spiked
- triacontane indexed
- rpgmpacker gap map completed
- EasyRPG Tools mapped

### Milestone M3 — Fixture Program Running
- plugin fixture catalog generated
- legacy project fixtures ingested
- localization extraction test corpus ready

### Milestone M4 — Product Adoption Decisions Complete
- adoption matrix finalized
- implementation tickets opened
- non-fitting repos explicitly deferred or quarantined

### Milestone M5 — First Shipping Value
- compat suite expanded
- legacy import path visibly improved
- exporter backlog strengthened
- asset/reference pack indexed

---

## Risks

### License risk
Mitigation:
- nothing copied before legal classification
- custom/credit licenses kept fixture-only unless clarified

### Architecture contamination risk
Mitigation:
- wrap behind URPG facades
- prefer clean-room reimplementation for core behavior

### Scope explosion risk
Mitigation:
- enforce adopt/defer/ignore outcome for every repo
- reject “interesting but vague” work

### Fixture bloat risk
Mitigation:
- small CI subset + large manual subset
- hash manifests and refresh tooling

### False confidence risk
Mitigation:
- more plugin repos does not equal real compatibility
- prioritize diverse and difficult cases, not just volume

---

## Immediate Next 20 Tasks

1. Create `docs/external-intake/repo-watchlist.md`
2. Create `docs/external-intake/license-matrix.md`
3. Create `docs/external-intake/repo-audit-template.md`
4. Scaffold `third_party/external-repos/`
5. Scaffold `imports/fixtures/compat/`
6. Scaffold `imports/fixtures/legacy-projects/`
7. Scaffold `imports/fixtures/localization/`
8. Scaffold `imports/fixtures/assets/`
9. Add attribution manifest schema
10. Add fixture manifest schema
11. Snapshot `triacontane/RPGMakerMV`
12. Snapshot `EasyRPG/liblcf`
13. Snapshot `EasyRPG/Tools`
14. Snapshot `erri120/rpgmpacker`
15. Write preliminary usefulness/risk scores for all 12
16. Start triacontane plugin metadata crawler
17. Start liblcf wrapper spike design doc
18. Start exporter gap-map against rpgmpacker
19. Start text extraction coverage map from rpgmaker-mv-translator
20. Ingest EasyRPG/TestGame as a legacy fixture target

---

## Definition of Done
This intake program is done when:

- all 12 repos have explicit legal and technical dispositions
- useful repos are turned into structured fixtures, adapters, or adoption docs
- non-useful repos are clearly rejected and documented
- at least one meaningful improvement lands in each of these URPG lanes:
  - compat
  - importer
  - exporter or localization
- no repo remains in a vague “maybe useful later” state without a recorded decision

---

## Bottom Line
The only way this pays off is if these repos become one of the following:

- **test corpus**
- **wrapped import/export dependency**
- **reference notes that become concrete URPG backlog items**
- **cleanly attributed asset/reference packs**

Anything else is repo-hoarding.
