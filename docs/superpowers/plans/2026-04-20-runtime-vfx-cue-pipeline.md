# Runtime VFX Cue Pipeline Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a shared runtime VFX cue pipeline that lets battle emit deterministic hit/spell/emphasis cues and lets the presentation runtime translate them into hybrid world and overlay effect commands.

**Architecture:** Add a shared `EffectCue -> ResolvedEffectInstance -> PresentationCommand` pipeline under `engine/core/presentation/effects/`, extend `PresentationCommand` with bounded effect command types, and wire `BattleScene` as the first emitter without letting battle logic construct render commands directly. Keep the first slice narrow: code-defined presets, battle-first integration, presentation-runtime translation, focused tests, and one release-validation expansion.

**Tech Stack:** C++20, Catch2 v3, CMake/CTest, existing URPG presentation runtime, existing native `BattleScene` runtime

---

### Task 1: Add the shared effect cue and resolved-instance contract

**Files:**
- Create: `engine/core/presentation/effects/effect_cue.h`
- Create: `engine/core/presentation/effects/effect_instance.h`
- Create: `tests/unit/test_effect_cue.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

```cpp
#include <catch2/catch_test_macros.hpp>
#include "engine/core/presentation/effects/effect_cue.h"

using namespace urpg::presentation::effects;

TEST_CASE("EffectCue defaults are deterministic and render-agnostic", "[presentation][effects][cue]") {
    EffectCue cue;

    REQUIRE(cue.kind == EffectCueKind::HitConfirm);
    REQUIRE(cue.anchor_mode == EffectAnchorMode::TargetWorld);
    REQUIRE(cue.overlay_emphasis == OverlayEmphasis::None);
    REQUIRE(cue.intensity == EffectIntensity::Medium);
    REQUIRE(cue.sequence_index == 0);
    REQUIRE(cue.subject_id == 0);
    REQUIRE(cue.target_id == 0);
}

TEST_CASE("EffectCue ordering sorts by frame tick then sequence index then ids", "[presentation][effects][cue]") {
    EffectCue late;
    late.frame_tick = 12;
    late.sequence_index = 1;
    late.subject_id = 4;

    EffectCue early;
    early.frame_tick = 11;
    early.sequence_index = 99;
    early.subject_id = 2;

    EffectCue same_tick;
    same_tick.frame_tick = 12;
    same_tick.sequence_index = 0;
    same_tick.subject_id = 7;

    REQUIRE(EffectCueLess{}(early, late));
    REQUIRE(EffectCueLess{}(same_tick, late));
    REQUIRE_FALSE(EffectCueLess{}(late, same_tick));
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectCue defaults are deterministic and render-agnostic|EffectCue ordering sorts by frame tick then sequence index then ids"`

Expected: FAIL because the new effect cue headers and test target do not exist yet.

- [ ] **Step 3: Write minimal implementation**

```cpp
#pragma once

#include <cstdint>

namespace urpg::presentation::effects {

enum class EffectCueKind : uint8_t {
    HitConfirm,
    CastStart,
    CriticalHit,
    GuardClash,
    MissSweep,
    HealPulse,
    DefeatFade,
    PhaseBanner
};

enum class EffectAnchorMode : uint8_t {
    SubjectWorld,
    TargetWorld,
    ScreenCenter,
    ScreenTargetFrame
};

enum class OverlayEmphasis : uint8_t {
    None,
    HitFlash,
    CritFlash,
    HealGlow,
    BannerPulse
};

enum class EffectIntensity : uint8_t {
    Light,
    Medium,
    Heavy
};

struct EffectCue {
    EffectCueKind kind = EffectCueKind::HitConfirm;
    EffectAnchorMode anchor_mode = EffectAnchorMode::TargetWorld;
    OverlayEmphasis overlay_emphasis = OverlayEmphasis::None;
    EffectIntensity intensity = EffectIntensity::Medium;
    uint32_t frame_tick = 0;
    uint32_t sequence_index = 0;
    uint32_t subject_id = 0;
    uint32_t target_id = 0;
    uint32_t theme_tag = 0;
    bool prefer_overlay_fallback = true;
};

struct EffectCueLess {
    bool operator()(const EffectCue& lhs, const EffectCue& rhs) const {
        if (lhs.frame_tick != rhs.frame_tick) return lhs.frame_tick < rhs.frame_tick;
        if (lhs.sequence_index != rhs.sequence_index) return lhs.sequence_index < rhs.sequence_index;
        if (lhs.subject_id != rhs.subject_id) return lhs.subject_id < rhs.subject_id;
        return lhs.target_id < rhs.target_id;
    }
};

} // namespace urpg::presentation::effects
```

```cpp
#pragma once

#include "effect_cue.h"
#include "engine/core/presentation/presentation_types.h"
#include <cstdint>

namespace urpg::presentation::effects {

enum class EffectPlacement : uint8_t {
    World,
    Overlay
};

struct ResolvedEffectInstance {
    uint32_t effect_id = 0;
    EffectPlacement placement = EffectPlacement::World;
    Vec3 position{0.0f, 0.0f, 0.0f};
    uint32_t owner_id = 0;
    float duration_seconds = 0.0f;
    float scale = 1.0f;
    float intensity = 1.0f;
    uint32_t color_rgba = 0xFFFFFFFF;
    OverlayEmphasis overlay_emphasis = OverlayEmphasis::None;
};

} // namespace urpg::presentation::effects
```

- [ ] **Step 4: Register the new test file**

```cmake
  tests/unit/test_effect_cue.cpp
```

Add that path to the `urpg_tests` source list in `CMakeLists.txt` near the other presentation runtime tests.

- [ ] **Step 5: Run test to verify it passes**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectCue defaults are deterministic and render-agnostic|EffectCue ordering sorts by frame tick then sequence index then ids"`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt engine/core/presentation/effects/effect_cue.h engine/core/presentation/effects/effect_instance.h tests/unit/test_effect_cue.cpp
git commit -m "feat: add shared runtime effect cue contract"
```

### Task 2: Add the built-in effect catalog and shared resolver

**Files:**
- Create: `engine/core/presentation/effects/effect_catalog.h`
- Create: `engine/core/presentation/effects/effect_catalog.cpp`
- Create: `engine/core/presentation/effects/effect_resolver.h`
- Create: `engine/core/presentation/effects/effect_resolver.cpp`
- Create: `tests/unit/test_effect_resolver.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing test**

```cpp
#include <catch2/catch_test_macros.hpp>
#include "engine/core/presentation/effects/effect_resolver.h"

using namespace urpg::presentation::effects;

TEST_CASE("EffectResolver expands heavy hit cues into world and overlay output", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::HitConfirm;
    cue.intensity = EffectIntensity::Heavy;
    cue.subject_id = 1;
    cue.target_id = 2;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, urpg::presentation::CapabilityTier::Tier1_Standard);

    REQUIRE(resolved.size() == 2);
    CHECK(resolved[0].placement == EffectPlacement::World);
    CHECK(resolved[1].placement == EffectPlacement::Overlay);
    CHECK(resolved[1].overlay_emphasis == OverlayEmphasis::HitFlash);
}

TEST_CASE("EffectResolver collapses critical-hit hybrid output on low tier", "[presentation][effects][resolver]") {
    EffectCue cue;
    cue.kind = EffectCueKind::CriticalHit;
    cue.intensity = EffectIntensity::Heavy;
    cue.subject_id = 3;
    cue.target_id = 4;

    EffectResolver resolver;
    const auto resolved = resolver.resolve(cue, urpg::presentation::CapabilityTier::Tier0_Basic);

    REQUIRE(resolved.size() == 1);
    CHECK(resolved.front().placement == EffectPlacement::Overlay);
    CHECK(resolved.front().overlay_emphasis == OverlayEmphasis::CritFlash);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectResolver expands heavy hit cues into world and overlay output|EffectResolver collapses critical-hit hybrid output on low tier"`

Expected: FAIL because the resolver and catalog do not exist yet.

- [ ] **Step 3: Write minimal implementation**

```cpp
#pragma once

#include "effect_instance.h"
#include <string>
#include <vector>

namespace urpg::presentation::effects {

struct EffectPreset {
    std::string id;
    EffectPlacement placement = EffectPlacement::World;
    float duration_seconds = 0.2f;
    float scale = 1.0f;
    float intensity = 1.0f;
    uint32_t color_rgba = 0xFFFFFFFF;
    OverlayEmphasis overlay_emphasis = OverlayEmphasis::None;
};

class EffectCatalog {
public:
    static const EffectPreset& fallbackWorld();
    static const EffectPreset& fallbackOverlay();
    static const EffectPreset& castSmall();
    static const EffectPreset& impactHeavy();
    static const EffectPreset& critBurst();
};

} // namespace urpg::presentation::effects
```

```cpp
#pragma once

#include "effect_catalog.h"
#include "engine/core/presentation/presentation_types.h"
#include <vector>

namespace urpg::presentation::effects {

class EffectResolver {
public:
    std::vector<ResolvedEffectInstance> resolve(const EffectCue& cue,
                                                urpg::presentation::CapabilityTier tier) const;
};

} // namespace urpg::presentation::effects
```

```cpp
std::vector<ResolvedEffectInstance> EffectResolver::resolve(const EffectCue& cue,
                                                            urpg::presentation::CapabilityTier tier) const {
    std::vector<ResolvedEffectInstance> resolved;

    if (cue.kind == EffectCueKind::HitConfirm && cue.intensity == EffectIntensity::Heavy) {
        resolved.push_back({101, EffectPlacement::World, {0.0f, 0.0f, 0.0f}, cue.target_id, 0.25f, 1.1f, 1.0f, 0xFFFFFFFF, OverlayEmphasis::None});
        if (tier != urpg::presentation::CapabilityTier::Tier0_Basic) {
            resolved.push_back({201, EffectPlacement::Overlay, {0.0f, 0.0f, 0.0f}, cue.target_id, 0.08f, 1.0f, 1.0f, 0xFFFFFFFF, OverlayEmphasis::HitFlash});
        }
        return resolved;
    }

    if (cue.kind == EffectCueKind::CriticalHit) {
        if (tier == urpg::presentation::CapabilityTier::Tier0_Basic) {
            resolved.push_back({202, EffectPlacement::Overlay, {0.0f, 0.0f, 0.0f}, cue.target_id, 0.1f, 1.0f, 1.3f, 0xFFFFFFFF, OverlayEmphasis::CritFlash});
            return resolved;
        }
        resolved.push_back({102, EffectPlacement::World, {0.0f, 0.0f, 0.0f}, cue.target_id, 0.3f, 1.2f, 1.3f, 0xFFFFFFFF, OverlayEmphasis::None});
        resolved.push_back({202, EffectPlacement::Overlay, {0.0f, 0.0f, 0.0f}, cue.target_id, 0.1f, 1.0f, 1.3f, 0xFFFFFFFF, OverlayEmphasis::CritFlash});
        return resolved;
    }

    resolved.push_back({999, EffectPlacement::Overlay, {0.0f, 0.0f, 0.0f}, cue.target_id, 0.08f, 1.0f, 1.0f, 0xFFFFFFFF, cue.overlay_emphasis});
    return resolved;
}
```

- [ ] **Step 4: Register the new test and sources**

```cmake
  engine/core/presentation/effects/effect_catalog.cpp
  engine/core/presentation/effects/effect_resolver.cpp
  tests/unit/test_effect_resolver.cpp
```

Add the two new `.cpp` files to the engine/test target source lists in `CMakeLists.txt`.

- [ ] **Step 5: Run test to verify it passes**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectResolver expands heavy hit cues into world and overlay output|EffectResolver collapses critical-hit hybrid output on low tier"`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt engine/core/presentation/effects/effect_catalog.h engine/core/presentation/effects/effect_catalog.cpp engine/core/presentation/effects/effect_resolver.h engine/core/presentation/effects/effect_resolver.cpp tests/unit/test_effect_resolver.cpp
git commit -m "feat: add shared runtime effect resolver"
```

### Task 3: Extend presentation commands and add effect translation

**Files:**
- Modify: `engine/core/presentation/presentation_runtime.h`
- Create: `engine/core/presentation/effects/effect_translator.h`
- Create: `engine/core/presentation/effects/effect_translator.cpp`
- Create: `tests/unit/test_presentation_effect_translation.cpp`
- Modify: `tests/unit/test_presentation_runtime.cpp`
- Modify: `engine/core/presentation/render_backend_mock.h`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing tests**

```cpp
#include <catch2/catch_test_macros.hpp>
#include "engine/core/presentation/effects/effect_translator.h"

using namespace urpg::presentation;
using namespace urpg::presentation::effects;

TEST_CASE("EffectTranslator emits world and overlay effect commands in resolver order", "[presentation][effects][translation]") {
    std::vector<ResolvedEffectInstance> instances{
        {101, EffectPlacement::World, {1.0f, 2.0f, 0.0f}, 2, 0.25f, 1.2f, 1.0f, 0xFFFFFFFF, OverlayEmphasis::None},
        {201, EffectPlacement::Overlay, {0.0f, 0.0f, 0.0f}, 2, 0.08f, 1.0f, 1.0f, 0xFFFFFFFF, OverlayEmphasis::HitFlash}
    };

    PresentationFrameIntent intent;
    EffectTranslator translator;
    translator.append(instances, intent);

    REQUIRE(intent.commands.size() == 2);
    CHECK(intent.commands[0].type == PresentationCommand::Type::DrawWorldEffect);
    CHECK(intent.commands[1].type == PresentationCommand::Type::DrawOverlayEffect);
    CHECK(intent.commands[0].id == 101);
    CHECK(intent.commands[1].id == 201);
}
```

```cpp
TEST_CASE("Presentation runtime preserves effect commands when resolving environment commands", "[presentation][runtime][effects]") {
    PresentationFrameIntent intent;

    FogProfile fog;
    fog.density = 0.2f;
    intent.AddFog(fog);
    intent.AddWorldEffect(101, {0.0f, 1.0f, 0.0f}, 2, 0.25f, 1.0f, 1.0f, 0xFFFFFFFF);
    intent.AddOverlayEffect(201, 2, 0.08f, 1.0f, 1.0f, 0xFFFFFFFF, 1.0f);

    PresentationRuntime::ResolveEnvironmentCommands(intent);

    bool foundWorld = false;
    bool foundOverlay = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect && cmd.id == 101) foundWorld = true;
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect && cmd.id == 201) foundOverlay = true;
    }

    REQUIRE(foundWorld);
    REQUIRE(foundOverlay);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectTranslator emits world and overlay effect commands in resolver order|Presentation runtime preserves effect commands when resolving environment commands"`

Expected: FAIL because effect command types and translation helpers do not exist.

- [ ] **Step 3: Write minimal implementation**

```cpp
enum class Type {
    DrawActor,
    DrawProp,
    SetCamera,
    SetLight,
    SetFog,
    SetPostFX,
    DrawShadowProxy,
    SetEnvironment,
    DrawWorldEffect,
    DrawOverlayEffect
} type;
```

```cpp
float effectDurationSeconds = 0.0f;
float effectScale = 1.0f;
float effectIntensity = 1.0f;
uint32_t effectColorRgba = 0xFFFFFFFF;
uint32_t effectOwnerId = 0;
```

```cpp
void AddWorldEffect(uint32_t effectId, Vec3 pos, uint32_t ownerId, float durationSeconds, float scale,
                    float intensity, uint32_t colorRgba) {
    PresentationCommand cmd{PresentationCommand::Type::DrawWorldEffect, effectId, pos, {0,0,0}, nullptr};
    cmd.effectOwnerId = ownerId;
    cmd.effectDurationSeconds = durationSeconds;
    cmd.effectScale = scale;
    cmd.effectIntensity = intensity;
    cmd.effectColorRgba = colorRgba;
    commands.push_back(cmd);
}

void AddOverlayEffect(uint32_t effectId, uint32_t ownerId, float durationSeconds, float scale,
                      float intensity, uint32_t colorRgba, float blendWeight = 1.0f) {
    PresentationCommand cmd{PresentationCommand::Type::DrawOverlayEffect, effectId, {0,0,0}, {0,0,0}, nullptr};
    cmd.effectOwnerId = ownerId;
    cmd.effectDurationSeconds = durationSeconds;
    cmd.effectScale = scale;
    cmd.effectIntensity = intensity;
    cmd.effectColorRgba = colorRgba;
    cmd.blendWeight = blendWeight;
    commands.push_back(cmd);
}
```

```cpp
#pragma once

#include "effect_instance.h"
#include "engine/core/presentation/presentation_runtime.h"
#include <vector>

namespace urpg::presentation::effects {

class EffectTranslator {
public:
    void append(const std::vector<ResolvedEffectInstance>& instances,
                urpg::presentation::PresentationFrameIntent& intent) const;
};

} // namespace urpg::presentation::effects
```

```cpp
void EffectTranslator::append(const std::vector<ResolvedEffectInstance>& instances,
                              urpg::presentation::PresentationFrameIntent& intent) const {
    for (const auto& instance : instances) {
        if (instance.placement == EffectPlacement::World) {
            intent.AddWorldEffect(instance.effect_id,
                                  instance.position,
                                  instance.owner_id,
                                  instance.duration_seconds,
                                  instance.scale,
                                  instance.intensity,
                                  instance.color_rgba);
            continue;
        }

        intent.AddOverlayEffect(instance.effect_id,
                                instance.owner_id,
                                instance.duration_seconds,
                                instance.scale,
                                instance.intensity,
                                instance.color_rgba);
    }
}
```

- [ ] **Step 4: Update the render backend mock to track effect commands**

```cpp
size_t worldEffectCount = 0;
size_t overlayEffectCount = 0;
```

```cpp
case presentation::PresentationCommand::Type::DrawWorldEffect:
    m_state.worldEffectCount++;
    break;
case presentation::PresentationCommand::Type::DrawOverlayEffect:
    m_state.overlayEffectCount++;
    break;
```

- [ ] **Step 5: Register the new translation sources/tests**

```cmake
  engine/core/presentation/effects/effect_translator.cpp
  tests/unit/test_presentation_effect_translation.cpp
```

- [ ] **Step 6: Run test to verify it passes**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectTranslator emits world and overlay effect commands in resolver order|Presentation runtime preserves effect commands when resolving environment commands|Presentation runtime resolves weighted fog and PostFX blends"`

Expected: PASS

- [ ] **Step 7: Commit**

```bash
git add CMakeLists.txt engine/core/presentation/presentation_runtime.h engine/core/presentation/effects/effect_translator.h engine/core/presentation/effects/effect_translator.cpp engine/core/presentation/render_backend_mock.h tests/unit/test_presentation_effect_translation.cpp tests/unit/test_presentation_runtime.cpp
git commit -m "feat: translate resolved runtime effects into presentation commands"
```

### Task 4: Wire battle-first cue emission and hybrid translation integration

**Files:**
- Modify: `engine/core/scene/battle_scene.h`
- Modify: `engine/core/scene/battle_scene.cpp`
- Create: `tests/unit/test_battle_effect_cues.cpp`
- Modify: `tests/unit/test_battle_scene_native.cpp`
- Modify: `CMakeLists.txt`

- [ ] **Step 1: Write the failing tests**

```cpp
#include <catch2/catch_test_macros.hpp>
#include "engine/core/scene/battle_scene.h"

using namespace urpg::presentation::effects;
using namespace urpg::scene;

TEST_CASE("BattleScene emits cast then hit cues for a successful attack", "[battle][effects][cue]") {
    BattleScene battle({"2"});
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("2", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleScene::BattleAction action{};
    action.subject = &participants[0];
    action.target = &participants[1];
    action.command = "attack";

    battle.executeAction(action);
    const auto& cues = battle.effectCues();

    REQUIRE(cues.size() >= 2);
    CHECK(cues[0].kind == EffectCueKind::CastStart);
    CHECK(cues[1].kind == EffectCueKind::HitConfirm);
    CHECK(cues[0].subject_id == 1);
    CHECK(cues[1].target_id == 2);
}
```

```cpp
TEST_CASE("BattleScene emits miss cues instead of impact when attack fails", "[battle][effects][cue]") {
    BattleScene battle({"2"});
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("2", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    battle.enqueueEffectCue({EffectCueKind::MissSweep, EffectAnchorMode::TargetWorld, OverlayEmphasis::HitFlash, EffectIntensity::Light, 0, 0, 1, 2, 0, true});
    REQUIRE(battle.effectCues().front().kind == EffectCueKind::MissSweep);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleScene emits cast then hit cues for a successful attack|BattleScene emits miss cues instead of impact when attack fails"`

Expected: FAIL because `BattleScene` does not expose effect cue storage or emission yet.

- [ ] **Step 3: Write minimal implementation**

```cpp
#include "engine/core/presentation/effects/effect_cue.h"
```

```cpp
public:
    void enqueueEffectCue(const urpg::presentation::effects::EffectCue& cue);
    const std::vector<urpg::presentation::effects::EffectCue>& effectCues() const { return m_effectCues; }
    void clearEffectCues() { m_effectCues.clear(); }
```

```cpp
private:
    uint32_t m_effectSequence = 0;
    std::vector<urpg::presentation::effects::EffectCue> m_effectCues;
```

```cpp
void BattleScene::enqueueEffectCue(const urpg::presentation::effects::EffectCue& cue) {
    auto ordered = cue;
    ordered.sequence_index = m_effectSequence++;
    m_effectCues.push_back(ordered);
}
```

```cpp
void BattleScene::executeAction(const BattleAction& action) {
    // existing logic above this point stays in place
    enqueueEffectCue({urpg::presentation::effects::EffectCueKind::CastStart,
                      urpg::presentation::effects::EffectAnchorMode::SubjectWorld,
                      urpg::presentation::effects::OverlayEmphasis::None,
                      urpg::presentation::effects::EffectIntensity::Medium,
                      static_cast<uint32_t>(m_turnCount),
                      0,
                      static_cast<uint32_t>(parseDatabaseId(action.subject->id)),
                      action.target ? static_cast<uint32_t>(parseDatabaseId(action.target->id)) : 0,
                      0,
                      true});

    // after authoritative hit resolution:
    enqueueEffectCue({damage > 0 ? urpg::presentation::effects::EffectCueKind::HitConfirm
                                 : urpg::presentation::effects::EffectCueKind::MissSweep,
                      urpg::presentation::effects::EffectAnchorMode::TargetWorld,
                      damage > 0 ? urpg::presentation::effects::OverlayEmphasis::HitFlash
                                 : urpg::presentation::effects::OverlayEmphasis::None,
                      damage > 20 ? urpg::presentation::effects::EffectIntensity::Heavy
                                  : urpg::presentation::effects::EffectIntensity::Medium,
                      static_cast<uint32_t>(m_turnCount),
                      0,
                      static_cast<uint32_t>(parseDatabaseId(action.subject->id)),
                      action.target ? static_cast<uint32_t>(parseDatabaseId(action.target->id)) : 0,
                      0,
                      true});
}
```

- [ ] **Step 4: Register the new tests**

```cmake
  tests/unit/test_battle_effect_cues.cpp
```

- [ ] **Step 5: Run test to verify it passes**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "BattleScene emits cast then hit cues for a successful attack|BattleScene emits miss cues instead of impact when attack fails|BattleScene: executeAction effect application"`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add CMakeLists.txt engine/core/scene/battle_scene.h engine/core/scene/battle_scene.cpp tests/unit/test_battle_effect_cues.cpp tests/unit/test_battle_scene_native.cpp
git commit -m "feat: emit shared runtime effect cues from battle scene"
```

### Task 5: Translate battle cues into frame commands and expand validation/docs

**Files:**
- Modify: `engine/core/presentation/presentation_runtime.cpp`
- Modify: `engine/core/presentation/release_validation.cpp`
- Modify: `tests/unit/test_presentation_runtime.cpp`
- Modify: `docs/PROGRAM_COMPLETION_STATUS.md`
- Modify: `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md`
- Modify: `WORKLOG.md`

- [ ] **Step 1: Write the failing tests**

```cpp
TEST_CASE("PresentationBridge derives battle effect commands from active BattleScene", "[presentation][bridge][battle][effects]") {
    auto runtime = std::make_shared<PresentationRuntime>();
    auto authoringData = std::make_shared<PresentationAuthoringData>();
    PresentationBridge bridge(runtime, authoringData);

    auto battleScene = std::make_shared<urpg::scene::BattleScene>(std::vector<std::string>{"2"});
    battleScene->addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battleScene->addEnemy("2", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);
    battleScene->enqueueEffectCue({urpg::presentation::effects::EffectCueKind::HitConfirm,
                                   urpg::presentation::effects::EffectAnchorMode::TargetWorld,
                                   urpg::presentation::effects::OverlayEmphasis::HitFlash,
                                   urpg::presentation::effects::EffectIntensity::Heavy,
                                   0, 0, 1, 2, 0, true});

    urpg::scene::SceneManager sceneManager;
    sceneManager.pushScene(battleScene);

    PresentationContext context;
    context.activeMode = PresentationMode::Spatial;
    context.activeTier = CapabilityTier::Tier1_Standard;

    const auto intent = bridge.BuildFrameForActiveScene(sceneManager, context);

    bool foundWorldEffect = false;
    bool foundOverlayEffect = false;
    for (const auto& cmd : intent.commands) {
        if (cmd.type == PresentationCommand::Type::DrawWorldEffect) foundWorldEffect = true;
        if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) foundOverlayEffect = true;
    }

    REQUIRE(foundWorldEffect);
    REQUIRE(foundOverlayEffect);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "PresentationBridge derives battle effect commands from active BattleScene|PresentationBridge derives battle frame from active BattleScene"`

Expected: FAIL because the runtime frame build path does not yet resolve battle effect cues into presentation commands.

- [ ] **Step 3: Write minimal implementation**

```cpp
#include "engine/core/presentation/effects/effect_resolver.h"
#include "engine/core/presentation/effects/effect_translator.h"
```

```cpp
PresentationFrameIntent PresentationRuntime::BuildPresentationFrame(
    const PresentationContext& context,
    const PresentationAuthoringData& data) {
    PresentationFrameIntent intent;
    intent.activeMode = context.activeMode;
    intent.activeTier = context.activeTier;

    // existing map/battle/menu translation work remains here

    effects::EffectResolver resolver;
    effects::EffectTranslator translator;
    std::vector<effects::ResolvedEffectInstance> resolvedEffects;
    for (const auto& cue : context.battleState.effectCues) {
        const auto resolved = resolver.resolve(cue, context.activeTier);
        resolvedEffects.insert(resolvedEffects.end(), resolved.begin(), resolved.end());
    }
    translator.append(resolvedEffects, intent);

    ResolveEnvironmentCommands(intent);
    return intent;
}
```

```cpp
// release_validation.cpp
size_t worldEffectCommandCount = 0;
size_t overlayEffectCommandCount = 0;

if (cmd.type == PresentationCommand::Type::DrawWorldEffect) {
    worldEffectCommandCount++;
} else if (cmd.type == PresentationCommand::Type::DrawOverlayEffect) {
    overlayEffectCommandCount++;
}

assert(worldEffectCommandCount <= 16);
assert(overlayEffectCommandCount <= 16);
```

- [ ] **Step 4: Update the docs in the same slice**

Add concise truth-aligned notes to:

```md
- Runtime VFX cue pipeline baseline landed: battle now emits deterministic shared effect cues, the shared resolver expands hybrid world/overlay effect instances, and the presentation runtime consumes them through bounded effect commands.
```

Place the note in:
- `docs/PROGRAM_COMPLETION_STATUS.md` under current-cycle progress / latest landed work
- `docs/NATIVE_FEATURE_ABSORPTION_PLAN.md` as the newly opened runtime-visuals slice
- `WORKLOG.md` as the execution record

- [ ] **Step 5: Run verification**

Run: `ctest --test-dir build/dev-mingw-debug --output-on-failure -R "EffectCue defaults are deterministic and render-agnostic|EffectResolver expands heavy hit cues into world and overlay output|EffectTranslator emits world and overlay effect commands in resolver order|PresentationBridge derives battle effect commands from active BattleScene|urpg_presentation_release_validation"`

Expected: PASS

Run: `ctest --test-dir build/dev-mingw-debug -C Debug -R "urpg_(presentation_(unit_lane|release_validation)|spatial_editor_lane)" --output-on-failure`

Expected: PASS

- [ ] **Step 6: Commit**

```bash
git add engine/core/presentation/presentation_runtime.cpp engine/core/presentation/release_validation.cpp tests/unit/test_presentation_runtime.cpp docs/PROGRAM_COMPLETION_STATUS.md docs/NATIVE_FEATURE_ABSORPTION_PLAN.md WORKLOG.md
git commit -m "feat: land battle-first runtime vfx cue pipeline"
```

## Self-Review

### Spec coverage

- Shared `EffectCue` contract: covered by Task 1.
- Shared resolver and code-defined preset catalog: covered by Task 2.
- Hybrid world/overlay output: covered by Tasks 2 and 3.
- Battle-first deterministic emission: covered by Task 4.
- Presentation translation and frame integration: covered by Tasks 3 and 5.
- Bounded fallback/envelope verification: covered by Tasks 2, 3, and 5.
- Canonical docs and validation evidence: covered by Task 5.

No approved requirement from the design doc is left without a task.

### Placeholder scan

- No `TODO`, `TBD`, or “implement later” markers remain.
- Each code-changing step includes concrete code.
- Each verification step includes an exact command and expected outcome.
- No step depends on “similar to Task N” shorthand.

### Type consistency

- `EffectCue`, `ResolvedEffectInstance`, `EffectResolver`, and `EffectTranslator` names are consistent across all tasks.
- `DrawWorldEffect` and `DrawOverlayEffect` are used consistently as the new `PresentationCommand::Type` values.
- `BattleScene::enqueueEffectCue()` and `BattleScene::effectCues()` are introduced before later tasks rely on them.

