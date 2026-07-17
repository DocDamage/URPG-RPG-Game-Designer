#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace urpg::accessibility { struct InclusiveSettings; }

namespace urpg::settings {

struct AppSettingsPaths {
    std::filesystem::path root;
    std::filesystem::path runtime_settings;
    std::filesystem::path editor_settings;
    std::filesystem::path editor_imgui_ini;
    std::filesystem::path editor_workspace;
};

struct WindowSettings {
    std::uint32_t width = 1280;
    std::uint32_t height = 720;
    bool fullscreen = false;
    bool resizable = true;
    float safe_area_scale = 1.0f;
};

struct AudioSettings {
    float master_volume = 1.0f;
    float bgm_volume = 1.0f;
    float bgs_volume = 1.0f;
    float se_volume = 1.0f;
    float me_volume = 1.0f;
    float system_volume = 1.0f;
    float voice_volume = 1.0f;
};

struct AccessibilitySettings {
    bool high_contrast = false;
    bool reduce_motion = false;
    float ui_scale = 1.0f;
    float text_scale = 1.0f;
    bool shortcuts_enabled = true;
    std::string color_filter = "none";
    bool non_color_cues = true;
    float screen_shake = 1.0f;
    float flash_intensity = 1.0f;
    bool subtitles = true;
    bool captions = true;
    float caption_scale = 1.0f;
    bool mono_audio = false;
};

struct RuntimeCalibrationSettings {
    bool completed = false;
    bool skipped = false;
    std::uint32_t revision = 0;
    std::string preferred_input_device = "auto";
};

struct MapWorkspaceLayoutSettings {
    float palette_width_fraction = 0.22f;
    float inspector_width_fraction = 0.24f;
    float diagnostics_height_fraction = 0.24f;
    bool palette_visible = true;
    bool inspector_visible = true;
    bool diagnostics_visible = true;
};

// User-only curation. These opaque asset keys never establish promotion,
// attachment, licensing, or package eligibility.
struct AssetLibraryCollectionSettings {
    std::string id;
    std::string label;
    std::vector<std::string> asset_keys;
};

struct AssetLibrarySavedSearchSettings {
    std::string id;
    std::string label;
    std::string media_kind;
    std::string category;
    std::string required_tag;
    std::string required_game_use_tag;
    std::string source_bundle_id;
    bool referenced_only = false;
    bool runtime_ready_only = false;
    bool previewable_only = false;
    bool project_attached_only = false;
    bool attachable_only = false;
    bool release_eligible_only = false;
};

struct RuntimeSettings {
    WindowSettings window;
    AudioSettings audio;
    AccessibilitySettings accessibility;
    std::filesystem::path input_mapping_path;
    std::filesystem::path controller_mapping_path;
    RuntimeCalibrationSettings calibration;
};

struct EditorSettings {
    WindowSettings window;
    AccessibilitySettings accessibility;
    std::filesystem::path imgui_ini_path;
    std::filesystem::path workspace_path;
    bool restore_workspace = true;
    std::string analytics_consent_state = "unknown";
    bool analytics_upload_enabled = false;
    std::string last_project;
    std::vector<std::string> recent_projects;
    std::vector<std::string> pinned_projects;
    std::vector<std::string> hidden_missing_projects;
    bool onboarding_enabled = true;
    bool help_tips_enabled = true;
    std::string asset_browser_layout = "left_collapsible_folder_tree";
    std::vector<std::string> asset_favorite_keys;
    std::vector<AssetLibraryCollectionSettings> asset_collections;
    std::vector<AssetLibrarySavedSearchSettings> asset_saved_searches;
    MapWorkspaceLayoutSettings map_workspace_layout;
    std::filesystem::path external_asset_library_root;
};

struct SettingsLoadReport {
    bool loaded = false;
    bool recovered_from_malformed = false;
    std::vector<std::string> warnings;
};

struct RuntimeSettingsLoadResult {
    RuntimeSettings settings;
    SettingsLoadReport report;
};

struct EditorSettingsLoadResult {
    EditorSettings settings;
    SettingsLoadReport report;
};

struct SettingsQuarantineResult {
    bool success = false;
    std::string code;
    std::filesystem::path quarantined_path;
    std::string message;
};

AppSettingsPaths appSettingsPaths(const std::filesystem::path& project_root);
AppSettingsPaths editorUserSettingsPaths();

RuntimeSettings defaultRuntimeSettings();
EditorSettings defaultEditorSettings(const AppSettingsPaths& paths);

RuntimeSettingsLoadResult loadRuntimeSettings(const std::filesystem::path& path);
EditorSettingsLoadResult loadEditorSettings(const std::filesystem::path& path, const AppSettingsPaths& paths);
SettingsQuarantineResult quarantineMalformedSettings(const std::filesystem::path& path);

bool saveRuntimeSettings(const std::filesystem::path& path, const RuntimeSettings& settings, std::string* error = nullptr);
bool saveEditorSettings(const std::filesystem::path& path, const EditorSettings& settings, std::string* error = nullptr);

accessibility::InclusiveSettings inclusiveSettingsFromRuntime(const RuntimeSettings& settings);
void applyInclusiveSettings(RuntimeSettings& settings, const accessibility::InclusiveSettings& inclusive);

} // namespace urpg::settings
