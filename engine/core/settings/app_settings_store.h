#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

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
};

struct AudioSettings {
    float master_volume = 1.0f;
    float bgm_volume = 1.0f;
    float bgs_volume = 1.0f;
    float se_volume = 1.0f;
    float me_volume = 1.0f;
    float system_volume = 1.0f;
};

struct AccessibilitySettings {
    bool high_contrast = false;
    bool reduce_motion = false;
    float ui_scale = 1.0f;
};

struct RuntimeSettings {
    WindowSettings window;
    AudioSettings audio;
    AccessibilitySettings accessibility;
    std::filesystem::path input_mapping_path;
};

struct EditorSettings {
    WindowSettings window;
    AccessibilitySettings accessibility;
    std::filesystem::path imgui_ini_path;
    std::filesystem::path workspace_path;
    bool restore_workspace = true;
    std::string analytics_consent_state = "unknown";
    bool analytics_upload_enabled = false;
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

AppSettingsPaths appSettingsPaths(const std::filesystem::path& project_root);

RuntimeSettings defaultRuntimeSettings();
EditorSettings defaultEditorSettings(const AppSettingsPaths& paths);

RuntimeSettingsLoadResult loadRuntimeSettings(const std::filesystem::path& path);
EditorSettingsLoadResult loadEditorSettings(const std::filesystem::path& path, const AppSettingsPaths& paths);

bool saveRuntimeSettings(const std::filesystem::path& path, const RuntimeSettings& settings, std::string* error = nullptr);
bool saveEditorSettings(const std::filesystem::path& path, const EditorSettings& settings, std::string* error = nullptr);

} // namespace urpg::settings
