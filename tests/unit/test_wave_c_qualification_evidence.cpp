#include "engine/core/events/event_static_analyzer.h"
#include "engine/core/events/native_event_command_matrix.h"
#include "engine/core/narrative/branching_quest_document.h"
#include "engine/core/narrative/branching_quest_project_service.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace {

nlohmann::json loadJson(const std::filesystem::path& relative_path) {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / relative_path);
    REQUIRE(input.is_open());
    return nlohmann::json::parse(input);
}

std::set<std::string> strings(const nlohmann::json& values) {
    return values.get<std::set<std::string>>();
}

urpg::events::EventDocument makeBadEventCorpus() {
    using namespace urpg::events;
    EventDocument document;
    document.addMap({"town", 10, 10});
    document.addMap({"dead_end", 10, 10});
    document.setKnownSwitches({"known"});
    EventPage bad;
    bad.id = "bad";
    bad.trigger = EventTrigger::Autorun;
    bad.commands = {
        {"parallel_a", EventCommandKind::Switch, "known", "true", 0, {}, {}, {{"parallel_lane", true}}},
        {"parallel_b", EventCommandKind::Switch, "known", "false", 0, {}, {}, {{"parallel_lane", true}}},
        {"loop", EventCommandKind::Loop},
        {"loop_end", EventCommandKind::EndLoop},
        {"voice", EventCommandKind::Message, "", "", 0, {}, {}, {{"voice_id", "voice.guide"}}},
        {"transfer", EventCommandKind::Transfer, "dead_end", "", 0, {}, {}, {{"terminal", true}}},
        {"unknown", EventCommandKind::Switch, "missing", "true"},
        {"unreachable", EventCommandKind::Gold, "", "", 10}};
    document.addEvent({"event.bad", "town", 1, 1, {bad}});
    EventPage softlock;
    softlock.id = "autorun";
    softlock.trigger = EventTrigger::Autorun;
    softlock.commands = {{"text", EventCommandKind::Message, "", "", 0, {}, {},
                          {{"localization_id", "softlock.text"}}}};
    document.addEvent({"event.softlock", "town", 2, 1, {softlock}});
    return document;
}

} // namespace

TEST_CASE("Wave C event evidence stays synchronized with runtime and curated diagnostics",
          "[wave_c][qualification][events]") {
    const auto matrix = loadJson("content/readiness/event_command_runtime_diagnostic_matrix.json");
    REQUIRE(matrix.at("schema") == "urpg.event_command_runtime_diagnostic_matrix.v1");
    const auto& native = urpg::events::nativeEventCommandMatrix();
    REQUIRE(matrix.at("commands").size() == native.size());
    for (const auto& capability : native) {
        const auto row = std::ranges::find_if(matrix.at("commands"), [&](const auto& candidate) {
            return candidate.at("id") == capability.id;
        });
        REQUIRE(row != matrix.at("commands").end());
        REQUIRE(row->at("authoring") == capability.authoring_ui);
        REQUIRE(row->at("serialization") == capability.serialization_key);
        REQUIRE(row->at("runtime") == capability.runtime_effect);
        REQUIRE(row->at("diagnostics") == capability.diagnostic_codes);
        REQUIRE(row->at("undo") == capability.undo_supported);
        REQUIRE(row->value("controlledCall", false) == capability.controlled_call);
    }

    const auto corpus = loadJson("content/fixtures/event_static_analysis_corpus.json");
    REQUIRE(corpus.at("schema") == "urpg.event_static_analysis_corpus.v1");
    const auto bad_case = std::ranges::find_if(corpus.at("cases"), [](const auto& candidate) {
        return candidate.at("id") == "all_requested_hazards";
    });
    REQUIRE(bad_case != corpus.at("cases").end());
    std::set<std::string> actual_codes;
    for (const auto& finding : urpg::events::analyzeEventDocument(makeBadEventCorpus())) {
        actual_codes.insert(finding.code);
    }
    REQUIRE(actual_codes == strings(bad_case->at("expectedCodes")));
}

TEST_CASE("Wave C governed branching quest fixture executes and preserves package closure",
          "[wave_c][qualification][narrative]") {
    const auto fixture = loadJson("content/fixtures/branching_voiced_quest_fixture.json");
    bool migrated = true;
    const auto document = urpg::narrative::BranchingQuestDocument::fromJson(fixture, &migrated);
    REQUIRE(document.has_value());
    REQUIRE_FALSE(migrated);
    REQUIRE(document->validate().empty());
    REQUIRE(urpg::narrative::packageClosureToJson(document->packageClosure()) == fixture.at("package_closure"));

    urpg::narrative::BranchingQuestRuntime runtime;
    REQUIRE(runtime.start(*document));
    runtime.dialogue_values["reputation"] = 2;
    REQUIRE(runtime.choose(*document, "accept_quest"));
    const auto progress = runtime.advanceQuest(*document, {}, "2026-07-17T00:00:00Z");
    REQUIRE(progress.completed_objective_ids == std::vector<std::string>{"find_echo"});
    const auto reopened = urpg::narrative::BranchingQuestRuntime::load(runtime.save());
    REQUIRE(reopened.has_value());
    REQUIRE(reopened->dialogue_node_id == "accept");
}

TEST_CASE("Wave C branching quest round trips through project and installed package owners",
          "[wave_c][qualification][narrative][package]") {
    const auto fixture = loadJson("content/fixtures/branching_voiced_quest_fixture.json");
    const auto document = urpg::narrative::BranchingQuestDocument::fromJson(fixture);
    REQUIRE(document.has_value());
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg-wave-c-narrative-" +
                       std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto project = root / "project";
    const auto package = root / "package";
    urpg::narrative::BranchingQuestProjectService service;
    const auto saved = service.save(project, *document);
    REQUIRE(saved.success);
    REQUIRE(std::filesystem::exists(saved.document_path));
    const auto reopened = service.load(project, document->id);
    REQUIRE(reopened.has_value());
    REQUIRE(reopened->toJson() == document->toJson());

    const auto published = service.publishPackage(package, *reopened);
    REQUIRE(published.success);
    REQUIRE(std::filesystem::exists(published.document_path));
    REQUIRE(std::filesystem::exists(published.closure_path));
    const auto installed = service.loadInstalledPackage(package, document->id);
    REQUIRE(installed.has_value());
    urpg::narrative::BranchingQuestRuntime runtime;
    REQUIRE(runtime.start(*installed));
    std::filesystem::remove_all(root);
}

TEST_CASE("Wave C database menu and semantic matrices cover the release operation sets",
          "[wave_c][qualification][database][menu][pcq654]") {
    const auto database = loadJson("content/readiness/database_qualification_matrix.json");
    REQUIRE(database.at("recordKinds").size() == 13);
    REQUIRE(database.at("referenceOperations").at("operations").size() == 3);
    REQUIRE(database.at("referenceOperations").at("requiresTypedRecordOwner") == true);
    REQUIRE(database.at("referenceOperations").at("requiresEveryAffectedDocumentOwner") == true);
    REQUIRE(database.at("referenceOperations").at("requiresVisibleImpactReview") == true);
    REQUIRE(database.at("referenceOperations").at("requiresSaveReopen") == true);
    REQUIRE(database.at("referenceOperations").at("requiresNoDanglingReference") == true);
    REQUIRE(database.at("referenceOperations").at("history").size() == 2);
    REQUIRE(database.at("largeFixture").at("rows") >= 50000);
    REQUIRE(database.at("largeFixture").at("maximumVisibleRows") <= 128);
    REQUIRE(database.at("atomicInvalidBatch").at("requiresCompletePreview") == true);
    REQUIRE(database.at("atomicInvalidBatch").at("requiresZeroMutation") == true);
    REQUIRE(database.at("integrationDomains").size() == 9);
    REQUIRE_FALSE(database.at("seededBalanceReports").empty());

    const auto menu = loadJson("content/readiness/menu_authoring_qualification_matrix.json");
    REQUIRE(menu.at("viewports").size() == 5);
    REQUIRE(menu.at("states").size() == 7);
    REQUIRE(menu.at("bindingSources").size() == 4);
    REQUIRE(menu.at("templates").size() == 11);

    const auto semantic = loadJson("content/readiness/semantic_editor_operation_matrix.json");
    REQUIRE(strings(semantic.at("inputRoutes")) == std::set<std::string>{"controller", "keyboard"});
    REQUIRE(semantic.at("editors").size() == 5);
    for (const auto& editor : semantic.at("editors")) {
        const auto operations = strings(editor.at("operations"));
        for (const auto& required : {"ordered_navigation", "select", "edit_properties", "focus_diagnostic"}) {
            REQUIRE(operations.contains(required));
        }
        if (editor.at("id") != "tile_map") REQUIRE(operations.contains("create_connection"));
    }
}
