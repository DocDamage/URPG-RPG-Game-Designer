#include "engine/core/settings/app_settings_store.h"

#include <algorithm>
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>

namespace urpg::settings {

namespace {

constexpr std::uint32_t kMinWindowDimension = 320;
constexpr std::uint32_t kMaxWindowDimension = 16384;

std::uint32_t readDimension(const nlohmann::json& object, const char* key, std::uint32_t fallback) {
    if (!object.contains(key) || !object.at(key).is_number_unsigned()) {
        return fallback;
    }
    const auto value = object.at(key).get<std::uint32_t>();
    return std::clamp(value, kMinWindowDimension, kMaxWindowDimension);
}

float readUnitFloat(const nlohmann::json& object, const char* key, float fallback) {
    if (!object.contains(key) || !object.at(key).is_number()) {
        return fallback;
    }
    return std::clamp(object.at(key).get<float>(), 0.0f, 1.0f);
}

bool readBool(const nlohmann::json& object, const char* key, bool fallback) {
    if (!object.contains(key) || !object.at(key).is_boolean()) {
        return fallback;
    }
    return object.at(key).get<bool>();
}

std::filesystem::path readPath(const nlohmann::json& object, const char* key, const std::filesystem::path& fallback) {
    if (!object.contains(key) || !object.at(key).is_string()) {
        return fallback;
    }
    return std::filesystem::path(object.at(key).get<std::string>());
}

std::string readString(const nlohmann::json& object, const char* key, const std::string& fallback) {
    if (!object.contains(key) || !object.at(key).is_string()) {
        return fallback;
    }
    return object.at(key).get<std::string>();
}

std::string normalizeConsentState(std::string state) {
    if (state == "granted" || state == "denied" || state == "unknown") {
        return state;
    }
    return "unknown";
}

WindowSettings readWindowSettings(const nlohmann::json& root, WindowSettings defaults) {
    if (!root.contains("window") || !root.at("window").is_object()) {
        return defaults;
    }
    const auto& window = root.at("window");
    defaults.width = readDimension(window, "width", defaults.width);
    defaults.height = readDimension(window, "height", defaults.height);
    defaults.fullscreen = readBool(window, "fullscreen", defaults.fullscreen);
    defaults.resizable = readBool(window, "resizable", defaults.resizable);
    return defaults;
}

AudioSettings readAudioSettings(const nlohmann::json& root, AudioSettings defaults) {
    if (!root.contains("audio") || !root.at("audio").is_object()) {
        return defaults;
    }
    const auto& audio = root.at("audio");
    defaults.master_volume = readUnitFloat(audio, "master_volume", defaults.master_volume);
    defaults.bgm_volume = readUnitFloat(audio, "bgm_volume", defaults.bgm_volume);
    defaults.bgs_volume = readUnitFloat(audio, "bgs_volume", defaults.bgs_volume);
    defaults.se_volume = readUnitFloat(audio, "se_volume", defaults.se_volume);
    defaults.me_volume = readUnitFloat(audio, "me_volume", defaults.me_volume);
    defaults.system_volume = readUnitFloat(audio, "system_volume", defaults.system_volume);
    return defaults;
}

AccessibilitySettings readAccessibilitySettings(const nlohmann::json& root, AccessibilitySettings defaults) {
    if (!root.contains("accessibility") || !root.at("accessibility").is_object()) {
        return defaults;
    }
    const auto& accessibility = root.at("accessibility");
    defaults.high_contrast = readBool(accessibility, "high_contrast", defaults.high_contrast);
    defaults.reduce_motion = readBool(accessibility, "reduce_motion", defaults.reduce_motion);
    if (accessibility.contains("ui_scale") && accessibility.at("ui_scale").is_number()) {
        defaults.ui_scale = std::clamp(accessibility.at("ui_scale").get<float>(), 0.5f, 3.0f);
    }
    return defaults;
}

nlohmann::json windowToJson(const WindowSettings& settings) {
    return {
        {"width", settings.width},
        {"height", settings.height},
        {"fullscreen", settings.fullscreen},
        {"resizable", settings.resizable},
    };
}

nlohmann::json audioToJson(const AudioSettings& settings) {
    return {
        {"master_volume", settings.master_volume},
        {"bgm_volume", settings.bgm_volume},
        {"bgs_volume", settings.bgs_volume},
        {"se_volume", settings.se_volume},
        {"me_volume", settings.me_volume},
        {"system_volume", settings.system_volume},
    };
}

nlohmann::json accessibilityToJson(const AccessibilitySettings& settings) {
    return {
        {"high_contrast", settings.high_contrast},
        {"reduce_motion", settings.reduce_motion},
        {"ui_scale", settings.ui_scale},
    };
}

bool writeJsonFile(const std::filesystem::path& path, const nlohmann::json& payload, std::string* error) {
    try {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path, std::ios::binary);
        if (!out) {
            if (error != nullptr) {
                *error = "Unable to open settings file for write: " + path.string();
            }
            return false;
        }
        out << payload.dump(2) << "\n";
        return true;
    } catch (const std::exception& ex) {
        if (error != nullptr) {
            *error = ex.what();
        }
        return false;
    }
}

} // namespace

AppSettingsPaths appSettingsPaths(const std::filesystem::path& project_root) {
    AppSettingsPaths paths;
    paths.root = project_root / ".urpg" / "settings";
    paths.runtime_settings = paths.root / "runtime.json";
    paths.editor_settings = paths.root / "editor.json";
    paths.editor_imgui_ini = paths.root / "editor_imgui.ini";
    paths.editor_workspace = paths.root / "editor_workspace.json";
    return paths;
}

RuntimeSettings defaultRuntimeSettings() {
    RuntimeSettings settings;
    settings.window.width = 1280;
    settings.window.height = 720;
    settings.input_mapping_path = "config/input_mappings.json";
    return settings;
}

EditorSettings defaultEditorSettings(const AppSettingsPaths& paths) {
    EditorSettings settings;
    settings.window.width = 1440;
    settings.window.height = 900;
    settings.imgui_ini_path = paths.editor_imgui_ini;
    settings.workspace_path = paths.editor_workspace;
    return settings;
}

RuntimeSettingsLoadResult loadRuntimeSettings(const std::filesystem::path& path) {
    RuntimeSettingsLoadResult result;
    result.settings = defaultRuntimeSettings();
    if (!std::filesystem::exists(path)) {
        return result;
    }

    try {
        std::ifstream in(path, std::ios::binary);
        const auto payload = nlohmann::json::parse(in);
        if (!payload.is_object()) {
            result.report.recovered_from_malformed = true;
            result.report.warnings.push_back("runtime_settings_root_not_object_using_defaults");
            return result;
        }
        result.report.loaded = true;
        result.settings.window = readWindowSettings(payload, result.settings.window);
        result.settings.audio = readAudioSettings(payload, result.settings.audio);
        result.settings.accessibility = readAccessibilitySettings(payload, result.settings.accessibility);
        result.settings.input_mapping_path = readPath(payload, "input_mapping_path", result.settings.input_mapping_path);
    } catch (const std::exception& ex) {
        result.settings = defaultRuntimeSettings();
        result.report.recovered_from_malformed = true;
        result.report.warnings.push_back(std::string("runtime_settings_malformed_using_defaults: ") + ex.what());
    }

    return result;
}

EditorSettingsLoadResult loadEditorSettings(const std::filesystem::path& path, const AppSettingsPaths& paths) {
    EditorSettingsLoadResult result;
    result.settings = defaultEditorSettings(paths);
    if (!std::filesystem::exists(path)) {
        return result;
    }

    try {
        std::ifstream in(path, std::ios::binary);
        const auto payload = nlohmann::json::parse(in);
        if (!payload.is_object()) {
            result.report.recovered_from_malformed = true;
            result.report.warnings.push_back("editor_settings_root_not_object_using_defaults");
            return result;
        }
        result.report.loaded = true;
        result.settings.window = readWindowSettings(payload, result.settings.window);
        result.settings.accessibility = readAccessibilitySettings(payload, result.settings.accessibility);
        result.settings.imgui_ini_path = readPath(payload, "imgui_ini_path", result.settings.imgui_ini_path);
        result.settings.workspace_path = readPath(payload, "workspace_path", result.settings.workspace_path);
        result.settings.restore_workspace = readBool(payload, "restore_workspace", result.settings.restore_workspace);
        if (payload.contains("analytics") && payload.at("analytics").is_object()) {
            const auto& analytics = payload.at("analytics");
            result.settings.analytics_consent_state =
                normalizeConsentState(readString(analytics, "consent_state", result.settings.analytics_consent_state));
            result.settings.analytics_upload_enabled =
                readBool(analytics, "upload_enabled", result.settings.analytics_upload_enabled);
        }
    } catch (const std::exception& ex) {
        result.settings = defaultEditorSettings(paths);
        result.report.recovered_from_malformed = true;
        result.report.warnings.push_back(std::string("editor_settings_malformed_using_defaults: ") + ex.what());
    }

    return result;
}

bool saveRuntimeSettings(const std::filesystem::path& path, const RuntimeSettings& settings, std::string* error) {
    const nlohmann::json payload = {
        {"schema", "urpg.runtime_settings.v1"},
        {"window", windowToJson(settings.window)},
        {"audio", audioToJson(settings.audio)},
        {"accessibility", accessibilityToJson(settings.accessibility)},
        {"input_mapping_path", settings.input_mapping_path.generic_string()},
    };
    return writeJsonFile(path, payload, error);
}

bool saveEditorSettings(const std::filesystem::path& path, const EditorSettings& settings, std::string* error) {
    const nlohmann::json payload = {
        {"schema", "urpg.editor_settings.v1"},
        {"window", windowToJson(settings.window)},
        {"accessibility", accessibilityToJson(settings.accessibility)},
        {"imgui_ini_path", settings.imgui_ini_path.generic_string()},
        {"workspace_path", settings.workspace_path.generic_string()},
        {"restore_workspace", settings.restore_workspace},
        {"analytics",
         {
             {"consent_state", normalizeConsentState(settings.analytics_consent_state)},
             {"upload_enabled", settings.analytics_upload_enabled},
         }},
    };
    return writeJsonFile(path, payload, error);
}

} // namespace urpg::settings
