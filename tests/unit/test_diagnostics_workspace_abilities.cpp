#include "tests/unit/diagnostics_workspace_test_helpers.h"

TEST_CASE("DiagnosticsWorkspace - Audio and ability runtimes clear and rebind cleanly",
          "[editor][diagnostics][integration][runtime_clear]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    urpg::audio::AudioCore firstAudioCore;
    firstAudioCore.playSound("first_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(firstAudioCore);

    const auto firstAudioSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio);
    REQUIRE(firstAudioSummary.item_count == 1);
    REQUIRE(firstAudioSummary.has_data);

    workspace.clearAudioRuntime();

    const auto clearedAudioSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio);
    REQUIRE(clearedAudioSummary.item_count == 0);
    REQUIRE_FALSE(clearedAudioSummary.has_data);

    urpg::audio::AudioCore reboundAudioCore;
    reboundAudioCore.playSound("rebound_audio_a", urpg::audio::AudioCategory::SE);
    reboundAudioCore.playSound("rebound_audio_b", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(reboundAudioCore);

    const auto reboundAudioSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Audio);
    REQUIRE(reboundAudioSummary.item_count == 2);
    REQUIRE(reboundAudioSummary.has_data);

    urpg::ability::AbilitySystemComponent firstAsc;
    firstAsc.addTag(urpg::ability::GameplayTag("State.Empowered"));
    auto firstAbility = std::make_shared<WorkspaceAbility>("skill.first");
    firstAsc.grantAbility(firstAbility);
    workspace.bindAbilityRuntime(firstAsc);

    const auto firstAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(firstAbilitySummary.item_count == 1);
    REQUIRE(firstAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().size() == 1);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.update();
    const auto firstAbilityJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(firstAbilityJson["active_tab"] == "abilities");
    REQUIRE(firstAbilityJson["active_tab_detail"]["tab"] == "abilities");
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"].is_array());
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"][0]["name"] == "skill.first");
    REQUIRE(firstAbilityJson["active_tab_detail"]["abilities"][0]["can_activate"] == true);
    REQUIRE(firstAbilityJson["active_tab_detail"]["active_tags"].is_array());
    REQUIRE(firstAbilityJson["active_tab_detail"]["active_tags"].size() == 1);
    REQUIRE(firstAbilityJson["active_tab_detail"]["active_tags"][0]["tag"] == "State.Empowered");

    workspace.clearAbilityRuntime();

    const auto clearedAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(clearedAbilitySummary.item_count == 0);
    REQUIRE_FALSE(clearedAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().empty());
    const auto clearedAbilityJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(clearedAbilityJson["active_tab"] == "abilities");
    REQUIRE(clearedAbilityJson["active_tab_detail"]["abilities"].is_array());
    REQUIRE(clearedAbilityJson["active_tab_detail"]["abilities"].empty());
    REQUIRE(clearedAbilityJson["active_tab_detail"]["active_tags"].is_array());
    REQUIRE(clearedAbilityJson["active_tab_detail"]["active_tags"].empty());

    urpg::ability::AbilitySystemComponent reboundAsc;
    reboundAsc.addTag(urpg::ability::GameplayTag("State.Charged"));
    auto reboundAbilityA = std::make_shared<WorkspaceAbility>("skill.rebound_a");
    auto reboundAbilityB = std::make_shared<WorkspaceAbility>("skill.rebound_b");
    reboundAsc.grantAbility(reboundAbilityA);
    reboundAsc.grantAbility(reboundAbilityB);
    workspace.bindAbilityRuntime(reboundAsc);

    const auto reboundAbilitySummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(reboundAbilitySummary.item_count == 2);
    REQUIRE(reboundAbilitySummary.has_data);
    REQUIRE(workspace.abilityPanel().getModel().getActiveTags().size() == 1);
    const auto reboundAbilityJson = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(reboundAbilityJson["active_tab"] == "abilities");
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"].is_array());
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"].size() == 2);
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"][0]["name"] == "skill.rebound_a");
    REQUIRE(reboundAbilityJson["active_tab_detail"]["abilities"][1]["name"] == "skill.rebound_b");
    REQUIRE(reboundAbilityJson["active_tab_detail"]["active_tags"].size() == 1);
    REQUIRE(reboundAbilityJson["active_tab_detail"]["active_tags"][0]["tag"] == "State.Charged");
}

TEST_CASE("DiagnosticsWorkspace - ability tab follows live scene-owned ASC updates",
          "[editor][diagnostics][integration][abilities][scene]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    urpg::scene::BattleScene battle({"1"});
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle.getParticipants());
    auto* hero = findParticipant(participants, false);
    REQUIRE(hero != nullptr);

    hero->abilitySystem.setAttribute("MP", 20.0f);
    auto liveAbility = std::make_shared<SceneBoundWorkspaceAbility>();
    hero->abilitySystem.grantAbility(liveAbility);

    workspace.bindAbilityRuntime(hero->abilitySystem);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.update();

    const auto before = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(before["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(before["active_tab_detail"]["abilities"][0]["name"] == "skill.live_scene");
    REQUIRE(before["active_tab_detail"]["abilities"][0]["can_activate"] == true);

    REQUIRE(hero->abilitySystem.tryActivateAbility(*liveAbility));
    workspace.update();
    workspace.render();

    const auto after = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(after["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(after["active_tab_detail"]["abilities"][0]["name"] == "skill.live_scene");
    REQUIRE(after["active_tab_detail"]["abilities"][0]["can_activate"] == false);
    REQUIRE(after["active_tab_detail"]["abilities"][0]["cooldown_remaining"].get<float>() > 0.0f);

    const auto snapshot = workspace.abilityPanel().getRenderSnapshot();
    REQUIRE(snapshot.latest_ability_id == "skill.live_scene");
    REQUIRE(snapshot.diagnostic_count == 1);
}

TEST_CASE("DiagnosticsWorkspace - ability workflow actions expose selection and live preview activation",
          "[editor][diagnostics][integration][abilities][actions]") {
    urpg::ability::AbilitySystemComponent asc;
    asc.setAttribute("MP", 20.0f);
    asc.grantAbility(std::make_shared<WorkspacePreviewAbility>("skill.preview_fire", 3.0f, 5.0f));
    asc.grantAbility(std::make_shared<WorkspacePreviewAbility>("skill.preview_ice", 0.0f, 2.0f));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindAbilityRuntime(asc);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.update();

    REQUIRE(workspace.selectAbilityRow(0));
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "abilities");
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.preview_fire");
    REQUIRE(exported["active_tab_detail"]["selected_ability_can_activate"] == true);
    REQUIRE(exported["active_tab_detail"]["can_preview_selected_ability"] == true);

    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.preview_fire");
    REQUIRE(exported["active_tab_detail"]["latest_outcome"] == "executed");
    REQUIRE(exported["active_tab_detail"]["diagnostic_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["diagnostic_lines"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["selected_ability_can_activate"] == false);
    REQUIRE(exported["active_tab_detail"]["selected_ability_blocking_reason"].get<std::string>().find("Cooldown") !=
            std::string::npos);
    REQUIRE(asc.getAttribute("MP", 0.0f) == 15.0f);

    REQUIRE(workspace.selectAbilityRow(1));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.preview_ice");
    REQUIRE(exported["active_tab_detail"]["selected_ability_can_activate"] == true);
}

TEST_CASE("DiagnosticsWorkspace - ability draft editing drives owned preview runtime and pattern/effect export",
          "[editor][diagnostics][integration][abilities][draft]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.beginAbilityDraftPreview();

    REQUIRE(workspace.setAbilityDraftId("skill.editor_authored"));
    REQUIRE(workspace.setAbilityDraftCooldownSeconds(4.0f));
    REQUIRE(workspace.setAbilityDraftMpCost(7.0f));
    REQUIRE(workspace.setAbilityDraftEffectId("effect.editor_guard"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("Defense"));
    REQUIRE(workspace.setAbilityDraftEffectValue(18.0f));
    REQUIRE(workspace.setAbilityDraftEffectDuration(9.0f));
    REQUIRE(workspace.applyAbilityDraftPatternPreset("skill_cross_small"));
    REQUIRE(workspace.setAbilityDraftPatternName("Editor Guard Pattern"));
    REQUIRE(workspace.toggleAbilityDraftPatternPoint(1, 1));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "abilities");
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["draft"]["ability_id"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_attribute"] == "Defense");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_value"] == 18.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_mp_before"] == 30.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_mp_after"] == 23.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_attribute_before"] == 100.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["preview_attribute_after"] == 118.0);
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["name"] == "Editor Guard Pattern");
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["is_valid"] == true);
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["grid_rows"][2].get<std::string>().find("[O]") !=
            std::string::npos);

    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.editor_authored");
    REQUIRE(exported["active_tab_detail"]["latest_outcome"] == "executed");
    REQUIRE(exported["active_tab_detail"]["diagnostic_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["can_activate"] == false);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["blocking_reason"].get<std::string>().find("Cooldown") !=
            std::string::npos);
}

TEST_CASE("DiagnosticsWorkspace - ability draft state save and load round-trip",
          "[editor][diagnostics][integration][abilities][draft_io]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.beginAbilityDraftPreview();

    REQUIRE(workspace.setAbilityDraftId("skill.saved_authored"));
    REQUIRE(workspace.setAbilityDraftCooldownSeconds(6.0f));
    REQUIRE(workspace.setAbilityDraftMpCost(9.0f));
    REQUIRE(workspace.setAbilityDraftEffectId("effect.saved_guard"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("MagicDefense"));
    REQUIRE(workspace.setAbilityDraftEffectOperation(urpg::ModifierOp::Override));
    REQUIRE(workspace.setAbilityDraftEffectValue(42.0f));
    REQUIRE(workspace.setAbilityDraftEffectDuration(11.0f));
    REQUIRE(workspace.applyAbilityDraftPatternPreset("skill_cross_small"));
    REQUIRE(workspace.setAbilityDraftPatternName("Saved Guard Pattern"));

    const auto exportedDraft = nlohmann::json::parse(workspace.exportAbilityDraftStateJson());
    REQUIRE(exportedDraft["ability_id"] == "skill.saved_authored");
    REQUIRE(exportedDraft["effect_attribute"] == "MagicDefense");
    REQUIRE(exportedDraft["effect_operation"] == "Override");
    REQUIRE(exportedDraft["pattern"]["name"] == "Saved Guard Pattern");

    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_ability_draft.json").string();
    std::filesystem::remove(temp_path);
    REQUIRE(workspace.saveAbilityDraftStateToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    urpg::editor::DiagnosticsWorkspace loadWorkspace;
    loadWorkspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(loadWorkspace.loadAbilityDraftStateFromFile(temp_path));

    const auto loaded = nlohmann::json::parse(loadWorkspace.exportAsJson());
    REQUIRE(loaded["active_tab_detail"]["draft"]["ability_id"] == "skill.saved_authored");
    REQUIRE(loaded["active_tab_detail"]["draft"]["effect_attribute"] == "MagicDefense");
    REQUIRE(loaded["active_tab_detail"]["draft"]["effect_operation"] == "Override");
    REQUIRE(loaded["active_tab_detail"]["draft"]["effect_value"] == 42.0);
    REQUIRE(loaded["active_tab_detail"]["draft"]["pattern_preview"]["name"] == "Saved Guard Pattern");
    REQUIRE(loaded["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(loaded["active_tab_detail"]["abilities"][0]["name"] == "skill.saved_authored");

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - ability draft IO reports paths and preserves valid drafts on failure",
          "[editor][diagnostics][integration][abilities][draft_io][error]") {
    const auto temp_root = std::filesystem::temp_directory_path() / "urpg_workspace_ability_draft_io_errors";
    std::filesystem::remove_all(temp_root);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.beginAbilityDraftPreview();
    REQUIRE(workspace.setAbilityDraftId("skill.valid_before_failed_load"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("MagicDefense"));

    const auto nested_path = (temp_root / "nested" / "drafts" / "saved.json").string();
    REQUIRE(workspace.saveAbilityDraftStateToFile(nested_path));
    REQUIRE(std::filesystem::exists(nested_path));
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["last_io"]["operation"] == "save_draft_state");
    REQUIRE(exported["active_tab_detail"]["last_io"]["success"] == true);
    REQUIRE(exported["active_tab_detail"]["last_io"]["path"] == (temp_root / "nested" / "drafts" / "saved.json").generic_string());

    const auto invalid_path = temp_root / "nested" / "drafts" / "invalid.json";
    {
        std::ofstream invalid(invalid_path);
        invalid << "{not-json";
    }
    REQUIRE_FALSE(workspace.loadAbilityDraftStateFromFile(invalid_path.string()));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["draft"]["ability_id"] == "skill.valid_before_failed_load");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_attribute"] == "MagicDefense");
    REQUIRE(exported["active_tab_detail"]["last_io"]["operation"] == "load_draft_state");
    REQUIRE(exported["active_tab_detail"]["last_io"]["success"] == false);
    REQUIRE(exported["active_tab_detail"]["last_io"]["path"] == invalid_path.generic_string());
    REQUIRE(exported["active_tab_detail"]["last_io"]["message"].get<std::string>().find(invalid_path.generic_string()) !=
            std::string::npos);

    std::filesystem::remove_all(temp_root);
}

TEST_CASE("DiagnosticsWorkspace - authored ability can be applied to a live battle participant runtime",
          "[editor][diagnostics][integration][abilities][battle_apply]") {
    urpg::scene::BattleScene battle({"1"});
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle.getParticipants());
    auto* hero = findParticipant(participants, false);
    REQUIRE(hero != nullptr);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.bindAbilityRuntime(hero->abilitySystem);

    workspace.setAbilityDraftId("skill.battle_authored");
    workspace.setAbilityDraftCooldownSeconds(5.0f);
    workspace.setAbilityDraftMpCost(4.0f);
    workspace.setAbilityDraftEffectId("effect.battle_focus");
    workspace.setAbilityDraftEffectValue(12.0f);
    workspace.setAbilityDraftEffectDuration(7.0f);

    REQUIRE(workspace.applyAbilityDraftToRuntime());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.battle_authored");
    REQUIRE(exported["active_tab_detail"]["selected_ability_id"] == "skill.battle_authored");

    hero->abilitySystem.setAttribute("MP", 20.0f);
    hero->abilitySystem.setAttribute("Attack", 100.0f);
    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.battle_authored");
    REQUIRE(hero->abilitySystem.getAttribute("MP", 0.0f) == 16.0f);
    REQUIRE(hero->abilitySystem.getAttribute("Attack", 0.0f) == 112.0f);
}

TEST_CASE("DiagnosticsWorkspace - authored ability can be applied to a live map scene runtime",
          "[editor][diagnostics][integration][abilities][map_apply]") {
    urpg::scene::MapScene map("001", 10, 10);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.bindAbilityRuntime(map.playerAbilitySystem());

    workspace.setAbilityDraftId("skill.map_authored");
    workspace.setAbilityDraftCooldownSeconds(4.0f);
    workspace.setAbilityDraftMpCost(6.0f);
    workspace.setAbilityDraftEffectId("effect.map_guard");
    workspace.setAbilityDraftEffectAttribute("Defense");
    workspace.setAbilityDraftEffectValue(15.0f);
    workspace.setAbilityDraftEffectDuration(8.0f);
    workspace.applyAbilityDraftPatternPreset("skill_cross_small");
    workspace.setAbilityDraftPatternName("Map Guard Pattern");

    REQUIRE(workspace.applyAbilityDraftToRuntime());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.map_authored");
    REQUIRE(exported["active_tab_detail"]["draft"]["pattern_preview"]["name"] == "Map Guard Pattern");

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    REQUIRE(workspace.previewSelectedAbility());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["latest_ability_id"] == "skill.map_authored");
    REQUIRE(map.playerAbilitySystem().getAttribute("MP", 0.0f) == 24.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 115.0f);
}

TEST_CASE("DiagnosticsWorkspace - project ability content is discoverable and loadable through the picker",
          "[editor][diagnostics][integration][abilities][content_picker]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_workspace_ability_content_picker";
    std::filesystem::remove_all(project_root);
    std::filesystem::create_directories(project_root / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset fire;
    fire.ability_id = "skill.content_fire";
    fire.effect_id = "effect.content_fire";
    fire.effect_attribute = "Attack";
    fire.effect_value = 18.0f;
    fire.pattern.setName("Fire Pattern");
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(fire, project_root / "content" / "abilities" / "fire.json"));

    urpg::ability::AuthoredAbilityAsset guard;
    guard.ability_id = "skill.content_guard";
    guard.effect_id = "effect.content_guard";
    guard.effect_attribute = "Defense";
    guard.effect_operation = urpg::ModifierOp::Override;
    guard.effect_value = 55.0f;
    guard.pattern.setName("Guard Pattern");
    REQUIRE(
        urpg::ability::saveAuthoredAbilityAssetToFile(guard, project_root / "content" / "abilities" / "guard.json"));

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    REQUIRE(workspace.setAbilityProjectRoot(project_root.string()));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["project_content"]["canonical_directory"] ==
            (project_root / "content" / "abilities").generic_string());
    REQUIRE(exported["active_tab_detail"]["project_content"]["asset_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][0]["ability_id"] == "skill.content_fire");
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["ability_id"] == "skill.content_guard");

    REQUIRE(workspace.selectAbilityProjectAsset(1));
    REQUIRE(workspace.loadSelectedAbilityProjectAsset());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["draft"]["ability_id"] == "skill.content_guard");
    REQUIRE(exported["active_tab_detail"]["draft"]["effect_attribute"] == "Defense");
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["selected"] == true);

    std::filesystem::remove_all(project_root);
}

TEST_CASE(
    "DiagnosticsWorkspace - selected project ability asset can bind into a live map runtime and save back to content",
    "[editor][diagnostics][integration][abilities][content_binding][map]") {
    const auto project_root = std::filesystem::temp_directory_path() / "urpg_workspace_ability_content_binding";
    std::filesystem::remove_all(project_root);
    std::filesystem::create_directories(project_root / "content" / "abilities");

    urpg::ability::AuthoredAbilityAsset mapAsset;
    mapAsset.ability_id = "skill.content_map";
    mapAsset.mp_cost = 7.0f;
    mapAsset.effect_id = "effect.content_map";
    mapAsset.effect_attribute = "Defense";
    mapAsset.effect_value = 21.0f;
    mapAsset.pattern.setName("Map Content Pattern");
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(mapAsset,
                                                          project_root / "content" / "abilities" / "map_guard.json"));

    urpg::scene::MapScene map("001", 10, 10);
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Abilities);
    workspace.bindAbilityRuntime(map.playerAbilitySystem());
    REQUIRE(workspace.setAbilityProjectRoot(project_root.string()));
    REQUIRE(workspace.applySelectedAbilityProjectAssetToRuntime());

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["abilities"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["abilities"][0]["name"] == "skill.content_map");
    REQUIRE(exported["active_tab_detail"]["project_content"]["can_apply_selected"] == true);

    map.playerAbilitySystem().setAttribute("MP", 30.0f);
    map.playerAbilitySystem().setAttribute("Defense", 100.0f);
    REQUIRE(workspace.previewSelectedAbility());
    REQUIRE(map.playerAbilitySystem().getAttribute("MP", 0.0f) == 23.0f);
    REQUIRE(map.playerAbilitySystem().getAttribute("Defense", 0.0f) == 121.0f);

    REQUIRE(workspace.setAbilityDraftId("skill.content_saved"));
    REQUIRE(workspace.setAbilityDraftEffectId("effect.content_saved"));
    REQUIRE(workspace.setAbilityDraftEffectAttribute("MagicDefense"));
    REQUIRE(workspace.setAbilityDraftEffectOperation(urpg::ModifierOp::Override));
    REQUIRE(workspace.setAbilityDraftEffectValue(64.0f));
    REQUIRE(workspace.saveAbilityDraftToProjectContent("saved/content_saved.json"));
    REQUIRE(std::filesystem::exists(project_root / "content" / "abilities" / "saved" / "content_saved.json"));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["last_io"]["operation"] == "save_project_asset");
    REQUIRE(exported["active_tab_detail"]["last_io"]["success"] == true);
    REQUIRE(exported["active_tab_detail"]["last_io"]["path"] ==
            (project_root / "content" / "abilities" / "saved" / "content_saved.json").generic_string());
    REQUIRE(exported["active_tab_detail"]["project_content"]["asset_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["relative_path"] ==
            "content/abilities/saved/content_saved.json");
    REQUIRE(exported["active_tab_detail"]["project_content"]["assets"][1]["selected"] == true);

    std::filesystem::remove_all(project_root);
}
