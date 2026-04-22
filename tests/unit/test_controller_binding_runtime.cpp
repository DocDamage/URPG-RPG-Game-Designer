#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_exception.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#include "engine/core/action/controller_binding_runtime.h"

using Catch::Matchers::ContainsSubstring;
using urpg::action::ControllerBindingIssueCategory;
using urpg::action::ControllerBindingRuntime;
using urpg::action::ControllerButton;
using urpg::input::InputAction;

TEST_CASE("ControllerBindingRuntime provides default navigation bindings", "[input][controller]") {
    ControllerBindingRuntime runtime;

    REQUIRE(runtime.getBinding(ControllerButton::DPadUp) == InputAction::MoveUp);
    REQUIRE(runtime.getBinding(ControllerButton::DPadDown) == InputAction::MoveDown);
    REQUIRE(runtime.getBinding(ControllerButton::DPadLeft) == InputAction::MoveLeft);
    REQUIRE(runtime.getBinding(ControllerButton::DPadRight) == InputAction::MoveRight);
    REQUIRE(runtime.getBinding(ControllerButton::FaceBottom) == InputAction::Confirm);
    REQUIRE(runtime.getBinding(ControllerButton::FaceRight) == InputAction::Cancel);
    REQUIRE(runtime.getBinding(ControllerButton::Start) == InputAction::Menu);
    REQUIRE(runtime.getIssues().empty());
    REQUIRE_FALSE(runtime.hasUnsavedChanges());
}

TEST_CASE("ControllerBindingRuntime save and load preserves bindings", "[input][controller]") {
    ControllerBindingRuntime runtime;
    runtime.bindButton(ControllerButton::LeftShoulder, InputAction::Menu);
    runtime.bindButton(ControllerButton::FaceTop, InputAction::Debug);

    const auto serialized = runtime.saveToJson();
    REQUIRE(serialized["version"] == "1.0.0");
    REQUIRE(serialized["bindings"].is_array());

    ControllerBindingRuntime reloaded;
    reloaded.loadFromJson(serialized);

    REQUIRE(reloaded.getBinding(ControllerButton::LeftShoulder) == InputAction::Menu);
    REQUIRE(reloaded.getBinding(ControllerButton::FaceTop) == InputAction::Debug);
    REQUIRE_FALSE(reloaded.hasUnsavedChanges());
}

TEST_CASE("ControllerBindingRuntime reports missing required actions after bindings are cleared", "[input][controller]") {
    ControllerBindingRuntime runtime;
    runtime.clearBinding(ControllerButton::DPadUp);
    runtime.clearBinding(ControllerButton::FaceRight);

    const auto issues = runtime.getIssues();
    REQUIRE(issues.size() == 2);
    REQUIRE(issues[0].category == ControllerBindingIssueCategory::MissingRequiredAction);
    REQUIRE(issues[0].action == InputAction::MoveUp);
    REQUIRE(issues[1].action == InputAction::Cancel);
    REQUIRE(runtime.hasUnsavedChanges());
}

TEST_CASE("ControllerBindingRuntime load rejects missing version", "[input][controller]") {
    ControllerBindingRuntime runtime;
    const nlohmann::json malformed = {
        {"bindings", nlohmann::json::array()}
    };

    REQUIRE_THROWS_MATCHES(
        runtime.loadFromJson(malformed),
        std::invalid_argument,
        Catch::Matchers::MessageMatches(ContainsSubstring("version"))
    );
}

TEST_CASE("ControllerBindingRuntime load rejects unknown button names", "[input][controller]") {
    ControllerBindingRuntime runtime;
    const nlohmann::json malformed = {
        {"version", "1.0.0"},
        {"bindings", nlohmann::json::array({
            {
                {"button", "TriggerFoo"},
                {"action", "Confirm"}
            }
        })}
    };

    REQUIRE_THROWS_MATCHES(
        runtime.loadFromJson(malformed),
        std::invalid_argument,
        Catch::Matchers::MessageMatches(ContainsSubstring("unknown controller button"))
    );
}

TEST_CASE("ControllerBindingRuntime governance script validates artifacts", "[input][controller][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_input_governance.ps1";
    const auto uniqueSuffix =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto outputPath =
        std::filesystem::temp_directory_path() / ("urpg_input_controller_gov_out_" + uniqueSuffix + ".json");

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                         std::istreambuf_iterator<char>());
    resultFile.close();

    const auto result = nlohmann::json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}
