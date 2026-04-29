#include "editor/battle/battle_presentation_panel.h"
#include "editor/battle/battle_vfx_timeline_panel.h"
#include "editor/battle/boss_designer_panel.h"
#include "editor/battle/formula_debugger_panel.h"
#include "engine/core/battle/enemy_ai_profile.h"
#include "engine/core/battle/party_tactics_profile.h"
#include "engine/core/scene/battle_scene.h"

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path repoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

nlohmann::json loadJsonFile(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    nlohmann::json json;
    stream >> json;
    return json;
}

} // namespace

TEST_CASE("battle authoring validates battlebacks, HUD elements, and deterministic cue replay", "[battle][authoring][ffs05]") {
    urpg::battle::BattlePresentationProfile profile;
    profile.id = "arena";
    profile.battleback1 = "img/battlebacks1/CrystalCave.png";
    profile.battleback2 = "img/battlebacks2/MissingFog.png";
    profile.media_layers = {
        {"lava_video", "background", "movies/battle/lava_loop.webm", true, 1.5f, 0.8f, 5, 12},
    };
    profile.light_cues = {
        {"enemy_glow", "enemy", 180, "#ff8844", 0.65f, true, true, true},
    };
    profile.hud_elements = {
        {"hp", "gauge", 8, 8, true},
        {"state", "state_icon", 8, 32, true},
        {"turns", "turn_order", 160, 8, true},
        {"popup", "damage_popup", 200, 80, true},
        {"guard", "guard_marker", 64, 64, true},
    };
    profile.cue_timeline = {
        {"victory", "victory", 90, "me_victory"},
        {"cast", "cast", 10, "skill_1"},
        {"hit", "hit", 30, "slash"},
    };

    urpg::editor::BattlePresentationPanel panel;
    panel.loadProfile(profile, {"img/battlebacks1/CrystalCave.png", "movies/battle/lava_loop.webm"});
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().hud_element_count == 5);
    REQUIRE(panel.snapshot().cue_count == 3);
    REQUIRE(panel.snapshot().media_layer_count == 1);
    REQUIRE(panel.snapshot().light_cue_count == 1);
    REQUIRE(panel.snapshot().replay_cue_count == 3);
    REQUIRE(panel.validation().replay_cues[0].id == "cast");
    REQUIRE(panel.validation().replay_cues[1].id == "hit");
    REQUIRE(panel.validation().replay_cues[2].id == "victory");
    REQUIRE_FALSE(panel.validation().diagnostics.empty());
    REQUIRE(panel.validation().diagnostics[0].code == "missing_battleback");
}

TEST_CASE("battle presentation absorbs animated media layers and battler light cues",
          "[battle][authoring][presentation][native-plugin-absorption]") {
    const auto fixture = loadJsonFile(repoRoot() / "content" / "fixtures" / "battle_authoring_fixture.json");
    auto profile = urpg::battle::BattlePresentationProfileFromJson(fixture["presentation"]);

    REQUIRE(profile.media_layers.size() == 1);
    REQUIRE(profile.media_layers[0].asset == "movies/battle/lava_loop.webm");
    REQUIRE(profile.media_layers[0].region_id == 5);
    REQUIRE(profile.light_cues.size() == 1);
    REQUIRE(profile.light_cues[0].pulsate);
    REQUIRE(profile.light_cues[0].flicker);

    const std::set<std::string> assets = {
        "img/battlebacks1/CrystalCave.png",
        "img/battlebacks2/CrystalFog.png",
        "movies/battle/lava_loop.webm",
    };
    const auto result = urpg::battle::ValidateBattlePresentationProfile(profile, assets);
    REQUIRE(std::none_of(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.severity == urpg::battle::BattleAuthoringSeverity::Error;
    }));

    urpg::editor::BattlePresentationPanel panel;
    panel.loadProfile(profile, assets);
    panel.render();
    REQUIRE(panel.snapshot().media_layer_count == 1);
    REQUIRE(panel.snapshot().light_cue_count == 1);

    const auto saved = urpg::battle::BattlePresentationProfileToJson(profile);
    REQUIRE(saved["media_layers"][0]["asset"] == "movies/battle/lava_loop.webm");
    REQUIRE(saved["light_cues"][0]["color"] == "#ff8844");
}

TEST_CASE("battle VFX timeline is visually authorable, previewable, saved, and executable",
          "[battle][authoring][vfx][wysiwyg]") {
    urpg::battle::BattleVfxTimelineDocument document;
    document.id = "slash_combo";
    document.fps = 60;
    document.duration_frames = 48;
    document.addTrack({"cast_track", "Cast", "vfx", true, false});
    document.addTrack({"impact_track", "Impact", "vfx", true, false});
    document.addEvent({
        "cast_flash",
        "cast_track",
        4,
        "Cast flash",
        urpg::presentation::effects::EffectCueKind::CastStart,
        urpg::presentation::effects::EffectAnchorMode::Owner,
        1,
        1,
        1.1f,
        0.15f,
        {{"asset", "vfx/cast_flash"}}
    });
    document.addEvent({
        "target_impact",
        "impact_track",
        18,
        "Impact",
        urpg::presentation::effects::EffectCueKind::HitConfirm,
        urpg::presentation::effects::EffectAnchorMode::Target,
        1,
        2,
        1.6f,
        0.65f,
        {{"asset", "vfx/slash"}}
    });
    document.addEvent({
        "blood_splatter",
        "impact_track",
        20,
        "Blood splatter",
        urpg::presentation::effects::EffectCueKind::BloodSplatter,
        urpg::presentation::effects::EffectAnchorMode::Target,
        1,
        2,
        1.0f,
        0.45f,
        {{"asset", "vfx/blood_splatter"}}
    });

    urpg::editor::BattleVfxTimelinePanel panel;
    panel.loadDocument(document);
    panel.scrubToFrame(4);
    panel.render();

    REQUIRE(panel.snapshot().has_document);
    REQUIRE(panel.snapshot().track_count == 2);
    REQUIRE(panel.snapshot().visible_track_count == 2);
    REQUIRE(panel.snapshot().event_count == 3);
    REQUIRE(panel.snapshot().visible_event_count == 1);
    REQUIRE(panel.snapshot().visible_event_ids[0] == "cast_flash");
    REQUIRE(panel.snapshot().runtime_preview_cue_count == 1);
    REQUIRE(panel.snapshot().runtime_preview_command_count == 2);
    REQUIRE(panel.snapshot().timeline_progress > 0.0f);
    REQUIRE(panel.snapshot().active_track_id == "cast_track");
    REQUIRE(panel.snapshot().next_event_id == "target_impact");
    REQUIRE(panel.snapshot().ux_focus_lane == "live_cue");
    REQUIRE(panel.snapshot().primary_action.find("active cue") != std::string::npos);
    REQUIRE(std::find(panel.snapshot().runtime_preview_commands.begin(),
                      panel.snapshot().runtime_preview_commands.end(),
                      "battle_vfx_cue:cast_flash:cast:track=cast_track:asset=vfx/cast_flash") !=
            panel.snapshot().runtime_preview_commands.end());
    REQUIRE(panel.snapshot().diagnostic_count == 0);

    REQUIRE(panel.setTrackVisible("cast_track", false));
    REQUIRE(panel.snapshot().visible_track_count == 1);
    REQUIRE(panel.snapshot().visible_event_count == 0);
    REQUIRE(panel.snapshot().runtime_preview_cue_count == 0);
    REQUIRE_FALSE(panel.setTrackVisible("missing_track", false));

    const auto saved = panel.saveProjectData();
    REQUIRE(saved["schema_version"] == "urpg.battle_vfx_timeline.v1");
    REQUIRE(saved["tracks"].size() == 2);
    REQUIRE(saved["events"].size() == 3);

    const auto restored = urpg::battle::BattleVfxTimelineDocument::fromJson(saved);
    REQUIRE(restored.toJson() == saved);

    const auto timeline = restored.toTimelineDocument();
    REQUIRE(timeline.commands().size() == 3);
    REQUIRE(timeline.commands()[0].kind == urpg::timeline::TimelineCommandKind::BattleCue);
    REQUIRE(timeline.validate().empty());

    urpg::scene::BattleScene scene({});
    const auto applied = urpg::battle::applyBattleVfxTimeline(scene, restored);
    REQUIRE(applied == 3);
    REQUIRE(scene.effectCues().size() == 3);
    CHECK(scene.effectCues()[0].frameTick == 4);
    CHECK(scene.effectCues()[0].kind == urpg::presentation::effects::EffectCueKind::CastStart);
    CHECK(scene.effectCues()[1].frameTick == 18);
    CHECK(scene.effectCues()[1].ownerId == 2);
    CHECK(scene.effectCues()[1].intensity.value == Catch::Approx(1.6f));
    CHECK(scene.effectCues()[1].overlayEmphasis.value == Catch::Approx(0.65f));
    CHECK(scene.effectCues()[2].kind == urpg::presentation::effects::EffectCueKind::BloodSplatter);
}

TEST_CASE("battle VFX timeline project-data fixture round-trips through the authored schema surface",
          "[battle][authoring][vfx][schema]") {
    const auto fixture = loadJsonFile(repoRoot() / "content" / "fixtures" / "battle_vfx_timeline_fixture.json");
    const auto schema = loadJsonFile(repoRoot() / "content" / "schemas" / "battle_vfx_timeline.schema.json");

    REQUIRE(schema["properties"].contains("events"));
    REQUIRE(schema["properties"].contains("tracks"));
    REQUIRE(schema["properties"]["events"]["items"]["required"].size() == 9);

    const auto document = urpg::battle::BattleVfxTimelineDocument::fromJson(fixture);
    REQUIRE(document.validate().empty());
    REQUIRE(document.tracks().size() == 2);
    const auto saved = document.toJson();
    REQUIRE(saved["schema_version"] == fixture["schema_version"]);
    REQUIRE(saved["id"] == fixture["id"]);
    REQUIRE(saved["events"].size() == fixture["events"].size());
    REQUIRE(saved["events"][0]["id"] == "cast_flash");
    REQUIRE(saved["events"][1]["id"] == "target_impact");
    REQUIRE(document.toEffectCues().size() == 2);
    REQUIRE(document.runtimeCommandsAtFrame(18).size() == 2);
    REQUIRE(document.runtimeCommandsAtFrame(18)[1] ==
            "battle_vfx_cue:target_impact:hit:track=impact_track:asset=vfx/slash");
}

TEST_CASE("battle presentation schema exposes feedback policy authoring contract",
          "[battle][authoring][schema][feedback]") {
    const auto schema = loadJsonFile(repoRoot() / "content" / "schemas" / "battle_presentation.schema.json");

    REQUIRE(schema["properties"].contains("feedback_policy"));
    const auto feedback = schema["properties"]["feedback_policy"];
    REQUIRE(feedback["required"].size() == 4);
    REQUIRE(feedback["properties"]["schemaVersion"]["const"] == "1.0.0");
    REQUIRE(feedback["properties"]["chipDamagePercent"]["maximum"] == 100);
    REQUIRE(feedback["properties"]["chipHealingPercent"]["minimum"] == 0);
    REQUIRE(feedback["properties"]["zeroDamagePolicy"]["enum"].size() == 4);
    REQUIRE(feedback["properties"].contains("reuseTroopPositions"));
}

TEST_CASE("battle VFX timeline diagnostics block false done claims", "[battle][authoring][vfx][diagnostics]") {
    urpg::battle::BattleVfxTimelineDocument document;
    document.id.clear();
    document.duration_frames = 12;
    document.addTrack({"duplicate", "Duplicate", "vfx", true, false});
    document.addTrack({"duplicate", "Duplicate Two", "vfx", true, false});
    document.addEvent({
        "bad_hit",
        "missing_track",
        20,
        "Bad hit",
        urpg::presentation::effects::EffectCueKind::HitConfirm,
        urpg::presentation::effects::EffectAnchorMode::Target,
        0,
        0,
        -1.0f,
        1.5f,
        {}
    });

    urpg::editor::BattleVfxTimelinePanel panel;
    panel.loadDocument(document);
    panel.render();

    REQUIRE(panel.snapshot().diagnostic_count >= 4);
    REQUIRE(panel.snapshot().ux_focus_lane == "diagnostics");
    REQUIRE(std::any_of(panel.snapshot().diagnostics.begin(), panel.snapshot().diagnostics.end(), [](const auto& row) {
        return row.find("event_after_duration:bad_hit") != std::string::npos;
    }));
    REQUIRE(std::any_of(panel.snapshot().diagnostics.begin(), panel.snapshot().diagnostics.end(), [](const auto& row) {
        return row.find("missing_anchor_participant:bad_hit") != std::string::npos;
    }));
    REQUIRE(std::any_of(panel.snapshot().diagnostics.begin(), panel.snapshot().diagnostics.end(), [](const auto& row) {
        return row.find("unknown_event_track:bad_hit") != std::string::npos;
    }));
    REQUIRE(std::any_of(panel.snapshot().diagnostics.begin(), panel.snapshot().diagnostics.end(), [](const auto& row) {
        return row.find("duplicate_track_id:duplicate") != std::string::npos;
    }));
}

TEST_CASE("battle authoring requires an explicit release battleback", "[battle][authoring][assets]") {
    urpg::battle::BattlePresentationProfile profile;
    profile.id = "arena_without_background";

    const auto result = urpg::battle::ValidateBattlePresentationProfile(profile, {});

    REQUIRE(std::any_of(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "required_battleback" &&
               diagnostic.severity == urpg::battle::BattleAuthoringSeverity::Error;
    }));
    REQUIRE(std::any_of(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "missing_battleback" &&
               diagnostic.severity == urpg::battle::BattleAuthoringSeverity::Error;
    }));
}

TEST_CASE("Boss profile validates phase threshold ordering", "[battle][authoring][ffs05]") {
    urpg::battle::BossProfile profile;
    profile.id = "lich";
    profile.phases = {
        {"phase_1", 75, {}, false, "You dare?", "", "bgm_phase1"},
        {"phase_2", 80, {"bone_guard"}, true, "Enough.", "crown", "bgm_phase2"},
    };

    urpg::editor::BossDesignerPanel panel;
    panel.loadProfile(profile);
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().phase_count == 2);
    REQUIRE(panel.snapshot().diagnostic_count >= 1);
    REQUIRE(panel.validation().diagnostics[0].code == "phase_threshold_order");
}

TEST_CASE("Formula debugger uses bounded combat formula contract and reports fallbacks", "[battle][authoring][ffs05]") {
    urpg::editor::FormulaDebuggerPanel panel;
    panel.loadCases({
        {"supported", "a.atk * 4 - b.def * 2"},
        {"unsupported", "a.customStat * 2"},
        {"malformed", "a.atk +"},
    });
    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.snapshot().probe_count == 3);
    REQUIRE(panel.snapshot().fallback_count == 2);
    REQUIRE(panel.results()[0].id == "malformed");
    REQUIRE(panel.results()[0].reason == "malformed_formula_expression");
    REQUIRE(panel.results()[2].id == "unsupported");
    REQUIRE(panel.results()[2].reason == "unsupported_formula_symbol:a.customStat");
}

TEST_CASE("Enemy AI chooses deterministic weighted actions and rejects zero-weight profiles", "[battle][authoring][ffs05]") {
    urpg::battle::EnemyAiProfile profile;
    profile.id = "slime";
    profile.actions = {
        {"tackle", 10, ""},
        {"acid", 5, "enraged"},
        {"skip", 0, ""},
    };

    const urpg::battle::EnemyAiState normal_state;
    const auto normal = urpg::battle::ChooseEnemyAiAction(profile, normal_state, 4);
    REQUIRE(normal.has_value());
    REQUIRE(*normal == "tackle");

    const urpg::battle::EnemyAiState enraged_state{{"enraged"}};
    const auto enraged_a = urpg::battle::ChooseEnemyAiAction(profile, enraged_state, 12);
    const auto enraged_b = urpg::battle::ChooseEnemyAiAction(profile, enraged_state, 12);
    REQUIRE(enraged_a == enraged_b);
    REQUIRE(enraged_a.has_value());

    profile.actions = {{"skip", 0, ""}};
    REQUIRE_FALSE(urpg::battle::ChooseEnemyAiAction(profile, normal_state, 1).has_value());
}

TEST_CASE("Party tactics heal below threshold and defend when no heal is possible", "[battle][authoring][ffs05]") {
    urpg::battle::PartyTacticsProfile profile;
    profile.heal_below_percent = 40;
    profile.heal_action = "first_aid";
    profile.attack_action = "strike";
    profile.defend_action = "brace";

    const auto heal = urpg::battle::ChoosePartyTacticsAction(profile, {"actor_1", 20, 100, true});
    REQUIRE(heal.action_id == "first_aid");
    REQUIRE(heal.reason == "hp_below_threshold");

    const auto defend = urpg::battle::ChoosePartyTacticsAction(profile, {"actor_1", 20, 100, false});
    REQUIRE(defend.action_id == "brace");
    REQUIRE(defend.reason == "low_hp_no_heal_available");

    const auto attack = urpg::battle::ChoosePartyTacticsAction(profile, {"actor_1", 80, 100, false});
    REQUIRE(attack.action_id == "strike");
    REQUIRE(attack.reason == "default_attack");
}
