#include "engine/core/message/dialogue_script_compiler.h"
#include "engine/core/message/dialogue_script_exporter.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Snapshot: dialogue script roundtrip canonical text remains stable", "[snapshot][dialogue_script][message]") {
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

    REQUIRE(urpg::message::exportDialogueScript(compiled.pages) == ":: intro\n"
                                                                   "Alicia: Welcome home.\n"
                                                                   "? Choose a destination\n"
                                                                   "- town | Visit town\n"
                                                                   "- forest | Enter forest\n"
                                                                   "! grant_item herb\n"
                                                                   "\n"
                                                                   ":: town\n"
                                                                   "> The market is quiet.\n");
}
