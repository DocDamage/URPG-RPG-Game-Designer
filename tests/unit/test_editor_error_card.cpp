#include "editor/diagnostics/editor_error_card.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

TEST_CASE("Editor errors expose complete actionable cards and redacted copy details", "[editor][diagnostics][error_card]") {
    urpg::editor::EditorErrorCard card{
        "project_open_failed", "The project could not be opened.", "Project Willow",
        "Editing cannot begin until the project is available.", "Locate the project folder or choose another project.",
        {"Locate Project", "startup.locate", true}, {"Retry Open", "startup.open", true},
        {{"project_path", "C:/Users/Ada/Secret/Willow"}, {"authorization", "Bearer private"},
         {"nested", {{"token", "abc"}, {"reason", "project.json is missing"}}}},
    };
    const auto validation = urpg::editor::validateEditorErrorCard(card);
    REQUIRE(validation.valid);
    REQUIRE(validation.issues.empty());
    const auto json = urpg::editor::editorErrorCardJson(card);
    REQUIRE(json["schema"] == "urpg.editor_error.v1");
    REQUIRE(json["go_to"]["route"] == "startup.locate");
    REQUIRE(json["retry"]["enabled"] == true);
    REQUIRE(json["copy_details"] == true);
    const auto restored = urpg::editor::editorErrorCardFromJson(json);
    REQUIRE(restored.has_value());
    REQUIRE(restored->code == card.code);
    REQUIRE(restored->go_to.route == "startup.locate");

    const auto support = urpg::editor::redactedEditorErrorSupportExport(card);
    REQUIRE(support["details"]["project_path"] == "[redacted]");
    REQUIRE(support["details"]["authorization"] == "[redacted]");
    REQUIRE(support["details"]["nested"]["token"] == "[redacted]");
    REQUIRE(support["details"]["nested"]["reason"] == "project.json is missing");
    const auto copied = urpg::editor::copyEditorErrorDetails(card);
    REQUIRE(copied.find("Bearer private") == std::string::npos);
    REQUIRE(copied.find("C:/Users") == std::string::npos);
}

TEST_CASE("Editor error JSON adapter rejects malformed or dead-end cards", "[editor][diagnostics][error_card]") {
    REQUIRE_FALSE(urpg::editor::editorErrorCardFromJson(nlohmann::json::array()).has_value());
    auto invalid = nlohmann::json{{"schema", "urpg.editor_error.v1"},
                                  {"code", "broken"},
                                  {"go_to", nlohmann::json::object()},
                                  {"retry", nlohmann::json::object()},
                                  {"details", nlohmann::json::object()}};
    REQUIRE_FALSE(urpg::editor::editorErrorCardFromJson(invalid).has_value());
}

TEST_CASE("Editor error review corpus contains only actionable redacted cards",
          "[editor][diagnostics][error_card][corpus]") {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "fixtures" /
                        "editor_error_corpus.json");
    const auto corpus = nlohmann::json::parse(input);
    REQUIRE(corpus["schema"] == "urpg.editor_error_corpus.v1");
    REQUIRE(corpus["cases"].size() >= 6);
    for (const auto& value : corpus["cases"]) {
        auto encoded = value;
        encoded["schema"] = "urpg.editor_error.v1";
        const auto card = urpg::editor::editorErrorCardFromJson(encoded);
        REQUIRE(card.has_value());
        const auto support = urpg::editor::copyEditorErrorDetails(*card);
        REQUIRE(support.find("C:/private") == std::string::npos);
        REQUIRE(support.find("private\"") == std::string::npos);
    }
}

TEST_CASE("Editor error validation rejects raw-only and dead-end diagnostics", "[editor][diagnostics][error_card]") {
    urpg::editor::EditorErrorCard raw;
    raw.code = "raw_failure";
    raw.summary = "std::runtime_error\nstack frame";
    raw.details = nlohmann::json::object();
    const auto validation = urpg::editor::validateEditorErrorCard(raw);
    REQUIRE_FALSE(validation.valid);
    REQUIRE(validation.issues.size() == 6);
}
