#include "editor/diagnostics/editor_error_card.h"

#include <catch2/catch_test_macros.hpp>

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

    const auto support = urpg::editor::redactedEditorErrorSupportExport(card);
    REQUIRE(support["details"]["project_path"] == "[redacted]");
    REQUIRE(support["details"]["authorization"] == "[redacted]");
    REQUIRE(support["details"]["nested"]["token"] == "[redacted]");
    REQUIRE(support["details"]["nested"]["reason"] == "project.json is missing");
    const auto copied = urpg::editor::copyEditorErrorDetails(card);
    REQUIRE(copied.find("Bearer private") == std::string::npos);
    REQUIRE(copied.find("C:/Users") == std::string::npos);
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
