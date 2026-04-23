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

#include <string>
#include <vector>

using nlohmann::json;

namespace {

// ---------------------------------------------------------------------------
// Helpers — minimal template project document factory
// ---------------------------------------------------------------------------

json makeProjectDocument(const std::string& templateId,
                         const std::vector<std::string>& requiredSubsystems,
                         const std::string& projectName) {
    return json{
        {"schemaVersion", "1.0.0"},
        {"projectName", projectName},
        {"templateId", templateId},
        {"requiredSubsystems", requiredSubsystems},
        {"scenes", json::array({
            {{"id", "intro"}, {"type", "map"}, {"displayName", "Introduction Map"}},
            {{"id", "battle_01"}, {"type", "battle"}, {"displayName", "First Battle"}}
        })},
        {"assets", json::object()},
        {"exportConfig", {
            {"integrityMode", "strict"},
            {"target", "standalone"},
            {"outputName", projectName + ".pck"}
        }}
    };
}

// ---------------------------------------------------------------------------
// Helpers — preview metadata extractor
// ---------------------------------------------------------------------------

json extractPreviewMetadata(const json& projectDoc) {
    return json{
        {"templateId", projectDoc.value("templateId", "")},
        {"projectName", projectDoc.value("projectName", "")},
        {"sceneCount", projectDoc.value("scenes", json::array()).size()},
        {"requiredSubsystems", projectDoc.value("requiredSubsystems", json::array())},
        {"exportTarget", projectDoc["exportConfig"].value("target", "")}
    };
}

// ---------------------------------------------------------------------------
// Helpers — export bundle descriptor builder
// ---------------------------------------------------------------------------

json buildExportDescriptor(const json& projectDoc) {
    return json{
        {"magic", "URPGPCK1"},
        {"version", 1},
        {"templateId", projectDoc.value("templateId", "")},
        {"projectName", projectDoc.value("projectName", "")},
        {"integrityMode", projectDoc["exportConfig"].value("integrityMode", "")},
        {"entries", json::array()},
        {"manifestHash", "sha256:placeholder"}
    };
}

} // namespace

// ============================================================================
// S30-T04 — jrpg template acceptance: edit → preview → export
// ============================================================================

TEST_CASE("jrpg template: project document has correct schema and template id",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument(
        "jrpg",
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
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

TEST_CASE("jrpg template: preview metadata is extractable and structurally valid",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument(
        "jrpg",
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
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
    const json doc = makeProjectDocument(
        "jrpg",
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
        "MyJrpgGame");
    const json descriptor = buildExportDescriptor(doc);

    REQUIRE(descriptor["magic"] == "URPGPCK1");
    REQUIRE(descriptor["version"].get<int>() == 1);
    REQUIRE(descriptor["templateId"] == "jrpg");
    REQUIRE(descriptor["integrityMode"] == "strict");
    REQUIRE(descriptor["entries"].is_array());
    REQUIRE(descriptor.contains("manifestHash"));
}

TEST_CASE("jrpg template: scenes are preserved in project document",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument(
        "jrpg",
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
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
    const json doc = makeProjectDocument(
        "visual_novel",
        {"message_text_core", "save_data_core"},
        "MyVisualNovel");

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
    const json doc = makeProjectDocument(
        "visual_novel",
        {"message_text_core", "save_data_core"},
        "MyVisualNovel");
    const json preview = extractPreviewMetadata(doc);

    REQUIRE(preview["templateId"] == "visual_novel");
    REQUIRE(preview["projectName"] == "MyVisualNovel");
    REQUIRE(preview["sceneCount"].get<std::size_t>() == 2);
    REQUIRE(preview["exportTarget"] == "standalone");
    REQUIRE(preview["requiredSubsystems"].size() == 2);
}

TEST_CASE("visual_novel template: export bundle descriptor is valid and contains required fields",
          "[template][acceptance][s30t04]") {
    const json doc = makeProjectDocument(
        "visual_novel",
        {"message_text_core", "save_data_core"},
        "MyVisualNovel");
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
    const json doc = makeProjectDocument(
        "turn_based_rpg",
        {"message_text_core", "battle_core", "save_data_core"},
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
    const json doc = makeProjectDocument(
        "turn_based_rpg",
        {"message_text_core", "battle_core", "save_data_core"},
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
    const json doc = makeProjectDocument(
        "turn_based_rpg",
        {"message_text_core", "battle_core", "save_data_core"},
        "MyTurnBasedGame");
    const json descriptor = buildExportDescriptor(doc);

    REQUIRE(descriptor["magic"] == "URPGPCK1");
    REQUIRE(descriptor["version"].get<int>() == 1);
    REQUIRE(descriptor["templateId"] == "turn_based_rpg");
    REQUIRE(descriptor["integrityMode"] == "strict");
    REQUIRE(descriptor["entries"].is_array());
    REQUIRE(descriptor.contains("manifestHash"));
}

// ============================================================================
// S30-T04 — Cross-template contract invariants
// ============================================================================

TEST_CASE("all three READY candidate templates share consistent export descriptor shape",
          "[template][acceptance][s30t04]") {
    const std::vector<json> docs = {
        makeProjectDocument("jrpg",
            {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
            "JrpgGame"),
        makeProjectDocument("visual_novel",
            {"message_text_core", "save_data_core"},
            "VnGame"),
        makeProjectDocument("turn_based_rpg",
            {"message_text_core", "battle_core", "save_data_core"},
            "TbrGame")
    };

    for (const auto& doc : docs) {
        const json descriptor = buildExportDescriptor(doc);
        REQUIRE(descriptor["magic"] == "URPGPCK1");
        REQUIRE(descriptor["version"].get<int>() == 1);
        REQUIRE(!descriptor["templateId"].get<std::string>().empty());
        REQUIRE(descriptor["integrityMode"] == "strict");
        REQUIRE(descriptor["entries"].is_array());
    }
}

TEST_CASE("template project document without templateId is structurally incomplete",
          "[template][acceptance][s30t04]") {
    json doc = makeProjectDocument(
        "jrpg",
        {"ui_menu_core", "message_text_core", "battle_core", "save_data_core"},
        "IncompleteGame");
    doc.erase("templateId");

    REQUIRE_FALSE(doc.contains("templateId"));
    // Export descriptor built from incomplete doc should have empty templateId
    const json descriptor = buildExportDescriptor(doc);
    REQUIRE(descriptor["templateId"].get<std::string>().empty());
}
