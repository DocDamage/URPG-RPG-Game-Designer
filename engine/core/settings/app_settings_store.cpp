#include "engine/core/settings/app_settings_store.h"

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <fstream>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

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

std::vector<std::string> readStringList(const nlohmann::json& object, const char* key) {
    std::vector<std::string> values;
    if (!object.contains(key) || !object.at(key).is_array()) {
        return values;
    }
    for (const auto& value : object.at(key)) {
        if (!value.is_string() || value.get<std::string>().empty()) {
            continue;
        }
        const auto entry = value.get<std::string>();
        if (std::find(values.begin(), values.end(), entry) == values.end()) {
            values.push_back(entry);
        }
        if (values.size() == 10) {
            break;
        }
    }
    return values;
}

std::vector<std::string> readAssetKeyList(const nlohmann::json& object, const char* key) {
    std::vector<std::string> values;
    if (!object.contains(key) || !object.at(key).is_array()) {
        return values;
    }
    for (const auto& value : object.at(key)) {
        if (!value.is_string() || value.get<std::string>().empty()) {
            continue;
        }
        const auto entry = value.get<std::string>();
        if (std::find(values.begin(), values.end(), entry) == values.end()) {
            values.push_back(entry);
        }
        if (values.size() == 10000) {
            break;
        }
    }
    return values;
}

std::vector<AssetLibraryCollectionSettings> readAssetCollections(const nlohmann::json& creator) {
    std::vector<AssetLibraryCollectionSettings> collections;
    if (!creator.contains("asset_collections") || !creator.at("asset_collections").is_array()) {
        return collections;
    }
    for (const auto& value : creator.at("asset_collections")) {
        if (!value.is_object()) {
            continue;
        }
        AssetLibraryCollectionSettings collection;
        collection.id = readString(value, "id", "");
        collection.label = readString(value, "label", "");
        collection.asset_keys = readAssetKeyList(value, "asset_keys");
        if (collection.id.empty() || collection.label.empty() ||
            std::any_of(collections.begin(), collections.end(), [&](const auto& existing) {
                return existing.id == collection.id;
            })) {
            continue;
        }
        collections.push_back(std::move(collection));
        if (collections.size() == 100) {
            break;
        }
    }
    return collections;
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

MapWorkspaceLayoutSettings readMapWorkspaceLayout(const nlohmann::json& creator,
                                                   MapWorkspaceLayoutSettings defaults) {
    if (!creator.contains("map_workspace_layout") || !creator.at("map_workspace_layout").is_object()) {
        return defaults;
    }
    const auto& layout = creator.at("map_workspace_layout");
    if (layout.contains("palette_width_fraction") && layout.at("palette_width_fraction").is_number()) {
        defaults.palette_width_fraction = std::clamp(layout.at("palette_width_fraction").get<float>(), 0.12f, 0.35f);
    }
    if (layout.contains("inspector_width_fraction") && layout.at("inspector_width_fraction").is_number()) {
        defaults.inspector_width_fraction = std::clamp(layout.at("inspector_width_fraction").get<float>(), 0.12f, 0.35f);
    }
    if (layout.contains("diagnostics_height_fraction") && layout.at("diagnostics_height_fraction").is_number()) {
        defaults.diagnostics_height_fraction =
            std::clamp(layout.at("diagnostics_height_fraction").get<float>(), 0.12f, 0.40f);
    }
    defaults.palette_visible = readBool(layout, "palette_visible", defaults.palette_visible);
    defaults.inspector_visible = readBool(layout, "inspector_visible", defaults.inspector_visible);
    defaults.diagnostics_visible = readBool(layout, "diagnostics_visible", defaults.diagnostics_visible);
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
        const auto temporary = path.string() + ".tmp";
        std::ofstream out(temporary, std::ios::binary | std::ios::trunc);
        if (!out) {
            if (error != nullptr) {
                *error = "Unable to open settings file for write: " + path.string();
            }
            return false;
        }
        out << payload.dump(2) << "\n";
        out.close();
        if (!out) {
            if (error != nullptr) {
                *error = "Unable to finish settings write: " + path.string();
            }
            return false;
        }
#ifdef _WIN32
        if (!MoveFileExW(std::filesystem::path(temporary).c_str(), path.c_str(),
                         MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
            if (error != nullptr) {
                *error = "Unable to atomically replace settings file: " + path.string();
            }
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
#else
        std::error_code rename_error;
        std::filesystem::rename(temporary, path, rename_error);
        if (rename_error) {
            if (error != nullptr) {
                *error = "Unable to atomically replace settings file: " + rename_error.message();
            }
            std::error_code ignored;
            std::filesystem::remove(temporary, ignored);
            return false;
        }
#endif
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

AppSettingsPaths editorUserSettingsPaths() {
#ifdef _WIN32
    if (const char* localAppData = std::getenv("LOCALAPPDATA"); localAppData != nullptr && *localAppData != '\0') {
        return appSettingsPaths(std::filesystem::path(localAppData) / "URPG Maker");
    }
#endif
    if (const char* home = std::getenv("HOME"); home != nullptr && *home != '\0') {
        return appSettingsPaths(std::filesystem::path(home) / ".local" / "share" / "urpg-maker");
    }
    return appSettingsPaths(std::filesystem::temp_directory_path() / "urpg-maker-user");
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
        if (payload.contains("creator") && payload.at("creator").is_object()) {
            const auto& creator = payload.at("creator");
            result.settings.last_project = readString(creator, "last_project", result.settings.last_project);
            result.settings.recent_projects = readStringList(creator, "recent_projects");
            result.settings.pinned_projects = readStringList(creator, "pinned_projects");
            result.settings.hidden_missing_projects = readStringList(creator, "hidden_missing_projects");
            result.settings.onboarding_enabled = readBool(creator, "onboarding_enabled", result.settings.onboarding_enabled);
            result.settings.help_tips_enabled = readBool(creator, "help_tips_enabled", result.settings.help_tips_enabled);
            result.settings.asset_browser_layout = readString(creator, "asset_browser_layout", result.settings.asset_browser_layout);
            result.settings.asset_favorite_keys = readAssetKeyList(creator, "asset_favorite_keys");
            result.settings.asset_collections = readAssetCollections(creator);
            result.settings.map_workspace_layout = readMapWorkspaceLayout(creator, result.settings.map_workspace_layout);
            result.settings.external_asset_library_root = readPath(creator, "external_asset_library_root", result.settings.external_asset_library_root);
        }
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
    nlohmann::json payload = {
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
        {"creator",
         {
             {"last_project", settings.last_project},
             {"recent_projects", settings.recent_projects},
             {"pinned_projects", settings.pinned_projects},
             {"hidden_missing_projects", settings.hidden_missing_projects},
             {"onboarding_enabled", settings.onboarding_enabled},
             {"help_tips_enabled", settings.help_tips_enabled},
             {"asset_browser_layout", settings.asset_browser_layout},
             {"asset_favorite_keys", settings.asset_favorite_keys},
             {"asset_collections", nlohmann::json::array()},
             {"map_workspace_layout",
              {{"palette_width_fraction", settings.map_workspace_layout.palette_width_fraction},
               {"inspector_width_fraction", settings.map_workspace_layout.inspector_width_fraction},
               {"diagnostics_height_fraction", settings.map_workspace_layout.diagnostics_height_fraction},
               {"palette_visible", settings.map_workspace_layout.palette_visible},
               {"inspector_visible", settings.map_workspace_layout.inspector_visible},
               {"diagnostics_visible", settings.map_workspace_layout.diagnostics_visible}}},
             {"external_asset_library_root", settings.external_asset_library_root.generic_string()},
         }},
    };
    for (const auto& collection : settings.asset_collections) {
        payload["creator"]["asset_collections"].push_back(
            {{"id", collection.id}, {"label", collection.label}, {"asset_keys", collection.asset_keys}});
    }
    return writeJsonFile(path, payload, error);
}

} // namespace urpg::settings
