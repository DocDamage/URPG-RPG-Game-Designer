#include "engine/core/app_cli.h"

#include <catch2/catch_test_macros.hpp>

#include <string_view>
#include <vector>

namespace {

std::vector<std::string_view> args(std::initializer_list<std::string_view> values) {
    return std::vector<std::string_view>(values);
}

} // namespace

TEST_CASE("Runtime CLI parses help and version without requiring engine startup", "[cli][runtime]") {
    const auto help = urpg::cli::parseRuntimeCli(args({"--help"}), false);
    REQUIRE(help.ok());
    REQUIRE(help.action == urpg::cli::CliAction::Help);
    REQUIRE(urpg::cli::runtimeHelpText().find("--frames <count>") != std::string::npos);

    const auto version = urpg::cli::parseRuntimeCli(args({"--version"}), false);
    REQUIRE(version.ok());
    REQUIRE(version.action == urpg::cli::CliAction::Version);
}

TEST_CASE("Runtime CLI rejects unknown flags and missing required values", "[cli][runtime]") {
    const auto unknown = urpg::cli::parseRuntimeCli(args({"--bogus"}), false);
    REQUIRE_FALSE(unknown.ok());
    REQUIRE(unknown.error == "unknown option: --bogus");

    const auto missingFrames = urpg::cli::parseRuntimeCli(args({"--frames"}), false);
    REQUIRE_FALSE(missingFrames.ok());
    REQUIRE(missingFrames.error == "missing value after --frames");

    const auto missingWidth = urpg::cli::parseRuntimeCli(args({"--width", "--height", "720"}), false);
    REQUIRE_FALSE(missingWidth.ok());
    REQUIRE(missingWidth.error == "missing value after --width");

    const auto missingProject = urpg::cli::parseRuntimeCli(args({"--project-root"}), false);
    REQUIRE_FALSE(missingProject.ok());
    REQUIRE(missingProject.error == "missing value after --project-root");
}

TEST_CASE("Runtime CLI preserves valid option parsing", "[cli][runtime]") {
    const auto parsed = urpg::cli::parseRuntimeCli(
        args({"--headless", "--frames", "3", "--width", "800", "--height", "600", "--project-root", "demo"}), false);

    REQUIRE(parsed.ok());
    REQUIRE(parsed.action == urpg::cli::CliAction::Run);
    REQUIRE(parsed.options.headless);
    REQUIRE(parsed.options.frames == 3);
    REQUIRE(parsed.options.width == 800);
    REQUIRE(parsed.options.height == 600);
    REQUIRE(parsed.options.width_provided);
    REQUIRE(parsed.options.height_provided);
    REQUIRE(parsed.options.project_root == "demo");

    const auto defaults = urpg::cli::parseRuntimeCli(args({}), false);
    REQUIRE(defaults.ok());
    REQUIRE_FALSE(defaults.options.width_provided);
    REQUIRE_FALSE(defaults.options.height_provided);
}

TEST_CASE("Editor CLI parses help and version without requiring engine startup", "[cli][editor]") {
    const auto help = urpg::cli::parseEditorCli(args({"--help"}), false);
    REQUIRE(help.ok());
    REQUIRE(help.action == urpg::cli::CliAction::Help);
    REQUIRE(urpg::cli::editorHelpText().find("--open-panel <id>") != std::string::npos);
    REQUIRE(urpg::cli::editorHelpText().find("--safe-mode") != std::string::npos);
    REQUIRE(urpg::cli::editorHelpText().find("--probe-platform") != std::string::npos);

    const auto version = urpg::cli::parseEditorCli(args({"--version"}), false);
    REQUIRE(version.ok());
    REQUIRE(version.action == urpg::cli::CliAction::Version);
}

TEST_CASE("Editor CLI rejects unknown flags and missing required values", "[cli][editor]") {
    const auto unknown = urpg::cli::parseEditorCli(args({"--unknown-editor-flag"}), false);
    REQUIRE_FALSE(unknown.ok());
    REQUIRE(unknown.error == "unknown option: --unknown-editor-flag");

    const auto missingHeight = urpg::cli::parseEditorCli(args({"--height"}), false);
    REQUIRE_FALSE(missingHeight.ok());
    REQUIRE(missingHeight.error == "missing value after --height");

    const auto missingOpenPanel = urpg::cli::parseEditorCli(args({"--open-panel", "--frames", "1"}), false);
    REQUIRE_FALSE(missingOpenPanel.ok());
    REQUIRE(missingOpenPanel.error == "missing value after --open-panel");

    const auto missingSmokeOutput = urpg::cli::parseEditorCli(args({"--smoke-output"}), false);
    REQUIRE_FALSE(missingSmokeOutput.ok());
    REQUIRE(missingSmokeOutput.error == "missing value after --smoke-output");
}

TEST_CASE("Editor CLI preserves valid option parsing and smoke defaults", "[cli][editor]") {
    const auto parsed = urpg::cli::parseEditorCli(args({
                                                       "--headless",
                                                       "--frames",
                                                       "7",
                                                       "--width",
                                                       "1280",
                                                       "--height",
                                                       "720",
                                                       "--project-root",
                                                       "demo",
                                                       "--list-panels",
                                                       "--render-all-panels",
                                                       "--open-panel",
                                                       "assets",
                                                   }),
                                                   false);

    REQUIRE(parsed.ok());
    REQUIRE(parsed.action == urpg::cli::CliAction::Run);
    REQUIRE(parsed.options.headless);
    REQUIRE(parsed.options.frames == 7);
    REQUIRE(parsed.options.width == 1280);
    REQUIRE(parsed.options.height == 720);
    REQUIRE(parsed.options.width_provided);
    REQUIRE(parsed.options.height_provided);
    REQUIRE(parsed.options.project_root == "demo");
    REQUIRE(parsed.options.list_panels);
    REQUIRE(parsed.options.render_all_panels);
    REQUIRE(parsed.options.open_panel_id.has_value());
    REQUIRE(*parsed.options.open_panel_id == "assets");

    const auto smoke = urpg::cli::parseEditorCli(args({"--smoke"}), false);
    REQUIRE(smoke.ok());
    REQUIRE(smoke.options.smoke);
    REQUIRE(smoke.options.headless);
    REQUIRE(smoke.options.frames == 0);
    REQUIRE_FALSE(smoke.options.width_provided);
    REQUIRE_FALSE(smoke.options.height_provided);
    REQUIRE_FALSE(smoke.options.smoke_output.empty());
    REQUIRE_FALSE(smoke.options.smoke_snapshot_root.empty());

    const auto safeMode = urpg::cli::parseEditorCli(args({"--safe-mode"}), false);
    REQUIRE(safeMode.ok());
    REQUIRE(safeMode.options.safe_mode);
    REQUIRE(safeMode.options.headless);
    REQUIRE(safeMode.options.frames == 1);

    const auto safeModeFrames = urpg::cli::parseEditorCli(args({"--safe-mode", "--frames", "3"}), false);
    REQUIRE(safeModeFrames.ok());
    REQUIRE(safeModeFrames.options.safe_mode);
    REQUIRE(safeModeFrames.options.headless);
    REQUIRE(safeModeFrames.options.frames == 3);

    const auto platformProbe = urpg::cli::parseEditorCli(args({"--probe-platform"}), false);
    REQUIRE(platformProbe.ok());
    REQUIRE(platformProbe.options.probe_platform);
    REQUIRE_FALSE(platformProbe.options.probe_opengl);

    const auto openglProbe = urpg::cli::parseEditorCli(args({"--probe-opengl"}), false);
    REQUIRE(openglProbe.ok());
    REQUIRE(openglProbe.options.probe_platform);
    REQUIRE(openglProbe.options.probe_opengl);
    REQUIRE_FALSE(openglProbe.options.probe_render);

    const auto renderProbe = urpg::cli::parseEditorCli(args({"--probe-render"}), false);
    REQUIRE(renderProbe.ok());
    REQUIRE(renderProbe.options.probe_platform);
    REQUIRE(renderProbe.options.probe_opengl);
    REQUIRE(renderProbe.options.probe_render);
}
