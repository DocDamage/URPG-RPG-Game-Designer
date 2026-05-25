#include "engine/core/message/dialogue_script_compiler.h"
#include "engine/core/message/dialogue_script_exporter.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Dialogue script exporter emits canonical script stable across roundtrip", "[message][dialogue_script]") {
    const std::string script = ":: intro\n"
                               "Alicia: Welcome home.\n"
                               "? Choose a destination\n"
                               "- town | Visit town\n"
                               "- forest | Enter forest\n"
                               "! grant_item herb\n"
                               "\n"
                               ":: town\n"
                               "> The market is quiet.\n";

    const auto compiled = urpg::message::compileDialogueScript(script);
    REQUIRE(compiled.ok());

    const auto exported_script = urpg::message::exportDialogueScript(compiled.pages);
    const auto compiled_again = urpg::message::compileDialogueScript(exported_script);

    REQUIRE(compiled_again.ok());
    REQUIRE(urpg::message::exportDialogueScript(compiled_again.pages) == exported_script);
    REQUIRE(exported_script == ":: intro\n"
                               "Alicia: Welcome home.\n"
                               "? Choose a destination\n"
                               "- town | Visit town\n"
                               "- forest | Enter forest\n"
                               "! grant_item herb\n"
                               "\n"
                               ":: town\n"
                               "> The market is quiet.\n");
}

TEST_CASE("Dialogue script exporter preserves system pages and disabled choices", "[message][dialogue_script]") {
    urpg::message::DialoguePage page;
    page.id = "notice";
    page.body = "Autosave complete.\nContinue?";
    page.variant = urpg::message::variantFromCompatRoute("system", "System", 0);
    page.choices = {{"continue", "Continue", true, ""}, {"locked", "Locked", false, "missing flag"}};

    const auto exported_script = urpg::message::exportDialogueScript({page});

    REQUIRE(exported_script == ":: notice\n"
                               "@mode system\n"
                               "System: Autosave complete.\n"
                               "? Continue?\n"
                               "- continue | Continue\n"
                               "- locked [disabled: missing flag] | Locked\n");
}
