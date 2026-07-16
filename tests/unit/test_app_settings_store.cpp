#include "engine/core/settings/app_settings_store.h"
#include "engine/core/accessibility/inclusive_experience.h"

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

TEST_CASE("Editor user settings paths are outside a project manifest", "[settings][persistence]") {
    const auto paths = urpg::settings::editorUserSettingsPaths();
    REQUIRE(paths.root.filename() == "settings");
    REQUIRE(paths.editor_settings.filename() == "editor.json");
    REQUIRE(paths.root.string().find("demo_project") == std::string::npos);
}

TEST_CASE("Runtime settings save and reload window audio input and accessibility", "[settings][persistence][runtime]") {
    const auto root = uniqueSettingsRoot("urpg_runtime_settings");
    std::filesystem::remove_all(root);
    const auto paths = urpg::settings::appSettingsPaths(root);

    auto settings = urpg::settings::defaultRuntimeSettings();
    settings.window.width = 1600;
    settings.window.height = 900;
    settings.window.fullscreen = true;
    settings.window.safe_area_scale = 0.9f;
    settings.audio.master_volume = 0.75f;
    settings.audio.bgm_volume = 0.5f;
    settings.audio.voice_volume = 0.65f;
    settings.accessibility.high_contrast = true;
    settings.accessibility.reduce_motion = true;
    settings.accessibility.ui_scale = 1.25f;
    settings.accessibility.text_scale = 1.4f;
    settings.accessibility.shortcuts_enabled = false;
    settings.accessibility.color_filter = "deuteranopia";
    settings.accessibility.non_color_cues = true;
    settings.accessibility.screen_shake = 0.8f;
    settings.accessibility.flash_intensity = 0.25f;
    settings.accessibility.subtitles = true;
    settings.accessibility.captions = true;
    settings.accessibility.caption_scale = 1.75f;
    settings.accessibility.mono_audio = true;
    settings.calibration.completed = true;
    settings.calibration.skipped = false;
    settings.calibration.revision = 1;
    settings.calibration.preferred_input_device = "controller";
    settings.input_mapping_path = "config/custom_input.json";
    settings.controller_mapping_path = "config/custom_controller.json";

    REQUIRE(urpg::settings::saveRuntimeSettings(paths.runtime_settings, settings));

    const auto loaded = urpg::settings::loadRuntimeSettings(paths.runtime_settings);
    REQUIRE(loaded.report.loaded);
    REQUIRE_FALSE(loaded.report.recovered_from_malformed);
    REQUIRE(loaded.settings.window.width == 1600);
    REQUIRE(loaded.settings.window.height == 900);
    REQUIRE(loaded.settings.window.fullscreen);
    REQUIRE(loaded.settings.window.safe_area_scale == 0.9f);
    REQUIRE(loaded.settings.audio.master_volume == 0.75f);
    REQUIRE(loaded.settings.audio.bgm_volume == 0.5f);
    REQUIRE(loaded.settings.audio.voice_volume == 0.65f);
    REQUIRE(loaded.settings.accessibility.high_contrast);
    REQUIRE(loaded.settings.accessibility.reduce_motion);
    REQUIRE(loaded.settings.accessibility.ui_scale == 1.25f);
    REQUIRE(loaded.settings.accessibility.text_scale == 1.4f);
    REQUIRE_FALSE(loaded.settings.accessibility.shortcuts_enabled);
    REQUIRE(loaded.settings.accessibility.color_filter == "deuteranopia");
    REQUIRE(loaded.settings.accessibility.non_color_cues);
    REQUIRE(loaded.settings.accessibility.screen_shake == 0.0f);
    REQUIRE(loaded.settings.accessibility.flash_intensity == 0.25f);
    REQUIRE(loaded.settings.accessibility.subtitles);
    REQUIRE(loaded.settings.accessibility.captions);
    REQUIRE(loaded.settings.accessibility.caption_scale == 1.75f);
    REQUIRE(loaded.settings.accessibility.mono_audio);
    REQUIRE(loaded.settings.calibration.completed);
    REQUIRE_FALSE(loaded.settings.calibration.skipped);
    REQUIRE(loaded.settings.calibration.revision == 1);
    REQUIRE(loaded.settings.calibration.preferred_input_device == "controller");
    REQUIRE(loaded.settings.input_mapping_path == "config/custom_input.json");
    REQUIRE(loaded.settings.controller_mapping_path == "config/custom_controller.json");

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
    settings.last_project = "C:/Projects/last";
    settings.recent_projects = {"C:/Projects/last", "D:/Projects/pinned"};
    settings.pinned_projects = {"D:/Projects/pinned"};
    settings.hidden_missing_projects = {"C:/Projects/moved"};
    settings.onboarding_enabled = false;
    settings.help_tips_enabled = false;
    settings.asset_browser_layout = "compact_list";
    settings.asset_favorite_keys = {"promoted:asset.hero", "catalog:abc123"};
    settings.asset_collections = {{"characters", "Characters", {"promoted:asset.hero"}},
                                  {"ui", "UI", {"catalog:abc123"}}};
    settings.asset_saved_searches = {{"ready-images", "Ready Images", "image", "characters", "hero", "", "bundle-a",
                                      true, true, true, false, false, true}};
    settings.map_workspace_layout.palette_width_fraction = 0.30f;
    settings.map_workspace_layout.inspector_width_fraction = 0.18f;
    settings.map_workspace_layout.diagnostics_height_fraction = 0.32f;
    settings.map_workspace_layout.palette_visible = false;
    settings.map_workspace_layout.inspector_visible = true;
    settings.map_workspace_layout.diagnostics_visible = false;
    settings.external_asset_library_root = "G:/All 2D Assets Stay Here";

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
    REQUIRE(loaded.settings.last_project == "C:/Projects/last");
    REQUIRE(loaded.settings.recent_projects == settings.recent_projects);
    REQUIRE(loaded.settings.pinned_projects == settings.pinned_projects);
    REQUIRE(loaded.settings.hidden_missing_projects == settings.hidden_missing_projects);
    REQUIRE_FALSE(loaded.settings.onboarding_enabled);
    REQUIRE_FALSE(loaded.settings.help_tips_enabled);
    REQUIRE(loaded.settings.asset_browser_layout == "compact_list");
    REQUIRE(loaded.settings.asset_favorite_keys == settings.asset_favorite_keys);
    REQUIRE(loaded.settings.asset_collections.size() == 2);
    REQUIRE(loaded.settings.asset_collections[0].id == "characters");
    REQUIRE(loaded.settings.asset_collections[1].asset_keys == std::vector<std::string>{"catalog:abc123"});
    REQUIRE(loaded.settings.asset_saved_searches.size() == 1);
    REQUIRE(loaded.settings.asset_saved_searches[0].id == "ready-images");
    REQUIRE(loaded.settings.asset_saved_searches[0].referenced_only);
    REQUIRE(loaded.settings.asset_saved_searches[0].release_eligible_only);
    REQUIRE(loaded.settings.map_workspace_layout.palette_width_fraction == 0.30f);
    REQUIRE(loaded.settings.map_workspace_layout.inspector_width_fraction == 0.18f);
    REQUIRE(loaded.settings.map_workspace_layout.diagnostics_height_fraction == 0.32f);
    REQUIRE_FALSE(loaded.settings.map_workspace_layout.palette_visible);
    REQUIRE(loaded.settings.map_workspace_layout.inspector_visible);
    REQUIRE_FALSE(loaded.settings.map_workspace_layout.diagnostics_visible);
    REQUIRE(loaded.settings.external_asset_library_root == "G:/All 2D Assets Stay Here");

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

TEST_CASE("Inclusive settings bridge applies runtime audio visual caption and motion policy",
          "[settings][persistence][inclusive][pcq651]") {
    auto runtime = urpg::settings::defaultRuntimeSettings();
    auto inclusive = urpg::accessibility::InclusiveSettings::safeDefaults();
    inclusive.text_scale = urpg::accessibility::TextScaleProfile::ExtraLarge;
    inclusive.high_contrast = true;
    inclusive.color_filter = urpg::accessibility::ColorFilter::Protanopia;
    inclusive.reduced_motion = true;
    inclusive.screen_shake = 0.0f;
    inclusive.flash_intensity = 0.2f;
    inclusive.caption_scale = 1.5f;
    inclusive.master_volume = 0.8f;
    inclusive.music_volume = 0.4f;
    inclusive.effects_volume = 0.6f;
    inclusive.voice_volume = 0.7f;
    inclusive.mono_audio = true;
    REQUIRE(inclusive.isValid());

    urpg::settings::applyInclusiveSettings(runtime, inclusive);
    REQUIRE(runtime.accessibility.text_scale == 2.0f);
    REQUIRE(runtime.accessibility.ui_scale == 2.0f);
    REQUIRE(runtime.accessibility.color_filter == "protanopia");
    REQUIRE(runtime.accessibility.screen_shake == 0.0f);
    REQUIRE(runtime.audio.voice_volume == 0.7f);
    const auto round_trip = urpg::settings::inclusiveSettingsFromRuntime(runtime);
    REQUIRE(round_trip.isValid());
    REQUIRE(round_trip.text_scale == urpg::accessibility::TextScaleProfile::ExtraLarge);
    REQUIRE(round_trip.color_filter == urpg::accessibility::ColorFilter::Protanopia);
    REQUIRE(round_trip.mono_audio);
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
