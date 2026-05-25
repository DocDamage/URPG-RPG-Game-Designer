#include "engine/core/message/dialogue_script_compiler.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Dialogue script compiler builds deterministic dialogue pages", "[message][dialogue_script]") {
    const std::string script = ":: intro\n"
                               "Alicia: Welcome home.\n"
                               "? Choose a destination\n"
                               "- town | Visit town\n"
                               "- forest | Enter forest\n"
                               "! grant_item herb\n"
                               "\n"
                               ":: town\n"
                               "> The market is quiet.\n";

    const auto result = urpg::message::compileDialogueScript(script);

    REQUIRE(result.ok());
    REQUIRE(result.pages.size() == 2);
    REQUIRE(result.pages[0].id == "intro");
    REQUIRE(result.pages[0].variant.mode == urpg::message::MessagePresentationMode::Speaker);
    REQUIRE(result.pages[0].variant.speaker == "Alicia");
    REQUIRE(result.pages[0].body == "Welcome home.\nChoose a destination");
    REQUIRE(result.pages[0].choices.size() == 2);
    REQUIRE(result.pages[0].choices[0].id == "town");
    REQUIRE(result.pages[0].choices[0].label == "Visit town");
    REQUIRE(result.pages[0].choices[1].id == "forest");
    REQUIRE(result.pages[0].choices[1].label == "Enter forest");
    REQUIRE(result.pages[0].command == "grant_item herb");
    REQUIRE(result.pages[1].id == "town");
    REQUIRE(result.pages[1].variant.mode == urpg::message::MessagePresentationMode::Narration);
    REQUIRE(result.pages[1].body == "The market is quiet.");
}

TEST_CASE("Dialogue script compiler returns structured diagnostics for malformed scripts",
          "[message][dialogue_script]") {
    const std::string script = "- orphan | Choice before page\n"
                               ":: intro\n"
                               "Alicia: Hello.\n"
                               ":: intro\n"
                               "> Duplicate page.\n";

    const auto result = urpg::message::compileDialogueScript(script);

    REQUIRE_FALSE(result.ok());
    REQUIRE(result.diagnostics.size() >= 2);
    REQUIRE(result.diagnostics[0].line == 1);
    REQUIRE(result.diagnostics[0].code == "choice_without_page");
    REQUIRE(result.diagnostics[1].code == "duplicate_page_id");
}
