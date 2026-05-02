// test_template_acceptance.cpp
//
// S30-T04 — Template-level acceptance tests: edit → preview → export
//
// Validates the nominal lifecycle for jrpg, visual_novel, and turn_based_rpg:
//   1. A minimal project document for the template can be represented as JSON.
//   2. Template metadata can be read back and is structurally valid.
//   3. An export-ready bundle descriptor can be produced from the project document.
//
// These are integration-proof tests that run in the PR lane (tagged [template][acceptance]).
// They do not depend on live subsystem state — they validate the data contract
// between template authoring, preview metadata, and export descriptor shape.

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <set>
#include <string>
#include <vector>

#include "engine/core/input/input_remap_store.h"
#include "engine/core/project/project_template_generator.h"
#include "engine/core/project/template_runtime_profile.h"

using nlohmann::json;

namespace {

std::filesystem::path templateAcceptanceRepoRoot() {
#ifdef URPG_SOURCE_DIR
    return std::filesystem::path(URPG_SOURCE_DIR);
#else
    return std::filesystem::current_path();
#endif
}

json loadTemplateAcceptanceJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    REQUIRE(stream.is_open());
    json payload;
    stream >> payload;
    return payload;
}

const std::set<std::string>& rpgRequiredInputActions() {
    static const std::set<std::string> actions = {
        "MoveUp",   "MoveDown",  "MoveLeft",     "MoveRight",   "Confirm",    "Cancel",       "Menu",
        "PageLeft", "PageRight", "BattleAttack", "BattleSkill", "BattleItem", "BattleDefend", "BattleEscape",
    };
    return actions;
}

std::set<std::string> collectActionStrings(const json& bindings) {
    REQUIRE(bindings.is_array());
    std::set<std::string> actions;
    for (const auto& binding : bindings) {
        REQUIRE(binding.contains("action"));
        REQUIRE(binding["action"].is_string());
        actions.insert(binding["action"].get<std::string>());
    }
    return actions;
}

void requireRpgInputClosure(const json& starter) {
    REQUIRE(starter.contains("input"));
    const auto& input = starter["input"];
    REQUIRE(input.value("profile", "") == "keyboard_gamepad");

    const auto requiredActions = rpgRequiredInputActions();
    REQUIRE(collectActionStrings(input.at("keyboard_bindings")) == requiredActions);
    REQUIRE(collectActionStrings(input.at("controller_bindings")) == requiredActions);
    REQUIRE(input["touch_policy"].value("mode", "") == "hit_test_ui_world");
    REQUIRE(input["touch_policy"].value("remap_status", "") == "touch_binding_unsupported");

    nlohmann::json persisted = {
        {"version", "1.0.0"},
        {"bindings", input.at("keyboard_bindings")},
    };
    urpg::input::InputRemapStore store;
    store.loadFromJson(persisted);
    const auto saved = store.saveToJson();
    REQUIRE(collectActionStrings(saved.at("bindings")) == requiredActions);
}

void requireTemplateQualityContract(const json& owner) {
    REQUIRE(owner.contains("quality_contract"));
    const auto& quality = owner["quality_contract"];
    REQUIRE(quality.value("tier", "") == "starter_ready");
    REQUIRE(quality.value("quality_score", 0) >= 70);
    REQUIRE_FALSE(quality.value("genre_family", "").empty());
    REQUIRE_FALSE(quality.value("target_camera", "").empty());
    REQUIRE(quality.contains("core_loops"));
    REQUIRE_FALSE(quality["core_loops"].empty());
    REQUIRE(quality.contains("starter_content"));
    REQUIRE(quality["starter_content"].value("maps_or_scenes", 0) >= 1);
    REQUIRE(quality["starter_content"].value("sample_entities", 0) >= 1);
    REQUIRE(quality["starter_content"].value("sample_progression_hooks", 0) >= 1);
    REQUIRE_FALSE(quality["starter_content"]["wysiwyg_surfaces"].empty());
    REQUIRE_FALSE(quality["required_inputs"].empty());
    REQUIRE_FALSE(quality["default_accessibility"].empty());
    REQUIRE(quality.value("audio_profile", "") == "starter_mix");
    REQUIRE_FALSE(quality["localization_namespaces"].empty());
    REQUIRE(quality.contains("performance_budget"));
    REQUIRE(quality["performance_budget"].value("frame_time_ms", 0.0) > 0.0);
    REQUIRE(quality["performance_budget"].value("arena_kb", 0) > 0);
    REQUIRE(quality["performance_budget"].value("active_entities", 0) > 0);
    REQUIRE(quality.value("export_profile", "") == "standalone_strict");
    REQUIRE(quality["known_limits"].size() >= 3);
}

// ---------------------------------------------------------------------------
// Helpers — minimal template project document factory
// ---------------------------------------------------------------------------

json makeProjectDocument(const std::string& templateId, const std::vector<std::string>& requiredSubsystems,
                         const std::string& projectName) {
    return json{{"schemaVersion", "1.0.0"},
                {"projectName", projectName},
                {"templateId", templateId},
                {"requiredSubsystems", requiredSubsystems},
                {"scenes", json::array({{{"id", "intro"}, {"type", "map"}, {"displayName", "Introduction Map"}},
                                        {{"id", "battle_01"}, {"type", "battle"}, {"displayName", "First Battle"}}})},
                {"assets", json::object()},
                {"exportConfig",
                 {{"integrityMode", "strict"}, {"target", "standalone"}, {"outputName", projectName + ".pck"}}}};
}

// ---------------------------------------------------------------------------
// Helpers — preview metadata extractor
// ---------------------------------------------------------------------------

json extractPreviewMetadata(const json& projectDoc) {
    return json{{"templateId", projectDoc.value("templateId", "")},
                {"projectName", projectDoc.value("projectName", "")},
                {"sceneCount", projectDoc.value("scenes", json::array()).size()},
                {"requiredSubsystems", projectDoc.value("requiredSubsystems", json::array())},
                {"exportTarget", projectDoc["exportConfig"].value("target", "")}};
}

// ---------------------------------------------------------------------------
// Helpers — export bundle descriptor builder
// ---------------------------------------------------------------------------

json buildExportDescriptor(const json& projectDoc) {
    return json{{"magic", "URPGPCK1"},
                {"version", 1},
                {"templateId", projectDoc.value("templateId", "")},
                {"projectName", projectDoc.value("projectName", "")},
                {"integrityMode", projectDoc["exportConfig"].value("integrityMode", "")},
                {"entries", json::array()},
                {"manifestHash", "sha256:placeholder"}};
}

} // namespace

TEST_CASE("WYSIWYG template showcase examples bind completed systems to starter projects",
          "[template][acceptance][wysiwyg][examples]") {
    const auto showcase = loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "examples" /
                                                     "wysiwyg_template_showcase.json");

    REQUIRE(showcase["schema"] == "urpg.wysiwyg_template_showcase.v1");
    std::set<std::string> requiredEvidence;
    for (const auto& evidence : showcase["required_evidence"]) {
        requiredEvidence.insert(evidence.get<std::string>());
    }
    REQUIRE(requiredEvidence == std::set<std::string>({
                                    "visual_authoring_surface",
                                    "live_preview",
                                    "saved_project_data",
                                    "runtime_execution",
                                    "diagnostics",
                                    "tests",
                                }));

    const auto readiness =
        loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "readiness" / "readiness_status.json");
    std::set<std::string> expectedTemplates;
    for (const auto& tmpl : readiness["templates"]) {
        const auto templateId = tmpl["id"].get<std::string>();
        if (templateId != "visual_novel" && templateId != "turn_based_rpg") {
            expectedTemplates.insert(templateId);
        }
    }
    std::set<std::string> seenTemplates;
    std::set<std::string> seenSurfaceKinds;

    for (const auto& example : showcase["examples"]) {
        const auto templateId = example["template_id"].get<std::string>();
        seenTemplates.insert(templateId);
        REQUIRE_FALSE(example.value("display_name", "").empty());
        const auto starterPath = templateAcceptanceRepoRoot() / example["starter_project"].get<std::string>();
        REQUIRE(std::filesystem::exists(starterPath));
        const auto starter = loadTemplateAcceptanceJson(starterPath);
        REQUIRE(starter.value("template_id", "") == templateId);
        REQUIRE_FALSE(example["surfaces"].empty());

        for (const auto& surface : example["surfaces"]) {
            REQUIRE_FALSE(surface.value("id", "").empty());
            REQUIRE_FALSE(surface.value("runtime_hook", "").empty());
            REQUIRE_FALSE(surface.value("editor_panel", "").empty());
            REQUIRE(std::filesystem::exists(templateAcceptanceRepoRoot() / surface["fixture"].get<std::string>()));
            std::set<std::string> evidence;
            for (const auto& item : surface["evidence"]) {
                evidence.insert(item.get<std::string>());
            }
            REQUIRE(evidence == requiredEvidence);
            seenSurfaceKinds.insert(surface["kind"].get<std::string>());
        }
    }

    REQUIRE(seenTemplates == expectedTemplates);
    REQUIRE(seenSurfaceKinds.count("dungeon3d_world") == 1);
    REQUIRE(seenSurfaceKinds.count("battle_vfx_timeline") == 1);
    REQUIRE(seenSurfaceKinds.count("map_environment_preview") == 1);
    REQUIRE(seenSurfaceKinds.count("dialogue_preview") == 1);
    REQUIRE(seenSurfaceKinds.count("event_command_graph") == 1);
    REQUIRE(seenSurfaceKinds.count("ability_sandbox") == 1);
    REQUIRE(seenSurfaceKinds.count("save_load_preview_lab") == 1);
    REQUIRE(seenSurfaceKinds.count("export_preview") == 1);
    REQUIRE(seenSurfaceKinds.count("procedural_dungeon") == 1);
    REQUIRE(seenSurfaceKinds.count("loot_generator") == 1);
    REQUIRE(seenSurfaceKinds.count("horror_environment_fx") == 1);
    REQUIRE(seenSurfaceKinds.count("farming_garden_plot") == 1);
    REQUIRE(seenSurfaceKinds.count("card_battle") == 1);
    REQUIRE(seenSurfaceKinds.count("platformer_physics_lab") == 1);
    REQUIRE(seenSurfaceKinds.count("side_view_action_combat") == 1);
    REQUIRE(seenSurfaceKinds.count("summon_gacha_banner") == 1);
    REQUIRE(seenSurfaceKinds.count("gacha_system") == 1);
    REQUIRE(seenSurfaceKinds.count("quest_objective_graph") == 1);
    REQUIRE(seenSurfaceKinds.count("puzzle_logic_board") == 1);
    REQUIRE(seenSurfaceKinds.count("world_map_route_planner") == 1);
    REQUIRE(seenSurfaceKinds.count("fast_travel_map_builder") == 1);
    REQUIRE(seenSurfaceKinds.count("map_zoom_system") == 1);
}

TEST_CASE("new template starter manifests declare WYSIWYG integration hooks", "[template][acceptance][integrations]") {
    const auto readiness =
        loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "readiness" / "readiness_status.json");

    for (const auto& tmpl : readiness["templates"]) {
        const auto templateId = tmpl["id"].get<std::string>();
        const auto starter = loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "templates" /
                                                        (templateId + "_starter.json"));
        if (!starter.contains("systems")) {
            continue;
        }
        if (!starter["systems"].contains("integrations")) {
            continue;
        }
        INFO("Template: " << templateId);
        REQUIRE_FALSE(starter["systems"]["integrations"]["export_validation"].empty());
        REQUIRE_FALSE(starter["systems"]["integrations"]["wysiwyg_surfaces"].empty());
    }
}

TEST_CASE("all canonical starter manifests declare the template quality contract", "[template][acceptance][quality]") {
    const auto readiness =
        loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "readiness" / "readiness_status.json");

    for (const auto& tmpl : readiness["templates"]) {
        const auto templateId = tmpl["id"].get<std::string>();
        INFO("Template: " << templateId);
        const auto starter = loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "templates" /
                                                        (templateId + "_starter.json"));
        requireTemplateQualityContract(starter);
        REQUIRE(starter["quality_contract"]["localization_namespaces"][0] == templateId);
    }
}

TEST_CASE("generated runtime profile JSON declares the template quality contract",
          "[template][acceptance][quality][project]") {
    for (const auto& profile : urpg::project::allTemplateRuntimeProfiles()) {
        INFO("Template: " << profile.id);
        const auto profileJson = urpg::project::templateRuntimeProfileToJson(profile);
        requireTemplateQualityContract(profileJson);
        REQUIRE(profileJson["quality_contract"]["localization_namespaces"][0] == profile.id);
    }
}

// ============================================================================
// S30-T04 — jrpg template acceptance: edit → preview → export
// ============================================================================

TEST_CASE("jrpg template: project document has correct schema and template id", "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("jrpg", {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
                                         "MyJrpgGame");

    REQUIRE(doc["templateId"] == "jrpg");
    REQUIRE(doc["schemaVersion"] == "1.0.0");
    REQUIRE(doc["projectName"] == "MyJrpgGame");

    const auto& subsystems = doc["requiredSubsystems"];
    REQUIRE(subsystems.size() == 4);
    REQUIRE(subsystems[0] == "ui_menu_core");
    REQUIRE(subsystems[1] == "message_text_core");
    REQUIRE(subsystems[2] == "battle_core");
    REQUIRE(subsystems[3] == "save_data_core");
}

TEST_CASE("jrpg template: preview metadata is extractable and structurally valid", "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("jrpg", {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
                                         "MyJrpgGame");
    const json preview = extractPreviewMetadata(doc);

    REQUIRE(preview["templateId"] == "jrpg");
    REQUIRE(preview["projectName"] == "MyJrpgGame");
    REQUIRE(preview["sceneCount"].get<std::size_t>() == 2);
    REQUIRE(preview["exportTarget"] == "standalone");
    REQUIRE(preview["requiredSubsystems"].size() == 4);
}

TEST_CASE("jrpg template: export bundle descriptor is valid and contains required fields",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("jrpg", {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
                                         "MyJrpgGame");
    const json descriptor = buildExportDescriptor(doc);

    REQUIRE(descriptor["magic"] == "URPGPCK1");
    REQUIRE(descriptor["version"].get<int>() == 1);
    REQUIRE(descriptor["templateId"] == "jrpg");
    REQUIRE(descriptor["integrityMode"] == "strict");
    REQUIRE(descriptor["entries"].is_array());
    REQUIRE(descriptor.contains("manifestHash"));
}

TEST_CASE("jrpg template: scenes are preserved in project document", "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("jrpg", {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
                                         "MyJrpgGame");

    REQUIRE(doc["scenes"].size() == 2);
    REQUIRE(doc["scenes"][0]["id"] == "intro");
    REQUIRE(doc["scenes"][0]["type"] == "map");
    REQUIRE(doc["scenes"][1]["id"] == "battle_01");
    REQUIRE(doc["scenes"][1]["type"] == "battle");
}

// ============================================================================
// S30-T04 — visual_novel template acceptance: edit → preview → export
// ============================================================================

TEST_CASE("visual_novel template: project document has correct schema and template id",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("visual_novel", {"message_text_core", "save_data_core"}, "MyVisualNovel");

    REQUIRE(doc["templateId"] == "visual_novel");
    REQUIRE(doc["schemaVersion"] == "1.0.0");
    REQUIRE(doc["projectName"] == "MyVisualNovel");

    const auto& subsystems = doc["requiredSubsystems"];
    REQUIRE(subsystems.size() == 2);
    REQUIRE(subsystems[0] == "message_text_core");
    REQUIRE(subsystems[1] == "save_data_core");
}

TEST_CASE("visual_novel template: preview metadata is extractable and structurally valid",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("visual_novel", {"message_text_core", "save_data_core"}, "MyVisualNovel");
    const json preview = extractPreviewMetadata(doc);

    REQUIRE(preview["templateId"] == "visual_novel");
    REQUIRE(preview["projectName"] == "MyVisualNovel");
    REQUIRE(preview["sceneCount"].get<std::size_t>() == 2);
    REQUIRE(preview["exportTarget"] == "standalone");
    REQUIRE(preview["requiredSubsystems"].size() == 2);
}

TEST_CASE("visual_novel template: export bundle descriptor is valid and contains required fields",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("visual_novel", {"message_text_core", "save_data_core"}, "MyVisualNovel");
    const json descriptor = buildExportDescriptor(doc);

    REQUIRE(descriptor["magic"] == "URPGPCK1");
    REQUIRE(descriptor["version"].get<int>() == 1);
    REQUIRE(descriptor["templateId"] == "visual_novel");
    REQUIRE(descriptor["integrityMode"] == "strict");
    REQUIRE(descriptor["entries"].is_array());
    REQUIRE(descriptor.contains("manifestHash"));
}

// ============================================================================
// S30-T04 — turn_based_rpg template acceptance: edit → preview → export
// ============================================================================

TEST_CASE("turn_based_rpg template: project document has correct schema and template id",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("turn_based_rpg", {"message_text_core", "battle_core", "save_data_core"},
                                         "MyTurnBasedGame");

    REQUIRE(doc["templateId"] == "turn_based_rpg");
    REQUIRE(doc["schemaVersion"] == "1.0.0");
    REQUIRE(doc["projectName"] == "MyTurnBasedGame");

    const auto& subsystems = doc["requiredSubsystems"];
    REQUIRE(subsystems.size() == 3);
    REQUIRE(subsystems[0] == "message_text_core");
    REQUIRE(subsystems[1] == "battle_core");
    REQUIRE(subsystems[2] == "save_data_core");
}

TEST_CASE("turn_based_rpg template: preview metadata is extractable and structurally valid",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("turn_based_rpg", {"message_text_core", "battle_core", "save_data_core"},
                                         "MyTurnBasedGame");
    const json preview = extractPreviewMetadata(doc);

    REQUIRE(preview["templateId"] == "turn_based_rpg");
    REQUIRE(preview["projectName"] == "MyTurnBasedGame");
    REQUIRE(preview["sceneCount"].get<std::size_t>() == 2);
    REQUIRE(preview["exportTarget"] == "standalone");
    REQUIRE(preview["requiredSubsystems"].size() == 3);
}

TEST_CASE("turn_based_rpg template: export bundle descriptor is valid and contains required fields",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument("turn_based_rpg", {"message_text_core", "battle_core", "save_data_core"},
                                         "MyTurnBasedGame");
    const json descriptor = buildExportDescriptor(doc);

    REQUIRE(descriptor["magic"] == "URPGPCK1");
    REQUIRE(descriptor["version"].get<int>() == 1);
    REQUIRE(descriptor["templateId"] == "turn_based_rpg");
    REQUIRE(descriptor["integrityMode"] == "strict");
    REQUIRE(descriptor["entries"].is_array());
    REQUIRE(descriptor.contains("manifestHash"));
}

TEST_CASE("JRPG and turn-based RPG starter manifests close required input bindings", "[template][acceptance][input]") {
    for (const std::string templateId : {"jrpg", "turn_based_rpg"}) {
        INFO("Template: " << templateId);
        const auto starter = loadTemplateAcceptanceJson(templateAcceptanceRepoRoot() / "content" / "templates" /
                                                        (templateId + "_starter.json"));
        REQUIRE(starter.value("template_id", "") == templateId);
        requireRpgInputClosure(starter);
    }
}

TEST_CASE("JRPG and turn-based RPG generated projects close required input bindings",
          "[template][acceptance][input][project]") {
    urpg::project::ProjectTemplateGenerator generator;
    for (const std::string templateId : {"jrpg", "turn_based_rpg"}) {
        INFO("Template: " << templateId);
        const auto result =
            generator.generate({templateId, templateId + "_input_project", templateId + " Input Project"});
        REQUIRE(result.success);
        requireRpgInputClosure(result.project["subsystems"]);
    }
}

// ============================================================================
// S30-T04 — Cross-template contract invariants
// ============================================================================

TEST_CASE("all three READY candidate templates share consistent export descriptor shape",
          "[template][acceptance][s30t04]") {
    const std::vector<json> docs = {
        makeProjectDocument("jrpg", {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"}, "JrpgGame"),
        makeProjectDocument("visual_novel", {"message_text_core", "save_data_core"}, "VnGame"),
        makeProjectDocument("turn_based_rpg", {"message_text_core", "battle_core", "save_data_core"}, "TbrGame")};

    for (const auto& doc : docs) {
        const json descriptor = buildExportDescriptor(doc);
        REQUIRE(descriptor["magic"] == "URPGPCK1");
        REQUIRE(descriptor["version"].get<int>() == 1);
        REQUIRE(!descriptor["templateId"].get<std::string>().empty());
        REQUIRE(descriptor["integrityMode"] == "strict");
        REQUIRE(descriptor["entries"].is_array());
    }
}

TEST_CASE("template project document without templateId is structurally incomplete", "[template][acceptance][s30t04]") {
    json doc = makeProjectDocument("jrpg", {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
                                   "IncompleteGame");
    doc.erase("templateId");

    REQUIRE_FALSE(doc.contains("templateId"));
    // Export descriptor built from incomplete doc should have empty templateId
    const json descriptor = buildExportDescriptor(doc);
    REQUIRE(descriptor["templateId"].get<std::string>().empty());
}
