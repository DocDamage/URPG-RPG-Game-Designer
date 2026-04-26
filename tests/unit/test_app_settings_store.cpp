#include "engine/core/settings/app_settings_store.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>

namespace {

std::filesystem::path uniqueSettingsRoot(const std::string& name) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (name + "_" + std::to_string(tick));
}

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

} // namespace

TEST_CASE("App settings paths are project-local", "[settings][persistence]") {
    const auto paths = urpg::settings::appSettingsPaths("demo_project");

    REQUIRE(paths.root == std::filesystem::path("demo_project") / ".urpg" / "settings");
    REQUIRE(paths.runtime_settings.filename() == "runtime.json");
    REQUIRE(paths.editor_settings.filename() == "editor.json");
    REQUIRE(paths.editor_imgui_ini.filename() == "editor_imgui.ini");
    REQUIRE(paths.editor_workspace.filename() == "editor_workspace.json");
}

TEST_CASE("Runtime settings save and reload window audio input and accessibility", "[settings][persistence][runtime]") {
    const auto root = uniqueSettingsRoot("urpg_runtime_settings");
    std::filesystem::remove_all(root);
    const auto paths = urpg::settings::appSettingsPaths(root);

    auto settings = urpg::settings::defaultRuntimeSettings();
    settings.window.width = 1600;
    settings.window.height = 900;
    settings.window.fullscreen = true;
    settings.audio.master_volume = 0.75f;
    settings.audio.bgm_volume = 0.5f;
    settings.accessibility.high_contrast = true;
    settings.accessibility.reduce_motion = true;
    settings.accessibility.ui_scale = 1.25f;
    settings.input_mapping_path = "config/custom_input.json";

    REQUIRE(urpg::settings::saveRuntimeSettings(paths.runtime_settings, settings));

    const auto loaded = urpg::settings::loadRuntimeSettings(paths.runtime_settings);
    REQUIRE(loaded.report.loaded);
    REQUIRE_FALSE(loaded.report.recovered_from_malformed);
    REQUIRE(loaded.settings.window.width == 1600);
    REQUIRE(loaded.settings.window.height == 900);
    REQUIRE(loaded.settings.window.fullscreen);
    REQUIRE(loaded.settings.audio.master_volume == 0.75f);
    REQUIRE(loaded.settings.audio.bgm_volume == 0.5f);
    REQUIRE(loaded.settings.accessibility.high_contrast);
    REQUIRE(loaded.settings.accessibility.reduce_motion);
    REQUIRE(loaded.settings.accessibility.ui_scale == 1.25f);
    REQUIRE(loaded.settings.input_mapping_path == "config/custom_input.json");

    std::filesystem::remove_all(root);
}

TEST_CASE("Editor settings persist ImGui and workspace paths", "[settings][persistence][imgui][editor]") {
    const auto root = uniqueSettingsRoot("urpg_editor_settings");
    std::filesystem::remove_all(root);
    const auto paths = urpg::settings::appSettingsPaths(root);

    auto settings = urpg::settings::defaultEditorSettings(paths);
    settings.window.width = 1920;
    settings.window.height = 1080;
    settings.imgui_ini_path = paths.root / "custom_imgui.ini";
    settings.workspace_path = paths.root / "custom_workspace.json";
    settings.restore_workspace = false;
    settings.analytics_consent_state = "granted";
    settings.analytics_upload_enabled = true;

    REQUIRE(urpg::settings::saveEditorSettings(paths.editor_settings, settings));

    const auto loaded = urpg::settings::loadEditorSettings(paths.editor_settings, paths);
    REQUIRE(loaded.report.loaded);
    REQUIRE(loaded.settings.window.width == 1920);
    REQUIRE(loaded.settings.window.height == 1080);
    REQUIRE(loaded.settings.imgui_ini_path == paths.root / "custom_imgui.ini");
    REQUIRE(loaded.settings.workspace_path == paths.root / "custom_workspace.json");
    REQUIRE_FALSE(loaded.settings.restore_workspace);
    REQUIRE(loaded.settings.analytics_consent_state == "granted");
    REQUIRE(loaded.settings.analytics_upload_enabled);

    std::filesystem::remove_all(root);
}

TEST_CASE("Malformed settings recover to defaults without crashing", "[settings][persistence][error]") {
    const auto root = uniqueSettingsRoot("urpg_malformed_settings");
    std::filesystem::remove_all(root);
    const auto paths = urpg::settings::appSettingsPaths(root);

    writeText(paths.runtime_settings, "{ not valid json");
    writeText(paths.editor_settings, "[]");

    const auto runtime = urpg::settings::loadRuntimeSettings(paths.runtime_settings);
    REQUIRE_FALSE(runtime.report.loaded);
    REQUIRE(runtime.report.recovered_from_malformed);
    REQUIRE(runtime.settings.window.width == 1280);
    REQUIRE(runtime.settings.window.height == 720);

    const auto editor = urpg::settings::loadEditorSettings(paths.editor_settings, paths);
    REQUIRE_FALSE(editor.report.loaded);
    REQUIRE(editor.report.recovered_from_malformed);
    REQUIRE(editor.settings.window.width == 1440);
    REQUIRE(editor.settings.window.height == 900);
    REQUIRE(editor.settings.imgui_ini_path == paths.editor_imgui_ini);
    REQUIRE(editor.settings.analytics_consent_state == "unknown");
    REQUIRE_FALSE(editor.settings.analytics_upload_enabled);

    std::filesystem::remove_all(root);
}

TEST_CASE("Editor analytics consent defaults disabled and normalizes unknown values",
          "[settings][persistence][analytics][privacy][consent]") {
    const auto root = uniqueSettingsRoot("urpg_editor_analytics_settings");
    std::filesystem::remove_all(root);
    const auto paths = urpg::settings::appSettingsPaths(root);

    auto defaults = urpg::settings::loadEditorSettings(paths.editor_settings, paths);
    REQUIRE(defaults.settings.analytics_consent_state == "unknown");
    REQUIRE_FALSE(defaults.settings.analytics_upload_enabled);

    writeText(paths.editor_settings, R"({
  "schema": "urpg.editor_settings.v1",
  "analytics": {
    "consent_state": "surprise",
    "upload_enabled": true
  }
})");

    const auto loaded = urpg::settings::loadEditorSettings(paths.editor_settings, paths);
    REQUIRE(loaded.report.loaded);
    REQUIRE(loaded.settings.analytics_consent_state == "unknown");
    REQUIRE(loaded.settings.analytics_upload_enabled);

    std::filesystem::remove_all(root);
}
