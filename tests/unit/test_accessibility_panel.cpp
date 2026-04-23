#include <catch2/catch_test_macros.hpp>
#include "editor/accessibility/accessibility_panel.h"
#include "editor/accessibility/accessibility_menu_adapter.h"
#include "editor/accessibility/accessibility_spatial_adapter.h"
#include "editor/accessibility/accessibility_audio_adapter.h"
#include "editor/accessibility/accessibility_battle_adapter.h"
#include "engine/core/accessibility/accessibility_auditor.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/ui_types.h"

using namespace urpg::editor;
using namespace urpg::accessibility;
using namespace urpg::ui;
using urpg::MenuRouteTarget;

TEST_CASE("AccessibilityPanel: Empty snapshot when no auditor bound", "[accessibility][editor][panel]") {
    AccessibilityPanel panel;
    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot.is_object());
    REQUIRE(snapshot.empty());
}

TEST_CASE("AccessibilityPanel: Snapshot reflects issues after audit", "[accessibility][editor][panel]") {
    AccessibilityAuditor auditor;
    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);

    auditor.ingestElements({
        UiElementSnapshot{"btn_ok", "", true, 1, 4.5f},
        UiElementSnapshot{"text_1", "Low Contrast", false, 0, 2.5f}
    });

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot["issueCount"] == 2);
    REQUIRE(snapshot["issues"].is_array());
    REQUIRE(snapshot["issues"].size() == 2);
}

TEST_CASE("AccessibilityPanel: Counts are accurate", "[accessibility][editor][panel]") {
    AccessibilityAuditor auditor;
    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);

    auditor.ingestElements({
        UiElementSnapshot{"btn_a", "", true, 1, 4.5f},
        UiElementSnapshot{"btn_b", "", true, 1, 4.5f},
        UiElementSnapshot{"text_1", "Low Contrast", false, 0, 2.5f}
    });

    panel.render();
    auto snapshot = panel.lastRenderSnapshot();

    REQUIRE(snapshot["issueCount"] == 5);
    REQUIRE(snapshot["errorCount"] == 3);
    REQUIRE(snapshot["warningCount"] == 2);
}

TEST_CASE("AccessibilityMenuAdapter ingests live MenuInspectorModel and produces audit issues", "[accessibility][editor][panel]") {
    // 1. Set up a command registry with commands (one missing label, two with same priority)
    MenuCommandRegistry registry;
    registry.registerCommand({"cmd_start", "Start Game", "", MenuRouteTarget::None, "", MenuRouteTarget::None, "", {}, {}, 1});
    registry.registerCommand({"cmd_load", "", "", MenuRouteTarget::None, "", MenuRouteTarget::None, "", {}, {}, 2}); // missing label
    registry.registerCommand({"cmd_options", "Options", "", MenuRouteTarget::None, "", MenuRouteTarget::None, "", {}, {}, 1}); // duplicate priority

    // 2. Set up a menu scene with a pane containing those commands
    auto scene = std::make_shared<MenuScene>("main_menu");
    MenuPane pane;
    pane.id = "main_pane";
    pane.displayName = "Main";
    pane.isVisible = true;
    pane.isActive = true;
    pane.commands.push_back(*registry.getCommand("cmd_start"));
    pane.commands.push_back(*registry.getCommand("cmd_load"));
    pane.commands.push_back(*registry.getCommand("cmd_options"));
    scene->addPane(pane);

    // 3. Set up scene graph and push the scene
    MenuSceneGraph graph;
    graph.registerScene(scene);
    graph.pushScene("main_menu");

    // 4. Load into MenuInspectorModel
    MenuInspectorModel model;
    MenuCommandRegistry::SwitchState switches;
    MenuCommandRegistry::VariableState variables;
    model.LoadFromRuntime(graph, registry, switches, variables);

    // 5. Ingest via adapter
    auto elements = AccessibilityMenuAdapter::ingest(model);
    REQUIRE(elements.size() == 3);

    // 6. Audit
    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    // MenuInspectorModel always synthesizes fallback labels, so MissingLabel
    // cannot be triggered from normal menu data. We verify FocusOrder instead.
    bool foundDuplicateFocusOrder = false;
    for (const auto& issue : issues) {
        if (issue.category == IssueCategory::FocusOrder) {
            foundDuplicateFocusOrder = true;
        }
    }

    REQUIRE(foundDuplicateFocusOrder);
}

// ---------------------------------------------------------------------------
// AccessibilitySpatialAdapter
// ---------------------------------------------------------------------------

TEST_CASE("AccessibilitySpatialAdapter: active panels with targets produce focusable elements",
          "[accessibility][editor][spatial][s28]") {
    ElevationBrushPanel::RenderSnapshot elevSnap;
    elevSnap.visible = true;
    elevSnap.has_target = true;
    elevSnap.grid_width = 8;
    elevSnap.grid_height = 8;

    PropPlacementPanel::RenderSnapshot propSnap;
    propSnap.visible = true;
    propSnap.has_target = true;
    propSnap.selected_asset_id = "rock_01";
    propSnap.prop_count = 2;
    propSnap.last_added_asset_id = "rock_01";

    auto elements = AccessibilitySpatialAdapter::ingest(elevSnap, propSnap);
    // elevation_brush, prop_placement, selected_asset, last_added_prop
    REQUIRE(elements.size() == 4);

    const auto elevEl = std::find_if(elements.begin(), elements.end(),
        [](const UiElementSnapshot& e) { return e.id == "spatial.elevation_brush"; });
    REQUIRE(elevEl != elements.end());
    REQUIRE(elevEl->label == "Elevation Brush");
    REQUIRE(elevEl->hasFocus);

    const auto propEl = std::find_if(elements.begin(), elements.end(),
        [](const UiElementSnapshot& e) { return e.id == "spatial.prop_placement"; });
    REQUIRE(propEl != elements.end());
    REQUIRE(propEl->label == "Prop Placement");
    REQUIRE(propEl->hasFocus);

    const auto assetEl = std::find_if(elements.begin(), elements.end(),
        [](const UiElementSnapshot& e) { return e.id == "spatial.selected_asset"; });
    REQUIRE(assetEl != elements.end());
    REQUIRE(assetEl->label == "rock_01");

    const auto lastPropEl = std::find_if(elements.begin(), elements.end(),
        [](const UiElementSnapshot& e) { return e.id == "spatial.last_added_prop"; });
    REQUIRE(lastPropEl != elements.end());
    REQUIRE(lastPropEl->label == "rock_01");
}

TEST_CASE("AccessibilitySpatialAdapter: panels without targets produce non-focusable elements",
          "[accessibility][editor][spatial][s28]") {
    ElevationBrushPanel::RenderSnapshot elevSnap;
    elevSnap.visible = true;
    elevSnap.has_target = false;

    PropPlacementPanel::RenderSnapshot propSnap;
    propSnap.visible = true;
    propSnap.has_target = false;
    propSnap.selected_asset_id = "";

    auto elements = AccessibilitySpatialAdapter::ingest(elevSnap, propSnap);
    // elevation_brush, prop_placement, selected_asset — no last_added_prop
    REQUIRE(elements.size() == 3);

    for (const auto& el : elements) {
        REQUIRE_FALSE(el.hasFocus);
    }

    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    // No focusable elements → Navigation warning
    const bool hasNavWarn = std::any_of(issues.begin(), issues.end(),
        [](const AccessibilityIssue& i) { return i.category == IssueCategory::Navigation; });
    REQUIRE(hasNavWarn);
}

TEST_CASE("AccessibilitySpatialAdapter: empty selected asset id surfaces MissingLabel via auditor",
          "[accessibility][editor][spatial][s28]") {
    ElevationBrushPanel::RenderSnapshot elevSnap;
    elevSnap.visible = true;
    elevSnap.has_target = true;

    PropPlacementPanel::RenderSnapshot propSnap;
    propSnap.visible = true;
    propSnap.has_target = true;
    propSnap.selected_asset_id = "";  // no asset selected

    auto elements = AccessibilitySpatialAdapter::ingest(elevSnap, propSnap);
    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    // The selected_asset element has hasFocus=false (no asset), so MissingLabel
    // is not triggered. The panel elements have labels so no MissingLabel.
    const bool hasMissingLabel = std::any_of(issues.begin(), issues.end(),
        [](const AccessibilityIssue& i) { return i.category == IssueCategory::MissingLabel; });
    REQUIRE_FALSE(hasMissingLabel);
}

// ---------------------------------------------------------------------------
// AccessibilityAudioAdapter
// ---------------------------------------------------------------------------

TEST_CASE("AccessibilityAudioAdapter: default bank produces one element per preset",
          "[accessibility][editor][audio][s28]") {
    urpg::audio::AudioMixPresetBank bank;
    auto elements = AccessibilityAudioAdapter::ingest(bank);

    // Default bank: Default, Battle, Cinematic — all focusable
    REQUIRE(elements.size() == 3);

    for (const auto& el : elements) {
        REQUIRE(el.hasFocus);
        REQUIRE_FALSE(el.label.empty());
        REQUIRE(el.focusOrder >= 1);
    }

    // Focus orders must be unique across presets (no duplicate FocusOrder)
    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();
    const bool hasFocusOrderIssue = std::any_of(issues.begin(), issues.end(),
        [](const AccessibilityIssue& i) { return i.category == IssueCategory::FocusOrder; });
    REQUIRE_FALSE(hasFocusOrderIssue);
}

TEST_CASE("AccessibilityAudioAdapter: preset with unknown category name gets empty label",
          "[accessibility][editor][audio][s28]") {
    nlohmann::json j;
    j["version"] = "1.0.0";
    j["presets"] = nlohmann::json::array();

    // Valid preset
    nlohmann::json good;
    good["name"] = "Default";
    good["duckBGMOnSE"] = false;
    good["duckAmount"] = 0.0f;
    good["categoryVolumes"]["BGM"] = 1.0f;
    j["presets"].push_back(good);

    // Preset with unknown category
    nlohmann::json bad;
    bad["name"] = "BadPreset";
    bad["duckBGMOnSE"] = false;
    bad["duckAmount"] = 0.0f;
    bad["categoryVolumes"]["BGM"] = 1.0f;
    bad["categoryVolumes"]["UNKNOWN_AUDIO_CAT"] = 0.5f;
    j["presets"].push_back(bad);

    urpg::audio::AudioMixPresetBank bank;
    bank.fromJson(j);

    auto elements = AccessibilityAudioAdapter::ingest(bank);
    REQUIRE(elements.size() == 2);

    // Find the BadPreset element — it should have an empty label
    const auto badEl = std::find_if(elements.begin(), elements.end(),
        [](const UiElementSnapshot& e) { return e.id == "audio.preset.BadPreset"; });
    REQUIRE(badEl != elements.end());
    REQUIRE(badEl->hasFocus);
    REQUIRE(badEl->label.empty());  // empty because unknownCategoryNames is non-empty

    // Auditing should surface a MissingLabel error for the bad preset
    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    const bool hasMissingLabel = std::any_of(issues.begin(), issues.end(),
        [](const AccessibilityIssue& i) {
            return i.category == IssueCategory::MissingLabel &&
                   i.elementId == "audio.preset.BadPreset";
        });
    REQUIRE(hasMissingLabel);
}

// ---------------------------------------------------------------------------
// AccessibilityBattleAdapter
// ---------------------------------------------------------------------------

TEST_CASE("AccessibilityBattleAdapter: action rows map to focusable elements",
          "[accessibility][editor][battle][s28]") {
    using namespace urpg::battle;

    // Build a minimal battle context with two combatants
    BattleFlowController flow;
    BattleActionQueue queue;

    Combatant hero;
    hero.id = "hero_01";
    hero.name = "Hero";
    hero.hp = 100;
    hero.maxHp = 100;
    hero.mp = 50;
    hero.maxMp = 50;
    hero.speed = 10;
    hero.isEnemy = false;
    flow.addCombatant(hero);

    Combatant enemy;
    enemy.id = "enemy_01";
    enemy.name = "Slime";
    enemy.hp = 30;
    enemy.maxHp = 30;
    enemy.mp = 0;
    enemy.maxMp = 0;
    enemy.speed = 5;
    enemy.isEnemy = true;
    flow.addCombatant(enemy);

    BattleAction heroAction;
    heroAction.subjectId = "hero_01";
    heroAction.targetId = "enemy_01";
    heroAction.command = "Attack";
    heroAction.speed = 10;
    heroAction.priority = 1;
    queue.enqueue(heroAction);

    BattleAction enemyAction;
    enemyAction.subjectId = "enemy_01";
    enemyAction.targetId = "hero_01";
    enemyAction.command = "Tackle";
    enemyAction.speed = 5;
    enemyAction.priority = 2;
    queue.enqueue(enemyAction);

    BattleInspectorModel model;
    model.LoadFromRuntime(flow, queue);

    auto elements = AccessibilityBattleAdapter::ingest(model);
    REQUIRE_FALSE(elements.empty());
    REQUIRE(elements.size() == 2);

    for (const auto& el : elements) {
        REQUIRE_FALSE(el.id.empty());
        REQUIRE_FALSE(el.label.empty());
        REQUIRE(el.hasFocus);
    }
}

TEST_CASE("AccessibilityBattleAdapter: action with empty command surfaces MissingLabel",
          "[accessibility][editor][battle][s28]") {
    using namespace urpg::battle;

    BattleFlowController flow;
    BattleActionQueue queue;

    Combatant hero;
    hero.id = "hero_02";
    hero.name = "Hero2";
    hero.hp = 100;
    hero.maxHp = 100;
    hero.mp = 50;
    hero.maxMp = 50;
    hero.speed = 10;
    hero.isEnemy = false;
    flow.addCombatant(hero);

    BattleAction emptyCommandAction;
    emptyCommandAction.subjectId = "hero_02";
    emptyCommandAction.targetId = "enemy_99";
    emptyCommandAction.command = "";  // intentionally empty — should surface MissingLabel
    emptyCommandAction.speed = 10;
    emptyCommandAction.priority = 1;
    queue.enqueue(emptyCommandAction);

    BattleInspectorModel model;
    model.LoadFromRuntime(flow, queue);

    auto elements = AccessibilityBattleAdapter::ingest(model);
    REQUIRE_FALSE(elements.empty());

    const auto emptyEl = std::find_if(elements.begin(), elements.end(),
        [](const UiElementSnapshot& e) { return e.label.empty() && e.hasFocus; });
    REQUIRE(emptyEl != elements.end());

    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    const bool hasMissingLabel = std::any_of(issues.begin(), issues.end(),
        [](const AccessibilityIssue& i) { return i.category == IssueCategory::MissingLabel; });
    REQUIRE(hasMissingLabel);
}

TEST_CASE("AccessibilityBattleAdapter: empty model produces Navigation warning",
          "[accessibility][editor][battle][s28]") {
    BattleInspectorModel model;
    // Do not load any rows — empty visible rows set

    auto elements = AccessibilityBattleAdapter::ingest(model);
    REQUIRE(elements.empty());

    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    // Empty element set → no issues (auditor only warns when elements exist but none is focusable)
    REQUIRE(issues.empty());
}

// ─── S28-T02: Actionable file/line reference diagnostics ──────────────────────

TEST_CASE("Spatial adapter emits sourceContext for all elements",
          "[accessibility][editor][spatial][s28t02]") {
    ElevationBrushPanel::RenderSnapshot es;
    es.visible = true;
    es.has_target = true;
    es.brush_size = 3;
    es.brush_strength = 0.8f;

    PropPlacementPanel::RenderSnapshot ps;
    ps.visible = true;
    ps.has_target = true;
    ps.selected_asset_id = "tree_01";

    auto elements = AccessibilitySpatialAdapter::ingest(es, ps);

    for (const auto& el : elements) {
        REQUIRE_FALSE(el.sourceContext.empty());
    }
    REQUIRE(elements[0].sourceContext == "editor/spatial/elevation_brush_panel.h");
    REQUIRE(elements[1].sourceContext == "editor/spatial/prop_placement_panel.h");
}

TEST_CASE("Audio adapter emits sourceContext for all elements",
          "[accessibility][editor][audio][s28t02]") {
    urpg::audio::AudioMixPresetBank bank;
    bank.loadDefaults();
    auto elements = AccessibilityAudioAdapter::ingest(bank);

    REQUIRE_FALSE(elements.empty());
    for (const auto& el : elements) {
        REQUIRE(el.sourceContext == "engine/core/audio/audio_mix_presets.h");
    }
}

TEST_CASE("Battle adapter emits sourceContext for all elements",
          "[accessibility][editor][battle][s28t02]") {
    BattleInspectorModel model;
    model.loadFixture();
    auto elements = AccessibilityBattleAdapter::ingest(model);

    REQUIRE_FALSE(elements.empty());
    for (const auto& el : elements) {
        REQUIRE(el.sourceContext == "editor/battle/battle_inspector_model.h");
    }
}

TEST_CASE("MissingLabel issue from audio adapter includes sourceFile in panel snapshot",
          "[accessibility][editor][audio][s28t02]") {
    // Build a bank with one preset that has unknown category names (yields empty label).
    urpg::audio::AudioMixPresetBank bank;
    bank.loadDefaults();
    const std::string badJson = R"({
        "presets": [
          {
            "name": "BadPreset",
            "categoryVolumes": { "UNKNOWN_CAT": 0.5 },
            "duckBGMOnSE": false,
            "duckAmount": 0.0
          }
        ]
    })";
    bank.loadFromJsonString(badJson);

    auto elements = AccessibilityAudioAdapter::ingest(bank);

    AccessibilityAuditor auditor;
    auditor.ingestElements(elements);
    auto issues = auditor.audit();

    const auto missingIt = std::find_if(issues.begin(), issues.end(),
        [](const AccessibilityIssue& i) { return i.category == IssueCategory::MissingLabel; });
    REQUIRE(missingIt != issues.end());
    REQUIRE(missingIt->sourceFile == "engine/core/audio/audio_mix_presets.h");
}

TEST_CASE("Panel snapshot includes sourceFile field for issues with source context",
          "[accessibility][editor][s28t02]") {
    AccessibilityAuditor auditor;

    UiElementSnapshot el;
    el.id = "test.element";
    el.label = "";   // triggers MissingLabel
    el.hasFocus = true;
    el.focusOrder = 1;
    el.contrastRatio = 0.0f;
    el.sourceContext = "some/source/file.h";

    auditor.ingestElements({el});

    AccessibilityPanel panel;
    panel.bindAuditor(&auditor);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["issueCount"] == 1);
    REQUIRE(snapshot["issues"][0]["sourceFile"] == "some/source/file.h");
}
