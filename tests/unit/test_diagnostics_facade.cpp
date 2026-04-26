#include "editor/diagnostics/diagnostics_facade.h"
#include "editor/diagnostics/diagnostics_workspace.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace urpg::editor;

TEST_CASE("DiagnosticsFacade - Emits accurate snapshot via facade", "[editor][diagnostics]") {
    DiagnosticsWorkspace workspace;
    DiagnosticsFacade facade(workspace);

    SECTION("Initial state emission") {
        std::string json_str = facade.emitSnapshot();
        auto json = nlohmann::json::parse(json_str);

        REQUIRE(json.contains("active_tab"));
        REQUIRE(json.contains("visible"));
        REQUIRE(json.contains("tabs"));
        REQUIRE(json["visible"] == true);
    }

    SECTION("Refresh and emit via callback") {
        bool callback_called = false;
        facade.refreshAndEmit([&](std::string_view json_str) {
            callback_called = true;
            auto json = nlohmann::json::parse(json_str);
            REQUIRE(json.is_object());
            REQUIRE(json["tabs"].is_array());
        });

        REQUIRE(callback_called);
    }
}
