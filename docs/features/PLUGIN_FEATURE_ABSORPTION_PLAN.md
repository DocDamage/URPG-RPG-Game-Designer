# Plugin Feature Absorption Plan

Status date: 2026-04-29

URPG should absorb the submitted RPG Maker plugin behavior as native maker features, not as bundled JavaScript plugins. The raw local drop is kept outside the repo root at `imports/raw/plugin_feature_intake/` and is ignored because it contains source-plugin material and a large ZIP. This document records the duplicate check and the implementation lanes.

## Duplicate Check

| Source | Requested behavior | Existing URPG surface | Decision |
| --- | --- | --- | --- |
| `features needing to be integrated.txt` / AnimatedBattleBackground | Animated battle background and foreground videos with map/region overrides | `BattlePresentationProfile`, `BattleScene`, export asset validation | Extend battle presentation media tracks; do not create a second battleback system. |
| `features needing to be integrated.txt` / VideoSceneBackground | Looping video backgrounds for title/menu/shop/options/status scenes | `RuntimeTitleScene`, title/save-screen builder fixtures, editor presentation panels | Extend scene presentation profiles and title/menu preview surfaces. |
| `features needing to be integrated.txt` / EdgeScroll | RTS-style edge camera pan, keyboard pan, click-follow, recenter | `CameraSystem`, `CameraFollowSystem`, map/spatial panels | Extend native camera profiles with edge-scroll controls. |
| `features needing to be integrated.txt` / MagicParallax | Unlimited parallax layers, masks, blend modes, opacity, scrolling | `parallax_mapping_editor_fixture`, presentation runtime, lighting/weather preview | Extend native map presentation layers; avoid a plugin-only parallax stack. |
| `features needing to be integrated.txt` / Fast Travel | Visual waypoint menu with preview, unlock conditions, fallback common events | World exploration template already claims fast travel conceptually | Implemented as native fast-travel destination data and deterministic preview diagnostics in this pass. |
| `features needing to be integrated.txt` / Smoke | Procedural region-driven smoke particles | `PresentationRuntime`, weather/lighting systems | Add region particle emitters to map environment preview/runtime. |
| `features needing to be integrated.txt` / Monster Capture | Capture item, enemy capture metadata, captured actor added, enemy removed, no exp/gold/drop reward | `MonsterCollectionDocument`, `MonsterCollectionPanel` | Implemented as native item-gated capture extension in this pass. |
| `UM7_AdvancedTerrain.js` | Region-based terrain mesh heights, curvature, collision regions | `TerrainBrush`, map environment preview, tactical terrain fixture | Extend map terrain rules instead of adding UltraMode7 JS dependency. |
| `BattleLightning.js` | Battler glow, pulse/flicker, night tint, notetag overrides | `LightingSystem`, `BattlePresentationProfile`, battle VFX timeline | Extend battle presentation lighting cues. |
| `BloodSplatterEffect.js` | Blood/death/critical animations, SFX, options toggle | `BattleVfxTimeline`, effect cues, accessibility/settings | Extend battle VFX cue taxonomy and options. |
| `VehicleEffects.js` | Vehicle BGM/BGS, speed, encounters, interiors, transitions | `mount_vehicle_system_fixture`, audio resolver, map transfer systems | Implemented as native vehicle profile preview data in this pass. |
| `randombgms.js` | Random map/battle BGM and boss overrides | `StateDrivenAudioResolver`, audio inspector/mix panels | Extend state-driven audio with weighted/random pools. |
| `SRG_Drag.js` | Drag map events tagged as draggable | Event authoring/runtime and input systems | Implemented as native draggable-event interaction metadata in this pass. |
| `LoopingTitleVideo.js` | Title splash videos, logos, social links, skip rules | `RuntimeTitleScene`, title/save-screen builder fixture | Extend title presentation profile. |
| `Synrec_LevelStats.js` | Level-up stat allocation UI and stat images | `ClassProgression`, character/progression panels | Add native stat allocation model and panel, not plugin UI. |
| `ExtendedPictureManager.js`, `TSk_Picture_Tasks.js` | More picture slots and common-event picture tasks | `pictures_ui_creator_fixture`, event runtime | Add native picture task bindings. |
| TSk variable/switch files | JS, map, scoped, self variables/switches | `GlobalStateHub`, switch-variable inspector fixture, compat state | Add native scoped/self/map variable banks and migration adapters. |
| TSk text fixes | Nested escapes and canvas text alignment fixes | `message_core`, UI text rendering | Add message escape recursion tests and text alignment guards where missing. |
| TSk battle helpers | chip damage/healing, zero damage as miss/evasion, custom buff levels, troop positions | `BattleRuleResolver`, battle presentation, troop fixtures | Extend battle rules and presentation policies. |
| LootTableDesigner | Visual loot table editor | `LootGeneratorDocument`, `LootGeneratorPanel` | Existing native feature; improve UX/import only. |

## Implementation Order

1. Monster capture native completion: item-gated capture, actor IDs, battle-removal and reward-suppression result. Completed in this pass.
2. Random BGM native completion: deterministic weighted track pools on `StateDrivenAudioResolver`. Completed in this pass.
3. Battle presentation media lane: animated battlebacks and battler light cues on `BattlePresentationProfile`. Completed in this pass.
4. Map presentation lane: parallax layers, region smoke emitters, terrain height/curve/walkability rules, and edge-scroll camera profiles. Completed in this pass.
5. Battle feedback lane: blood VFX cue kind completed in this pass; chip damage/healing, custom buff levels, zero-damage presentation policies, and troop-position reuse remain.
6. World/system lane: fast travel, vehicle profiles/interiors/transitions, and draggable event metadata completed in this pass.
7. State/message/picture lane: scoped/self/map/JS variable banks, nested text escapes, picture tasks, and high-count picture management.
8. Progression lane: level-up stat allocation model and editor panel.

## Acceptance Bar

Each absorbed feature needs saved project data, schema coverage when data-driven, runtime behavior, editor preview/state, diagnostics for invalid configuration, tests, and release/export validation where it touches packaged assets.
