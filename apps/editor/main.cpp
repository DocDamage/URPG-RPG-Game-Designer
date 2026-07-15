#include "apps/editor/editor_app_panels.h"
#include "editor/ability/ability_inspector_panel.h"
#include "editor/ability/pattern_field_panel.h"
#include "editor/analytics/analytics_panel.h"
#include "editor/ai/creator_command_panel.h"
#include "editor/accessibility/accessibility_audio_adapter.h"
#include "editor/accessibility/accessibility_battle_adapter.h"
#include "editor/accessibility/accessibility_menu_adapter.h"
#include "editor/accessibility/accessibility_panel.h"
#include "editor/accessibility/accessibility_spatial_adapter.h"
#include "editor/assets/asset_library_panel.h"
#include "editor/assets/editor_asset_drag_payload.h"
#include "editor/assets/editor_thumbnail_cache.h"
#include "editor/audio/audio_mix_panel.h"
#include "editor/character/character_creator_model.h"
#include "editor/character/character_creator_panel.h"
#include "editor/export/export_diagnostics_panel.h"
#include "editor/gameplay/gameplay_recipe_panel.h"
#include "editor/project/creator_checklist_panel.h"
#include "editor/project/main_menu_panel.h"
#include "editor/project/new_project_wizard_model.h"
#include "editor/project/editor_project_session.h"
#include "editor/project/editor_dirty_state_registry.h"
#include "editor/project/editor_recovery_service.h"
#include "editor/playtest/playtest_session_controller.h"
#include "editor/plugin/plugin_inspector_panel.h"
#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/mod/mod_manager_panel.h"
#include "editor/spatial/level_builder_workspace.h"
#include "editor/spatial/map_authoring_workspace.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/character/character_identity.h"
#include "editor/spatial/map_authoring_persistence.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_privacy_controller.h"
#include "engine/core/analytics/analytics_uploader.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/audio_mix_presets.h"
#include "engine/core/app_cli.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/assets/project_asset_reference_index.h"
#include "engine/core/security/sha256.h"

#include <type_traits>
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/database/rpg_database.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/engine_context.h"
#include "engine/core/engine_shell.h"
#include "engine/core/input/input_remap_store.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_catalog_loader.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_ruleset.h"
#include "engine/core/map/grid_part_serializer.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/localization/project_localization_audit.h"
#include "engine/core/localization/pseudo_localization.h"
#include "engine/core/message/message_core.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/save/save_catalog.h"
#include "engine/core/shop/vendor_catalog.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/menu_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/settings/app_settings_store.h"
#include "engine/core/ui/menu_serializer.h"
#include "engine/core/version.h"
#include <nlohmann/json.hpp>

#ifndef URPG_HEADLESS
#include "engine/core/platform/opengl_renderer.h"
#include "engine/core/platform/sdl_surface.h"
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl2.h>
#endif

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

#include <algorithm>
#include <array>
#include <chrono>
#include <cctype>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <iterator>
#include <memory>
#include <limits>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

bool defaultHeadless() {
#ifdef URPG_HEADLESS
    return true;
#else
    return false;
#endif
}

bool containsCaseInsensitive(const std::string& value, const std::string& needle) {
    if (needle.empty() || needle.size() > value.size()) {
        return needle.empty();
    }
    return std::search(value.begin(), value.end(), needle.begin(), needle.end(),
                       [](const char left, const char right) {
                           return std::tolower(static_cast<unsigned char>(left)) ==
                                  std::tolower(static_cast<unsigned char>(right));
                       }) != value.end();
}

void printVersion() {
    std::cout << "URPG Editor " << urpg::versionString() << "\n";
}

void clearSceneStack() {
    auto& sceneManager = urpg::scene::SceneManager::getInstance();
    while (sceneManager.stackSize() > 0) {
        sceneManager.popScene();
    }
}

struct EditorPanelRuntime {
    urpg::editor::DiagnosticsWorkspace diagnostics_workspace;
    urpg::editor::AssetLibraryPanel asset_library_panel;
    urpg::editor::EditorThumbnailCache asset_thumbnail_cache;
    std::vector<urpg::editor::EditorThumbnailRequest> asset_thumbnail_pinned_requests;
    urpg::editor::CreatorChecklistPanel creator_checklist_panel;
    urpg::editor::MainMenuModel main_menu_model;
    urpg::editor::MainMenuPanel main_menu_panel;
    urpg::editor::NewProjectWizardModel new_project_wizard;
    urpg::editor::EditorProjectSession project_session;
    urpg::editor::EditorRecoveryService recovery_service;
    urpg::editor::EditorDirtyStateRegistry dirty_state_registry;
    urpg::editor::AbilityInspectorPanel ability_inspector_panel;
    urpg::editor::gameplay::GameplayRecipePanel gameplay_recipe_panel;
    std::vector<urpg::gameplay::GameplayRecipe> gameplay_recipe_templates;
    urpg::editor::CharacterCreatorModel character_creator_model;
    urpg::editor::CharacterCreatorPanel character_creator_panel;
    std::optional<urpg::quest::QuestObjectiveGraphDocument> quest_draft;
    urpg::quest::QuestWorldState quest_preview_world;
    std::vector<urpg::quest::QuestObjectiveGraphDocument> quest_undo_history;
    std::vector<urpg::quest::QuestObjectiveGraphDocument> quest_redo_history;
    std::optional<urpg::dialogue::DialogueGraph> dialogue_draft;
    std::vector<urpg::dialogue::DialogueGraph> dialogue_undo_history;
    std::vector<urpg::dialogue::DialogueGraph> dialogue_redo_history;
    std::map<std::string, int> dialogue_preview_values;
    std::string dialogue_preview_node_id;
    std::vector<std::string> dialogue_preview_trace;
    std::vector<urpg::dialogue::DialogueGraphDiagnostic> dialogue_preview_diagnostics;
    urpg::database::RpgDatabase database_draft;
    urpg::shop::VendorCatalog vendor_draft;
    urpg::editor::PatternFieldModel pattern_field_model;
    urpg::editor::CreatorCommandPanel creator_command_panel;
    urpg::editor::PatternFieldPanel pattern_field_panel;
    urpg::editor::ModManagerPanel mod_manager_panel;
    urpg::editor::PluginInspectorPanel mz_plugin_inspector_panel;
    urpg::editor::AnalyticsPanel analytics_panel;
    urpg::editor::LevelBuilderWorkspace level_builder_workspace;
    urpg::editor::SpatialAuthoringWorkspace perspective_2d_workspace;
    // The two release routes remain available, but this shared coordinator owns
    // their creator-facing mode, selection, history, and project context.
    urpg::editor::MapAuthoringWorkspace map_authoring_workspace;
    urpg::editor::PlaytestSessionController playtest_session;
    urpg::SaveCatalog map_runtime_save_catalog;
    std::unique_ptr<urpg::SaveSessionCoordinator> map_runtime_save_session;
    urpg::message::MessageFlowRunner map_message_preview_flow;
    urpg::message::RichTextLayoutEngine map_message_preview_layout;
    urpg::ability::AbilitySystemComponent ability_runtime;
    urpg::audio::AudioCore audio_preview_core;
    urpg::audio::AudioMixPresetBank audio_mix_draft;
    urpg::editor::AudioMixPanel audio_mix_panel;
    urpg::input::InputRemapStore input_remap_draft;
    urpg::accessibility::AccessibilityAuditor accessibility_auditor;
    urpg::editor::AccessibilityPanel accessibility_panel;
    urpg::editor::ExportDiagnosticsPanel export_diagnostics_panel;
    urpg::battle::BattleFlowController battle_preview_flow;
    urpg::battle::BattleActionQueue battle_preview_actions;
    urpg::map::GridPartDocument level_builder_document{"EditorPreview", 16, 12};
    urpg::map::GridPartCatalog level_builder_catalog;
    urpg::presentation::SpatialMapOverlay level_builder_overlay;
    std::unique_ptr<urpg::scene::MapScene> perspective_2d_scene =
        std::make_unique<urpg::scene::MapScene>("EditorPreview", 16, 12);
    urpg::scene::MenuScene menu_studio_runtime{"editor_menu_studio"};
    urpg::mod::ModRegistry mod_registry;
    std::unique_ptr<urpg::mod::ModLoader> mod_loader;
    urpg::analytics::AnalyticsDispatcher analytics_dispatcher;
    urpg::analytics::AnalyticsUploader analytics_uploader;
    urpg::analytics::AnalyticsPrivacyController analytics_privacy_controller;
    std::filesystem::path project_root;
    std::filesystem::path external_asset_library_root;
    std::vector<std::string> available_map_ids;
    std::vector<std::string> dialogue_localization_key_options;
    std::vector<std::pair<std::string, std::string>> dialogue_speaker_options;
    std::vector<std::string> dialogue_voice_asset_options;
    nlohmann::json mz_plugin_lock_draft = nlohmann::json::object();
    bool mz_plugin_lock_source_current = false;
    std::string character_draft_id = "protagonist";
    std::string quest_draft_id = "quest_draft";
    std::string dialogue_draft_id = "dialogue_draft";
    std::string vendor_draft_id = "vendor_draft";
    std::string battle_preview_encounter_id;
    std::string audio_mix_preset = "Default";
    std::string audio_preview_asset_id;
    std::vector<std::string> audio_preview_asset_undo;
    std::vector<std::string> audio_preview_asset_redo;
    bool creator_mode = false;
    bool focus_workspace_next_frame = true;
    std::string last_workspace_panel_id;
    std::string map_save_status;
    std::string gameplay_recipe_load_status;
    std::string gameplay_recipe_save_status;
    std::string menu_studio_load_status;
    std::string menu_studio_save_status;
    std::string menu_studio_persisted_json;
    std::string mz_plugin_lock_status;
    std::string map_asset_drop_status;
    std::string project_session_status;
    std::string recovery_status;
    std::filesystem::path restored_recovery_project_path;
    std::chrono::steady_clock::time_point next_recovery_snapshot_at{};
    bool map_dirty_surface_registered = false;
    bool perspective_2d_dirty_surface_registered = false;
    bool ability_dirty_surface_registered = false;
    bool character_dirty_surface_registered = false;
    bool quest_dirty_surface_registered = false;
    bool dialogue_dirty_surface_registered = false;
    bool database_dirty_surface_registered = false;
    bool vendor_dirty_surface_registered = false;
    bool audio_mix_dirty_surface_registered = false;
    bool input_remap_dirty_surface_registered = false;
    bool gameplay_recipe_dirty_surface_registered = false;
    bool menu_studio_dirty_surface_registered = false;
    bool mz_plugin_lock_dirty_surface_registered = false;
};

constexpr const char* kMapDirtyDocumentId = "map.grid_parts";
constexpr const char* kPerspective2DDirtyDocumentId = "map.perspective_2d";
constexpr const char* kCharacterDirtyDocumentId = "character.creator";
constexpr const char* kQuestDirtyDocumentId = "quest.draft";
constexpr const char* kDialogueDirtyDocumentId = "dialogue.draft";
constexpr const char* kDatabaseDirtyDocumentId = "database.project";
constexpr const char* kVendorDirtyDocumentId = "vendor.catalog";
constexpr const char* kAudioMixDirtyDocumentId = "audio.mix";
constexpr const char* kInputRemapDirtyDocumentId = "input.remap";
constexpr const char* kGameplayRecipeDirtyDocumentId = "gameplay.recipes";
constexpr const char* kMenuStudioDirtyDocumentId = "menu.studio";
constexpr const char* kMzPluginLockDirtyDocumentId = "compat.mz_plugin_lock";

std::vector<std::string> collectProjectLocalizationKeys(const std::filesystem::path& project_root) {
    std::set<std::string> keys;
    std::error_code directory_error;
    const auto localization_directory = project_root / "content" / "localization";
    for (const auto& entry : std::filesystem::directory_iterator(localization_directory, directory_error)) {
        if (directory_error || !entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::ifstream input(entry.path(), std::ios::binary);
        const auto bundle = nlohmann::json::parse(input, nullptr, false);
        if (!urpg::localization::LocaleCatalog::validateBundleJson(bundle)) {
            continue;
        }
        urpg::localization::LocaleCatalog catalog;
        try {
            catalog.loadFromJson(bundle);
            const auto bundle_keys = catalog.getAllKeys();
            keys.insert(bundle_keys.begin(), bundle_keys.end());
        } catch (const std::invalid_argument&) {
            // A malformed locale bundle is a diagnostics concern; it must not
            // provide a misleading stable-reference option to dialogue editing.
        }
    }
    return {keys.begin(), keys.end()};
}

std::optional<urpg::localization::LocaleCatalog> loadProjectDialogueLocaleCatalog(
    const std::filesystem::path& project_root, const std::string& requested_locale) {
    std::string selected_locale = requested_locale;
    if (selected_locale.empty()) {
        std::ifstream manifest_input(project_root / "project.json", std::ios::binary);
        const auto manifest = nlohmann::json::parse(manifest_input, nullptr, false);
        if (manifest.is_object() && manifest.contains("localization") && manifest["localization"].is_object()) {
            selected_locale = manifest["localization"].value("default_locale", "");
        }
    }

    std::vector<std::filesystem::path> bundle_paths;
    std::error_code directory_error;
    const auto localization_directory = project_root / "content" / "localization";
    for (const auto& entry : std::filesystem::directory_iterator(localization_directory, directory_error)) {
        if (directory_error || !entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        bundle_paths.push_back(entry.path());
    }
    std::sort(bundle_paths.begin(), bundle_paths.end());
    for (const auto& bundle_path : bundle_paths) {
        std::ifstream input(bundle_path, std::ios::binary);
        const auto bundle = nlohmann::json::parse(input, nullptr, false);
        if (!urpg::localization::LocaleCatalog::validateBundleJson(bundle)) {
            continue;
        }
        try {
            urpg::localization::LocaleCatalog catalog;
            catalog.loadFromJson(bundle);
            if (selected_locale.empty() || catalog.getLocaleCode() == selected_locale) {
                return catalog;
            }
        } catch (const std::invalid_argument&) {
            // Invalid project locale data cannot be selected for runtime text.
        }
    }
    return std::nullopt;
}

std::vector<std::pair<std::string, std::string>> collectProjectDialogueSpeakerOptions(
    const std::filesystem::path& project_root) {
    std::vector<std::pair<std::string, std::string>> options;
    std::error_code directory_error;
    const auto character_directory = project_root / "content" / "characters";
    for (const auto& entry : std::filesystem::directory_iterator(character_directory, directory_error)) {
        if (directory_error || !entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::ifstream input(entry.path(), std::ios::binary);
        const auto identity_json = nlohmann::json::parse(input, nullptr, false);
        if (!identity_json.is_object()) {
            continue;
        }
        try {
            const auto identity = urpg::character::CharacterIdentity::fromJson(identity_json);
            const auto speaker_id = entry.path().stem().string();
            if (!speaker_id.empty()) {
                options.emplace_back(speaker_id, identity.getDisplayName());
            }
        } catch (const std::exception&) {
            // A malformed character draft is not a valid stable dialogue
            // speaker reference and must remain out of the picker.
        }
    }
    std::sort(options.begin(), options.end(), [](const auto& left, const auto& right) {
        return left.first < right.first;
    });
    return options;
}

std::vector<std::string> collectProjectDialogueVoiceAssetOptions(const std::filesystem::path& project_root) {
    std::set<std::string> asset_ids;
    std::error_code directory_error;
    const auto manifest_directory = project_root / "content" / "assets" / "manifests";
    for (const auto& entry : std::filesystem::directory_iterator(manifest_directory, directory_error)) {
        if (directory_error || !entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        try {
            std::ifstream input(entry.path(), std::ios::binary);
            const auto manifest = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(input));
            const auto payload_directory = project_root / "content" / "assets" / "imported" / manifest.assetId;
            if (manifest.assetId.empty() || manifest.preview.kind != "audio" ||
                !std::filesystem::is_directory(payload_directory, directory_error) || directory_error) {
                directory_error.clear();
                continue;
            }
            asset_ids.insert(manifest.assetId);
        } catch (const std::exception&) {
            // A malformed attachment manifest is never a selectable dialogue voice reference.
        }
    }
    return {asset_ids.begin(), asset_ids.end()};
}

std::string sha256File(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input.good()) {
        return {};
    }
    const std::vector<std::uint8_t> bytes{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
    return urpg::security::Sha256::toHex(urpg::security::Sha256::compute(bytes));
}

nlohmann::json buildMzPluginStaticLock(const std::filesystem::path& project_root) {
    const auto inspection = urpg::plugin::InspectMzPluginScriptsFromDirectory(project_root / "js" / "plugins");
    urpg::plugin::PluginCompatibilityAnalysisInput analysis_input;
    analysis_input.manifests = inspection.manifests;
    analysis_input.native_shim_hints = urpg::plugin::DefaultNativePluginShimHints();
    const auto report = urpg::plugin::AnalyzePluginCompatibility(analysis_input);

    nlohmann::json plugins = nlohmann::json::array();
    for (const auto& manifest : inspection.manifests) {
        std::error_code relative_error;
        auto relative_path = std::filesystem::relative(manifest.source_path, project_root, relative_error);
        if (relative_error || relative_path.empty() || relative_path.is_absolute()) {
            relative_path = std::filesystem::path("js") / "plugins" /
                            (manifest.plugin_id.empty() ? "unknown.js" : manifest.plugin_id + ".js");
        }
        nlohmann::json dependencies = nlohmann::json::array();
        for (const auto& dependency : manifest.dependencies) {
            dependencies.push_back({{"plugin_id", dependency.plugin_id},
                                    {"version_range", dependency.version_range},
                                    {"optional", dependency.optional}});
        }
        plugins.push_back({{"plugin_id", manifest.plugin_id},
                           {"version", manifest.version},
                           {"enabled", manifest.enabled},
                           {"relative_source_path", relative_path.generic_string()},
                           {"source_sha256", sha256File(manifest.source_path)},
                           {"dependencies", std::move(dependencies)}});
    }
    return {{"schema_version", "urpg.mz_plugin_static_lock.v1"},
            {"inspection_only", true},
            {"plugins", std::move(plugins)},
            {"load_order", report.load_order}};
}

bool isValidMzPluginStaticLock(const nlohmann::json& lock) {
    if (!lock.is_object() || lock.value("schema_version", "") != "urpg.mz_plugin_static_lock.v1" ||
        !lock.contains("inspection_only") || !lock["inspection_only"].is_boolean() || !lock["inspection_only"] ||
        !lock.contains("plugins") || !lock["plugins"].is_array() || !lock.contains("load_order") ||
        !lock["load_order"].is_array()) {
        return false;
    }
    const auto is_safe_relative_path = [](const std::string& value) {
        const auto path = std::filesystem::path(value).lexically_normal();
        if (value.empty() || path.is_absolute()) {
            return false;
        }
        return std::none_of(path.begin(), path.end(), [](const std::filesystem::path& segment) {
            return segment == "..";
        });
    };
    const auto is_sha256_hex = [](const std::string& value) {
        return value.size() == 64 && std::all_of(value.begin(), value.end(), [](const unsigned char character) {
            return std::isxdigit(character) != 0;
        });
    };
    std::set<std::string> plugin_ids;
    for (const auto& plugin : lock["plugins"]) {
        if (!plugin.is_object() || !plugin.contains("plugin_id") || !plugin["plugin_id"].is_string()) {
            return false;
        }
        const auto plugin_id = plugin["plugin_id"].get<std::string>();
        if (plugin_id.empty() || !plugin_ids.insert(plugin_id).second || !plugin.contains("version") ||
            !plugin["version"].is_string() || !plugin.contains("enabled") || !plugin["enabled"].is_boolean() ||
            !plugin.contains("relative_source_path") || !plugin["relative_source_path"].is_string() ||
            !is_safe_relative_path(plugin["relative_source_path"].get<std::string>()) ||
            !plugin.contains("source_sha256") || !plugin["source_sha256"].is_string() ||
            !is_sha256_hex(plugin["source_sha256"].get<std::string>()) ||
            !plugin.contains("dependencies") || !plugin["dependencies"].is_array()) {
            return false;
        }
        for (const auto& dependency : plugin["dependencies"]) {
            if (!dependency.is_object() || !dependency.contains("plugin_id") || !dependency["plugin_id"].is_string() ||
                !dependency.contains("version_range") || !dependency["version_range"].is_string() ||
                dependency["plugin_id"].get<std::string>().empty() || !dependency.contains("optional") ||
                !dependency["optional"].is_boolean()) {
                return false;
            }
        }
    }
    std::set<std::string> ordered_plugin_ids;
    for (const auto& plugin_id : lock["load_order"]) {
        if (!plugin_id.is_string() || plugin_id.get<std::string>().empty() ||
            !plugin_ids.contains(plugin_id.get<std::string>()) ||
            !ordered_plugin_ids.insert(plugin_id.get<std::string>()).second) {
            return false;
        }
    }
    return ordered_plugin_ids == plugin_ids;
}

std::string abilityAssetFileName(const urpg::ability::AuthoredAbilityAsset& asset) {
    std::string stem;
    stem.reserve(asset.ability_id.size());
    for (const unsigned char ch : asset.ability_id) {
        if (std::isalnum(ch) || ch == '_' || ch == '-') {
            stem.push_back(static_cast<char>(ch));
        } else if (ch == '.' || ch == '/' || ch == '\\' || ch == ' ') {
            stem.push_back('_');
        }
    }

    if (stem.empty()) {
        stem = "draft";
    }
    return stem + ".json";
}

bool loadGridPartCatalog(const std::filesystem::path& projectRoot, urpg::map::GridPartCatalog& catalog) {
    std::string error;
    return urpg::map::LoadGridPartCatalogFromProject(
               projectRoot, catalog, std::filesystem::path("content") / "part_catalogs" / "base_jrpg_parts.json", &error) &&
           catalog.size() > 0;
}

urpg::editor::PropPlacementPanel::ScreenProjectionSettings
makeEditorPreviewProjection(const urpg::map::GridPartDocument& document) {
    urpg::editor::PropPlacementPanel::ScreenProjectionSettings projection;
    projection.viewportWidth = 1280.0f;
    projection.viewportHeight = 720.0f;
    projection.cameraCenterX = static_cast<float>(document.width()) * 0.5f;
    projection.cameraCenterZ = static_cast<float>(document.height()) * 0.5f;
    projection.worldUnitsPerPixel = 1.0f / 48.0f;
    return projection;
}

std::vector<urpg::map::MapRegionRule> makeEditorPreviewRegionRules() {
    return {
        {"editor_rain_path", 2, 1, 5, 4, "", "rain_loop", "rain", "", "normal", ""},
    };
}

void bindLevelBuilder(EditorPanelRuntime& runtime) {
    runtime.level_builder_overlay.mapId = runtime.level_builder_document.mapId();
    runtime.level_builder_overlay.elevation.width = static_cast<uint32_t>(runtime.level_builder_document.width());
    runtime.level_builder_overlay.elevation.height = static_cast<uint32_t>(runtime.level_builder_document.height());
    runtime.level_builder_overlay.elevation.levels.assign(
        static_cast<size_t>(runtime.level_builder_overlay.elevation.width) *
            static_cast<size_t>(runtime.level_builder_overlay.elevation.height),
        0);

    if (runtime.perspective_2d_scene != nullptr) {
        runtime.perspective_2d_scene->setProjectRoot(runtime.project_root);
        runtime.perspective_2d_scene->setDialogueLocaleCatalog(
            loadProjectDialogueLocaleCatalog(runtime.project_root, {}));
        runtime.perspective_2d_scene->setAudioCore(std::shared_ptr<urpg::audio::AudioCore>(
            &runtime.audio_preview_core, [](urpg::audio::AudioCore*) {}));
    }

    const bool catalogLoaded = loadGridPartCatalog(runtime.project_root, runtime.level_builder_catalog);
    runtime.level_builder_workspace.SetTargets(&runtime.level_builder_document,
                                               catalogLoaded ? &runtime.level_builder_catalog : nullptr,
                                               &runtime.level_builder_overlay,
                                               runtime.perspective_2d_scene.get());
    runtime.perspective_2d_workspace.SetTargets(runtime.perspective_2d_scene.get(), &runtime.level_builder_overlay);
    runtime.perspective_2d_workspace.SetGridPartTargets(
        &runtime.level_builder_document, catalogLoaded ? &runtime.level_builder_catalog : nullptr);
    const auto projection = makeEditorPreviewProjection(runtime.level_builder_document);
    runtime.level_builder_workspace.SetProjectionSettings(projection);
    runtime.perspective_2d_workspace.SetProjectionSettings(projection);
    const auto previewRules = makeEditorPreviewRegionRules();
    runtime.perspective_2d_workspace.LoadRegionRules(previewRules);
    runtime.perspective_2d_workspace.LoadEnvironmentPreview(
        urpg::map::MapEnvironmentPreviewDocument::fromRegionRules(
            runtime.level_builder_document.mapId(),
            runtime.level_builder_document.width(),
            runtime.level_builder_document.height(),
            previewRules));
    runtime.perspective_2d_workspace.SelectEnvironmentTile(3, 2);
    urpg::map::TerrainBrush previewBrush;
    previewBrush.mode = urpg::map::TerrainBrushMode::Rectangle;
    previewBrush.width = 2;
    previewBrush.height = 2;
    previewBrush.tile_id = 1;
    runtime.perspective_2d_workspace.PreviewTerrainBrush(previewBrush, 3, 2, 1);
    runtime.perspective_2d_workspace.GenerateProceduralMap(
        {"editor_preview_seed",
         "dungeon",
         runtime.level_builder_document.width(),
         runtime.level_builder_document.height(),
         1,
         false,
         false,
         false});
    if (!runtime.project_root.empty()) {
        (void)runtime.perspective_2d_workspace.SetProjectRoot(runtime.project_root.string());
    }
    runtime.map_authoring_workspace.bind(&runtime.level_builder_workspace, &runtime.perspective_2d_workspace);
    runtime.map_authoring_workspace.setProjectRoot(runtime.project_root);
    runtime.map_authoring_workspace.setActiveMapId(runtime.level_builder_document.mapId());
}

std::string starterMapIdForProject(const std::filesystem::path& projectRoot) {
    std::ifstream input(projectRoot / "project.json", std::ios::binary);
    const auto manifest = nlohmann::json::parse(input, nullptr, false);
    if (manifest.is_object() && manifest.contains("startup") && manifest["startup"].is_object()) {
        const auto mapId = manifest["startup"].value("map", "");
        if (!mapId.empty()) return mapId;
    }
    return "EditorPreview";
}

bool atomicWriteTextFile(const std::filesystem::path& target, std::string_view contents, std::string* error);
std::filesystem::path gameplayRecipeProjectPath(const EditorPanelRuntime& runtime);
bool loadGameplayRecipeProject(EditorPanelRuntime& runtime, std::string* error);
urpg::editor::EditorDirtySaveResult saveGameplayRecipeProject(EditorPanelRuntime& runtime);
std::filesystem::path menuStudioProjectPath(const EditorPanelRuntime& runtime);
bool loadMenuStudioProject(EditorPanelRuntime& runtime, std::string* error);
urpg::editor::EditorDirtySaveResult saveMenuStudioProject(EditorPanelRuntime& runtime);

urpg::editor::EditorDirtySaveResult saveAbilityDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "ability_save_project_unavailable", "Open a project before saving an ability draft."};
    }
    const auto asset = runtime.ability_inspector_panel.getDraftAsset();
    const auto target_path =
        urpg::ability::canonicalAbilityContentDirectory(runtime.project_root) / abilityAssetFileName(asset);
    if (!urpg::ability::saveAuthoredAbilityAssetToFile(asset, target_path)) {
        return {false, "ability_save_failed", "Failed to save ability draft to " + target_path.generic_string() + "."};
    }
    runtime.ability_inspector_panel.markDraftPersisted();
    return {true, "ability_saved",
            "Saved draft ability to " + std::filesystem::relative(target_path, runtime.project_root).generic_string() + "."};
}

std::filesystem::path characterDraftPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "characters" / (runtime.character_draft_id + ".json");
}

urpg::editor::EditorDirtySaveResult saveCharacterDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "character_save_project_unavailable", "Open a project before saving a character draft."};
    }
    std::string error;
    const auto target = characterDraftPath(runtime);
    if (!atomicWriteTextFile(target, runtime.character_creator_model.getIdentity().toJson().dump(2) + "\n", &error)) {
        return {false, "character_save_failed", "Failed to save character draft: " + error};
    }
    runtime.character_creator_model.markDraftPersisted();
    return {true, "character_saved",
            "Saved character draft to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path questDraftPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "quests" / (runtime.quest_draft_id + ".json");
}

urpg::editor::EditorDirtySaveResult saveQuestDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty() || !runtime.quest_draft.has_value()) {
        return {false, "quest_save_unavailable", "Create or open a quest draft before saving it."};
    }
    std::string error;
    const auto target = questDraftPath(runtime);
    if (!atomicWriteTextFile(target, runtime.quest_draft->toJson().dump(2) + "\n", &error)) {
        return {false, "quest_save_failed", "Failed to save quest draft: " + error};
    }
    return {true, "quest_saved",
            "Saved quest draft to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path dialogueDraftPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "dialogues" / (runtime.dialogue_draft_id + ".json");
}

urpg::editor::EditorDirtySaveResult saveDialogueDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty() || !runtime.dialogue_draft.has_value()) {
        return {false, "dialogue_save_unavailable", "Create or open a dialogue draft before saving it."};
    }
    std::string error;
    const auto target = dialogueDraftPath(runtime);
    if (!atomicWriteTextFile(target, runtime.dialogue_draft->serialize().dump(2) + "\n", &error)) {
        return {false, "dialogue_save_failed", "Failed to save dialogue draft: " + error};
    }
    return {true, "dialogue_saved",
            "Saved dialogue draft to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path mzPluginStaticLockPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "compat" / "mz_plugin_lock.json";
}

urpg::editor::EditorDirtySaveResult saveMzPluginStaticLock(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty() || !isValidMzPluginStaticLock(runtime.mz_plugin_lock_draft)) {
        return {false, "mz_plugin_lock_save_invalid",
                "Refresh the static MZ plugin lock before saving it to the active project."};
    }
    std::string error;
    const auto target = mzPluginStaticLockPath(runtime);
    if (!atomicWriteTextFile(target, runtime.mz_plugin_lock_draft.dump(2) + "\n", &error)) {
        return {false, "mz_plugin_lock_save_failed", "Failed to save the MZ plugin lock: " + error};
    }
    return {true, "mz_plugin_lock_saved",
            "Saved static MZ plugin lock to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

void loadMzPluginStaticLock(EditorPanelRuntime& runtime) {
    runtime.mz_plugin_lock_draft = nlohmann::json::object();
    runtime.mz_plugin_lock_status.clear();
    std::ifstream input(mzPluginStaticLockPath(runtime), std::ios::binary);
    if (!input.good()) {
        runtime.mz_plugin_lock_status = "No static MZ plugin lock is saved for this project.";
        return;
    }
    const auto parsed = nlohmann::json::parse(input, nullptr, false);
    if (!isValidMzPluginStaticLock(parsed)) {
        runtime.mz_plugin_lock_status = "Saved MZ plugin lock is invalid and was not loaded.";
        return;
    }
    runtime.mz_plugin_lock_draft = parsed;
    runtime.mz_plugin_lock_status = "Loaded static MZ plugin lock; it remains non-executing compatibility metadata.";
}

constexpr size_t kQuestHistoryLimit = 64;
constexpr size_t kDialogueHistoryLimit = 64;

bool applyQuestGraphMutation(EditorPanelRuntime& runtime, urpg::quest::QuestObjectiveGraphDocument next) {
    if (!runtime.quest_draft.has_value() || runtime.quest_draft->toJson() == next.toJson()) {
        return false;
    }
    runtime.quest_undo_history.push_back(*runtime.quest_draft);
    if (runtime.quest_undo_history.size() > kQuestHistoryLimit) {
        runtime.quest_undo_history.erase(runtime.quest_undo_history.begin());
    }
    runtime.quest_redo_history.clear();
    runtime.quest_draft = std::move(next);
    (void)runtime.dirty_state_registry.markDirty(kQuestDirtyDocumentId, true);
    return true;
}

bool undoQuestGraphMutation(EditorPanelRuntime& runtime) {
    if (!runtime.quest_draft.has_value() || runtime.quest_undo_history.empty()) {
        return false;
    }
    runtime.quest_redo_history.push_back(*runtime.quest_draft);
    runtime.quest_draft = std::move(runtime.quest_undo_history.back());
    runtime.quest_undo_history.pop_back();
    (void)runtime.dirty_state_registry.markDirty(kQuestDirtyDocumentId, true);
    return true;
}

bool redoQuestGraphMutation(EditorPanelRuntime& runtime) {
    if (!runtime.quest_draft.has_value() || runtime.quest_redo_history.empty()) {
        return false;
    }
    runtime.quest_undo_history.push_back(*runtime.quest_draft);
    runtime.quest_draft = std::move(runtime.quest_redo_history.back());
    runtime.quest_redo_history.pop_back();
    (void)runtime.dirty_state_registry.markDirty(kQuestDirtyDocumentId, true);
    return true;
}

bool applyDialogueGraphMutation(EditorPanelRuntime& runtime, urpg::dialogue::DialogueGraph next) {
    if (!runtime.dialogue_draft.has_value() || runtime.dialogue_draft->serialize() == next.serialize()) {
        return false;
    }
    runtime.dialogue_undo_history.push_back(*runtime.dialogue_draft);
    if (runtime.dialogue_undo_history.size() > kDialogueHistoryLimit) {
        runtime.dialogue_undo_history.erase(runtime.dialogue_undo_history.begin());
    }
    runtime.dialogue_redo_history.clear();
    runtime.dialogue_draft = std::move(next);
    runtime.dialogue_preview_node_id.clear();
    runtime.dialogue_preview_trace.clear();
    runtime.dialogue_preview_diagnostics.clear();
    (void)runtime.dirty_state_registry.markDirty(kDialogueDirtyDocumentId, true);
    return true;
}

bool undoDialogueGraphMutation(EditorPanelRuntime& runtime) {
    if (!runtime.dialogue_draft.has_value() || runtime.dialogue_undo_history.empty()) {
        return false;
    }
    runtime.dialogue_redo_history.push_back(*runtime.dialogue_draft);
    runtime.dialogue_draft = std::move(runtime.dialogue_undo_history.back());
    runtime.dialogue_undo_history.pop_back();
    runtime.dialogue_preview_node_id.clear();
    runtime.dialogue_preview_trace.clear();
    runtime.dialogue_preview_diagnostics.clear();
    (void)runtime.dirty_state_registry.markDirty(kDialogueDirtyDocumentId, true);
    return true;
}

bool redoDialogueGraphMutation(EditorPanelRuntime& runtime) {
    if (!runtime.dialogue_draft.has_value() || runtime.dialogue_redo_history.empty()) {
        return false;
    }
    runtime.dialogue_undo_history.push_back(*runtime.dialogue_draft);
    runtime.dialogue_draft = std::move(runtime.dialogue_redo_history.back());
    runtime.dialogue_redo_history.pop_back();
    runtime.dialogue_preview_node_id.clear();
    runtime.dialogue_preview_trace.clear();
    runtime.dialogue_preview_diagnostics.clear();
    (void)runtime.dirty_state_registry.markDirty(kDialogueDirtyDocumentId, true);
    return true;
}

std::set<std::string> databaseItemIds(const urpg::database::RpgDatabase& database) {
    std::set<std::string> ids;
    for (const auto& [id, _] : database.items()) ids.insert(id);
    return ids;
}

void syncQuestPreviewWorldFromPerspectiveRuntime(
    const urpg::editor::SpatialAuthoringWorkspace::Perspective2DRuntimeResult& runtime_result,
    urpg::quest::QuestWorldState& world) {
    auto battle_preview_outcomes = std::move(world.battles);
    world = {};
    world.battles = std::move(battle_preview_outcomes);
    for (const auto& entry : runtime_result.switches) {
        world.switches[entry.key] = entry.value == "true" || entry.value == "1" || entry.value == "on";
    }
    for (const auto& entry : runtime_result.variables) {
        try {
            world.variables[entry.key] = std::stoi(entry.value);
        } catch (const std::exception&) {
            // Event execution only writes parsed integer variables. Keep this
            // boundary defensive so a malformed external draft cannot claim a
            // quest condition was met.
        }
    }
    for (const auto& entry : runtime_result.inventory) {
        if (entry.value != "0") {
            world.items.push_back(entry.key);
        }
    }
    world.dialogue_choices = runtime_result.dialogue_choices;
    // A start_battle command is an encounter launch, not a combat result. Do
    // not populate world.battles until the native battle runtime reports an
    // outcome through its own owner.
}

bool openMessageInspectorForPerspectiveRuntime(
    urpg::editor::EditorShell& editor_shell, EditorPanelRuntime& runtime,
    const urpg::editor::SpatialAuthoringWorkspace::Perspective2DRuntimeResult& runtime_result,
    const std::string& event_label) {
    if (runtime_result.messages.empty() && runtime_result.dialogue_choices.empty()) {
        return false;
    }

    std::vector<urpg::message::DialoguePage> pages;
    pages.reserve(std::max<size_t>(size_t{1}, runtime_result.messages.size()));
    for (size_t index = 0; index < runtime_result.messages.size(); ++index) {
        urpg::message::DialoguePage page;
        page.id = runtime_result.event_id + ".message." + std::to_string(index + 1);
        page.body = runtime_result.messages[index];
        page.variant.mode = urpg::message::MessagePresentationMode::Speaker;
        page.variant.tone = urpg::message::MessageTone::Portrait;
        page.variant.speaker = event_label.empty() ? runtime_result.event_id : event_label;
        pages.push_back(std::move(page));
    }
    if (pages.empty()) {
        urpg::message::DialoguePage page;
        page.id = runtime_result.event_id + ".choice";
        page.body = "Choose a response.";
        page.variant.mode = urpg::message::MessagePresentationMode::Speaker;
        page.variant.tone = urpg::message::MessageTone::Portrait;
        page.variant.speaker = event_label.empty() ? runtime_result.event_id : event_label;
        pages.push_back(std::move(page));
    }
    for (size_t index = 0; index < runtime_result.dialogue_choices.size(); ++index) {
        const auto& choice = runtime_result.dialogue_choices[index];
        pages.back().choices.push_back({runtime_result.event_id + ".choice." + std::to_string(index + 1), choice, true, {}});
    }

    runtime.map_message_preview_flow.resetWithPages(std::move(pages));
    runtime.diagnostics_workspace.bindMessageRuntime(runtime.map_message_preview_flow, runtime.map_message_preview_layout);
    runtime.diagnostics_workspace.setActiveTab(urpg::editor::DiagnosticsTab::MessageText);
    (void)editor_shell.openPanel("diagnostics");
    runtime.focus_workspace_next_frame = true;
    return true;
}

urpg::editor::EditorDirtySaveResult saveDatabaseDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "database_save_project_unavailable", "Open a project before saving database items."};
    }
    std::string error;
    const auto target = runtime.project_root / "content" / "database.json";
    if (!atomicWriteTextFile(target, runtime.database_draft.toJson().dump(2) + "\n", &error)) {
        return {false, "database_save_failed", "Failed to save database items: " + error};
    }
    return {true, "database_saved", "Saved project database items."};
}

std::filesystem::path vendorDraftPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "vendors" / (runtime.vendor_draft_id + ".json");
}

urpg::editor::EditorDirtySaveResult saveVendorDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty() || runtime.vendor_draft.findVendor(runtime.vendor_draft_id) == nullptr) {
        return {false, "vendor_save_unavailable", "Create or open vendor stock before saving it."};
    }
    std::string error;
    const auto target = vendorDraftPath(runtime);
    if (!atomicWriteTextFile(target, runtime.vendor_draft.toJson().dump(2) + "\n", &error)) {
        return {false, "vendor_save_failed", "Failed to save vendor stock: " + error};
    }
    return {true, "vendor_saved",
            "Saved vendor stock to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path audioMixDraftPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "config" / "audio_mix_presets.json";
}

nlohmann::json audioMixDraftJson(const EditorPanelRuntime& runtime) {
    auto json = runtime.audio_mix_draft.toJson();
    json["schema"] = "urpg.project_audio_mix.v1";
    json["active_preset"] = runtime.audio_mix_preset;
    json["encounter_preview_asset_id"] = runtime.audio_preview_asset_id;
    return json;
}

urpg::editor::EditorDirtySaveResult saveAudioMixDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "audio_mix_save_project_unavailable", "Open a project before saving an audio mix."};
    }
    std::string error;
    const auto target = audioMixDraftPath(runtime);
    if (!atomicWriteTextFile(target, audioMixDraftJson(runtime).dump(2) + "\n", &error)) {
        return {false, "audio_mix_save_failed", "Failed to save audio mix: " + error};
    }
    return {true, "audio_mix_saved",
            "Saved audio mix to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path inputRemapDraftPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "config" / "input_remap.json";
}

urpg::editor::EditorDirtySaveResult saveInputRemapDraft(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "input_remap_save_project_unavailable", "Open a project before saving input remaps."};
    }
    std::string error;
    const auto target = inputRemapDraftPath(runtime);
    if (!atomicWriteTextFile(target, runtime.input_remap_draft.saveToJson().dump(2) + "\n", &error)) {
        return {false, "input_remap_save_failed", "Failed to save input remaps: " + error};
    }
    return {true, "input_remap_saved",
            "Saved input remaps to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path gameplayRecipeProjectPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "gameplay" / "recipes.json";
}

bool loadGameplayRecipeProject(EditorPanelRuntime& runtime, std::string* error) {
    const auto target = gameplayRecipeProjectPath(runtime);
    if (!std::filesystem::exists(target)) {
        runtime.gameplay_recipe_panel.loadProject({});
        return true;
    }
    std::ifstream input(target, std::ios::binary);
    const auto json = nlohmann::json::parse(input, nullptr, false);
    if (!json.is_object()) {
        if (error) *error = "The gameplay recipe project document is not a JSON object.";
        return false;
    }
    const auto document = urpg::gameplay::GameplayRecipeProjectDocument::fromJson(json);
    const auto diagnostics = document.validate();
    if (!diagnostics.empty()) {
        if (error) {
            *error = "Gameplay recipe project validation failed: " + diagnostics.front().code + ".";
        }
        return false;
    }
    runtime.gameplay_recipe_panel.loadProject(document);
    return true;
}

urpg::editor::EditorDirtySaveResult saveGameplayRecipeProject(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "gameplay_recipe_save_project_unavailable", "Open a project before saving gameplay recipes."};
    }
    const auto diagnostics = runtime.gameplay_recipe_panel.project().validate();
    if (!diagnostics.empty()) {
        return {false, "gameplay_recipe_save_invalid", "Gameplay recipe project is invalid: " + diagnostics.front().code + "."};
    }
    std::string error;
    const auto target = gameplayRecipeProjectPath(runtime);
    if (!atomicWriteTextFile(target, runtime.gameplay_recipe_panel.project().toJson().dump(2) + "\n", &error)) {
        return {false, "gameplay_recipe_save_failed", "Failed to save gameplay recipes: " + error};
    }
    runtime.gameplay_recipe_panel.markProjectDocumentPersisted();
    return {true, "gameplay_recipe_saved",
            "Saved gameplay recipes to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

std::filesystem::path menuStudioProjectPath(const EditorPanelRuntime& runtime) {
    return runtime.project_root / "content" / "ui" / "menus.json";
}

void registerNativeMenuStudioCommands(urpg::ui::MenuCommandRegistry& registry) {
    registry.clear();
    const std::vector<urpg::MenuCommandMeta> commands = {
        {"urpg.menu.item", "Items", "", urpg::MenuRouteTarget::Item},
        {"urpg.menu.status", "Status", "", urpg::MenuRouteTarget::Status},
        {"urpg.menu.options", "Options", "", urpg::MenuRouteTarget::Options},
        {"urpg.menu.save", "Save", "", urpg::MenuRouteTarget::Save},
    };
    for (const auto& command : commands) {
        registry.registerCommand(command);
    }
}

void initializeNativeMenuStudioDefault(EditorPanelRuntime& runtime) {
    auto& menu_runtime = runtime.menu_studio_runtime;
    auto& graph = menu_runtime.getSceneGraphMutable();
    auto& registry = menu_runtime.getRegistryMutable();
    graph.clearRegisteredScenes();
    registerNativeMenuStudioCommands(registry);

    auto scene = std::make_shared<urpg::ui::MenuScene>("MainMenu");
    urpg::ui::MenuPane pane;
    pane.id = "main";
    pane.displayName = "Main Menu";
    pane.isActive = true;
    pane.layout = {.x = 64, .y = 64, .width = 360, .height = 260, .z_order = 0, .focus_order = 0};
    pane.commands = registry.listCommands();
    scene->addPane(pane);
    graph.registerScene(std::move(scene));
    (void)graph.restoreActiveScene("MainMenu");
    graph.setCommandStateFromRegistry(registry, {}, {});
}

std::string menuStudioSerializedGraph(const EditorPanelRuntime& runtime) {
    return urpg::ui::MenuSceneSerializer::SerializeGraph(runtime.menu_studio_runtime.getSceneGraph()).dump(2);
}

bool loadMenuStudioProject(EditorPanelRuntime& runtime, std::string* error) {
    const auto target = menuStudioProjectPath(runtime);
    if (!std::filesystem::exists(target)) {
        initializeNativeMenuStudioDefault(runtime);
        runtime.menu_studio_persisted_json = menuStudioSerializedGraph(runtime);
        return true;
    }

    std::ifstream input(target, std::ios::binary);
    const auto document = nlohmann::json::parse(input, nullptr, false);
    nlohmann::json graph_document;
    if (document.is_object() && document.contains("scenes") && document["scenes"].is_array()) {
        graph_document = document;
    } else {
        graph_document = {{"scenes", nlohmann::json::array({document})}};
    }
    if (!document.is_object() || !urpg::ui::MenuSceneSerializer::DeserializeGraph(
                                   graph_document, runtime.menu_studio_runtime.getSceneGraphMutable())) {
        if (error) *error = "The native menu document is invalid and was not loaded.";
        return false;
    }

    auto& registry = runtime.menu_studio_runtime.getRegistryMutable();
    registerNativeMenuStudioCommands(registry);
    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;
    urpg::ui::MenuCommandRegistry::captureGlobalState(switches, variables);
    runtime.menu_studio_runtime.getSceneGraphMutable().setCommandStateFromRegistry(registry, switches, variables);
    runtime.menu_studio_persisted_json = menuStudioSerializedGraph(runtime);
    return true;
}

urpg::editor::EditorDirtySaveResult saveMenuStudioProject(EditorPanelRuntime& runtime) {
    if (runtime.project_root.empty()) {
        return {false, "menu_studio_save_project_unavailable", "Open a project before saving the native menu document."};
    }
    std::string error;
    const auto target = menuStudioProjectPath(runtime);
    const auto serialized = menuStudioSerializedGraph(runtime);
    if (!atomicWriteTextFile(target, serialized + "\n", &error)) {
        return {false, "menu_studio_save_failed", "Failed to save the native menu document: " + error};
    }
    runtime.menu_studio_persisted_json = serialized;
    return {true, "menu_studio_saved",
            "Saved native menu document to " + std::filesystem::relative(target, runtime.project_root).generic_string() + "."};
}

void bindMenuStudioProject(EditorPanelRuntime& runtime, const std::filesystem::path& project_root) {
    runtime.project_root = project_root;
    std::string error;
    if (!loadMenuStudioProject(runtime, &error)) {
        initializeNativeMenuStudioDefault(runtime);
        runtime.menu_studio_persisted_json = menuStudioSerializedGraph(runtime);
        runtime.menu_studio_load_status = error;
    } else {
        runtime.menu_studio_load_status.clear();
    }
    runtime.menu_studio_save_status.clear();

    urpg::ui::MenuCommandRegistry::SwitchState switches;
    urpg::ui::MenuCommandRegistry::VariableState variables;
    urpg::ui::MenuCommandRegistry::captureGlobalState(switches, variables);
    runtime.diagnostics_workspace.bindMenuRuntime(runtime.menu_studio_runtime.getSceneGraphMutable(),
                                                  runtime.menu_studio_runtime.getRegistry(), switches, variables);
}

void captureRecoverySnapshot(EditorPanelRuntime& runtime) {
    if (!runtime.project_session.isOpen() || runtime.project_root.empty()) return;
    const auto mapDirty = runtime.dirty_state_registry.isDirty(kMapDirtyDocumentId) ||
                          runtime.dirty_state_registry.isDirty(kPerspective2DDirtyDocumentId);
    const auto abilityDirty = runtime.dirty_state_registry.isDirty("ability.draft");
    const auto characterDirty = runtime.dirty_state_registry.isDirty(kCharacterDirtyDocumentId);
    const auto questDirty = runtime.dirty_state_registry.isDirty(kQuestDirtyDocumentId);
    const auto dialogueDirty = runtime.dirty_state_registry.isDirty(kDialogueDirtyDocumentId);
    const auto databaseDirty = runtime.dirty_state_registry.isDirty(kDatabaseDirtyDocumentId);
    const auto vendorDirty = runtime.dirty_state_registry.isDirty(kVendorDirtyDocumentId);
    const auto audioMixDirty = runtime.dirty_state_registry.isDirty(kAudioMixDirtyDocumentId);
    const auto inputRemapDirty = runtime.dirty_state_registry.isDirty(kInputRemapDirtyDocumentId);
    const auto gameplayRecipeDirty = runtime.dirty_state_registry.isDirty(kGameplayRecipeDirtyDocumentId);
    const auto menuStudioDirty = runtime.dirty_state_registry.isDirty(kMenuStudioDirtyDocumentId);
    const auto mzPluginLockDirty = runtime.dirty_state_registry.isDirty(kMzPluginLockDirtyDocumentId);
    if (!mapDirty && !abilityDirty && !characterDirty && !questDirty && !dialogueDirty && !databaseDirty && !vendorDirty &&
        !audioMixDirty && !inputRemapDirty && !gameplayRecipeDirty && !menuStudioDirty && !mzPluginLockDirty) return;

    std::vector<urpg::editor::RecoveryDocumentDraft> drafts;
    if (mapDirty) {
        const auto perspectiveDraft = runtime.perspective_2d_workspace.PreparePerspectiveMapDraftSave();
        if (!perspectiveDraft.success) {
            runtime.recovery_status = "Recovery snapshot deferred: " + perspectiveDraft.message;
            return;
        }
        const auto mapStem = runtime.level_builder_document.mapId();
        drafts.push_back({kMapDirtyDocumentId, std::filesystem::path("content") / "maps" / (mapStem + ".grid.json"),
                          urpg::map::GridPartDocumentToJson(runtime.level_builder_document).dump(2) + "\n"});
        drafts.push_back({kPerspective2DDirtyDocumentId,
                          std::filesystem::path("content") / "maps" / (mapStem + ".p2d.json"),
                          perspectiveDraft.serialized_document_json + "\n"});
    }
    if (abilityDirty) {
        const auto ability = runtime.ability_inspector_panel.getDraftAsset();
        drafts.push_back({"ability.draft", std::filesystem::path("content") / "abilities" / abilityAssetFileName(ability),
                          nlohmann::json(ability).dump(2) + "\n"});
    }
    if (characterDirty) {
        drafts.push_back({kCharacterDirtyDocumentId, std::filesystem::path("content") / "characters" /
                                                        (runtime.character_draft_id + ".json"),
                          runtime.character_creator_model.getIdentity().toJson().dump(2) + "\n"});
    }
    if (questDirty && runtime.quest_draft.has_value()) {
        drafts.push_back({kQuestDirtyDocumentId, std::filesystem::path("content") / "quests" /
                                                   (runtime.quest_draft_id + ".json"),
                          runtime.quest_draft->toJson().dump(2) + "\n"});
    }
    if (dialogueDirty && runtime.dialogue_draft.has_value()) {
        drafts.push_back({kDialogueDirtyDocumentId, std::filesystem::path("content") / "dialogues" /
                                                     (runtime.dialogue_draft_id + ".json"),
                          runtime.dialogue_draft->serialize().dump(2) + "\n"});
    }
    if (databaseDirty) {
        drafts.push_back({kDatabaseDirtyDocumentId, std::filesystem::path("content") / "database.json",
                          runtime.database_draft.toJson().dump(2) + "\n"});
    }
    if (vendorDirty) {
        drafts.push_back({kVendorDirtyDocumentId, std::filesystem::path("content") / "vendors" /
                                                     (runtime.vendor_draft_id + ".json"),
                          runtime.vendor_draft.toJson().dump(2) + "\n"});
    }
    if (audioMixDirty) {
        drafts.push_back({kAudioMixDirtyDocumentId, std::filesystem::path("config") / "audio_mix_presets.json",
                          audioMixDraftJson(runtime).dump(2) + "\n"});
    }
    if (inputRemapDirty) {
        drafts.push_back({kInputRemapDirtyDocumentId, std::filesystem::path("config") / "input_remap.json",
                          runtime.input_remap_draft.saveToJson().dump(2) + "\n"});
    }
    if (gameplayRecipeDirty) {
        drafts.push_back({kGameplayRecipeDirtyDocumentId, std::filesystem::path("content") / "gameplay" / "recipes.json",
                          runtime.gameplay_recipe_panel.project().toJson().dump(2) + "\n"});
    }
    if (menuStudioDirty) {
        drafts.push_back({kMenuStudioDirtyDocumentId, std::filesystem::path("content") / "ui" / "menus.json",
                          menuStudioSerializedGraph(runtime) + "\n"});
    }
    if (mzPluginLockDirty && isValidMzPluginStaticLock(runtime.mz_plugin_lock_draft)) {
        drafts.push_back({kMzPluginLockDirtyDocumentId, std::filesystem::path("content") / "compat" /
                                                             "mz_plugin_lock.json",
                          runtime.mz_plugin_lock_draft.dump(2) + "\n"});
    }

    const auto dirtyDocumentIds = runtime.dirty_state_registry.dirtyDocumentIds();
    if (runtime.recovery_service.createRecoverySnapshot(runtime.project_root, runtime.project_session.activeProject().project_id,
                                                        dirtyDocumentIds, drafts)) {
        runtime.recovery_service.pruneSnapshots(runtime.project_root, 12, 512ULL * 1024ULL * 1024ULL);
        runtime.recovery_status = "Recovery snapshot captured privately; manual saves remain unchanged.";
    } else {
        runtime.recovery_status = "Recovery snapshot could not be captured; unsaved work remains only in this editor session.";
    }
}

void captureScheduledRecoverySnapshot(EditorPanelRuntime& runtime) {
    if (runtime.creator_mode || !runtime.project_session.isOpen()) return;
    const auto now = std::chrono::steady_clock::now();
    if (runtime.next_recovery_snapshot_at != std::chrono::steady_clock::time_point{} &&
        now < runtime.next_recovery_snapshot_at) {
        return;
    }
    captureRecoverySnapshot(runtime);
    runtime.next_recovery_snapshot_at = now + std::chrono::minutes(5);
}

void bindMapAuthoringProject(EditorPanelRuntime& runtime,
                             const std::filesystem::path& projectRoot,
                             std::string requestedMapId = {}) {
    runtime.project_root = projectRoot;
    runtime.dialogue_localization_key_options = collectProjectLocalizationKeys(projectRoot);
    runtime.dialogue_speaker_options = collectProjectDialogueSpeakerOptions(projectRoot);
    runtime.dialogue_voice_asset_options = collectProjectDialogueVoiceAssetOptions(projectRoot);
    std::string gameplayRecipeError;
    if (!loadGameplayRecipeProject(runtime, &gameplayRecipeError)) {
        runtime.gameplay_recipe_panel.loadProject({});
        runtime.map_save_status = gameplayRecipeError;
        runtime.gameplay_recipe_load_status = gameplayRecipeError;
    } else {
        runtime.gameplay_recipe_load_status.clear();
    }
    runtime.gameplay_recipe_save_status.clear();
    (void)runtime.mz_plugin_inspector_panel.model().inspectMzPluginScriptsFromDirectory(
        projectRoot / "js" / "plugins");
    runtime.mz_plugin_inspector_panel.render();
    loadMzPluginStaticLock(runtime);
    runtime.mz_plugin_lock_source_current = isValidMzPluginStaticLock(runtime.mz_plugin_lock_draft) &&
                                            runtime.mz_plugin_lock_draft == buildMzPluginStaticLock(projectRoot);
    runtime.available_map_ids.clear();
    std::error_code mapDirectoryError;
    const auto mapsDirectory = projectRoot / "content" / "maps";
    for (const auto& entry : std::filesystem::directory_iterator(mapsDirectory, mapDirectoryError)) {
        if (mapDirectoryError || !entry.is_regular_file() || entry.path().extension() != ".json") {
            continue;
        }
        std::ifstream mapInput(entry.path(), std::ios::binary);
        const auto mapJson = nlohmann::json::parse(mapInput, nullptr, false);
        if (!mapJson.is_object() || mapJson.value("schema", "") != "urpg.map.v1") {
            continue;
        }
        const auto mapId = mapJson.value("id", "");
        if (!mapId.empty()) runtime.available_map_ids.push_back(mapId);
    }
    std::sort(runtime.available_map_ids.begin(), runtime.available_map_ids.end());
    runtime.available_map_ids.erase(std::unique(runtime.available_map_ids.begin(), runtime.available_map_ids.end()),
                                    runtime.available_map_ids.end());
    runtime.character_draft_id = "protagonist";
    runtime.quest_draft_id = "quest_draft";
    runtime.dialogue_draft_id = "dialogue_draft";
    runtime.vendor_draft_id = "vendor_draft";
    bool isCreatorVerticalSlice = false;
    {
        std::ifstream manifestInput(projectRoot / "project.json", std::ios::binary);
        const auto manifest = nlohmann::json::parse(manifestInput, nullptr, false);
        if (manifest.is_object() && manifest.contains("creator") && manifest["creator"].is_object() &&
            (manifest["creator"].value("vertical_slice_seed", "") == "lantern_of_the_willow_draft" ||
             manifest["creator"].value("scenario", "") == "creator_vertical_slice")) {
            isCreatorVerticalSlice = true;
            runtime.character_draft_id = "willow_hero";
            runtime.quest_draft_id = "restore_moonwell_lantern";
            runtime.dialogue_draft_id = "moonwell_intro";
            runtime.vendor_draft_id = "rowan_tonics";
        }
    }
    const auto characterPath = characterDraftPath(runtime);
    if (std::ifstream characterInput(characterPath, std::ios::binary); characterInput.good()) {
        try {
            runtime.character_creator_model.loadIdentity(
                urpg::character::CharacterIdentity::fromJson(nlohmann::json::parse(characterInput)));
        } catch (const std::exception&) {
            runtime.character_creator_model.resetDraft();
        }
    } else if (runtime.character_draft_id == "willow_hero") {
        urpg::character::CharacterIdentity hero;
        hero.setName("Willow Hero");
        hero.setClassId("class_ranger");
        hero.setPortraitId("portrait_ranger_01");
        hero.setBodySpriteId("sprite_ranger_body");
        runtime.character_creator_model.loadIdentity(hero);
    } else {
        runtime.character_creator_model.resetDraft();
    }
    runtime.ability_runtime = {};
    runtime.ability_runtime.setAttribute("MP", 30.0f);
    const auto authoredAbilities = urpg::ability::discoverAuthoredAbilityAssets(projectRoot);
    if (!authoredAbilities.empty()) {
        const auto preferredAbility = std::find_if(authoredAbilities.begin(), authoredAbilities.end(),
                                                   [isCreatorVerticalSlice](const auto& candidate) {
                                                       return isCreatorVerticalSlice
                                                                  ? candidate.ability_id == "willow_strike"
                                                                  : false;
                                                   });
        const auto& selectedAbility = preferredAbility != authoredAbilities.end() ? *preferredAbility
                                                                                    : authoredAbilities.front();
        if (const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(selectedAbility.absolute_path);
            asset.has_value()) {
            runtime.ability_inspector_panel.setDraftFromAsset(*asset);
            runtime.ability_inspector_panel.applyDraftToRuntime(runtime.ability_runtime);
            runtime.ability_inspector_panel.update(runtime.ability_runtime);
        } else {
            runtime.map_save_status = "An authored ability draft could not be loaded for this Map context.";
        }
    }
    runtime.quest_draft.reset();
    runtime.quest_undo_history.clear();
    runtime.quest_redo_history.clear();
    if (std::ifstream questInput(questDraftPath(runtime), std::ios::binary); questInput.good()) {
        const auto questJson = nlohmann::json::parse(questInput, nullptr, false);
        if (questJson.is_object() && questJson.value("schema_version", "") == "urpg.quest_objective_graph.v1") {
            runtime.quest_draft = urpg::quest::QuestObjectiveGraphDocument::fromJson(questJson);
        }
    }
    runtime.dialogue_draft.reset();
    runtime.dialogue_undo_history.clear();
    runtime.dialogue_redo_history.clear();
    if (std::ifstream dialogueInput(dialogueDraftPath(runtime), std::ios::binary); dialogueInput.good()) {
        const auto dialogueJson = nlohmann::json::parse(dialogueInput, nullptr, false);
        if (const auto dialogue = urpg::dialogue::DialogueGraph::fromJson(dialogueJson); dialogue.has_value()) {
            runtime.dialogue_draft = std::move(*dialogue);
        } else {
            runtime.map_save_status = "Saved dialogue draft is invalid and was not loaded.";
        }
    }
    runtime.database_draft = {};
    if (std::ifstream databaseInput(projectRoot / "content" / "database.json", std::ios::binary); databaseInput.good()) {
        runtime.database_draft = urpg::database::RpgDatabase::fromJson(nlohmann::json::parse(databaseInput, nullptr, false));
    }
    runtime.vendor_draft = {};
    if (std::ifstream vendorInput(vendorDraftPath(runtime), std::ios::binary); vendorInput.good()) {
        runtime.vendor_draft = urpg::shop::VendorCatalog::fromJson(nlohmann::json::parse(vendorInput, nullptr, false));
    }
    runtime.vendor_draft.setKnownItems(databaseItemIds(runtime.database_draft));
    runtime.audio_mix_draft.loadDefaults();
    runtime.audio_mix_preset = "Default";
    runtime.audio_preview_asset_id.clear();
    runtime.audio_preview_asset_undo.clear();
    runtime.audio_preview_asset_redo.clear();
    if (std::ifstream audioMixInput(audioMixDraftPath(runtime), std::ios::binary); audioMixInput.good()) {
        try {
            const auto audioMixJson = nlohmann::json::parse(audioMixInput);
            runtime.audio_mix_draft.fromJson(audioMixJson);
            runtime.audio_mix_preset = audioMixJson.value("active_preset", "Default");
            runtime.audio_preview_asset_id = audioMixJson.value("encounter_preview_asset_id", "");
        } catch (const std::exception&) {
            runtime.map_save_status = "Saved audio mix is invalid; the default mix was loaded instead.";
            runtime.audio_mix_draft.loadDefaults();
            runtime.audio_mix_preset = "Default";
        }
    }
    runtime.audio_preview_core.setAssetRoot(projectRoot / "content");
    runtime.audio_mix_panel.bindBank(&runtime.audio_mix_draft);
    runtime.audio_mix_panel.bindCore(&runtime.audio_preview_core);
    if (!runtime.audio_mix_panel.selectPreset(runtime.audio_mix_preset)) {
        runtime.audio_mix_preset = "Default";
        (void)runtime.audio_mix_panel.selectPreset(runtime.audio_mix_preset);
    }
    runtime.diagnostics_workspace.bindAudioRuntime(runtime.audio_preview_core);
    runtime.input_remap_draft.resetToDefaults();
    if (std::ifstream inputRemapInput(inputRemapDraftPath(runtime), std::ios::binary); inputRemapInput.good()) {
        try {
            runtime.input_remap_draft.loadFromJson(nlohmann::json::parse(inputRemapInput));
        } catch (const std::exception&) {
            runtime.map_save_status = "Saved input remaps are invalid; default mappings were restored instead.";
            runtime.input_remap_draft.resetToDefaults();
        }
    }
    const auto starterMapId = starterMapIdForProject(projectRoot);
    const auto requestedMapExists = std::find(runtime.available_map_ids.begin(), runtime.available_map_ids.end(), requestedMapId) !=
                                    runtime.available_map_ids.end();
    const auto mapId = requestedMapExists ? requestedMapId : starterMapId;
    runtime.level_builder_document = urpg::map::GridPartDocument{mapId, 16, 12};
    const auto gridPath = projectRoot / "content" / "maps" / (mapId + ".grid.json");
    if (std::ifstream gridInput(gridPath, std::ios::binary); gridInput.good()) {
        const auto gridJson = nlohmann::json::parse(gridInput, nullptr, false);
        if (const auto restored = urpg::map::GridPartDocumentFromJson(gridJson); restored.has_value() &&
            restored->mapId() == mapId) {
            runtime.level_builder_document = *restored;
        } else {
            runtime.map_save_status = "Saved Grid Parts map draft is invalid; the starter map was opened without it.";
        }
    }
    runtime.perspective_2d_scene = std::make_unique<urpg::scene::MapScene>(mapId, 16, 12);
    bindLevelBuilder(runtime);
    const auto perspectivePath = projectRoot / "content" / "maps" / (mapId + ".p2d.json");
    if (std::ifstream perspectiveInput(perspectivePath, std::ios::binary); perspectiveInput.good()) {
        const auto result = runtime.perspective_2d_workspace.LoadPerspectiveMapDraft(
            std::string{std::istreambuf_iterator<char>(perspectiveInput), {}});
        if (!result.success) {
            runtime.map_save_status = "Saved Perspective 2D map draft could not be loaded: " + result.message;
        }
    }
    runtime.map_runtime_save_catalog.clear();
    runtime.map_runtime_save_session = std::make_unique<urpg::SaveSessionCoordinator>(runtime.map_runtime_save_catalog);
    runtime.diagnostics_workspace.bindSaveRuntime(runtime.map_runtime_save_catalog, *runtime.map_runtime_save_session);
    runtime.map_authoring_workspace.refresh();
}

bool atomicWriteTextFile(const std::filesystem::path& target, std::string_view contents, std::string* error) {
    std::error_code filesystemError;
    std::filesystem::create_directories(target.parent_path(), filesystemError);
    if (filesystemError) {
        if (error) *error = filesystemError.message();
        return false;
    }
    const auto temporary = target.parent_path() / ("." + target.filename().string() + ".tmp");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output << contents;
        if (!output) {
            if (error) *error = "Unable to write temporary map save file.";
            return false;
        }
    }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        if (error) *error = "Unable to publish the atomic map save file.";
        std::filesystem::remove(temporary, filesystemError);
        return false;
    }
#else
    std::filesystem::rename(temporary, target, filesystemError);
    if (filesystemError) {
        if (error) *error = filesystemError.message();
        std::filesystem::remove(temporary, filesystemError);
        return false;
    }
#endif
    return true;
}

bool saveMapAuthoringDocument(EditorPanelRuntime& runtime, std::string* error) {
    if (runtime.project_root.empty()) {
        if (error) *error = "Open a project before saving a map.";
        return false;
    }
    const auto& document = runtime.level_builder_document;
    const auto mapPath = runtime.project_root / "content" / "maps" / (document.mapId() + ".grid.json");
    const auto perspectiveSave = runtime.perspective_2d_workspace.PreparePerspectiveMapDraftSave();
    if (!perspectiveSave.success) {
        if (error) *error = perspectiveSave.message;
        return false;
    }
    const auto perspectivePath = runtime.project_root / "content" / "maps" / (document.mapId() + ".p2d.json");
    const auto persistence = urpg::editor::publishMapAuthoringDocuments({
        {mapPath, urpg::map::GridPartDocumentToJson(document).dump(2) + "\n"},
        {perspectivePath, perspectiveSave.serialized_document_json + "\n"},
    });
    if (!persistence.success) {
        if (error) *error = persistence.message;
        return false;
    }
    const nlohmann::json manualSave = {{"schema", "urpg.creator_manual_save.v1"},
                                       {"map_id", document.mapId()},
                                       {"path", mapPath.generic_string()}};
    if (!atomicWriteTextFile(runtime.project_root / ".urpg" / "creator" / "last_manual_save.json",
                             manualSave.dump(2) + "\n", error)) {
        return false;
    }
    runtime.level_builder_workspace.MarkLevelDraftPersisted();
    runtime.perspective_2d_workspace.MarkPerspectiveMapDraftPersisted();
    runtime.map_authoring_workspace.context().markSaved(urpg::editor::MapAuthoringDocumentOwner::GridParts);
    runtime.map_authoring_workspace.context().markSaved(urpg::editor::MapAuthoringDocumentOwner::Perspective2D);
    runtime.map_authoring_workspace.refresh();
    return true;
}

std::string playtestSpawnForDocument(const urpg::map::GridPartDocument& document) {
    const auto* playerStart = urpg::map::FindPlayerSpawnPart(document);
    if (playerStart == nullptr) return "0,0";
    return std::to_string(playerStart->grid_x) + "," + std::to_string(playerStart->grid_y);
}

std::string playtestSpawnForSelectedPart(const EditorPanelRuntime& runtime) {
    const auto& selectedInstanceId = runtime.level_builder_workspace.lastRenderSnapshot().inspector.selected_instance_id;
    const auto* selected = runtime.level_builder_document.findPart(selectedInstanceId);
    if (selected == nullptr) return {};
    return std::to_string(selected->grid_x) + "," + std::to_string(selected->grid_y);
}

std::string playtestDocumentBlocker(const EditorPanelRuntime& runtime) {
    const auto& document = runtime.level_builder_document;
    if (document.parts().empty()) {
        return {};
    }
    if (runtime.level_builder_catalog.size() == 0) {
        return "Map playtest is blocked: load the project's Grid Parts catalog before launching authored parts.";
    }
    const auto validation = urpg::map::ValidateGridPartDocument(document, runtime.level_builder_catalog);
    const auto blocker = std::find_if(validation.diagnostics.begin(), validation.diagnostics.end(),
                                      [](const auto& diagnostic) {
                                          return diagnostic.severity == urpg::map::GridPartSeverity::Blocker ||
                                                 diagnostic.severity == urpg::map::GridPartSeverity::Error;
                                      });
    return blocker == validation.diagnostics.end()
               ? std::string{}
               : "Map playtest is blocked by " + blocker->code + ": " + blocker->message;
}

bool startCurrentMapPlaytest(EditorPanelRuntime& runtime, bool fromSelectedPart = false) {
    if (runtime.project_root.empty()) {
        runtime.map_save_status = "Open a project before starting playtest.";
        return false;
    }
    if (const auto blocker = playtestDocumentBlocker(runtime); !blocker.empty()) {
        runtime.map_save_status = blocker;
        return false;
    }
    (void)runtime.map_authoring_workspace.activateMode(urpg::editor::MapAuthoringMode::Playtest);
    const auto perspectiveDraft = runtime.perspective_2d_workspace.PreparePerspectiveMapDraftSave();
    if (!perspectiveDraft.success) {
        runtime.map_save_status = "Map playtest could not start: " + perspectiveDraft.message;
        return false;
    }
    const auto gridDraft = urpg::map::GridPartDocumentToJson(runtime.level_builder_document).dump(2) + "\n";
    const auto selectedSpawn = fromSelectedPart ? playtestSpawnForSelectedPart(runtime) : std::string{};
    const auto spawn = selectedSpawn.empty() ? playtestSpawnForDocument(runtime.level_builder_document) : selectedSpawn;
    const bool started = runtime.playtest_session.start(runtime.project_root, runtime.level_builder_document.mapId(),
                                                        spawn, gridDraft, perspectiveDraft.serialized_document_json + "\n");
    runtime.map_save_status = runtime.playtest_session.message();
    if (!started && runtime.map_save_status.empty()) {
        runtime.map_save_status = "Map playtest could not start.";
    }
    return started;
}

bool registerEditorPanels(urpg::editor::EditorShell& editor_shell, EditorPanelRuntime& runtime) {
    runtime.ability_inspector_panel.update(runtime.ability_runtime);
    runtime.gameplay_recipe_templates = urpg::gameplay::builtInGameplayRecipeTemplates();
    if (!runtime.gameplay_recipe_templates.empty()) {
        runtime.gameplay_recipe_panel.selectRecipe(runtime.gameplay_recipe_templates.front());
    }
    runtime.ability_inspector_panel.setCommandCallbacks({
        [&runtime] {
            const bool ok = runtime.ability_inspector_panel.previewSelectedAbility(runtime.ability_runtime);
            runtime.ability_inspector_panel.recordCommandResult(
                "preview_selected", ok,
                ok ? "Previewed selected ability against the editor runtime."
                   : "Select a runtime ability before previewing.");
            return ok;
        },
        [&runtime] {
            runtime.ability_inspector_panel.applyDraftToRuntime(runtime.ability_runtime);
            runtime.ability_inspector_panel.update(runtime.ability_runtime);
            const bool selected = runtime.ability_inspector_panel.selectDraftAbility(runtime.ability_runtime);
            const auto asset = runtime.ability_inspector_panel.getDraftAsset();
            runtime.ability_inspector_panel.recordCommandResult(
                "apply_draft", selected,
                selected ? "Applied draft ability '" + asset.ability_id + "' to the editor runtime."
                         : "Draft ability was applied, but the runtime selection could not be refreshed.");
            return selected;
        },
        [&runtime] {
            const auto result = saveAbilityDraft(runtime);
            runtime.ability_inspector_panel.recordCommandResult("save_draft", result.success, result.message);
            return result.success;
        },
        [&runtime] {
            if (runtime.project_root.empty()) {
                runtime.ability_inspector_panel.recordCommandResult("load_draft", false,
                                                                    "Project root is not configured.");
                return false;
            }

            const auto records = urpg::ability::discoverAuthoredAbilityAssets(runtime.project_root);
            if (records.empty()) {
                runtime.ability_inspector_panel.recordCommandResult(
                    "load_draft", false, "No authored ability assets found under content/abilities.");
                return false;
            }

            const auto current_asset = runtime.ability_inspector_panel.getDraftAsset();
            auto selected = records.begin();
            const auto matching = std::find_if(records.begin(), records.end(), [&current_asset](const auto& record) {
                return record.ability_id == current_asset.ability_id;
            });
            if (matching != records.end()) {
                selected = matching;
            }

            const auto asset = urpg::ability::loadAuthoredAbilityAssetFromFile(selected->absolute_path);
            if (!asset.has_value()) {
                runtime.ability_inspector_panel.recordCommandResult(
                    "load_draft", false, "Failed to load " + selected->relative_path + ".");
                return false;
            }

            runtime.ability_inspector_panel.setDraftFromAsset(*asset);
            runtime.ability_inspector_panel.update(runtime.ability_runtime);
            runtime.ability_inspector_panel.recordCommandResult(
                "load_draft", true, "Loaded draft ability from " + selected->relative_path + ".");
            return true;
        },
    });
    runtime.ability_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        "ability.draft",
        "ability",
        false,
        [&runtime] { return saveAbilityDraft(runtime); },
        [] {},
        {},
    });
    runtime.character_creator_panel.bindModel(&runtime.character_creator_model);
    runtime.character_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kCharacterDirtyDocumentId,
        "character_creator",
        false,
        [&runtime] { return saveCharacterDraft(runtime); },
        [] {},
        {},
    });
    runtime.quest_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kQuestDirtyDocumentId,
        "map_authoring",
        false,
        [&runtime] { return saveQuestDraft(runtime); },
        [] {},
        {},
    });
    runtime.dialogue_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kDialogueDirtyDocumentId,
        "map_authoring",
        false,
        [&runtime] { return saveDialogueDraft(runtime); },
        [] {},
        {},
    });
    runtime.database_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kDatabaseDirtyDocumentId,
        "map_authoring",
        false,
        [&runtime] { return saveDatabaseDraft(runtime); },
        [] {},
        {},
    });
    runtime.vendor_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kVendorDirtyDocumentId,
        "map_authoring",
        false,
        [&runtime] { return saveVendorDraft(runtime); },
        [] {},
        {},
    });
    runtime.audio_mix_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kAudioMixDirtyDocumentId,
        "map_authoring",
        false,
        [&runtime] { return saveAudioMixDraft(runtime); },
        [] {},
        {},
    });
    runtime.input_remap_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kInputRemapDirtyDocumentId,
        "map_authoring",
        false,
        [&runtime] { return saveInputRemapDraft(runtime); },
        [] {},
        {},
    });
    runtime.gameplay_recipe_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kGameplayRecipeDirtyDocumentId,
        "ability",
        false,
        [&runtime] { return saveGameplayRecipeProject(runtime); },
        [] {},
        {},
    });
    runtime.menu_studio_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kMenuStudioDirtyDocumentId,
        "diagnostics",
        false,
        [&runtime] { return saveMenuStudioProject(runtime); },
        [] {},
        {},
    });
    runtime.mz_plugin_lock_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
        kMzPluginLockDirtyDocumentId,
        "mod_manager",
        false,
        [&runtime] { return saveMzPluginStaticLock(runtime); },
        [] {},
        {},
    });
    runtime.pattern_field_panel.bindModel(runtime.pattern_field_model);
    runtime.mod_loader = std::make_unique<urpg::mod::ModLoader>(runtime.mod_registry);
    runtime.mod_manager_panel.bindRegistry(&runtime.mod_registry);
    runtime.mod_manager_panel.bindLoader(runtime.mod_loader.get());
    runtime.analytics_uploader.setLocalJsonlExportPath(runtime.project_root / "reports" / "analytics" /
                                                       "editor_analytics.jsonl");
    runtime.analytics_panel.bindDispatcher(&runtime.analytics_dispatcher);
    runtime.analytics_panel.bindUploader(&runtime.analytics_uploader);
    runtime.analytics_panel.bindPrivacyController(&runtime.analytics_privacy_controller);
    runtime.diagnostics_workspace.bindAbilityRuntime(runtime.ability_runtime);
    runtime.audio_mix_panel.bindBank(&runtime.audio_mix_draft);
    runtime.audio_mix_panel.bindCore(&runtime.audio_preview_core);
    runtime.diagnostics_workspace.bindAudioRuntime(runtime.audio_preview_core);
    runtime.accessibility_panel.bindAuditor(&runtime.accessibility_auditor);
    bindLevelBuilder(runtime);

    using PanelRenderFactory = std::function<urpg::editor::EditorShell::RenderCallback(EditorPanelRuntime&)>;
    const std::unordered_map<std::string, PanelRenderFactory> renderFactories = {
        {"diagnostics",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.diagnostics_workspace.update();
                 panelRuntime.diagnostics_workspace.render();
             };
         }},
        {"assets",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.asset_library_panel.render();
             };
         }},
        {"ability",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.ability_inspector_panel.update(panelRuntime.ability_runtime);
                 panelRuntime.ability_inspector_panel.render();
             };
         }},
        {"character_creator",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.character_creator_panel.render();
             };
         }},
        {"patterns",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.pattern_field_panel.render();
             };
         }},
        {"mod",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.mod_manager_panel.render();
             };
         }},
        {"analytics",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext&) {
                 panelRuntime.analytics_panel.render();
             };
         }},
        {"level_builder",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext& context) {
                 (void)panelRuntime.map_authoring_workspace.activateMode(urpg::editor::MapAuthoringMode::Parts);
                 panelRuntime.level_builder_workspace.Render(
                     urpg::FrameContext{static_cast<float>(context.delta_seconds),
                                        static_cast<uint32_t>(context.frame_index)});
             };
         }},
        {"spatial_authoring",
         [](EditorPanelRuntime& panelRuntime) {
             return [&panelRuntime](const urpg::editor::EditorFrameContext& context) {
                 (void)panelRuntime.map_authoring_workspace.activateMode(urpg::editor::MapAuthoringMode::Canvas);
                 panelRuntime.perspective_2d_workspace.Render(
                     urpg::FrameContext{static_cast<float>(context.delta_seconds),
                                        static_cast<uint32_t>(context.frame_index)});
             };
         }},
    };

    for (const auto& factoryId : urpg::editor_app::editorAppRegisteredPanelFactoryIds()) {
        if (renderFactories.find(factoryId) == renderFactories.end()) {
            return false;
        }
    }

    if (!urpg::editor_app::editorAppMissingReleasePanelFactoryIds().empty()) {
        return false;
    }

    bool ok = true;
    for (const auto& panel : urpg::editor::topLevelEditorPanels()) {
        const auto factory = renderFactories.find(panel.id);
        if (factory == renderFactories.end()) {
            ok = false;
            continue;
        }

        ok = editor_shell.addPanel(
                 urpg::editor::EditorPanelDescriptor{panel.id, panel.title, panel.category}, factory->second(runtime)) &&
             ok;
    }

    return ok;
}

void printPanelList(const urpg::editor::EditorShell& editor_shell) {
    for (const auto& panel : editor_shell.panels()) {
        std::cout << panel.id << "\t" << panel.category << "\t" << panel.title << "\t"
                  << (panel.visible ? "visible" : "hidden") << "\n";
    }
}

void printStartupFailure(const urpg::diagnostics::StartupDiagnosticRecord& record,
                         const urpg::diagnostics::StartupDiagnosticWriteResult& writeResult) {
    std::cerr << "URPG editor startup " << urpg::diagnostics::toString(record.severity) << " [" << record.code
              << "]: " << record.message << "\n";
    if (!writeResult.log_path.empty()) {
        std::cerr << "URPG editor startup diagnostics log: " << writeResult.log_path.string() << "\n";
    }
    if (!writeResult.written && !writeResult.error.empty()) {
        std::cerr << "URPG editor startup diagnostics write failed: " << writeResult.error << "\n";
    }
}

void printRuntimeDiagnostics() {
    for (const auto& diagnostic : urpg::diagnostics::RuntimeDiagnostics::snapshot()) {
        std::cerr << "URPG editor runtime diagnostic [" << diagnostic.code << "]: " << diagnostic.message << "\n";
    }
}

urpg::analytics::ConsentState analyticsConsentFromSettings(const std::string& state) {
    if (state == "granted") {
        return urpg::analytics::ConsentState::Granted;
    }
    if (state == "denied") {
        return urpg::analytics::ConsentState::Denied;
    }
    return urpg::analytics::ConsentState::Unknown;
}

#ifdef URPG_IMGUI_ENABLED
const char* workspaceTitleForPanelId(const urpg::editor::EditorShellSnapshot& snapshot) {
    const auto active = std::find_if(snapshot.panels.begin(), snapshot.panels.end(), [&snapshot](const auto& panel) {
        return panel.id == snapshot.active_panel_id;
    });
    return active == snapshot.panels.end() ? "Workspace" : active->title.c_str();
}

void renderEditorChrome(urpg::editor::EditorShell& editorShell, EditorPanelRuntime* runtime) {
    ImGui::SetNextWindowBgAlpha(1.0f);
    ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(340.0f, 520.0f), ImGuiCond_Always);
    if (!ImGui::Begin("URPG Editor", nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    const auto snapshot = editorShell.snapshot();
    ImGui::Text("Project");
    ImGui::TextWrapped("%s", snapshot.project_root.string().c_str());
    ImGui::Separator();
    ImGui::Text("Active Panel");
    ImGui::TextWrapped("%s", snapshot.active_panel_id.c_str());
    ImGui::Separator();
    ImGui::Text("Release Panels");

    for (const auto& panel : snapshot.panels) {
        ImGui::BeginDisabled(!panel.enabled);
        const bool selected = panel.id == snapshot.active_panel_id;
        if (ImGui::Selectable((panel.title + "##" + panel.id).c_str(), selected)) {
            editorShell.openPanel(panel.id);
            if (runtime != nullptr) {
                runtime->focus_workspace_next_frame = true;
            }
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s/%s", panel.category.c_str(), panel.id.c_str());
        }
    }

    if (runtime != nullptr) {
        ImGui::Separator();
        if (ImGui::Button("Help: Creator Checklist", ImVec2(-1.0f, 0.0f))) {
            runtime->creator_checklist_panel.setVisible(true);
        }
    }

    ImGui::End();
}

const char* diagnosticsTabName(urpg::editor::DiagnosticsTab tab) {
    switch (tab) {
    case urpg::editor::DiagnosticsTab::Compat:
        return "Compat";
    case urpg::editor::DiagnosticsTab::Save:
        return "Save";
    case urpg::editor::DiagnosticsTab::EventAuthority:
        return "Events";
    case urpg::editor::DiagnosticsTab::MessageText:
        return "Messages";
    case urpg::editor::DiagnosticsTab::Battle:
        return "Battle";
    case urpg::editor::DiagnosticsTab::Menu:
        return "Menu";
    case urpg::editor::DiagnosticsTab::Audio:
        return "Audio";
    case urpg::editor::DiagnosticsTab::MigrationWizard:
        return "Migration";
    case urpg::editor::DiagnosticsTab::Abilities:
        return "Abilities";
    case urpg::editor::DiagnosticsTab::ProjectAudit:
        return "Audit";
    case urpg::editor::DiagnosticsTab::ProjectHealth:
        return "Health";
    }
    return "Diagnostics";
}

void renderDisabledReason(const std::string& reason) {
    if (!reason.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
        ImGui::SetTooltip("%s", reason.c_str());
    }
}

void renderJsonLines(const nlohmann::json& value, int maxRows = 8) {
    if (value.is_null()) {
        ImGui::TextDisabled("None");
        return;
    }
    if (value.is_array()) {
        int row = 0;
        for (const auto& item : value) {
            if (row++ >= maxRows) {
                ImGui::TextDisabled("...");
                break;
            }
            ImGui::BulletText("%s", item.is_string() ? item.get<std::string>().c_str() : item.dump().c_str());
        }
        if (row == 0) {
            ImGui::TextDisabled("None");
        }
        return;
    }
    if (value.is_object()) {
        int row = 0;
        for (auto it = value.begin(); it != value.end(); ++it) {
            if (row++ >= maxRows) {
                ImGui::TextDisabled("...");
                break;
            }
            const auto rendered = it.value().is_string() ? it.value().get<std::string>() : it.value().dump();
            ImGui::BulletText("%s: %s", it.key().c_str(), rendered.c_str());
        }
        if (row == 0) {
            ImGui::TextDisabled("None");
        }
        return;
    }
    ImGui::TextWrapped("%s", value.dump().c_str());
}

void renderCompatDiagnostics(urpg::editor::CompatReportPanel& panel) {
    auto& model = panel.getModel();
    ImGui::Text("Project Compatibility Score: %d%%", model.getProjectCompatibilityScore());
    ImGui::Separator();

    std::string selected = panel.getSelectedPlugin();
    if (!selected.empty()) {
        if (ImGui::Button("< Back to Summary")) {
            panel.clearSelection();
        }
        ImGui::SameLine();
        ImGui::Text("Selected Plugin: %s", selected.c_str());
        ImGui::Separator();

        ImGui::Text("API Calls:");
        auto calls = model.getPluginCalls(selected);
        if (calls.empty()) {
            ImGui::TextDisabled("No calls logged.");
        } else {
            if (ImGui::BeginTable("CompatCallsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Class");
                ImGui::TableSetupColumn("Method");
                ImGui::TableSetupColumn("Status");
                ImGui::TableSetupColumn("Calls");
                ImGui::TableHeadersRow();
                for (const auto& call : calls) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(call.className.c_str());
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(call.methodName.c_str());
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(call.status == urpg::compat::CompatStatus::FULL ? "FULL" : (call.status == urpg::compat::CompatStatus::PARTIAL ? "PARTIAL" : (call.status == urpg::compat::CompatStatus::STUB ? "STUB" : "UNSUPPORTED")));
                    ImGui::TableNextColumn(); ImGui::Text("%u", call.callCount);
                }
                ImGui::EndTable();
            }
        }
    } else {
        ImGui::Text("Plugins:");
        auto summaries = model.getAllPluginSummaries();
        if (summaries.empty()) {
            ImGui::TextDisabled("No plugin summaries found.");
        } else {
            if (ImGui::BeginTable("CompatPluginsTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                ImGui::TableSetupColumn("Plugin ID");
                ImGui::TableSetupColumn("Score");
                ImGui::TableSetupColumn("Calls");
                ImGui::TableSetupColumn("Action");
                ImGui::TableHeadersRow();
                for (const auto& summary : summaries) {
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn(); ImGui::TextUnformatted(summary.pluginId.c_str());
                    ImGui::TableNextColumn(); ImGui::Text("%d%%", summary.compatibilityScore);
                    ImGui::TableNextColumn(); ImGui::Text("%u", summary.totalCalls);
                    ImGui::TableNextColumn();
                    ImGui::PushID(summary.pluginId.c_str());
                    if (ImGui::Button("Inspect")) {
                        panel.selectPlugin(summary.pluginId);
                    }
                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        }
    }
}

void renderSaveDiagnostics(urpg::editor::SaveInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    if (!snapshot.message.empty()) {
        ImGui::TextWrapped("%s", snapshot.message.c_str());
    }
    if (!snapshot.remediation.empty()) {
        ImGui::TextWrapped("%s", snapshot.remediation.c_str());
    }

    if (!snapshot.runtime_bound) {
        return;
    }

    auto& model = panel.getModel();

    bool problemOnly = panel.showProblemSlotsOnly();
    if (ImGui::Checkbox("Show Problem Slots Only", &problemOnly)) {
        panel.setShowProblemSlotsOnly(problemOnly);
        panel.refresh();
    }
    ImGui::SameLine();
    bool includeAutosave = panel.includeAutosave();
    if (ImGui::Checkbox("Include Autosave", &includeAutosave)) {
        panel.setIncludeAutosave(includeAutosave);
        panel.refresh();
    }

    ImGui::Separator();
    ImGui::Text("Save Slots:");
    auto rows = model.VisibleRows();
    if (rows.empty()) {
        ImGui::TextDisabled("No visible save slots.");
    } else {
        if (ImGui::BeginTable("SaveSlotsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Slot ID");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Label");
            ImGui::TableSetupColumn("Map");
            ImGui::TableSetupColumn("Diagnostics");
            ImGui::TableHeadersRow();
            for (const auto& row : rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%d", row.slot_id);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.category_label.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.slot_label.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.map_display_name.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.diagnostic.empty() ? "-" : row.diagnostic.c_str());
            }
            ImGui::EndTable();
        }
    }

    ImGui::Separator();
    ImGui::Text("Save Policy Draft:");
    auto draft = model.PolicyDraft();
    bool autosaveEnabled = draft.autosave_enabled;
    if (ImGui::Checkbox("Autosave Enabled", &autosaveEnabled)) {
        panel.setPolicyAutosaveEnabled(autosaveEnabled);
    }
    int autosaveSlot = draft.autosave_slot_id;
    if (ImGui::InputInt("Autosave Slot ID", &autosaveSlot)) {
        panel.setPolicyAutosaveSlotId(autosaveSlot);
    }

    int maxAutosave = static_cast<int>(draft.max_autosave_slots);
    int maxQuicksave = static_cast<int>(draft.max_quicksave_slots);
    int maxManual = static_cast<int>(draft.max_manual_slots);
    bool prune = draft.prune_excess_on_save;

    bool limitChanged = false;
    if (ImGui::SliderInt("Max Autosave Slots", &maxAutosave, 1, 10)) limitChanged = true;
    if (ImGui::SliderInt("Max Quicksave Slots", &maxQuicksave, 1, 10)) limitChanged = true;
    if (ImGui::SliderInt("Max Manual Slots", &maxManual, 1, 100)) limitChanged = true;
    if (ImGui::Checkbox("Prune Excess on Save", &prune)) limitChanged = true;

    if (limitChanged) {
        panel.setPolicyRetentionLimits(maxAutosave, maxQuicksave, maxManual, prune);
    }

    if (snapshot.can_apply_policy) {
        if (ImGui::Button("Apply Policy")) {
            panel.applyPolicyToRuntime();
        }
    }
}

void renderEventAuthorityDiagnostics(urpg::EventAuthorityPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Visible Logs: %zu (Warnings: %zu, Errors: %zu)", snapshot.visible_rows, snapshot.warning_count, snapshot.error_count);

    char eventFilter[64];
    strncpy(eventFilter, snapshot.event_id_filter.c_str(), sizeof(eventFilter));
    eventFilter[sizeof(eventFilter)-1] = '\0';
    if (ImGui::InputText("Event ID Filter", eventFilter, sizeof(eventFilter))) {
        panel.setFilter(eventFilter);
        panel.refresh();
    }

    char levelFilter[64];
    strncpy(levelFilter, snapshot.level_filter.c_str(), sizeof(levelFilter));
    levelFilter[sizeof(levelFilter)-1] = '\0';
    if (ImGui::InputText("Level Filter", levelFilter, sizeof(levelFilter))) {
        panel.setLevelFilter(levelFilter);
        panel.refresh();
    }

    char modeFilter[64];
    strncpy(modeFilter, snapshot.mode_filter.c_str(), sizeof(modeFilter));
    modeFilter[sizeof(modeFilter)-1] = '\0';
    if (ImGui::InputText("Mode Filter", modeFilter, sizeof(modeFilter))) {
        panel.setModeFilter(modeFilter);
        panel.refresh();
    }

    if (ImGui::Button("Clear Filters")) {
        panel.clearFilters();
        panel.refresh();
    }

    ImGui::Separator();
    ImGui::Text("Event Blocks:");

    if (snapshot.visible_row_entries.empty()) {
        ImGui::TextDisabled("No event authority rows found matching criteria.");
    } else {
        if (ImGui::BeginTable("EventAuthorityTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Time");
            ImGui::TableSetupColumn("Level");
            ImGui::TableSetupColumn("Event ID");
            ImGui::TableSetupColumn("Block ID");
            ImGui::TableSetupColumn("Message");
            ImGui::TableHeadersRow();
            for (const auto& row : snapshot.visible_row_entries) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.ts.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.level.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.event_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.block_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.message.c_str());
            }
            ImGui::EndTable();
        }
    }
}

void renderMessageDiagnostics(urpg::editor::MessageInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Total Pages: %zu (Issues: %zu)", snapshot.total_pages, snapshot.issue_count);

    bool showIssues = snapshot.show_issues_only;
    if (ImGui::Checkbox("Show Issues Only", &showIssues)) {
        panel.setShowIssuesOnly(showIssues);
        panel.refresh();
    }

    ImGui::Separator();
    ImGui::Text("Dialogue Pages:");
    if (snapshot.visible_rows.empty()) {
        ImGui::TextDisabled("No dialogue pages visible.");
    } else {
        if (ImGui::BeginTable("MessagePagesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Index");
            ImGui::TableSetupColumn("Speaker");
            ImGui::TableSetupColumn("Route");
            ImGui::TableSetupColumn("Body Preview");
            ImGui::TableSetupColumn("Issues");
            ImGui::TableHeadersRow();
            for (const auto& row : snapshot.visible_rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row.page_index);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.speaker.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.route.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.body_preview.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%zu", row.issue_count);
            }
            ImGui::EndTable();
        }
    }
}

void renderBattleDiagnostics(urpg::editor::BattleInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Status: %s (Phase: %s, Issues: %zu)", snapshot.status.c_str(), snapshot.phase.c_str(), snapshot.issue_count);
    if (!snapshot.message.empty()) {
        ImGui::TextWrapped("%s", snapshot.message.c_str());
    }
    if (!snapshot.remediation.empty()) {
        ImGui::TextWrapped("%s", snapshot.remediation.c_str());
    }

    if (!snapshot.runtime_bound) {
        return;
    }

    auto& model = panel.getModel();
    ImGui::Separator();
    ImGui::Text("Battle Turn Queue:");
    auto rows = model.VisibleRows();
    if (rows.empty()) {
        ImGui::TextDisabled("No active action rows in queue.");
    } else {
        if (ImGui::BeginTable("BattleActionQueueTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Order");
            ImGui::TableSetupColumn("Subject");
            ImGui::TableSetupColumn("Target");
            ImGui::TableSetupColumn("Command");
            ImGui::TableSetupColumn("Speed");
            ImGui::TableSetupColumn("Summary");
            ImGui::TableHeadersRow();
            for (const auto& row : rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%zu", row.action_order);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.subject_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.target_id.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.command.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d", row.speed);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.summary.c_str());
            }
            ImGui::EndTable();
        }
    }
}

void renderAudioDiagnostics(urpg::editor::AudioInspectorPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Active Audio Sources: %zu (Issues: %zu)", snapshot.active_count, snapshot.issue_count);
    if (!snapshot.status_message.empty()) {
        ImGui::TextWrapped("%s", snapshot.status_message.c_str());
    }

    ImGui::Text("Master Volume: %.2f", snapshot.master_volume);

    ImGui::Separator();
    ImGui::Text("Live Audio Channels:");
    if (snapshot.live_rows.empty()) {
        ImGui::TextDisabled("No active audio channels.");
    } else {
        if (ImGui::BeginTable("AudioChannelsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Asset ID");
            ImGui::TableSetupColumn("Category");
            ImGui::TableSetupColumn("Volume");
            ImGui::TableSetupColumn("Pitch");
            ImGui::TableSetupColumn("Looping");
            ImGui::TableSetupColumn("Active");
            ImGui::TableHeadersRow();
            for (const auto& row : snapshot.live_rows) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.assetId.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d", static_cast<int>(row.category));
                ImGui::TableNextColumn(); ImGui::Text("%.2f", row.volume);
                ImGui::TableNextColumn(); ImGui::Text("%.2f", row.pitch);
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.isLooping ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(row.isActive ? "Yes" : "No");
            }
            ImGui::EndTable();
        }
    }
}

void renderMigrationWizardDiagnostics(urpg::editor::MigrationWizardPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Migration Status: %s", snapshot.headline.empty() ? "Idle" : snapshot.headline.c_str());
    ImGui::Text("Files Processed: %zu (Warnings: %zu, Errors: %zu)", snapshot.total_files_processed, snapshot.warning_count, snapshot.error_count);

    const auto buttonHelper = [&](const urpg::editor::MigrationWizardPanel::WorkflowActionState& action, const std::function<void()>& onClick) {
        if (!action.visible) return;
        if (!action.enabled) ImGui::BeginDisabled();
        if (ImGui::Button(action.label.c_str())) {
            onClick();
        }
        if (!action.enabled) ImGui::EndDisabled();
    };

    buttonHelper(snapshot.primary_actions.run_migration, [&]() {
        panel.rerunBoundProject();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.rerun_selected_subsystem, [&]() {
        panel.rerunBoundSelectedSubsystem();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.clear_selected_subsystem, [&]() {
        panel.clearSelectedSubsystemResult();
    });

    ImGui::NewLine();
    buttonHelper(snapshot.primary_actions.previous_subsystem, [&]() {
        panel.selectPreviousSubsystemResult();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.next_subsystem, [&]() {
        panel.selectNextSubsystemResult();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.previous_issue_subsystem, [&]() {
        panel.selectPreviousIssueSubsystemResult();
    });
    ImGui::SameLine();
    buttonHelper(snapshot.primary_actions.next_issue_subsystem, [&]() {
        panel.selectNextIssueSubsystemResult();
    });

    ImGui::Separator();
    ImGui::Text("Subsystems:");
    if (snapshot.subsystem_cards.empty()) {
        ImGui::TextDisabled("No subsystem migration cards.");
    } else {
        if (ImGui::BeginTable("MigrationSubsystemsTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Subsystem");
            ImGui::TableSetupColumn("Completed");
            ImGui::TableSetupColumn("Processed");
            ImGui::TableSetupColumn("Warnings");
            ImGui::TableSetupColumn("Errors");
            ImGui::TableHeadersRow();
            for (const auto& card : snapshot.subsystem_cards) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool is_selected = card.is_selected;
                if (ImGui::Selectable(card.display_name.c_str(), is_selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    panel.selectSubsystemResult(card.subsystem_id);
                }
                ImGui::TableNextColumn(); ImGui::TextUnformatted(card.completed ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::Text("%zu", card.processed_count);
                ImGui::TableNextColumn(); ImGui::Text("%zu", card.warning_count);
                ImGui::TableNextColumn(); ImGui::Text("%zu", card.error_count);
            }
            ImGui::EndTable();
        }
    }

    if (!snapshot.summary_logs.empty()) {
        ImGui::Separator();
        ImGui::Text("Logs:");
        for (const auto& log : snapshot.summary_logs) {
            ImGui::TextWrapped("%s", log.c_str());
        }
    }
}

void renderProjectAuditDiagnostics(const urpg::editor::ProjectAuditPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Project Completeness Audit: %s", snapshot.headline.empty() ? "No Report Data" : snapshot.headline.c_str());
    ImGui::Text("Issues found: %zu (Release Blockers: %zu, Export Blockers: %zu)", snapshot.issue_count, snapshot.release_blocker_count, snapshot.export_blocker_count);
    if (!snapshot.summary.empty()) {
        ImGui::TextWrapped("%s", snapshot.summary.c_str());
    }

    ImGui::Separator();
    ImGui::Text("Audit Issues:");
    if (snapshot.issues.empty()) {
        ImGui::TextDisabled("All checks passed! No issues found.");
    } else {
        if (ImGui::BeginTable("AuditIssuesTable", 5, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Code");
            ImGui::TableSetupColumn("Severity");
            ImGui::TableSetupColumn("Title");
            ImGui::TableSetupColumn("Blocks Release");
            ImGui::TableSetupColumn("Blocks Export");
            ImGui::TableHeadersRow();
            for (const auto& issue : snapshot.issues) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.code.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.severity == urpg::editor::ProjectAuditSeverity::Error ? "Error" : (issue.severity == urpg::editor::ProjectAuditSeverity::Warning ? "Warning" : "Info"));
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.title.c_str());
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.blocks_release ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::TextUnformatted(issue.blocks_export ? "Yes" : "No");
            }
            ImGui::EndTable();
        }
    }
}

void renderProjectHealthDiagnostics(const urpg::editor::ProjectHealthPanel& panel) {
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Project Health Status: %s", snapshot.headline.empty() ? "No Health Data" : snapshot.headline.c_str());
    ImGui::Text("Total Issues: %zu (Blockers: %zu)", snapshot.issue_count, snapshot.release_blocker_count);
    if (!snapshot.summary.empty()) {
        ImGui::TextWrapped("%s", snapshot.summary.c_str());
    }

    ImGui::Separator();
    ImGui::Text("Health Groups:");
    if (snapshot.groups.empty()) {
        ImGui::TextDisabled("No health groups available.");
    } else {
        for (const auto& group : snapshot.groups) {
            if (ImGui::CollapsingHeader(group.title.c_str())) {
                ImGui::Text("Issues: %zu (Blockers: %zu)", group.issue_count, group.blocker_count);
                if (group.fixes.empty()) {
                    ImGui::TextDisabled("No issues in this group.");
                } else {
                    for (const auto& fix : group.fixes) {
                        ImGui::BulletText("[%s] %s: %s", fix.code.c_str(), fix.title.c_str(), fix.detail.c_str());
                    }
                }
            }
        }
    }
}

void renderDiagnosticsWorkspace(EditorPanelRuntime& runtime) {
    auto& diagnostics = runtime.diagnostics_workspace;
    diagnostics.update();
    for (const auto& summary : diagnostics.allTabSummaries()) {
        ImGui::PushID(static_cast<int>(summary.tab));
        if (ImGui::Selectable(diagnosticsTabName(summary.tab), summary.active)) {
            diagnostics.setActiveTab(summary.tab);
        }
        ImGui::SameLine(150.0f);
        ImGui::Text("items %zu", summary.item_count);
        ImGui::SameLine(235.0f);
        ImGui::Text("issues %zu", summary.issue_count);
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::TextWrapped("Active diagnostics tab: %s", diagnosticsTabName(diagnostics.activeTab()));
    ImGui::Separator();

    auto activeTab = diagnostics.activeTab();
    if (activeTab == urpg::editor::DiagnosticsTab::Compat) {
        renderCompatDiagnostics(diagnostics.compatPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::Save) {
        renderSaveDiagnostics(diagnostics.savePanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::EventAuthority) {
        renderEventAuthorityDiagnostics(diagnostics.eventAuthorityPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::MessageText) {
        renderMessageDiagnostics(diagnostics.messagePanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::Battle) {
        renderBattleDiagnostics(diagnostics.battlePanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::Menu) {
        ImGui::TextUnformatted("Native Menu Studio");
        ImGui::TextDisabled("Project document: content/ui/menus.json");
        if (!runtime.menu_studio_load_status.empty()) {
            ImGui::TextWrapped("Menu document was not loaded: %s", runtime.menu_studio_load_status.c_str());
        }
        if (ImGui::Button("Save Native Menu Document")) {
            const auto result = runtime.dirty_state_registry.save(kMenuStudioDirtyDocumentId);
            runtime.menu_studio_save_status = result.message;
        }
        if (!runtime.menu_studio_save_status.empty()) {
            ImGui::TextDisabled("Save: %s", runtime.menu_studio_save_status.c_str());
        }
        diagnostics.render();
    } else if (activeTab == urpg::editor::DiagnosticsTab::Audio) {
        renderAudioDiagnostics(diagnostics.audioPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::MigrationWizard) {
        renderMigrationWizardDiagnostics(diagnostics.migrationWizardPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::ProjectAudit) {
        renderProjectAuditDiagnostics(diagnostics.projectAuditPanel());
    } else if (activeTab == urpg::editor::DiagnosticsTab::ProjectHealth) {
        renderProjectHealthDiagnostics(diagnostics.projectHealthPanel());
    }
}

void renderAssetWorkspace(EditorPanelRuntime& runtime) {
    auto& panel = runtime.asset_library_panel;
    // This is deliberately a narrow, governed path: an external source is
    // scanned into a review manifest, selected rows are promoted into the
    // configured library, and only then can a payload be copied into a
    // project. Raw paths never become map-ready from this view.
    static std::string importSourcePath;
    static std::string importSessionId = "creator-import";
    static std::string importLicenseId;
    static std::string selectedSpriteSliceAssetId;
    static int spriteSliceFrameWidth = 32;
    static int spriteSliceFrameHeight = 32;
    static int spriteSliceRows = 4;
    static int spriteSliceColumns = 4;
    static int spriteSliceDirection = 0;
    static bool spriteSliceLoop = true;
    static float spriteSliceFrameDuration = 0.12f;
    static std::unordered_map<std::string, uint32_t> gifFrameCounts;
    runtime.asset_thumbnail_pinned_requests.clear();
    static int attachmentConflictPolicy = 0;
    static nlohmann::json pendingAssetAttachmentPlan = nlohmann::json::object();
    static nlohmann::json pendingDerivedRevisionAttachmentPlan = nlohmann::json::object();
    static std::string derivedRevisionManifestPath;
    static std::string transformOperationId = "image-crop-scale";
    static int transformCropX = 0;
    static int transformCropY = 0;
    static int transformCropWidth = 0;
    static int transformCropHeight = 0;
    static int transformOutputWidth = 0;
    static int transformOutputHeight = 0;
    static std::string paletteOperationId = "image-fixed-palette";
    static float paletteColorA[4] = {1.0f, 0.0f, 0.0f, 1.0f};
    static float paletteColorB[4] = {0.0f, 0.0f, 0.0f, 1.0f};
    static std::string paletteExtractOperationId = "image-auto-palette";
    static int paletteExtractMaxColors = 16;
    static std::string audioOperationId = "audio-trim-fade-gain";
    static int audioStartFrame = 0;
    static int audioEndFrame = 0;
    static int audioFadeInFrames = 0;
    static int audioFadeOutFrames = 0;
    static int audioGainMilliDb = 0;
    static bool audioUseLoop = false;
    static int audioLoopStartFrame = 0;
    static int audioLoopEndFrame = 0;
    static std::string tilesetOperationId = "tileset-slice";
    static int tilesetTileWidth = 32;
    static int tilesetTileHeight = 32;
    static int tilesetMargin = 0;
    static int tilesetSpacing = 0;
    static std::string atlasOperationId = "atlas-metadata";
    static int atlasWidth = 0;
    static int atlasHeight = 0;
    static int atlasFrameWidth = 32;
    static int atlasFrameHeight = 32;
    static std::string inspectedMapReferenceAssetId;
    static std::vector<urpg::assets::ProjectAssetReference> inspectedMapReferences;
    static std::vector<std::string> inspectedMapReferenceDiagnostics;
    static std::string inspectedRemovalImpactAssetId;
    static urpg::assets::ProjectAssetRemovalImpactPlan inspectedRemovalImpact;
    static std::vector<std::string> inspectedOrphanedAssetIds;
    static bool orphanCandidatesInspected = false;
    static std::string assetWorkflowStatus;
    static std::string curationCollectionId;
    static std::string curationCollectionLabel;
    static std::string comparisonLeftPath;
    static std::string comparisonRightPath;
    const auto configuredLibraryRoot = runtime.external_asset_library_root;
    if (ImGui::Button("Load Reports")) {
        std::string error;
        (void)panel.model().loadReportsFromDirectory(runtime.project_root / "imports" / "reports", &error);
        panel.render();
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Filters")) {
        (void)panel.model().applyQuickFilter("all_assets");
        panel.render();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load External Index")) {
        std::string error;
        const auto catalogOwner = runtime.external_asset_library_root.empty()
                                      ? runtime.project_root
                                      : runtime.external_asset_library_root;
        (void)panel.model().loadExternalCatalog(catalogOwner / ".urpg" / "asset-index", &error);
        panel.render();
    }
    const auto& snapshot = panel.lastRenderSnapshot();
    ImGui::Separator();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    if (!snapshot.status_message.empty()) {
        ImGui::TextWrapped("%s", snapshot.status_message.c_str());
    }
    if (!snapshot.error_message.empty()) {
        ImGui::TextWrapped("%s", snapshot.error_message.c_str());
    }
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Governed Import and Attachment", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextWrapped("Review external material before promotion. Only promoted, runtime-ready payloads can be attached to this project.");
        ImGui::InputText("Source path", &importSourcePath);
        ImGui::InputText("Import session ID", &importSessionId);
        ImGui::InputText("License / private-project classification", &importLicenseId);
        if (configuredLibraryRoot.empty()) {
            ImGui::TextDisabled("Choose an External Asset Library root in Startup Settings before importing.");
        } else {
            ImGui::Text("Library: %s", configuredLibraryRoot.generic_string().c_str());
        }
        if (ImGui::Button("Choose Source")) {
            urpg::editor::AssetLibraryPanel::ImportSourcePickerRequest request;
            request.mode = urpg::editor::AssetLibraryPanel::ImportSourcePickerMode::FileOrArchive;
            request.library_root = configuredLibraryRoot;
            request.session_id = importSessionId;
            request.license_note = importLicenseId;
            const auto result = panel.requestImportSourceFromPicker(std::move(request));
            assetWorkflowStatus = result.value("message", "Import source was not selected.");
        }
        ImGui::SameLine();
        if (ImGui::Button("Review Entered Source") && !configuredLibraryRoot.empty() && !importSourcePath.empty() &&
            !importSessionId.empty()) {
            const auto result = panel.requestImportSource(importSourcePath, configuredLibraryRoot, importSessionId,
                                                          importLicenseId);
            assetWorkflowStatus = result.value("message", "Import source request was not prepared.");
        }
        ImGui::SameLine();
        const bool hasPendingImport = panel.lastImportWizardSnapshot().pending_request.is_object() &&
                                      !panel.lastImportWizardSnapshot().pending_request.empty();
        if (!hasPendingImport) ImGui::BeginDisabled();
        if (ImGui::Button("Run Importer")) {
            const auto result = panel.executePendingImportRequest();
            assetWorkflowStatus = result.value("message", "Importer did not return a status.");
        }
        if (!hasPendingImport) ImGui::EndDisabled();
        if (!assetWorkflowStatus.empty()) ImGui::TextWrapped("%s", assetWorkflowStatus.c_str());

        const auto& reviewRows = panel.lastRenderSnapshot().import_review_rows;
        if (!reviewRows.empty()) {
            ImGui::Text("Reviewed records: %zu", reviewRows.size());
            size_t shown = 0;
            for (const auto& row : reviewRows) {
                if (shown++ == 8) {
                    ImGui::TextDisabled("Additional review rows are retained in the session manifest.");
                    break;
                }
                const auto assetId = row.value("asset_id", "");
                const bool selectedForSlice = selectedSpriteSliceAssetId == assetId;
                if (ImGui::Selectable((row.value("relative_path", "unknown") + " — " +
                                       row.value("review_state", "unknown")).c_str(), selectedForSlice)) {
                    selectedSpriteSliceAssetId = assetId;
                }
            }
            const auto sliceRecord = std::find_if(reviewRows.begin(), reviewRows.end(), [&](const auto& row) {
                return row.value("asset_id", "") == selectedSpriteSliceAssetId &&
                       row.value("session_id", "") == importSessionId;
            });
            if (sliceRecord != reviewRows.end() && sliceRecord->value("media_kind", "") == "image") {
                ImGui::Separator();
                ImGui::TextUnformatted("Sprite Sheet Preview and Grid Slice");
                ImGui::TextWrapped("Selected image: %s (%d x %d). The preview profile is saved only in the governed import manifest.",
                                   sliceRecord->value("relative_path", "").c_str(), sliceRecord->value("width", 0),
                                   sliceRecord->value("height", 0));
                const auto sessionRow = std::find_if(snapshot.import_session_rows.begin(), snapshot.import_session_rows.end(),
                                                     [&](const auto& row) { return row.value("session_id", "") == importSessionId; });
                if (sessionRow != snapshot.import_session_rows.end()) {
                    urpg::editor::EditorThumbnailRequest request;
                    request.sourcePath = std::filesystem::path(sessionRow->value("managed_source_root", "")) /
                                         sliceRecord->value("relative_path", "");
                    request.sizeBytes = sliceRecord->value("size_bytes", uint64_t{0});
                    request.requestedWidth = 128;
                    request.requestedHeight = 128;
                    const bool isGif = sliceRecord->value("extension", "") == ".gif";
                    if (isGif) {
                        const auto known = std::max(1u, gifFrameCounts[request.sourcePath.generic_string()]);
                        request.gifFrameIndex = static_cast<uint32_t>(ImGui::GetTime() * 10.0) % known;
                    }
                    runtime.asset_thumbnail_pinned_requests.push_back(request);
                    runtime.asset_thumbnail_cache.pumpUploads();
                    const auto thumbnail = runtime.asset_thumbnail_cache.snapshotFor(request);
                    if (isGif && thumbnail.frameCount > 0) {
                        gifFrameCounts[request.sourcePath.generic_string()] = thumbnail.frameCount;
                    }
                    if (thumbnail.state == urpg::editor::EditorThumbnailState::Ready && thumbnail.textureId != 0) {
                        const auto texture = [] (uint32_t textureId) -> ImTextureID {
                            if constexpr (std::is_pointer_v<ImTextureID>) {
                                return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(textureId));
                            }
                            return static_cast<ImTextureID>(textureId);
                        }(thumbnail.textureId);
                        ImGui::Image(texture, ImVec2(96.0f, 96.0f));
                    } else {
                        ImGui::Button(thumbnail.state == urpg::editor::EditorThumbnailState::Queued ? "Loading preview" :
                                      "Preview unavailable", ImVec2(128.0f, 48.0f));
                    }
                    if (isGif) {
                        ImGui::TextDisabled("GIF animation: frame %u of %u (%u ms).", request.gifFrameIndex + 1,
                                            thumbnail.frameCount, thumbnail.frameDurationMs);
                    }
                }
                ImGui::InputInt("Frame Width", &spriteSliceFrameWidth);
                ImGui::SameLine();
                ImGui::InputInt("Frame Height", &spriteSliceFrameHeight);
                ImGui::InputInt("Rows", &spriteSliceRows);
                ImGui::SameLine();
                ImGui::InputInt("Columns", &spriteSliceColumns);
                const char* directionLabels[] = {"Down", "Left", "Right", "Up"};
                ImGui::Combo("Direction", &spriteSliceDirection, directionLabels, IM_ARRAYSIZE(directionLabels));
                ImGui::Checkbox("Loop", &spriteSliceLoop);
                ImGui::SliderFloat("Frame Duration", &spriteSliceFrameDuration, 0.01f, 2.0f, "%.2f s");
                const int frameCount = std::max(0, spriteSliceRows) * std::max(0, spriteSliceColumns);
                ImGui::Text("Preview: %d frame%s at %d x %d", frameCount, frameCount == 1 ? "" : "s",
                            spriteSliceFrameWidth, spriteSliceFrameHeight);
                if (ImGui::Button("Save Grid Slice to Import Manifest")) {
                    static constexpr const char* directions[] = {"down", "left", "right", "up"};
                    const auto result = panel.model().setImportRecordSpriteSheetSlice(
                        importSessionId, selectedSpriteSliceAssetId, spriteSliceFrameWidth, spriteSliceFrameHeight,
                        spriteSliceRows, spriteSliceColumns, directions[std::clamp(spriteSliceDirection, 0, 3)],
                        spriteSliceLoop, spriteSliceFrameDuration);
                    assetWorkflowStatus = result.value("message", "Sprite slicing metadata was not saved.");
                    panel.render();
                }
            }
            const auto ready = std::find_if(reviewRows.begin(), reviewRows.end(), [&](const auto& row) {
                return row.value("session_id", "") == importSessionId && row.value("promotable", false);
            });
            const bool canPromote = ready != reviewRows.end() && !configuredLibraryRoot.empty() && !importLicenseId.empty();
            if (!canPromote) ImGui::BeginDisabled();
            if (ImGui::Button("Promote Reviewed Records")) {
                std::vector<std::string> assetIds;
                for (const auto& row : reviewRows) {
                    if (row.value("session_id", "") == importSessionId && row.value("promotable", false)) {
                        assetIds.push_back(row.value("asset_id", ""));
                    }
                }
                const auto result = panel.model().promoteImportRecordsToGlobalLibrary(
                    importSessionId, std::move(assetIds), importLicenseId, configuredLibraryRoot);
                panel.render();
                assetWorkflowStatus = result.value("message", "Promotion did not return a status.");
            }
            if (!canPromote) ImGui::EndDisabled();
            if (importLicenseId.empty()) ImGui::TextDisabled("Promotion requires a license ID or private-project-only classification.");
        }

        const char* conflictLabels[] = {"Cancel on conflict", "Replace", "Keep Both", "Relink Existing"};
        const char* conflictPolicyIds[] = {"cancel", "replace", "keep_both", "relink_existing"};
        ImGui::Combo("If attachment conflicts", &attachmentConflictPolicy, conflictLabels, IM_ARRAYSIZE(conflictLabels));
        const auto policy = static_cast<urpg::assets::ProjectAssetAttachmentConflictPolicy>(attachmentConflictPolicy);
        ImGui::InputText("Derived revision manifest", &derivedRevisionManifestPath);
        ImGui::TextDisabled("A reviewed PNG/WAV revision is attached through the same project transaction as promoted media.");
        const bool cropRevisionOpen =
            ImGui::CollapsingHeader("Deterministic Image Crop and Scale", ImGuiTreeNodeFlags_DefaultOpen);
        if (cropRevisionOpen) {
            ImGui::TextWrapped("Create a non-destructive PNG revision from a promoted image. Zero width or height uses the source preview dimension.");
            ImGui::InputText("Transform operation ID", &transformOperationId);
            ImGui::InputInt("Crop X", &transformCropX);
            ImGui::SameLine();
            ImGui::InputInt("Crop Y", &transformCropY);
            ImGui::InputInt("Crop width", &transformCropWidth);
            ImGui::SameLine();
            ImGui::InputInt("Crop height", &transformCropHeight);
            ImGui::InputInt("Output width", &transformOutputWidth);
            ImGui::SameLine();
            ImGui::InputInt("Output height", &transformOutputHeight);
            if (configuredLibraryRoot.empty()) {
                ImGui::TextDisabled("A configured external asset library is required to store derived revisions.");
            }
        }
        const bool paletteRevisionOpen =
            ImGui::CollapsingHeader("Deterministic Fixed Palette", ImGuiTreeNodeFlags_DefaultOpen);
        if (paletteRevisionOpen) {
            ImGui::TextWrapped("Map every pixel to the nearest explicitly chosen RGBA entry. The initial editor control intentionally exposes two colors.");
            ImGui::InputText("Palette operation ID", &paletteOperationId);
            ImGui::ColorEdit4("Palette color A", paletteColorA);
            ImGui::ColorEdit4("Palette color B", paletteColorB);
            if (configuredLibraryRoot.empty()) {
                ImGui::TextDisabled("A configured external asset library is required to store derived revisions.");
            }
        }
        const bool paletteExtractRevisionOpen =
            ImGui::CollapsingHeader("Deterministic Automatic Palette", ImGuiTreeNodeFlags_DefaultOpen);
        if (paletteExtractRevisionOpen) {
            ImGui::TextWrapped("Extract up to 256 exact RGBA colors by frequency, breaking ties by RGBA value, then reduce with deterministic nearest-color mapping.");
            ImGui::InputText("Automatic palette operation ID", &paletteExtractOperationId);
            ImGui::SliderInt("Maximum extracted colors", &paletteExtractMaxColors, 2, 256);
            if (configuredLibraryRoot.empty()) {
                ImGui::TextDisabled("A configured external asset library is required to store derived revisions.");
            }
        }
        const bool audioRevisionOpen =
            ImGui::CollapsingHeader("PCM16 Audio Trim, Fade, and Gain", ImGuiTreeNodeFlags_DefaultOpen);
        if (audioRevisionOpen) {
            ImGui::TextWrapped("Create a non-destructive PCM16 WAV revision. Frame positions are exact source/output frame indices; unsupported codecs remain governed conversion work.");
            ImGui::InputText("Audio operation ID", &audioOperationId);
            ImGui::InputInt("Audio start frame", &audioStartFrame);
            ImGui::SameLine();
            ImGui::InputInt("Audio end frame", &audioEndFrame);
            ImGui::InputInt("Fade in frames", &audioFadeInFrames);
            ImGui::SameLine();
            ImGui::InputInt("Fade out frames", &audioFadeOutFrames);
            ImGui::InputInt("Gain (milli-dB)", &audioGainMilliDb);
            ImGui::Checkbox("Enable output loop", &audioUseLoop);
            if (audioUseLoop) {
                ImGui::InputInt("Loop start frame", &audioLoopStartFrame);
                ImGui::SameLine();
                ImGui::InputInt("Loop end frame", &audioLoopEndFrame);
            }
            if (configuredLibraryRoot.empty()) {
                ImGui::TextDisabled("A configured external asset library is required to store derived revisions.");
            }
        }
        const bool tilesetRevisionOpen =
            ImGui::CollapsingHeader("Deterministic Tileset Slice", ImGuiTreeNodeFlags_DefaultOpen);
        if (tilesetRevisionOpen) {
            ImGui::TextWrapped("Produce a row-major PNG tile directory from an exact image grid. Tileset assignment remains a separate native owner.");
            ImGui::InputText("Tileset operation ID", &tilesetOperationId);
            ImGui::InputInt("Tile width", &tilesetTileWidth);
            ImGui::SameLine();
            ImGui::InputInt("Tile height", &tilesetTileHeight);
            ImGui::InputInt("Tileset margin", &tilesetMargin);
            ImGui::SameLine();
            ImGui::InputInt("Tileset spacing", &tilesetSpacing);
            if (configuredLibraryRoot.empty()) {
                ImGui::TextDisabled("A configured external asset library is required to store derived revisions.");
            }
        }
        const bool atlasRevisionOpen =
            ImGui::CollapsingHeader("Deterministic Atlas Metadata", ImGuiTreeNodeFlags_DefaultOpen);
        if (atlasRevisionOpen) {
            ImGui::TextWrapped("Record validated atlas/frame dimensions without changing source pixels. Atlas metadata is not a directly attachable media revision.");
            ImGui::InputText("Atlas operation ID", &atlasOperationId);
            ImGui::InputInt("Atlas width", &atlasWidth);
            ImGui::SameLine();
            ImGui::InputInt("Atlas height", &atlasHeight);
            ImGui::InputInt("Atlas frame width", &atlasFrameWidth);
            ImGui::SameLine();
            ImGui::InputInt("Atlas frame height", &atlasFrameHeight);
            if (configuredLibraryRoot.empty()) {
                ImGui::TextDisabled("A configured external asset library is required to store derived revisions.");
            }
        }
        const auto& actionRows = panel.lastRenderSnapshot().asset_action_rows;
        if (!runtime.project_root.empty()) {
            if (ImGui::Button("Find Orphan Candidates")) {
                inspectedOrphanedAssetIds = urpg::assets::findOrphanedAttachedAssetIds(runtime.project_root);
                orphanCandidatesInspected = true;
            }
            ImGui::SameLine();
            ImGui::TextDisabled("Read-only: unindexed owners can still reference these assets.");
            if (!inspectedOrphanedAssetIds.empty()) {
                ImGui::TextDisabled("Orphan candidates (%zu):", inspectedOrphanedAssetIds.size());
                for (const auto& asset_id : inspectedOrphanedAssetIds) {
                    ImGui::BulletText("%s", asset_id.c_str());
                }
            } else if (orphanCandidatesInspected) {
                ImGui::TextDisabled("No orphan candidates were found among the currently indexed owners.");
            }
        }
        if (ImGui::CollapsingHeader("Favorites and Collections", ImGuiTreeNodeFlags_DefaultOpen)) {
            const auto& curation = panel.lastRenderSnapshot().user_curation;
            ImGui::Text("Favorites: %zu | Collections: %zu", panel.lastRenderSnapshot().favorite_asset_count,
                        panel.lastRenderSnapshot().asset_collection_count);
            ImGui::InputText("Collection ID", &curationCollectionId);
            ImGui::SameLine();
            ImGui::InputText("Collection label", &curationCollectionLabel);
            ImGui::SameLine();
            if (ImGui::Button("Create Collection")) {
                const bool created = panel.model().createAssetCollection(curationCollectionId, curationCollectionLabel);
                assetWorkflowStatus = created ? "Collection created." : "Collection ID and label must be unique and non-empty.";
                if (created) { curationCollectionId.clear(); curationCollectionLabel.clear(); panel.render(); }
            }
            for (const auto& collection : curation.value("collections", nlohmann::json::array())) {
                ImGui::TextDisabled("%s (%zu assets)", collection.value("label", "collection").c_str(),
                                    collection.value("asset_count", size_t{0}));
            }
        }
        for (const auto& row : actionRows) {
            const auto attach = row.value("attach_button", nlohmann::json::object());
            const bool projectAttached = row.value("project_attached", false);
            if (!attach.value("enabled", false) && !projectAttached) continue;
            const auto path = row.value("path", "");
            ImGui::PushID(row.value("asset_id", path).c_str());
            ImGui::Text("%s: %s", projectAttached ? "Attached" : "Ready", row.value("asset_id", "asset").c_str());
            ImGui::SameLine();
            if (ImGui::SmallButton("Compare Left")) comparisonLeftPath = path;
            ImGui::SameLine();
            if (ImGui::SmallButton("Compare Right")) comparisonRightPath = path;
            ImGui::SameLine();
            const bool favorite = panel.model().isAssetFavorite(path);
            if (ImGui::SmallButton(favorite ? "Unfavorite" : "Favorite")) {
                (void)panel.model().setAssetFavorite(path, !favorite);
                panel.render();
            }
            if (projectAttached && !runtime.project_root.empty()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("Project References")) {
                    const auto index = urpg::assets::buildProjectAssetReferenceIndex(runtime.project_root);
                    inspectedMapReferenceAssetId = row.value("asset_id", "");
                    inspectedMapReferences = index.inboundForAsset(inspectedMapReferenceAssetId);
                    inspectedMapReferenceDiagnostics = index.diagnostics;
                }
                ImGui::SameLine();
                if (ImGui::SmallButton("Removal Impact")) {
                    inspectedRemovalImpactAssetId = row.value("asset_id", "");
                    inspectedRemovalImpact = urpg::assets::buildProjectAssetRemovalImpactPlan(
                        runtime.project_root, inspectedRemovalImpactAssetId);
                }
                if (inspectedMapReferenceAssetId == row.value("asset_id", "")) {
                    ImGui::TextDisabled("Project references: %zu", inspectedMapReferences.size());
                    for (const auto& reference : inspectedMapReferences) {
                        ImGui::BulletText("%s | %s | %s", reference.document_path.generic_string().c_str(),
                                          reference.owner_kind.c_str(), reference.local_id.c_str());
                    }
                    for (const auto& diagnostic : inspectedMapReferenceDiagnostics) {
                        ImGui::TextDisabled("Index diagnostic: %s", diagnostic.c_str());
                    }
                }
                if (inspectedRemovalImpactAssetId == row.value("asset_id", "")) {
                    ImGui::TextDisabled("Removal impact: %zu indexed inbound reference(s).",
                                        inspectedRemovalImpact.inbound_references.size());
                    ImGui::TextDisabled("Read-only: removal is not authorized; unindexed owners may still reference it.");
                    for (const auto& reference : inspectedRemovalImpact.inbound_references) {
                        ImGui::BulletText("%s | %s | %s", reference.document_path.generic_string().c_str(),
                                          reference.owner_kind.c_str(), reference.local_id.c_str());
                    }
                    for (const auto& diagnostic : inspectedRemovalImpact.diagnostics) {
                        ImGui::TextDisabled("Impact diagnostic: %s", diagnostic.c_str());
                    }
                }
                if (row.value("media_kind", "") == "image" &&
                    ImGui::CollapsingHeader("Replace Active Map References")) {
                    ImGui::TextDisabled("Owner-scoped: changes only the active Perspective 2D Map and keeps this source attached.");
                    ImGui::BeginChild("ActiveMapAssetReplacementDrop", ImVec2(0.0f, 46.0f), true);
                    ImGui::TextUnformatted("Drop an attached image asset here to replace supported active Map references");
                    if (ImGui::BeginDragDropTarget()) {
                        if (const auto* drag = ImGui::AcceptDragDropPayload("URPG_EDITOR_ASSET_V1")) {
                            const auto* begin = static_cast<const std::uint8_t*>(drag->Data);
                            std::vector<std::uint8_t> bytes(begin, begin + drag->DataSize);
                            urpg::editor::EditorAssetDragPayload replacement;
                            const auto parsed = urpg::editor::deserializeEditorAssetDragPayload(bytes, &replacement);
                            if (!parsed.accepted) {
                                assetWorkflowStatus = parsed.message;
                            } else {
                                const auto replacementResult =
                                    runtime.map_authoring_workspace.replaceActiveMapAttachedAssetReferences(
                                        row.value("asset_id", ""), replacement);
                                assetWorkflowStatus = replacementResult.message +
                                                      (replacementResult.remediation.empty()
                                                           ? ""
                                                           : " " + replacementResult.remediation);
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::EndChild();
                }
            }
            for (const auto& collection : panel.lastRenderSnapshot().user_curation.value("collections", nlohmann::json::array())) {
                const auto collectionId = collection.value("id", "");
                ImGui::SameLine();
                if (ImGui::SmallButton(("Toggle " + collection.value("label", collectionId)).c_str())) {
                    const bool included = panel.model().isAssetInCollection(collectionId, path);
                    (void)panel.model().setAssetCollectionMembership(collectionId, path, !included);
                    panel.render();
                }
            }
            if (attach.value("enabled", false)) {
                ImGui::SameLine();
                const bool isPendingPlan = pendingAssetAttachmentPlan.value("success", false) &&
                                           pendingAssetAttachmentPlan.value("path", "") == path &&
                                           pendingAssetAttachmentPlan.value("project_root", "") ==
                                               runtime.project_root.generic_string() &&
                                           pendingAssetAttachmentPlan.value("conflict_policy", "") ==
                                               conflictPolicyIds[attachmentConflictPolicy];
                if (!isPendingPlan && ImGui::Button("Review Attachment")) {
                    pendingAssetAttachmentPlan = panel.planPromotedAssetAttachmentToProject(path, runtime.project_root, policy);
                    assetWorkflowStatus = pendingAssetAttachmentPlan.value("message", "Attachment plan did not return a status.");
                }
                if (isPendingPlan) {
                    ImGui::SameLine();
                    if (ImGui::Button("Confirm Attach")) {
                        const auto result = panel.confirmPromotedAssetAttachmentToProject(
                            path, runtime.project_root, pendingAssetAttachmentPlan.value("expected_source_revision", ""),
                            pendingAssetAttachmentPlan.value("operation_id", ""), policy);
                        assetWorkflowStatus = result.value("message", "Attachment confirmation did not return a status.");
                        if (result.value("success", false)) {
                            pendingAssetAttachmentPlan = nlohmann::json::object();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Refresh Review")) {
                        pendingAssetAttachmentPlan = panel.planPromotedAssetAttachmentToProject(path, runtime.project_root, policy);
                        assetWorkflowStatus = pendingAssetAttachmentPlan.value("message", "Attachment plan did not return a status.");
                    }
                    ImGui::TextDisabled("Revision: %.16s", pendingAssetAttachmentPlan.value("expected_source_revision", "").c_str());
                }
            }
            if (!derivedRevisionManifestPath.empty() && (attach.value("enabled", false) || projectAttached)) {
                ImGui::SameLine();
                const bool isPendingDerivedPlan = pendingDerivedRevisionAttachmentPlan.value("success", false) &&
                                                  pendingDerivedRevisionAttachmentPlan.value("path", "") == path &&
                                                  pendingDerivedRevisionAttachmentPlan.value("derived_manifest_path", "") ==
                                                      std::filesystem::path(derivedRevisionManifestPath).generic_string() &&
                                                  pendingDerivedRevisionAttachmentPlan.value("project_root", "") ==
                                                      runtime.project_root.generic_string() &&
                                                  pendingDerivedRevisionAttachmentPlan.value("conflict_policy", "") ==
                                                      conflictPolicyIds[attachmentConflictPolicy];
                if (!isPendingDerivedPlan && ImGui::Button("Review Derived Revision")) {
                    pendingDerivedRevisionAttachmentPlan = panel.planDerivedRevisionAttachmentToProject(
                        path, derivedRevisionManifestPath, runtime.project_root, policy);
                    assetWorkflowStatus = pendingDerivedRevisionAttachmentPlan.value(
                        "message", "Derived revision attachment plan did not return a status.");
                }
                if (isPendingDerivedPlan) {
                    ImGui::SameLine();
                    if (ImGui::Button("Confirm Derived Revision")) {
                        const auto result = panel.confirmDerivedRevisionAttachmentToProject(
                            path, derivedRevisionManifestPath, runtime.project_root,
                            pendingDerivedRevisionAttachmentPlan.value("expected_source_revision", ""),
                            pendingDerivedRevisionAttachmentPlan.value("operation_id", ""), policy);
                        assetWorkflowStatus = result.value("message", "Derived revision attachment confirmation did not return a status.");
                        if (result.value("success", false)) {
                            pendingDerivedRevisionAttachmentPlan = nlohmann::json::object();
                        }
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Refresh Derived Review")) {
                        pendingDerivedRevisionAttachmentPlan = panel.planDerivedRevisionAttachmentToProject(
                            path, derivedRevisionManifestPath, runtime.project_root, policy);
                        assetWorkflowStatus = pendingDerivedRevisionAttachmentPlan.value(
                            "message", "Derived revision attachment plan did not return a status.");
                    }
                    ImGui::TextDisabled("Derived revision: %.16s",
                                        pendingDerivedRevisionAttachmentPlan.value("expected_source_revision", "").c_str());
                }
            }
            if (cropRevisionOpen && !configuredLibraryRoot.empty() && row.value("media_kind", "") == "image") {
                ImGui::SameLine();
                if (ImGui::Button("Create Crop Revision")) {
                    const int sourceWidth = row.value("preview_width", 0);
                    const int sourceHeight = row.value("preview_height", 0);
                    const int cropWidth = transformCropWidth > 0 ? transformCropWidth : sourceWidth;
                    const int cropHeight = transformCropHeight > 0 ? transformCropHeight : sourceHeight;
                    const int outputWidth = transformOutputWidth > 0 ? transformOutputWidth : cropWidth;
                    const int outputHeight = transformOutputHeight > 0 ? transformOutputHeight : cropHeight;
                    const auto derivedRoot = configuredLibraryRoot.parent_path() / "derived";
                    const auto result = panel.createImageCropScaleRevision(
                        path, derivedRoot, transformOperationId, transformCropX, transformCropY, cropWidth, cropHeight,
                        outputWidth, outputHeight);
                    assetWorkflowStatus = result.value("message", "Image revision did not return a status.");
                    if (result.value("success", false)) {
                        derivedRevisionManifestPath = result.value("manifest_path", "");
                    }
                }
            }
            if (paletteRevisionOpen && !configuredLibraryRoot.empty() && row.value("media_kind", "") == "image") {
                ImGui::SameLine();
                if (ImGui::Button("Create Palette Revision")) {
                    const auto toRgba = [](const float* color) {
                        const auto channel = [](const float value) {
                            return static_cast<uint32_t>(std::clamp(value, 0.0f, 1.0f) * 255.0f + 0.5f);
                        };
                        return (channel(color[0]) << 24U) | (channel(color[1]) << 16U) |
                               (channel(color[2]) << 8U) | channel(color[3]);
                    };
                    const auto derivedRoot = configuredLibraryRoot.parent_path() / "derived";
                    const auto result = panel.createImagePaletteRevision(
                        path, derivedRoot, paletteOperationId, {toRgba(paletteColorA), toRgba(paletteColorB)});
                    assetWorkflowStatus = result.value("message", "Palette revision did not return a status.");
                    if (result.value("success", false)) {
                        derivedRevisionManifestPath = result.value("manifest_path", "");
                    }
                }
            }
            if (paletteExtractRevisionOpen && !configuredLibraryRoot.empty() && row.value("media_kind", "") == "image") {
                ImGui::SameLine();
                if (ImGui::Button("Extract Palette Revision")) {
                    const auto derivedRoot = configuredLibraryRoot.parent_path() / "derived";
                    const auto result = panel.createImagePaletteExtractRevision(
                        path, derivedRoot, paletteExtractOperationId, paletteExtractMaxColors);
                    assetWorkflowStatus = result.value("message", "Automatic palette revision did not return a status.");
                    if (result.value("success", false)) {
                        derivedRevisionManifestPath = result.value("manifest_path", "");
                    }
                }
            }
            if (audioRevisionOpen && !configuredLibraryRoot.empty() && row.value("media_kind", "") == "audio") {
                ImGui::SameLine();
                if (ImGui::Button("Create Audio Revision")) {
                    const auto nonnegativeFrames = [](const int value) {
                        return static_cast<uint64_t>(std::max(value, 0));
                    };
                    const auto derivedRoot = configuredLibraryRoot.parent_path() / "derived";
                    const auto result = panel.createAudioTrimFadeGainRevision(
                        path, derivedRoot, audioOperationId, nonnegativeFrames(audioStartFrame),
                        nonnegativeFrames(audioEndFrame), nonnegativeFrames(audioFadeInFrames),
                        nonnegativeFrames(audioFadeOutFrames), audioGainMilliDb,
                        audioUseLoop ? static_cast<int64_t>(std::max(audioLoopStartFrame, 0)) : -1,
                        audioUseLoop ? static_cast<int64_t>(std::max(audioLoopEndFrame, 0)) : -1);
                    assetWorkflowStatus = result.value("message", "Audio revision did not return a status.");
                    if (result.value("success", false)) {
                        derivedRevisionManifestPath = result.value("manifest_path", "");
                    }
                }
            }
            if (tilesetRevisionOpen && !configuredLibraryRoot.empty() && row.value("media_kind", "") == "image") {
                ImGui::SameLine();
                if (ImGui::Button("Create Tileset Revision")) {
                    const auto derivedRoot = configuredLibraryRoot.parent_path() / "derived";
                    const auto result = panel.createTilesetSliceRevision(
                        path, derivedRoot, tilesetOperationId, tilesetTileWidth, tilesetTileHeight, tilesetMargin,
                        tilesetSpacing);
                    assetWorkflowStatus = result.value("message", "Tileset revision did not return a status.");
                }
            }
            if (atlasRevisionOpen && !configuredLibraryRoot.empty() && row.value("media_kind", "") == "image") {
                ImGui::SameLine();
                if (ImGui::Button("Create Atlas Metadata")) {
                    const auto derivedRoot = configuredLibraryRoot.parent_path() / "derived";
                    const auto result = panel.createAtlasMetadataRevision(
                        path, derivedRoot, atlasOperationId, atlasWidth > 0 ? atlasWidth : row.value("preview_width", 0),
                        atlasHeight > 0 ? atlasHeight : row.value("preview_height", 0), atlasFrameWidth,
                        atlasFrameHeight);
                    assetWorkflowStatus = result.value("message", "Atlas metadata revision did not return a status.");
                }
            }
            if (projectAttached && ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                const auto assetId = row.value("asset_id", "");
                const auto sourceFilename = std::filesystem::path(path).filename();
                urpg::editor::EditorAssetDragPayload dragPayload;
                dragPayload.assetId = assetId;
                dragPayload.projectPath =
                    (runtime.project_root / "content" / "assets" / "imported" / assetId / sourceFilename).generic_string();
                dragPayload.mediaKind = row.value("media_kind", "image");
                if (std::ifstream manifestInput(runtime.project_root / "content" / "assets" / "manifests" /
                                                (assetId + ".json"),
                                                std::ios::binary);
                    manifestInput.good()) {
                    const auto manifestJson = nlohmann::json::parse(manifestInput, nullptr, false);
                    if (!manifestJson.is_discarded()) {
                        const auto manifest = urpg::assets::deserializeAssetPromotionManifest(manifestJson);
                        if (manifest.assetId == assetId) dragPayload.attachmentRevision = manifest.sourceSha256;
                    }
                }
                dragPayload.width = row.value("preview_width", uint32_t{0});
                dragPayload.height = row.value("preview_height", uint32_t{0});
                dragPayload.provenance = urpg::editor::EditorAssetProvenanceState::Attached;
                const auto bytes = urpg::editor::serializeEditorAssetDragPayload(dragPayload);
                ImGui::SetDragDropPayload("URPG_EDITOR_ASSET_V1", bytes.data(), static_cast<int>(bytes.size()));
                ImGui::TextUnformatted("Drop onto the Map canvas or Character Creator (attached assets only)");
                ImGui::EndDragDropSource();
            }
            ImGui::PopID();
        }
        if (!comparisonLeftPath.empty() && !comparisonRightPath.empty()) {
            const auto findComparisonRow = [&](const std::string& path) {
                return std::find_if(actionRows.begin(), actionRows.end(), [&](const auto& row) {
                    return row.value("path", "") == path;
                });
            };
            const auto left = findComparisonRow(comparisonLeftPath);
            const auto right = findComparisonRow(comparisonRightPath);
            if (left != actionRows.end() && right != actionRows.end() &&
                ImGui::CollapsingHeader("Read-only Asset Comparison", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::TextDisabled("Comparison is read-only; it does not promote, attach, or modify either asset.");
                const auto renderComparison = [&](const nlohmann::json& row, const char* label) {
                    ImGui::BeginGroup();
                    ImGui::TextUnformatted(label);
                    ImGui::TextWrapped("%s", row.value("asset_id", row.value("path", "asset")).c_str());
                    ImGui::Text("Kind: %s | %d x %d", row.value("media_kind", "unknown").c_str(),
                                row.value("preview_width", 0), row.value("preview_height", 0));
                    ImGui::Text("Category: %s", row.value("category", "unclassified").c_str());
                    const auto previewPath = row.value("preview_path", "");
                    if (row.value("preview_kind", "") == "image" && !previewPath.empty()) {
                        urpg::editor::EditorThumbnailRequest request;
                        request.sourcePath = previewPath;
                        request.requestedWidth = 128;
                        request.requestedHeight = 128;
                        runtime.asset_thumbnail_pinned_requests.push_back(request);
                        runtime.asset_thumbnail_cache.pumpUploads();
                        const auto thumbnail = runtime.asset_thumbnail_cache.snapshotFor(request);
                        if (thumbnail.state == urpg::editor::EditorThumbnailState::Ready && thumbnail.textureId != 0) {
                            const auto texture = [](uint32_t textureId) -> ImTextureID {
                                if constexpr (std::is_pointer_v<ImTextureID>) return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(textureId));
                                return static_cast<ImTextureID>(textureId);
                            }(thumbnail.textureId);
                            ImGui::Image(texture, ImVec2(128.0f, 128.0f));
                        } else ImGui::Button("Preview unavailable", ImVec2(128.0f, 48.0f));
                    }
                    ImGui::EndGroup();
                };
                renderComparison(*left, "Left");
                ImGui::SameLine();
                renderComparison(*right, "Right");
            }
        }
    }
    ImGui::Separator();
    ImGui::Text("Assets: %zu", snapshot.asset_count);
    ImGui::Text("Runtime ready: %zu", snapshot.runtime_ready_count);
    ImGui::Text("Previewable: %zu", snapshot.previewable_count);
    ImGui::Text("Duplicates: %zu groups / %zu assets", snapshot.duplicate_group_count, snapshot.duplicate_asset_count);
    ImGui::Text("Import rows: %zu", snapshot.import_review_row_count);
    ImGui::Text("Project attached: %zu", snapshot.project_attached_count);
    ImGui::Separator();
    ImGui::Text("External virtual catalog: %zu discovered / %zu hash pending / %zu archives",
                snapshot.external_catalog_asset_count, snapshot.external_catalog_hash_pending_count,
                snapshot.external_catalog_archive_count);
    if (snapshot.external_catalog.value("loaded", false)) {
        static std::string externalSearch;
        static std::string externalMediaKind;
        static std::string externalExtension;
        static std::string externalPack;
        static std::string externalCategory;
        static bool archiveOnly = false;
        const auto& externalCatalog = snapshot.external_catalog;
        const auto querySnapshot = externalCatalog.value("query", nlohmann::json::object());
        const auto applyExternalQuery = [&] {
            urpg::assets::LocalAssetCatalogQuery query;
            query.text = externalSearch;
            query.mediaKind = externalMediaKind;
            query.extension = externalExtension;
            query.pack = externalPack;
            query.category = externalCategory;
            query.archiveOnly = archiveOnly;
            panel.model().setExternalCatalogQuery(std::move(query));
            panel.render();
        };
        ImGui::InputText("Search external catalog", &externalSearch);
        ImGui::InputText("Media kind", &externalMediaKind);
        ImGui::SameLine();
        ImGui::InputText("Extension", &externalExtension);
        ImGui::InputText("Pack", &externalPack);
        ImGui::SameLine();
        ImGui::InputText("Category", &externalCategory);
        ImGui::Checkbox("Archives only", &archiveOnly);
        if (ImGui::Button("Apply External Filters")) {
            applyExternalQuery();
        }
        ImGui::SameLine();
        const auto catalogActions = externalCatalog.value("actions", nlohmann::json::object());
        const bool canRefresh = catalogActions.value("refresh_index", nlohmann::json::object()).value("enabled", false);
        if (!canRefresh) ImGui::BeginDisabled();
        if (ImGui::Button("Refresh Index")) {
            const auto result = panel.refreshExternalCatalog();
            ImGui::TextWrapped("%s", result.value("message", "Catalog refresh did not return a status.").c_str());
        }
        if (!canRefresh) ImGui::EndDisabled();
        ImGui::SameLine();
        const bool canOpenSource =
            catalogActions.value("open_source_location", nlohmann::json::object()).value("enabled", false);
        if (!canOpenSource) ImGui::BeginDisabled();
        if (ImGui::Button("Open Source Location")) {
            const auto result = panel.openSelectedExternalCatalogSource();
            ImGui::TextWrapped("%s", result.value("message", "Source location did not return a status.").c_str());
        }
        if (!canOpenSource) ImGui::EndDisabled();
        const auto page = externalCatalog.value("page", nlohmann::json::object());
        ImGui::Text("External results: %zu", page.value("total_matches", size_t{0}));
        const auto offset = querySnapshot.value("offset", size_t{0});
        const auto pageSize = querySnapshot.value("page_size", size_t{50});
        if (offset > 0 && ImGui::Button("Previous External Page")) {
            urpg::assets::LocalAssetCatalogQuery query;
            query.text = externalSearch;
            query.mediaKind = externalMediaKind;
            query.extension = externalExtension;
            query.pack = externalPack;
            query.category = externalCategory;
            query.archiveOnly = archiveOnly;
            query.offset = offset > pageSize ? offset - pageSize : 0;
            panel.model().setExternalCatalogQuery(std::move(query));
            panel.render();
        }
        ImGui::SameLine();
        if (page.value("has_more", false) && ImGui::Button("Next External Page")) {
            urpg::assets::LocalAssetCatalogQuery query;
            query.text = externalSearch;
            query.mediaKind = externalMediaKind;
            query.extension = externalExtension;
            query.pack = externalPack;
            query.category = externalCategory;
            query.archiveOnly = archiveOnly;
            query.offset = offset + pageSize;
            panel.model().setExternalCatalogQuery(std::move(query));
            panel.render();
        }
        const auto records = page.value("records", nlohmann::json::array());
        // Clip the result list before queuing previews. This is intentionally
        // not a page-wide preload: only rows ImGui says are on screen may
        // request a decode or consume a GPU texture.
        runtime.asset_thumbnail_cache.pumpUploads();
        std::vector<urpg::editor::EditorThumbnailRequest> visibleThumbnailRequests;
        if (ImGui::BeginChild("ExternalCatalogResults", ImVec2(0.0f, 300.0f), true)) {
            ImGuiListClipper clipper;
            clipper.Begin(static_cast<int>(records.size()), 44.0f);
            while (clipper.Step()) {
                for (int index = clipper.DisplayStart; index < clipper.DisplayEnd; ++index) {
                    const auto& record = records.at(static_cast<size_t>(index));
                    const auto assetId = record.value("asset_id", "");
                    const auto label = record.value("virtual_path", assetId);
                    const auto mediaKind = record.value("media_kind", "unknown");
                    urpg::editor::EditorThumbnailRequest thumbnailRequest;
                    thumbnailRequest.sourcePath =
                        std::filesystem::path(record.value("source_root", "")) / label;
                    thumbnailRequest.sizeBytes = record.value("size_bytes", uint64_t{0});
                    thumbnailRequest.modifiedTimeNs = record.value("modified_time_ns", int64_t{0});
                    thumbnailRequest.hashPending = record.value("hash_pending", false);
                    const bool canPreview = mediaKind != "archive" && !thumbnailRequest.sourcePath.empty();
                    if (canPreview) {
                        visibleThumbnailRequests.push_back(thumbnailRequest);
                    }
                    const auto thumbnail = runtime.asset_thumbnail_cache.snapshotFor(thumbnailRequest);

                    ImGui::PushID(assetId.c_str());
                    if (canPreview && thumbnail.state == urpg::editor::EditorThumbnailState::Ready &&
                        thumbnail.textureId != 0) {
                        const auto imguiTextureId = [] (uint32_t textureId) -> ImTextureID {
                            if constexpr (std::is_pointer_v<ImTextureID>) {
                                return reinterpret_cast<ImTextureID>(static_cast<uintptr_t>(textureId));
                            }
                            return static_cast<ImTextureID>(textureId);
                        }(thumbnail.textureId);
                        ImGui::Image(imguiTextureId, ImVec2(36.0f, 36.0f));
                    } else {
                        const char* fallback = canPreview && thumbnail.state == urpg::editor::EditorThumbnailState::Queued
                                                   ? "Loading"
                                                   : "No preview";
                        ImGui::Button(fallback, ImVec2(72.0f, 36.0f));
                    }
                    ImGui::SameLine();
                    if (ImGui::Selectable(label.c_str(), record.value("selected", false), 0, ImVec2(0.0f, 36.0f))) {
                        panel.model().selectExternalCatalogAsset(assetId);
                        panel.render();
                    }
                    ImGui::SameLine();
                    ImGui::TextDisabled("%s", mediaKind.c_str());
                    ImGui::PopID();
                }
            }
            ImGui::EndChild();
        }
        visibleThumbnailRequests.insert(visibleThumbnailRequests.end(), runtime.asset_thumbnail_pinned_requests.begin(),
                                        runtime.asset_thumbnail_pinned_requests.end());
        runtime.asset_thumbnail_cache.setVisibleRequests(visibleThumbnailRequests);
        runtime.asset_thumbnail_cache.pumpUploads();
    } else {
        renderJsonLines(snapshot.external_catalog.value("diagnostics", nlohmann::json::array()), 3);
    }
    ImGui::Separator();
    ImGui::Text("Archive inspection (read-only external metadata)");
    static std::string archivePath;
    static std::string selectedArchivePath;
    static std::vector<std::string> selectedArchiveEntries;
    ImGui::InputText("Archive path", &archivePath);
    ImGui::SameLine();
    if (ImGui::Button("Inspect Archive") && !archivePath.empty()) {
        (void)panel.browseArchive(archivePath);
        selectedArchivePath = archivePath;
        selectedArchiveEntries.clear();
    }
    const auto& archiveBrowser = panel.lastRenderSnapshot().archive_browser;
    if (!archiveBrowser.empty()) {
        ImGui::Text("Archive: %s", archiveBrowser.value("status", "unknown").c_str());
        ImGui::TextWrapped("%s", archiveBrowser.value("message", "").c_str());
        ImGui::Text("Entries: %zu%s", archiveBrowser.value("entry_count", size_t{0}),
                    archiveBrowser.value("entries_truncated", false) ? " (display capped)" : "");
        ImGui::TextDisabled("Select only the entries to stage into the governed import review; no archive-wide extraction occurs.");
        for (const auto& entry : archiveBrowser.value("entries", nlohmann::json::array())) {
            if (entry.value("directory", false)) {
                continue;
            }
            const auto entryPath = entry.value("path", "");
            const bool selected = std::find(selectedArchiveEntries.begin(), selectedArchiveEntries.end(), entryPath) !=
                                  selectedArchiveEntries.end();
            bool checked = selected;
            ImGui::PushID(entryPath.c_str());
            if (ImGui::Checkbox(entryPath.c_str(), &checked)) {
                if (checked) {
                    selectedArchiveEntries.push_back(entryPath);
                } else {
                    selectedArchiveEntries.erase(std::remove(selectedArchiveEntries.begin(), selectedArchiveEntries.end(), entryPath),
                                                 selectedArchiveEntries.end());
                }
            }
            ImGui::SameLine();
            ImGui::TextDisabled("%llu bytes", static_cast<unsigned long long>(entry.value("expanded_bytes", uint64_t{0})));
            ImGui::PopID();
        }
        const bool nativeSelectedEntryImport = archiveBrowser.value("code", "") == "archive_listed";
        const bool externalSelectedEntryImport = archiveBrowser.value("code", "") == "archive_listed_external" &&
                                               panel.lastImportWizardSnapshot().extractor_configuration.value(
                                                   "supports_selected_entry_staging", false);
        const bool supportsSelectedEntryImport = nativeSelectedEntryImport || externalSelectedEntryImport;
        const bool canReviewSelectedEntries = supportsSelectedEntryImport && !configuredLibraryRoot.empty() &&
                                              !importSessionId.empty() && !selectedArchiveEntries.empty() &&
                                              selectedArchivePath == archivePath;
        if (!canReviewSelectedEntries) ImGui::BeginDisabled();
        if (ImGui::Button("Review Selected Archive Entries")) {
            const auto result = panel.requestImportSource(archivePath, configuredLibraryRoot, importSessionId,
                                                          importLicenseId, {}, selectedArchiveEntries);
            assetWorkflowStatus = result.value("message", "Selected archive entries were not prepared for review.");
        }
        if (!canReviewSelectedEntries) ImGui::EndDisabled();
        if (selectedArchiveEntries.empty()) {
            ImGui::TextDisabled("Select at least one non-directory archive entry.");
        } else if (!supportsSelectedEntryImport) {
            ImGui::TextDisabled("RAR/7z needs URPG_ASSET_ARCHIVE_EXTRACTOR with a standalone {selected_entries} argument for isolated staging.");
        }
    }
    ImGui::Separator();
    ImGui::Text("Last Action");
    renderJsonLines(snapshot.last_action, 6);
}

void renderModWorkspace(EditorPanelRuntime& runtime) {
    const auto snapshot = runtime.mod_manager_panel.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.value("status", "unknown").c_str());
    ImGui::Text("Registered: %zu", snapshot.value("registered_count", size_t{0}));
    ImGui::Text("Active: %zu", snapshot.value("active_count", size_t{0}));
    if (ImGui::Button("Clear Last Action")) {
        runtime.mod_manager_panel.clearLastAction();
        runtime.mod_manager_panel.render();
    }
    ImGui::Separator();
    ImGui::Text("Messages");
    renderJsonLines(snapshot.value("status_messages", nlohmann::json::array()), 6);
    ImGui::Separator();
    ImGui::Text("Mods");
    renderJsonLines(snapshot.value("mods", nlohmann::json::array()), 6);

    runtime.mz_plugin_inspector_panel.render();
    const auto& mzSnapshot = runtime.mz_plugin_inspector_panel.lastRenderSnapshot();
    const auto mzReport = runtime.mz_plugin_inspector_panel.model().exportSnapshotJson();
    ImGui::Separator();
    ImGui::TextUnformatted("RPG Maker MZ Plugin Compatibility (Read-only)");
    ImGui::TextDisabled("Static source inspection only: no JavaScript is loaded, evaluated, or activated here.");
    ImGui::Text("Source: %s", mzSnapshot.source_kind.empty() ? "not inspected" : mzSnapshot.source_kind.c_str());
    ImGui::Text("Discovered: %zu | Issues: %zu | Score: %d", mzSnapshot.plugin_count, mzSnapshot.issue_count,
                mzSnapshot.project_score);
    renderJsonLines(nlohmann::json(mzSnapshot.discovery_diagnostics), 4);
    if (mzSnapshot.has_data) {
        renderJsonLines(mzReport.value("plugins", nlohmann::json::array()), 8);
    } else {
        ImGui::TextDisabled("No MZ plugin source was available for inspection in this project.");
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Project MZ Plugin Lock (Compatibility Only)");
    ImGui::TextDisabled("Locks static source hashes and dependency order; it never authorizes, loads, or executes MZ plugins.");
    if (ImGui::Button("Refresh Static MZ Plugin Lock")) {
        runtime.mz_plugin_lock_draft = buildMzPluginStaticLock(runtime.project_root);
        if (isValidMzPluginStaticLock(runtime.mz_plugin_lock_draft)) {
            (void)runtime.dirty_state_registry.markDirty(kMzPluginLockDirtyDocumentId, true);
            runtime.mz_plugin_lock_source_current = true;
            runtime.mz_plugin_lock_status = "Static MZ plugin lock refreshed from source text without JavaScript execution.";
        } else {
            runtime.mz_plugin_lock_source_current = false;
            runtime.mz_plugin_lock_status =
                "Static MZ plugin lock was not created because at least one scanned source could not be locked safely.";
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Check Static MZ Plugin Lock")) {
        runtime.mz_plugin_lock_source_current = isValidMzPluginStaticLock(runtime.mz_plugin_lock_draft) &&
                                                  runtime.mz_plugin_lock_draft == buildMzPluginStaticLock(runtime.project_root);
        runtime.mz_plugin_lock_status = runtime.mz_plugin_lock_source_current
                                            ? "Static MZ plugin lock matches the current non-executing source scan."
                                            : "Static MZ plugin lock differs from the current source scan; refresh and review it.";
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Static MZ Plugin Lock")) {
        const auto result = runtime.dirty_state_registry.save(kMzPluginLockDirtyDocumentId);
        runtime.mz_plugin_lock_status = result.message;
    }
    if (isValidMzPluginStaticLock(runtime.mz_plugin_lock_draft)) {
        ImGui::Text("Locked plugins: %zu | State: %s",
                    runtime.mz_plugin_lock_draft["plugins"].size(),
                    runtime.mz_plugin_lock_source_current ? "current" : "check required/source changed");
        if (!runtime.mz_plugin_lock_source_current) {
            ImGui::TextColored(ImVec4(1.0f, 0.65f, 0.25f, 1.0f),
                               "Refresh and review the static lock before relying on this compatibility report.");
        }
        renderJsonLines(runtime.mz_plugin_lock_draft.value("plugins", nlohmann::json::array()), 8);
    } else {
        ImGui::TextDisabled("No valid project lock is loaded. Refresh to create a static compatibility record.");
    }
    if (!runtime.mz_plugin_lock_status.empty()) {
        ImGui::TextWrapped("%s", runtime.mz_plugin_lock_status.c_str());
    }
}

void renderLevelBuilderWorkspace(EditorPanelRuntime& runtime) {
    auto& workspace = runtime.level_builder_workspace;
    const auto& snapshot = workspace.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    ImGui::TextWrapped("%s", snapshot.message.c_str());
    ImGui::Text("Mode: %s", snapshot.active_mode.c_str());
    ImGui::Text("Placed parts: %zu", snapshot.placement.placed_count);
    ImGui::Text("Palette parts: %zu", snapshot.palette.part_count);
    ImGui::Text("Diagnostics: %zu (%zu blocking)", snapshot.validation.diagnostic_count,
                snapshot.validation.blocking_count);
    ImGui::Separator();
    ImGui::Text("Actions");
    for (const auto& action : snapshot.actions) {
        ImGui::PushID(action.id.c_str());
        if (!action.enabled) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(action.label.c_str(), ImVec2(118.0f, 0.0f))) {
            (void)workspace.ActivateToolbarAction(action.id);
        }
        if (!action.enabled) {
            ImGui::EndDisabled();
        }
        if (!action.enabled) {
            renderDisabledReason("Action is unavailable in the current level state.");
        }
        ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::Text("Palette");
    int shown = 0;
    for (const auto& entry : snapshot.palette.entries) {
        if (shown++ >= 10) {
            ImGui::TextDisabled("...");
            break;
        }
        ImGui::PushID(entry.part_id.c_str());
        if (ImGui::Selectable(entry.display_name.c_str(), entry.selected)) {
            (void)workspace.SelectGridPart(entry.part_id);
        }
        ImGui::SameLine(220.0f);
        ImGui::TextDisabled("%s", entry.category.c_str());
        ImGui::PopID();
    }
    if (!snapshot.placement.selected_part_id.empty() && ImGui::CollapsingHeader("Grid Part Rectangle Fill")) {
        static int rectangleFillMinX = 0;
        static int rectangleFillMinY = 0;
        static int rectangleFillMaxX = 0;
        static int rectangleFillMaxY = 0;
        ImGui::TextDisabled("Review the selected catalog part before applying one all-or-nothing native Map edit.");
        ImGui::InputInt("Minimum X", &rectangleFillMinX);
        ImGui::InputInt("Minimum Y", &rectangleFillMinY);
        ImGui::InputInt("Maximum X", &rectangleFillMaxX);
        ImGui::InputInt("Maximum Y", &rectangleFillMaxY);
        if (ImGui::Button("Review Grid Part Rectangle Fill")) {
            const auto review = workspace.placementPanel().PreviewSelectedPartRectangle(
                rectangleFillMinX, rectangleFillMinY, rectangleFillMaxX, rectangleFillMaxY);
            runtime.map_save_status = review.accepted
                                          ? "Grid Part rectangle review is ready: " +
                                                std::to_string(review.operation_count) + " placements will be applied together."
                                          : "Grid Part rectangle review blocked: " + review.message;
        }
        ImGui::SameLine();
        if (ImGui::Button("Apply Grid Part Rectangle Fill")) {
            const bool applied = workspace.placementPanel().FillSelectedPartRectangle(
                rectangleFillMinX, rectangleFillMinY, rectangleFillMaxX, rectangleFillMaxY);
            const auto& result = workspace.placementPanel().lastRenderSnapshot().last_rectangle_fill_result;
            runtime.map_save_status = applied ? std::string{"Grid Part rectangle applied as one native undoable Map operation."}
                                               : "Grid Part rectangle apply blocked: " + result.message;
        }
        const auto& result = workspace.placementPanel().lastRenderSnapshot().last_rectangle_fill_result;
        if (!result.code.empty()) {
            ImGui::TextDisabled("Rectangle fill status: %s", result.message.c_str());
            ImGui::TextDisabled("Reviewed placement count: %zu", result.operation_count);
        }
    }
    if (!snapshot.placement.smart_prefabs.empty() && ImGui::CollapsingHeader("Native Smart Prefabs")) {
        static int smartPrefabGridX = 0;
        static int smartPrefabGridY = 0;
        static std::unordered_map<std::string, std::string> smartPrefabParameterValues;
        ImGui::TextDisabled("Versioned operation groups validate every referenced part, parameter, conflict tag, and footprint before one undoable Map edit.");
        for (const auto& prefab : snapshot.placement.smart_prefabs) {
            ImGui::PushID(prefab.prefab_id.c_str());
            const auto label = prefab.display_name + " (v" + prefab.version + ")";
            if (ImGui::Selectable(label.c_str(), prefab.selected)) {
                smartPrefabParameterValues.clear();
                for (const auto& parameter : prefab.parameters) {
                    smartPrefabParameterValues[parameter.key] = parameter.default_value;
                }
                (void)workspace.placementPanel().SetSelectedSmartPrefabId(prefab.prefab_id);
            }
            if (prefab.selected) {
                ImGui::TextDisabled("%s", prefab.description.c_str());
                ImGui::TextDisabled("%zu operation(s), %zu dependency reference(s), %zu conflict tag(s)",
                                    prefab.operation_count, prefab.dependencies.size(), prefab.conflict_tags.size());
                ImGui::InputInt("Anchor X", &smartPrefabGridX);
                ImGui::InputInt("Anchor Y", &smartPrefabGridY);
                for (const auto& parameter : prefab.parameters) {
                    const auto inserted = smartPrefabParameterValues.emplace(parameter.key, parameter.default_value);
                    ImGui::InputText((parameter.key + "##smart_prefab_parameter").c_str(), &inserted.first->second);
                    if (!parameter.allowed_values.empty()) {
                        std::string allowedValues;
                        for (const auto& value : parameter.allowed_values) {
                            if (!allowedValues.empty()) allowedValues += ", ";
                            allowedValues += value;
                        }
                        ImGui::TextDisabled("Allowed: %s", allowedValues.c_str());
                    }
                    if (parameter.required) ImGui::TextDisabled("Required");
                }
                if (ImGui::Button("Review Smart Prefab Placement")) {
                    const auto review = workspace.placementPanel().PreviewSelectedSmartPrefabAtGrid(
                        smartPrefabGridX, smartPrefabGridY, smartPrefabParameterValues);
                    runtime.map_save_status = review.accepted
                                                  ? "Smart prefab review is ready: " +
                                                        std::to_string(review.accepted_operation_count) +
                                                        " native operations will be applied together."
                                                  : "Smart prefab review blocked: " + review.message;
                }
                ImGui::SameLine();
                if (ImGui::Button("Apply Smart Prefab")) {
                    const bool applied = workspace.placementPanel().PlaceSelectedSmartPrefabAtGrid(
                        smartPrefabGridX, smartPrefabGridY, smartPrefabParameterValues);
                    const auto& result = workspace.placementPanel().lastRenderSnapshot().last_smart_prefab_result;
                    runtime.map_save_status = applied ? std::string{"Smart prefab applied as one native undoable Map operation."}
                                                       : "Smart prefab apply blocked: " + result.message;
                }
                const auto& result = snapshot.placement.last_smart_prefab_result;
                if (!result.code.empty()) {
                    ImGui::TextDisabled("Prefab status: %s", result.message.c_str());
                    if (!result.reviewed_operation_ids.empty()) {
                        std::string reviewedOperations;
                        for (const auto& operationId : result.reviewed_operation_ids) {
                            if (!reviewedOperations.empty()) reviewedOperations += ", ";
                            reviewedOperations += operationId;
                        }
                        ImGui::TextDisabled("Preflight-ready operation IDs: %s", reviewedOperations.c_str());
                    }
                    if (!result.rejected_operation_ids.empty()) {
                        std::string rejectedOperations;
                        for (const auto& operationId : result.rejected_operation_ids) {
                            if (!rejectedOperations.empty()) rejectedOperations += ", ";
                            rejectedOperations += operationId.empty() ? "<missing>" : operationId;
                        }
                        ImGui::TextDisabled("Blocked operation IDs: %s", rejectedOperations.c_str());
                    }
                }
            }
            ImGui::PopID();
        }
    }
}

void renderPerspectiveWorkspace(urpg::editor::EditorShell& editorShell, EditorPanelRuntime& runtime) {
    auto& workspace = runtime.perspective_2d_workspace;
    const auto& snapshot = workspace.lastRenderSnapshot();
    ImGui::Text("Status: %s", snapshot.status.c_str());
    ImGui::TextWrapped("%s", snapshot.message.c_str());
    ImGui::Text("Mode: %s", snapshot.toolbar.active_mode.c_str());
    ImGui::Text("Layers: %zu", snapshot.perspective_2d_layers.size());
    ImGui::Text("Events: %zu", snapshot.perspective_2d_events.size());
    ImGui::Text("Visible tile options: %zu", snapshot.perspective_2d_palette.visible_tile_option_count);
    ImGui::Separator();
    ImGui::Text("Tools");
    for (const auto& action : snapshot.toolbar.actions) {
        ImGui::PushID(action.id.c_str());
        if (!action.enabled) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(action.label.c_str(), ImVec2(118.0f, 0.0f))) {
            (void)workspace.ActivateToolbarAction(action.id);
        }
        if (!action.enabled) {
            ImGui::EndDisabled();
        }
        ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::NewLine();
    ImGui::Separator();
    ImGui::Text("Layers");
    int shown = 0;
    for (const auto& layer : snapshot.perspective_2d_layers) {
        if (shown++ >= 10) {
            ImGui::TextDisabled("...");
            break;
        }
        ImGui::PushID(layer.id.c_str());
        if (ImGui::Selectable(layer.label.c_str(), layer.selected)) {
            (void)workspace.SelectPerspectiveLayer(layer.id);
        }
        ImGui::SameLine(220.0f);
        ImGui::TextDisabled("%s", layer.kind.c_str());
        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Developer: Reviewed Creator Commands")) {
        static std::string creatorPrompt = "paint tile";
        static int creatorTileX = 0;
        static int creatorTileY = 0;
        static int creatorPlannedTileId = 2;
        static int creatorTilePaintWidth = 1;
        static int creatorTilePaintHeight = 1;
        static std::string creatorLayerId;
        static std::string creatorPaletteOptionId;
        static int creatorPropX = 0;
        static int creatorPropY = 0;
        static std::string creatorPropPaletteAssetId;
        static int creatorEventX = 0;
        static int creatorEventY = 0;
        static std::string creatorEventLayerId;
        static std::string creatorEventLabel = "Creator Message";
        static std::string creatorEventMessage = "A reviewed native Map message event.";

        const auto tileLayer = std::find_if(
            snapshot.perspective_2d_layers.begin(), snapshot.perspective_2d_layers.end(),
            [](const auto& layer) { return layer.kind == "tile" && layer.visible && !layer.locked; });
        const auto selectedCreatorLayer = std::find_if(
            snapshot.perspective_2d_layers.begin(), snapshot.perspective_2d_layers.end(),
            [&](const auto& layer) { return layer.id == creatorLayerId; });
        if (creatorLayerId.empty() || selectedCreatorLayer == snapshot.perspective_2d_layers.end() ||
            selectedCreatorLayer->kind != "tile" || !selectedCreatorLayer->visible || selectedCreatorLayer->locked) {
            creatorLayerId = tileLayer == snapshot.perspective_2d_layers.end() ? "" : tileLayer->id;
        }
        if (creatorPaletteOptionId.empty() ||
            std::none_of(snapshot.perspective_2d_palette.tile_options.begin(),
                         snapshot.perspective_2d_palette.tile_options.end(), [&](const auto& option) {
                             return option.option_id == creatorPaletteOptionId;
                         })) {
            creatorPaletteOptionId = snapshot.perspective_2d_palette.tile_options.empty()
                                         ? ""
                                         : snapshot.perspective_2d_palette.tile_options.front().option_id;
        }
        const auto selectedPaletteOption = std::find_if(
            snapshot.perspective_2d_palette.tile_options.begin(), snapshot.perspective_2d_palette.tile_options.end(),
            [&](const auto& option) { return option.option_id == creatorPaletteOptionId; });
        const auto selectedPropOption = std::find_if(
            snapshot.props.project_asset_options.begin(), snapshot.props.project_asset_options.end(),
            [&](const auto& option) { return option.asset_id == creatorPropPaletteAssetId; });
        if (creatorPropPaletteAssetId.empty() || selectedPropOption == snapshot.props.project_asset_options.end() ||
            !selectedPropOption->targeted_for_perspective_2d) {
            const auto firstPropOption = std::find_if(
                snapshot.props.project_asset_options.begin(), snapshot.props.project_asset_options.end(),
                [](const auto& option) { return option.targeted_for_perspective_2d; });
            creatorPropPaletteAssetId = firstPropOption == snapshot.props.project_asset_options.end()
                                             ? ""
                                             : firstPropOption->asset_id;
        }
        const auto activePropOption = std::find_if(
            snapshot.props.project_asset_options.begin(), snapshot.props.project_asset_options.end(),
            [&](const auto& option) { return option.asset_id == creatorPropPaletteAssetId; });
        const auto selectedCreatorEventLayer = std::find_if(
            snapshot.perspective_2d_layers.begin(), snapshot.perspective_2d_layers.end(),
            [&](const auto& layer) { return layer.id == creatorEventLayerId; });
        if (creatorEventLayerId.empty() || selectedCreatorEventLayer == snapshot.perspective_2d_layers.end() ||
            (selectedCreatorEventLayer->kind != "event" && selectedCreatorEventLayer->kind != "object") ||
            !selectedCreatorEventLayer->visible || selectedCreatorEventLayer->locked) {
            const auto firstCreatorEventLayer = std::find_if(
                snapshot.perspective_2d_layers.begin(), snapshot.perspective_2d_layers.end(), [](const auto& layer) {
                    return (layer.kind == "event" || layer.kind == "object") && layer.visible && !layer.locked;
                });
            creatorEventLayerId = firstCreatorEventLayer == snapshot.perspective_2d_layers.end()
                                      ? ""
                                      : firstCreatorEventLayer->id;
        }
        const auto activeCreatorEventLayer = std::find_if(
            snapshot.perspective_2d_layers.begin(), snapshot.perspective_2d_layers.end(),
            [&](const auto& layer) { return layer.id == creatorEventLayerId; });

        ImGui::TextDisabled("Developer-only. Reviews a local deterministic plan; provider transport is dry-run only.");
        ImGui::TextDisabled("Only separate tile-only, prop-only, and fixed message-event plans can apply through the active Map owner.");
        ImGui::InputText("Creator Tile Prompt", &creatorPrompt);
        ImGui::InputInt("Creator Tile X", &creatorTileX);
        ImGui::InputInt("Creator Tile Y", &creatorTileY);
        ImGui::InputInt("Creator Planned Tile ID", &creatorPlannedTileId);
        ImGui::InputInt("Creator Tile Paint Width (1-64)", &creatorTilePaintWidth);
        ImGui::InputInt("Creator Tile Paint Height (1-64)", &creatorTilePaintHeight);
        if (ImGui::BeginCombo("Creator Target Layer", creatorLayerId.empty() ? "Select tile layer" : creatorLayerId.c_str())) {
            for (const auto& layer : snapshot.perspective_2d_layers) {
                const bool supported = layer.kind == "tile" && layer.visible && !layer.locked;
                if (!supported) ImGui::BeginDisabled();
                const bool selected = layer.id == creatorLayerId;
                if (ImGui::Selectable(layer.label.c_str(), selected) && supported) {
                    creatorLayerId = layer.id;
                }
                if (!supported) ImGui::EndDisabled();
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        const char* creatorPaletteLabel = selectedPaletteOption == snapshot.perspective_2d_palette.tile_options.end()
                                              ? "Select active palette tile"
                                              : selectedPaletteOption->label.c_str();
        if (ImGui::BeginCombo("Creator Palette Tile", creatorPaletteLabel)) {
            for (const auto& option : snapshot.perspective_2d_palette.tile_options) {
                const bool selected = option.option_id == creatorPaletteOptionId;
                if (ImGui::Selectable(option.label.c_str(), selected)) {
                    creatorPaletteOptionId = option.option_id;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        const bool creatorTileIntentSupported = containsCaseInsensitive(creatorPrompt, "paint tile") ||
                                                containsCaseInsensitive(creatorPrompt, "stamp tile");
        const bool canReviewCreatorTilePlan = creatorTileIntentSupported && !workspace.activePerspectiveMapId().empty() &&
                                              tileLayer != snapshot.perspective_2d_layers.end() && !creatorLayerId.empty() &&
                                              selectedPaletteOption != snapshot.perspective_2d_palette.tile_options.end();
        if (!creatorTileIntentSupported) {
            ImGui::TextDisabled("Tile review accepts only the deterministic 'paint tile' or 'stamp tile' intent.");
        }
        if (!canReviewCreatorTilePlan) ImGui::BeginDisabled();
        if (ImGui::Button("Review Native Tile Plan")) {
            urpg::ai::CreatorCommandRequest request;
            request.prompt = creatorPrompt;
            request.project_id = runtime.project_session.isOpen()
                                     ? runtime.project_session.activeProject().project_id
                                     : "";
            request.map_id = workspace.activePerspectiveMapId();
            request.tile_x = creatorTileX;
            request.tile_y = creatorTileY;
            request.width = static_cast<int32_t>(snapshot.perspective_2d_project.width);
            request.height = static_cast<int32_t>(snapshot.perspective_2d_project.height);
            request.selected_tile_id = creatorPlannedTileId;
            request.tile_paint_width = creatorTilePaintWidth;
            request.tile_paint_height = creatorTilePaintHeight;
            request.provider = urpg::ai::CreatorAiProvider::LocalDeterministic;
            runtime.creator_command_panel.setMapWorkspace(&workspace);
            runtime.creator_command_panel.setTilePaletteBindings(
                {{creatorPlannedTileId, "terrain", creatorLayerId, selectedPaletteOption->tileset_id,
                  selectedPaletteOption->tile_id}});
            runtime.creator_command_panel.setPropAssetBindings({});
            runtime.creator_command_panel.setEventLayerBindings({});
            runtime.creator_command_panel.setRequest(std::move(request));
            runtime.creator_command_panel.render();
        }
        if (!canReviewCreatorTilePlan) ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextDisabled("Native prop plan: resolves one planned prop only to an existing attached Perspective 2D palette entry.");
        ImGui::InputInt("Creator Prop X", &creatorPropX);
        ImGui::InputInt("Creator Prop Y", &creatorPropY);
        const char* creatorPropLabel = activePropOption == snapshot.props.project_asset_options.end()
                                           ? "Select attached prop palette asset"
                                           : activePropOption->asset_id.c_str();
        if (ImGui::BeginCombo("Creator Prop Palette Asset", creatorPropLabel)) {
            for (const auto& option : snapshot.props.project_asset_options) {
                if (!option.targeted_for_perspective_2d) continue;
                const bool selected = option.asset_id == creatorPropPaletteAssetId;
                if (ImGui::Selectable(option.asset_id.c_str(), selected)) {
                    creatorPropPaletteAssetId = option.asset_id;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        const bool canReviewCreatorPropPlan = !workspace.activePerspectiveMapId().empty() &&
                                              activePropOption != snapshot.props.project_asset_options.end() &&
                                              activePropOption->targeted_for_perspective_2d &&
                                              !activePropOption->project_path.empty();
        if (!canReviewCreatorPropPlan) ImGui::BeginDisabled();
        if (ImGui::Button("Review Native Prop Plan")) {
            urpg::ai::CreatorCommandRequest request;
            request.prompt = "place prop";
            request.project_id = runtime.project_session.isOpen()
                                     ? runtime.project_session.activeProject().project_id
                                     : "";
            request.map_id = workspace.activePerspectiveMapId();
            request.tile_x = creatorPropX;
            request.tile_y = creatorPropY;
            request.width = static_cast<int32_t>(snapshot.perspective_2d_project.width);
            request.height = static_cast<int32_t>(snapshot.perspective_2d_project.height);
            request.selected_prop_asset_id = activePropOption->asset_id;
            request.provider = urpg::ai::CreatorAiProvider::LocalDeterministic;
            runtime.creator_command_panel.setMapWorkspace(&workspace);
            runtime.creator_command_panel.setTilePaletteBindings({});
            runtime.creator_command_panel.setPropAssetBindings(
                {{activePropOption->asset_id, activePropOption->asset_id, activePropOption->project_path}});
            runtime.creator_command_panel.setEventLayerBindings({});
            runtime.creator_command_panel.setRequest(std::move(request));
            runtime.creator_command_panel.render();
        }
        if (!canReviewCreatorPropPlan) ImGui::EndDisabled();

        ImGui::Separator();
        ImGui::TextDisabled("Native message-event plan: creates one confirm-interact show-text event on a visible unlocked Map layer.");
        ImGui::InputInt("Creator Event X", &creatorEventX);
        ImGui::InputInt("Creator Event Y", &creatorEventY);
        ImGui::InputText("Creator Event Label", &creatorEventLabel);
        ImGui::InputTextMultiline("Creator Event Message", &creatorEventMessage, ImVec2(-1.0f, 60.0f));
        const char* creatorEventLayerLabel = activeCreatorEventLayer == snapshot.perspective_2d_layers.end()
                                                 ? "Select visible event layer"
                                                 : activeCreatorEventLayer->label.c_str();
        if (ImGui::BeginCombo("Creator Event Layer", creatorEventLayerLabel)) {
            for (const auto& layer : snapshot.perspective_2d_layers) {
                const bool supported = (layer.kind == "event" || layer.kind == "object") && layer.visible && !layer.locked;
                if (!supported) ImGui::BeginDisabled();
                const bool selected = layer.id == creatorEventLayerId;
                if (ImGui::Selectable(layer.label.c_str(), selected) && supported) {
                    creatorEventLayerId = layer.id;
                }
                if (!supported) ImGui::EndDisabled();
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        const bool canReviewCreatorEventPlan = !workspace.activePerspectiveMapId().empty() &&
                                               activeCreatorEventLayer != snapshot.perspective_2d_layers.end() &&
                                               !creatorEventLabel.empty() && creatorEventLabel.size() <= 120 &&
                                               !creatorEventMessage.empty() &&
                                               creatorEventMessage.size() <= 1024;
        if (!canReviewCreatorEventPlan) ImGui::BeginDisabled();
        if (ImGui::Button("Review Native Message Event Plan")) {
            urpg::ai::CreatorCommandRequest request;
            request.prompt = "place message event";
            request.project_id = runtime.project_session.isOpen()
                                     ? runtime.project_session.activeProject().project_id
                                     : "";
            request.map_id = workspace.activePerspectiveMapId();
            request.tile_x = creatorEventX;
            request.tile_y = creatorEventY;
            request.width = static_cast<int32_t>(snapshot.perspective_2d_project.width);
            request.height = static_cast<int32_t>(snapshot.perspective_2d_project.height);
            request.selected_event_layer_id = activeCreatorEventLayer->id;
            request.event_label = creatorEventLabel;
            request.event_message = creatorEventMessage;
            request.provider = urpg::ai::CreatorAiProvider::LocalDeterministic;
            runtime.creator_command_panel.setMapWorkspace(&workspace);
            runtime.creator_command_panel.setTilePaletteBindings({});
            runtime.creator_command_panel.setPropAssetBindings({});
            runtime.creator_command_panel.setEventLayerBindings(
                {{activeCreatorEventLayer->id, activeCreatorEventLayer->id}});
            runtime.creator_command_panel.setRequest(std::move(request));
            runtime.creator_command_panel.render();
        }
        if (!canReviewCreatorEventPlan) ImGui::EndDisabled();

        const auto& creatorSnapshot = runtime.creator_command_panel.lastRenderSnapshot();
        if (!creatorSnapshot.empty()) {
            const auto applyPreview = creatorSnapshot.value("apply_preview", nlohmann::json::object());
            ImGui::TextWrapped("Review: %s", applyPreview.value("message", "No native Map review is available.").c_str());
            ImGui::TextDisabled("Plan: %s | tile edits: %zu | prop edits: %zu | event edits: %zu | diagnostics: %zu",
                                creatorSnapshot["plan"].value("intent", "unplanned").c_str(),
                                creatorSnapshot["plan"].value("tile_edits", nlohmann::json::array()).size(),
                                creatorSnapshot["plan"].value("prop_edits", nlohmann::json::array()).size(),
                                creatorSnapshot["plan"].value("logic_edits", nlohmann::json::array()).size(),
                                creatorSnapshot.value("validation_diagnostics", size_t{0}));
            const bool canApplyCreatorPlan = applyPreview.value("would_apply", false);
            if (!canApplyCreatorPlan) ImGui::BeginDisabled();
            if (ImGui::Button("Apply Reviewed Native Map Plan")) {
                const bool applied = runtime.creator_command_panel.applyCurrentPlan();
                const auto& apply = runtime.creator_command_panel.lastRenderSnapshot()["last_apply"];
                runtime.map_save_status = applied
                                              ? "Creator Map plan applied through the active Map owner; save Map to publish it."
                                              : "Creator Map plan was not applied: " +
                                                    apply.value("message", apply.value("code", "unknown failure"));
            }
            if (!canApplyCreatorPlan) ImGui::EndDisabled();
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Event Authoring");
    ImGui::TextDisabled("Creates a durable map event, its first page, and one supported native command.");
    static std::string eventId = "elder_mira_intro";
    static std::string eventLabel = "Elder Mira";
    static std::string eventTrigger = "confirm_interact";
    static constexpr const char* eventCommandCodes[] = {
        "show_text", "show_choice", "start_dialogue", "transfer_player", "change_switch", "change_variable", "change_self_switch", "change_gold",
        "change_item", "move_route", "call_common_event", "start_battle", "open_vendor",
    };
    static int eventCommandIndex = 0;
    static std::string commandArgument = "The moonwell lantern has gone dark.";
    static float eventScreenX = 80.0f;
    static float eventScreenY = 80.0f;
    static bool eventBlocksMovement = false;
    ImGui::InputText("Event ID", &eventId);
    ImGui::InputText("Label", &eventLabel);
    ImGui::InputText("Trigger", &eventTrigger);
    ImGui::Combo("Command", &eventCommandIndex, eventCommandCodes, IM_ARRAYSIZE(eventCommandCodes));
    ImGui::InputText("Command Argument", &commandArgument);
    if (std::string_view(eventCommandCodes[eventCommandIndex]) == "start_dialogue") {
        ImGui::TextDisabled("Use a saved Dialogue Graph ID from content/dialogues (letters, digits, '_' and '-' only).");
    } else if (std::string_view(eventCommandCodes[eventCommandIndex]) == "transfer_player") {
        ImGui::TextDisabled("Use the bound Map ID and in-bounds tile coordinates: map_id:x,y.");
    }
    ImGui::InputFloat("Map X", &eventScreenX, 1.0f, 8.0f, "%.0f");
    ImGui::InputFloat("Map Y", &eventScreenY, 1.0f, 8.0f, "%.0f");
    ImGui::Checkbox("First Page Blocks Movement", &eventBlocksMovement);
    if (ImGui::Button("Create Event Page")) {
        const auto layer = std::find_if(snapshot.perspective_2d_layers.begin(), snapshot.perspective_2d_layers.end(),
                                        [](const auto& candidate) {
                                            return candidate.kind == "event" || candidate.kind == "object";
                                        });
        if (layer == snapshot.perspective_2d_layers.end()) {
            runtime.map_save_status = "Event creation needs a visible event or object layer.";
        } else {
            (void)workspace.SelectPerspectiveLayer(layer->id);
            const auto pageId = eventId + "_main";
            const bool created = workspace.AddPerspectiveEventFromScreen(eventId, eventLabel, eventTrigger,
                                                                           eventScreenX, eventScreenY);
            const bool addedPage = created && workspace.AddPerspectiveEventPage(eventId, pageId, eventLabel, eventTrigger);
            const bool addedCommand = addedPage &&
                                      workspace.AddPerspectiveEventPageCommand(eventId, pageId,
                                                                              eventCommandCodes[eventCommandIndex],
                                                                              commandArgument);
            const bool configuredCollision = addedCommand &&
                                             (!eventBlocksMovement ||
                                              workspace.SetPerspectiveEventPageBlocksMovement(eventId, pageId, true));
            runtime.map_save_status = configuredCollision
                                          ? "Event page authored in the active Map document; save Map to publish it."
                                          : "Event creation was rejected; event IDs and page IDs must be unique.";
        }
    }
    if (!snapshot.perspective_2d_events.empty()) {
        ImGui::Text("Authored events: %zu", snapshot.perspective_2d_events.size());
        for (const auto& event : snapshot.perspective_2d_events) {
            ImGui::PushID(event.event_id.c_str());
            ImGui::Text("%s (%zu pages)", event.event_id.c_str(), event.page_count);
            bool blocksMovement = event.blocks_movement;
            if (ImGui::Checkbox("Default Blocks Movement", &blocksMovement)) {
                (void)workspace.SetPerspectiveEventBlocksMovement(event.event_id, blocksMovement);
            }
            bool spriteVisible = event.sprite_visible;
            if (ImGui::Checkbox("Default Sprite Visible", &spriteVisible)) {
                (void)workspace.SetPerspectiveEventSpriteVisible(event.event_id, spriteVisible);
            }
            int frameWidth = event.sprite_frame_width;
            int frameHeight = event.sprite_frame_height;
            int frameCount = event.sprite_frame_count;
            float frameDuration = event.sprite_frame_duration;
            bool spriteLoop = event.sprite_loop;
            bool changedSpriteAnimation = ImGui::InputInt("Sprite Frame Width", &frameWidth);
            ImGui::SameLine();
            changedSpriteAnimation = ImGui::InputInt("Sprite Frame Height", &frameHeight) || changedSpriteAnimation;
            changedSpriteAnimation = ImGui::InputInt("Sprite Frame Count", &frameCount) || changedSpriteAnimation;
            ImGui::SameLine();
            changedSpriteAnimation = ImGui::InputFloat("Sprite Frame Duration", &frameDuration, 0.01f, 0.1f, "%.2f") ||
                                     changedSpriteAnimation;
            changedSpriteAnimation = ImGui::Checkbox("Sprite Animation Loops", &spriteLoop) || changedSpriteAnimation;
            if (changedSpriteAnimation) {
                (void)workspace.SetPerspectiveEventSpriteAnimation(event.event_id, frameWidth, frameHeight,
                                                                    frameCount, frameDuration, spriteLoop);
            }
            for (const auto& page : event.pages) {
                ImGui::PushID(page.page_id.c_str());
                int collisionMode = !page.has_blocks_movement_override ? 0 : page.blocks_movement ? 1 : 2;
                ImGui::TextDisabled("Page: %s", page.label.empty() ? page.page_id.c_str() : page.label.c_str());
                ImGui::SameLine();
                if (ImGui::Combo("Collision", &collisionMode, "Inherit\0Block Movement\0Pass Through\0")) {
                    const std::optional<bool> blocksForPage =
                        collisionMode == 0 ? std::nullopt : std::optional<bool>{collisionMode == 1};
                    (void)workspace.SetPerspectiveEventPageBlocksMovement(event.event_id, page.page_id,
                                                                            blocksForPage);
                }
                int visibilityMode = !page.has_sprite_visible_override ? 0 : page.sprite_visible ? 1 : 2;
                ImGui::SameLine();
                if (ImGui::Combo("Sprite Visibility", &visibilityMode, "Inherit\0Visible\0Hidden\0")) {
                    const std::optional<bool> visibleForPage =
                        visibilityMode == 0 ? std::nullopt : std::optional<bool>{visibilityMode == 1};
                    (void)workspace.SetPerspectiveEventPageSpriteVisible(event.event_id, page.page_id,
                                                                          visibleForPage);
                }
                ImGui::PopID();
            }
            if (!event.asset_id.empty()) {
                ImGui::TextDisabled("Runtime event sprite: %s", event.asset_id.c_str());
                ImGui::TextDisabled("Project path: %s", event.asset_project_path.c_str());
            }
            if (event.asset_id.empty()) ImGui::SameLine();
            if (ImGui::SmallButton("Preview")) {
                const auto result = workspace.PreviewPerspectiveEventExecution(event.event_id);
                runtime.map_save_status = result.success
                                              ? "Native event preview completed: " + std::to_string(result.executed_command_count) +
                                                    " command(s) executed."
                                              : "Native event preview blocked: " + result.message;
            }
            ImGui::SameLine();
            if (ImGui::SmallButton("Run")) {
                const auto result = workspace.ExecutePerspectiveRuntimeEvent(event.event_id);
                if (!result.success) {
                    const std::string detail = result.blocker_codes.empty() ? std::string{} :
                        " [" + result.blocker_codes.back() + "]";
                    runtime.map_save_status = "Native event runtime blocked: " + result.message + detail;
                } else {
                    syncQuestPreviewWorldFromPerspectiveRuntime(result, runtime.quest_preview_world);
                    const bool openedMessageInspector =
                        openMessageInspectorForPerspectiveRuntime(editorShell, runtime, result, event.label);
                    if (!result.battles.empty()) {
                        const auto& encounterId = result.battles.front();
                        const auto ability = runtime.ability_inspector_panel.getDraftAsset();
                        runtime.battle_preview_actions.clear();
                        runtime.battle_preview_flow.beginBattle(true);
                        runtime.battle_preview_flow.enterInput();
                        runtime.battle_preview_actions.enqueue(
                            {runtime.character_draft_id, encounterId, ability.ability_id, 100, 0});
                        runtime.battle_preview_encounter_id = encounterId;
                        runtime.diagnostics_workspace.bindBattleRuntime(runtime.battle_preview_flow,
                                                                       runtime.battle_preview_actions);
                        runtime.diagnostics_workspace.setActiveTab(urpg::editor::DiagnosticsTab::Battle);
                        (void)editorShell.openPanel("diagnostics");
                        runtime.focus_workspace_next_frame = true;
                        runtime.map_save_status = "Native event launched encounter preview '" + encounterId +
                                                  "'. It remains a preview until a playtest records its combat result.";
                    } else if (!result.vendors.empty()) {
                        const auto& vendorId = result.vendors.front();
                        const auto* vendor = runtime.vendor_draft.findVendor(vendorId);
                        if (vendor == nullptr) {
                            runtime.map_save_status = "Native event referenced unknown vendor '" + vendorId + "'.";
                        } else {
                            runtime.vendor_draft_id = vendorId;
                            const auto stock = runtime.vendor_draft.refreshStock(vendorId, {});
                            runtime.map_save_status = "Native event opened vendor preview '" + vendorId + "' with " +
                                                      std::to_string(stock.size()) + " visible stock row(s).";
                        }
                    } else if (openedMessageInspector) {
                        runtime.map_save_status = "Native event opened the Message Inspector for its authored dialogue preview.";
                    } else {
                        runtime.map_save_status = "Native event runtime completed: " +
                                                  std::to_string(result.executed_command_count) + " command(s) executed.";
                    }
                }
            }
            ImGui::PopID();
        }
    }
}

void renderMapAuthoringWorkspace(urpg::editor::EditorShell& editorShell, EditorPanelRuntime& runtime) {
    auto& workspace = runtime.map_authoring_workspace;
    if (!runtime.map_dirty_surface_registered) {
        runtime.map_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
            kMapDirtyDocumentId,
            "level_builder",
            false,
            [&runtime] {
                std::string error;
                if (saveMapAuthoringDocument(runtime, &error)) {
                    (void)runtime.dirty_state_registry.markDirty("map.perspective_2d", false);
                    return urpg::editor::EditorDirtySaveResult{true, "map_saved", "Map saved atomically."};
                }
                return urpg::editor::EditorDirtySaveResult{false, "map_save_failed", std::move(error)};
            },
            [] {},
            {},
        });
    }
    if (!runtime.perspective_2d_dirty_surface_registered) {
        runtime.perspective_2d_dirty_surface_registered = runtime.dirty_state_registry.registerSurface({
            kPerspective2DDirtyDocumentId,
            "spatial_authoring",
            false,
            [&runtime] {
                std::string error;
                if (saveMapAuthoringDocument(runtime, &error)) {
                    (void)runtime.dirty_state_registry.markDirty("map.grid_parts", false);
                    return urpg::editor::EditorDirtySaveResult{true, "map_saved", "Map saved atomically."};
                }
                return urpg::editor::EditorDirtySaveResult{false, "map_save_failed", std::move(error)};
            },
            [] {},
            {},
        });
    }
    workspace.context().setDocumentDirty(urpg::editor::MapAuthoringDocumentOwner::GridParts,
                                         !runtime.level_builder_document.dirtyChunks().empty());
    const auto& levelSnapshot = runtime.level_builder_workspace.lastRenderSnapshot();
    const auto& perspectiveSnapshot = runtime.perspective_2d_workspace.lastRenderSnapshot();
    auto sharedSelection = workspace.context().snapshot().selection;
    // Child workspaces remain source-of-truth for their own documents. The
    // shared Map strip mirrors their current selection rather than maintaining
    // a second, competing selection model.
    sharedSelection.partId = levelSnapshot.palette.selected_part_id;
    sharedSelection.objectId = levelSnapshot.inspector.selected_instance_id;
    sharedSelection.layerId = perspectiveSnapshot.perspective_2d_project.selected_layer_id;
    sharedSelection.eventId.clear();
    sharedSelection.activeTool = perspectiveSnapshot.toolbar.active_mode.empty()
                                     ? levelSnapshot.active_mode
                                     : perspectiveSnapshot.toolbar.active_mode;
    sharedSelection.viewportFocus = workspace.snapshot().activeMode;
    for (const auto& event : perspectiveSnapshot.perspective_2d_events) {
        if (!event.selected_page_id.empty()) {
            sharedSelection.eventId = event.event_id;
            break;
        }
    }
    workspace.context().setSelection(std::move(sharedSelection));
    workspace.context().setDocumentDirty(urpg::editor::MapAuthoringDocumentOwner::Perspective2D,
                                         perspectiveSnapshot.perspective_2d_project.has_unsaved_changes);
    workspace.context().setValidation({levelSnapshot.validation.diagnostic_count,
                                       levelSnapshot.validation.blocking_count,
                                       levelSnapshot.validation.blocking_count > 0
                                           ? "Resolve Map validation blockers before playtest or package."
                                           : "Map validation has no blocking diagnostics."});
    runtime.playtest_session.update();
    const auto playtestState = runtime.playtest_session.state();
    const char* playtestStateLabel = "idle";
    switch (playtestState) {
    case urpg::editor::PlaytestSessionState::Starting: playtestStateLabel = "starting"; break;
    case urpg::editor::PlaytestSessionState::Running: playtestStateLabel = "running"; break;
    case urpg::editor::PlaytestSessionState::Stopping: playtestStateLabel = "stopping"; break;
    case urpg::editor::PlaytestSessionState::Exited: playtestStateLabel = "exited"; break;
    case urpg::editor::PlaytestSessionState::Crashed: playtestStateLabel = "crashed"; break;
    case urpg::editor::PlaytestSessionState::Returned: playtestStateLabel = "returned"; break;
    case urpg::editor::PlaytestSessionState::Inactive: break;
    }
    workspace.context().setPlaytestState(playtestStateLabel);
    workspace.context().setPackageState(workspace.snapshot().activeMode == "package"
                                            ? levelSnapshot.package.readiness
                                            : "draft");
    (void)runtime.dirty_state_registry.markDirty(kMapDirtyDocumentId,
                                                 !runtime.level_builder_document.dirtyChunks().empty());
    (void)runtime.dirty_state_registry.markDirty(
        kPerspective2DDirtyDocumentId, perspectiveSnapshot.perspective_2d_project.has_unsaved_changes);
    runtime.project_session.setDirtySurfaceSummaries(runtime.dirty_state_registry.dirtyDocumentIds());
    workspace.refresh();
    const auto& snapshot = workspace.snapshot();
    ImGui::Text("One Map workspace — deep links: Level Builder and Spatial Authoring");
    ImGui::Text("Map: %s", snapshot.context.activeMapId.empty() ? "(select a map)" : snapshot.context.activeMapId.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("Tool: %s", snapshot.context.selection.activeTool.empty() ? "select"
                                                                                    : snapshot.context.selection.activeTool.c_str());
    ImGui::Text("Layer: %s | Part: %s | Object: %s | Event: %s",
                snapshot.context.selection.layerId.empty() ? "(none)" : snapshot.context.selection.layerId.c_str(),
                snapshot.context.selection.partId.empty() ? "(none)" : snapshot.context.selection.partId.c_str(),
                snapshot.context.selection.objectId.empty() ? "(none)" : snapshot.context.selection.objectId.c_str(),
                snapshot.context.selection.eventId.empty() ? "(none)" : snapshot.context.selection.eventId.c_str());
    ImGui::Text("Validation: %zu diagnostics / %zu blocking | Playtest: %s | Package: %s",
                snapshot.context.validation.diagnosticCount, snapshot.context.validation.blockingCount,
                snapshot.context.playtestState.c_str(), snapshot.context.packageState.c_str());
    ImGui::TextWrapped("%s", snapshot.nextAction.c_str());
    if (runtime.available_map_ids.size() > 1) {
        ImGui::SameLine();
        if (ImGui::BeginCombo("##MapSelector", snapshot.context.activeMapId.c_str())) {
            for (const auto& mapId : runtime.available_map_ids) {
                const bool selected = mapId == snapshot.context.activeMapId;
                if (ImGui::Selectable(mapId.c_str(), selected)) {
                    const auto guard = runtime.dirty_state_registry.resolveNavigation(
                        urpg::editor::EditorNavigationDecision::Save);
                    if (!guard.allowed) {
                        runtime.map_save_status = "Map switch blocked: " + guard.diagnostic.message;
                    } else {
                        bindMapAuthoringProject(runtime, runtime.project_root, mapId);
                        runtime.map_save_status = "Opened map '" + mapId + "' through the native Map workspace.";
                    }
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
    }
    ImGui::Separator();
    ImGui::TextUnformatted("Contextual Authoring");
    ImGui::TextDisabled("Current character draft: %s", runtime.character_draft_id.c_str());
    if (ImGui::Button("Edit Character for This Map")) {
        (void)editorShell.openPanel("character_creator");
        runtime.focus_workspace_next_frame = true;
        workspace.setNextActionHint("Character authoring opened from the active Map context.");
    }
    ImGui::SameLine();
    if (ImGui::Button("Edit Ability for This Map")) {
        (void)editorShell.openPanel("ability");
        runtime.focus_workspace_next_frame = true;
        workspace.setNextActionHint("Ability authoring opened from the active Map context.");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Keeps the active project and Map context intact.");
    static std::string encounterId = "shrine_wisp";
    static std::string previewAbilityId = "willow_strike";
    if (ImGui::CollapsingHeader("Encounter Preview", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("Preview uses the native Battle Inspector; encounter launch remains authored by a Map event.");
        ImGui::InputText("Encounter ID", &encounterId);
        ImGui::InputText("Preview Ability", &previewAbilityId);
        if (ImGui::Button("Open Encounter Preview")) {
            if (encounterId.empty() || previewAbilityId.empty()) {
                runtime.map_save_status = "Encounter preview needs a non-empty encounter and ability ID.";
            } else {
                runtime.battle_preview_actions.clear();
                runtime.battle_preview_flow.beginBattle(true);
                runtime.battle_preview_flow.enterInput();
                runtime.battle_preview_actions.enqueue(
                    {runtime.character_draft_id, encounterId, previewAbilityId, 100, 0});
                runtime.battle_preview_encounter_id = encounterId;
                runtime.diagnostics_workspace.bindBattleRuntime(runtime.battle_preview_flow,
                                                               runtime.battle_preview_actions);
                runtime.diagnostics_workspace.setActiveTab(urpg::editor::DiagnosticsTab::Battle);
                (void)editorShell.openPanel("diagnostics");
                runtime.focus_workspace_next_frame = true;
                workspace.setNextActionHint("Battle preview opened from the active Map context.");
                runtime.map_save_status = "Encounter preview opened for '" + encounterId + "'.";
            }
        }
        ImGui::SameLine();
        const bool canRecordPreviewOutcome = !runtime.battle_preview_encounter_id.empty() &&
                                             runtime.battle_preview_flow.isActive();
        if (!canRecordPreviewOutcome) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button("Mark Preview Victory")) {
            runtime.battle_preview_flow.enterAction();
            runtime.battle_preview_flow.markVictory();
            if (std::find(runtime.quest_preview_world.battles.begin(), runtime.quest_preview_world.battles.end(),
                          runtime.battle_preview_encounter_id) == runtime.quest_preview_world.battles.end()) {
                runtime.quest_preview_world.battles.push_back(runtime.battle_preview_encounter_id);
            }
            runtime.diagnostics_workspace.bindBattleRuntime(runtime.battle_preview_flow,
                                                           runtime.battle_preview_actions);
            runtime.diagnostics_workspace.setActiveTab(urpg::editor::DiagnosticsTab::Battle);
            runtime.map_save_status = "Battle preview marked victory for '" + runtime.battle_preview_encounter_id +
                                      "' and updated the quest preview only; this does not claim a playtest combat result.";
        }
        if (!canRecordPreviewOutcome) {
            ImGui::EndDisabled();
        }
    }
    if (ImGui::CollapsingHeader("Audio Mix", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::TextDisabled("Project mix presets apply to the native editor audio core and save with this project.");
        const auto presetNames = runtime.audio_mix_draft.listPresets();
        if (ImGui::BeginCombo("Mix Preset", runtime.audio_mix_preset.c_str())) {
            for (const auto& presetName : presetNames) {
                const bool selected = presetName == runtime.audio_mix_preset;
                if (ImGui::Selectable(presetName.c_str(), selected)) {
                    runtime.audio_mix_preset = presetName;
                }
                if (selected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        ImGui::TextUnformatted("Governed Encounter Audio");
        ImGui::TextDisabled("Assigned asset: %s",
                            runtime.audio_preview_asset_id.empty() ? "(none)" : runtime.audio_preview_asset_id.c_str());
        ImGui::BeginChild("AudioMixAssetDrop", ImVec2(0.0f, 54.0f), true);
        ImGui::TextUnformatted("Drop an attached audio asset here");
        if (ImGui::BeginDragDropTarget()) {
            if (const auto* drag = ImGui::AcceptDragDropPayload("URPG_EDITOR_ASSET_V1")) {
                const auto* begin = static_cast<const std::uint8_t*>(drag->Data);
                std::vector<std::uint8_t> bytes(begin, begin + drag->DataSize);
                urpg::editor::EditorAssetDragPayload asset;
                const auto parsed = urpg::editor::deserializeEditorAssetDragPayload(bytes, &asset);
                auto accepted = parsed.accepted && asset.mediaKind == "audio"
                                    ? urpg::editor::assessEditorAssetDrop(asset, true)
                                    : urpg::editor::EditorAssetDropDecision{
                                          false, "audio_mix_asset_drop_requires_audio",
                                          "Audio Mix accepts attached audio assets only.",
                                          "Attach an audio asset in Assets, then drag it here."};
                if (accepted.accepted) {
                    accepted = urpg::editor::validateEditorAssetAttachmentRevision(asset, runtime.project_root);
                }
                if (!accepted.accepted) {
                    runtime.map_asset_drop_status = accepted.message +
                                                    (accepted.remediation.empty() ? "" : " " + accepted.remediation);
                } else if (runtime.audio_preview_asset_id == asset.assetId) {
                    runtime.map_asset_drop_status = "Audio asset assignment made no change.";
                } else {
                    runtime.audio_preview_asset_undo.push_back(runtime.audio_preview_asset_id);
                    runtime.audio_preview_asset_id = asset.assetId;
                    runtime.audio_preview_asset_redo.clear();
                    (void)runtime.dirty_state_registry.markDirty(kAudioMixDirtyDocumentId, true);
                    runtime.map_asset_drop_status =
                        "Attached audio asset assigned as one undoable Audio Mix action.";
                }
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::EndChild();
        if (ImGui::Button("Undo Audio Asset Assignment")) {
            if (!runtime.audio_preview_asset_undo.empty()) {
                runtime.audio_preview_asset_redo.push_back(runtime.audio_preview_asset_id);
                runtime.audio_preview_asset_id = runtime.audio_preview_asset_undo.back();
                runtime.audio_preview_asset_undo.pop_back();
                (void)runtime.dirty_state_registry.markDirty(kAudioMixDirtyDocumentId, true);
                runtime.map_asset_drop_status = "Audio asset assignment undone.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Redo Audio Asset Assignment")) {
            if (!runtime.audio_preview_asset_redo.empty()) {
                runtime.audio_preview_asset_undo.push_back(runtime.audio_preview_asset_id);
                runtime.audio_preview_asset_id = runtime.audio_preview_asset_redo.back();
                runtime.audio_preview_asset_redo.pop_back();
                (void)runtime.dirty_state_registry.markDirty(kAudioMixDirtyDocumentId, true);
                runtime.map_asset_drop_status = "Audio asset assignment redone.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Preview Assigned Audio")) {
            if (runtime.audio_preview_asset_id.empty()) {
                runtime.map_asset_drop_status = "Assign an attached audio asset before previewing it.";
            } else if (runtime.audio_preview_core.playSound(runtime.audio_preview_asset_id) == 0) {
                runtime.map_asset_drop_status = "Native audio preview could not start; inspect Audio diagnostics.";
            } else {
                runtime.diagnostics_workspace.bindAudioRuntime(runtime.audio_preview_core);
                runtime.map_asset_drop_status = "Native audio preview started for the assigned project asset.";
            }
        }
        if (!runtime.map_asset_drop_status.empty()) ImGui::TextWrapped("%s", runtime.map_asset_drop_status.c_str());
        if (ImGui::Button("Apply Mix Preview")) {
            if (!runtime.audio_mix_panel.selectPreset(runtime.audio_mix_preset)) {
                runtime.map_save_status = "Audio mix preview rejected an unknown preset.";
            } else {
                runtime.diagnostics_workspace.bindAudioRuntime(runtime.audio_preview_core);
                runtime.diagnostics_workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
                (void)runtime.dirty_state_registry.markDirty(kAudioMixDirtyDocumentId, true);
                runtime.map_save_status = "Applied audio mix '" + runtime.audio_mix_preset +
                                          "' to the native editor preview; save to publish it.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Open Audio Diagnostics")) {
            runtime.diagnostics_workspace.bindAudioRuntime(runtime.audio_preview_core);
            runtime.diagnostics_workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
            (void)editorShell.openPanel("diagnostics");
            runtime.focus_workspace_next_frame = true;
            workspace.setNextActionHint("Audio diagnostics opened from the active Map context.");
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Audio Mix")) {
            const auto result = runtime.dirty_state_registry.save(kAudioMixDirtyDocumentId);
            runtime.map_save_status = result.message;
        }
    }
    if (ImGui::CollapsingHeader("Input Remap")) {
        static int keyCode = 90;
        static int actionIndex = 5;
        static constexpr const char* inputActionLabels[] = {
            "None", "Move Up", "Move Down", "Move Left", "Move Right", "Confirm", "Cancel", "Menu",
            "Page Left", "Page Right", "Battle Attack", "Battle Skill", "Battle Item", "Battle Defend",
            "Battle Escape", "Debug",
        };
        static constexpr urpg::input::InputAction inputActions[] = {
            urpg::input::InputAction::None,       urpg::input::InputAction::MoveUp,
            urpg::input::InputAction::MoveDown,   urpg::input::InputAction::MoveLeft,
            urpg::input::InputAction::MoveRight,  urpg::input::InputAction::Confirm,
            urpg::input::InputAction::Cancel,     urpg::input::InputAction::Menu,
            urpg::input::InputAction::PageLeft,   urpg::input::InputAction::PageRight,
            urpg::input::InputAction::BattleAttack, urpg::input::InputAction::BattleSkill,
            urpg::input::InputAction::BattleItem, urpg::input::InputAction::BattleDefend,
            urpg::input::InputAction::BattleEscape, urpg::input::InputAction::Debug,
        };
        ImGui::TextDisabled("Project key mappings use the native versioned input-remap store.");
        ImGui::InputInt("Virtual Key Code", &keyCode);
        ImGui::Combo("Input Action", &actionIndex, inputActionLabels, IM_ARRAYSIZE(inputActionLabels));
        ImGui::TextDisabled("%zu active mapping(s)", runtime.input_remap_draft.getAllMappings().size());
        if (ImGui::Button("Apply Input Binding")) {
            if (keyCode < 0) {
                runtime.map_save_status = "Input binding needs a non-negative virtual key code.";
            } else {
                runtime.input_remap_draft.setMapping(keyCode, inputActions[actionIndex]);
                (void)runtime.dirty_state_registry.markDirty(kInputRemapDirtyDocumentId, true);
                runtime.map_save_status = "Input binding updated; save it before switching context.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Restore Default Bindings")) {
            runtime.input_remap_draft.resetToDefaults();
            (void)runtime.dirty_state_registry.markDirty(kInputRemapDirtyDocumentId, true);
            runtime.map_save_status = "Default input bindings restored; save to publish them.";
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Input Remaps")) {
            const auto result = runtime.dirty_state_registry.save(kInputRemapDirtyDocumentId);
            runtime.map_save_status = result.message;
        }
    }
    if (ImGui::CollapsingHeader("Accessibility Audit")) {
        ImGui::TextDisabled("Audits the active native audio mix, battle-preview, menu focus rows, and spatial Map controls.");
        if (ImGui::Button("Audit Current Creator Surfaces")) {
            std::vector<urpg::accessibility::UiElementSnapshot> elements;
            int32_t nextAccessibilityFocusOrder = 1;
            const auto appendAccessibilityElements = [&](std::vector<urpg::accessibility::UiElementSnapshot> source) {
                int32_t firstLocalFocusOrder = std::numeric_limits<int32_t>::max();
                int32_t lastLocalFocusOrder = 0;
                for (const auto& element : source) {
                    if (element.focusOrder > 0) {
                        firstLocalFocusOrder = std::min(firstLocalFocusOrder, element.focusOrder);
                        lastLocalFocusOrder = std::max(lastLocalFocusOrder, element.focusOrder);
                    }
                }
                if (lastLocalFocusOrder > 0) {
                    for (auto& element : source) {
                        if (element.focusOrder > 0) {
                            element.focusOrder += nextAccessibilityFocusOrder - firstLocalFocusOrder;
                        }
                    }
                    nextAccessibilityFocusOrder += lastLocalFocusOrder - firstLocalFocusOrder + 1;
                }
                elements.insert(elements.end(), std::make_move_iterator(source.begin()), std::make_move_iterator(source.end()));
            };
            appendAccessibilityElements(urpg::editor::AccessibilityAudioAdapter::ingest(runtime.audio_mix_draft));
            auto battleElements = urpg::editor::AccessibilityBattleAdapter::ingest(
                runtime.diagnostics_workspace.battlePanel().getModel());
            appendAccessibilityElements(std::move(battleElements));
            auto menuElements = urpg::editor::AccessibilityMenuAdapter::ingest(
                runtime.diagnostics_workspace.menuPanel().getModel());
            appendAccessibilityElements(std::move(menuElements));
            const auto& spatialSnapshot = runtime.perspective_2d_workspace.lastRenderSnapshot();
            auto spatialElements = urpg::editor::AccessibilitySpatialAdapter::ingest(
                spatialSnapshot.elevation, spatialSnapshot.props, spatialSnapshot.parts_placement,
                &runtime.map_authoring_workspace.snapshot());
            appendAccessibilityElements(std::move(spatialElements));
            auto creatorCommandElements = urpg::editor::AccessibilitySpatialAdapter::ingestCreatorCommand(
                runtime.creator_command_panel.lastRenderSnapshot());
            appendAccessibilityElements(std::move(creatorCommandElements));
            runtime.accessibility_auditor.clear();
            runtime.accessibility_auditor.ingestElements(elements);
            runtime.accessibility_panel.render();
            runtime.map_save_status = "Accessibility audit completed for " + std::to_string(elements.size()) +
                                      " current creator element(s).";
        }
        const auto accessibilitySnapshot = runtime.accessibility_panel.lastRenderSnapshot();
        if (accessibilitySnapshot.value("status", "not_run") == "ready") {
            ImGui::Text("Issues: %zu (errors %zu, warnings %zu)",
                        accessibilitySnapshot.value("issueCount", size_t{0}),
                        accessibilitySnapshot.value("errorCount", size_t{0}),
                        accessibilitySnapshot.value("warningCount", size_t{0}));
            for (const auto& issue : accessibilitySnapshot.value("issues", nlohmann::json::array())) {
                ImGui::BulletText("[%s] %s", issue.value("category", "issue").c_str(),
                                  issue.value("message", "Accessibility issue.").c_str());
            }
        } else {
            ImGui::TextDisabled("Run the audit to inspect the current native creator surfaces.");
        }
    }
    if (ImGui::CollapsingHeader("Localization Reference Audit")) {
        static urpg::localization::ProjectLocalizationAudit localizationReferenceAudit;
        static bool localizationReferenceAuditRan = false;
        static std::string pseudoLocalizationPreview = "Review localization layout before shipping.";
        ImGui::TextDisabled("Read-only audit of saved Dialogue and Quest localization references.");
        ImGui::InputText("Pseudo-localization Preview", &pseudoLocalizationPreview);
        ImGui::TextWrapped("%s", urpg::localization::pseudoLocalize(pseudoLocalizationPreview).c_str());
        ImGui::TextDisabled("Preview only: no locale bundle is changed.");
        if (runtime.project_root.empty()) ImGui::BeginDisabled();
        if (ImGui::Button("Audit Project Localization References")) {
            localizationReferenceAudit = urpg::localization::buildProjectLocalizationAudit(runtime.project_root);
            localizationReferenceAuditRan = true;
        }
        if (runtime.project_root.empty()) ImGui::EndDisabled();
        if (localizationReferenceAuditRan) {
            ImGui::Text("Referenced keys: %zu | Available keys: %zu", localizationReferenceAudit.references.size(),
                        localizationReferenceAudit.available_keys.size());
            for (const auto& key : localizationReferenceAudit.missing_referenced_keys) {
                ImGui::BulletText("Missing referenced key: %s", key.c_str());
            }
            for (const auto& locale : localizationReferenceAudit.missing_font_profile_locales) {
                ImGui::BulletText("Locale without font profile: %s", locale.c_str());
            }
            ImGui::Text("Locale coverage gaps: %zu", localizationReferenceAudit.missing_referenced_locale_keys.size());
            if (ImGui::TreeNode("Missing Referenced Locale Keys")) {
                constexpr size_t kMaxDisplayedLocaleCoverageGaps = 64;
                size_t displayed = 0;
                for (const auto& gap : localizationReferenceAudit.missing_referenced_locale_keys) {
                    if (displayed++ == kMaxDisplayedLocaleCoverageGaps) {
                        ImGui::TextDisabled("Additional locale coverage gaps are retained by the audit result.");
                        break;
                    }
                    ImGui::BulletText("%s | %s", gap.locale.c_str(), gap.key.c_str());
                }
                ImGui::TreePop();
            }
            for (const auto& key : localizationReferenceAudit.unused_key_candidates) {
                ImGui::BulletText("Unused-key candidate: %s", key.c_str());
            }
            for (const auto& diagnostic : localizationReferenceAudit.diagnostics) {
                ImGui::TextDisabled("Localization audit diagnostic: %s", diagnostic.c_str());
            }
            if (ImGui::TreeNode("Indexed Localization References")) {
                constexpr size_t kMaxDisplayedLocalizationReferences = 64;
                size_t displayed = 0;
                for (const auto& reference : localizationReferenceAudit.references) {
                    if (displayed++ == kMaxDisplayedLocalizationReferences) {
                        ImGui::TextDisabled("Additional references are retained by the audit result.");
                        break;
                    }
                    ImGui::BulletText("%s | %s | %s | %s", reference.key.c_str(),
                                      reference.document_path.generic_string().c_str(), reference.owner_kind.c_str(),
                                      reference.local_id.c_str());
                }
                ImGui::TreePop();
            }
            ImGui::TextDisabled("Unused-key candidates are not deletion authority; unindexed owners may still use them.");
        }
    }
    if (ImGui::CollapsingHeader("Export Diagnostics")) {
        static int exportTargetIndex = 0;
        static constexpr const char* exportTargetLabels[] = {
            "Windows x64", "Linux x64", "macOS Universal", "Web WASM",
        };
        static constexpr urpg::tools::ExportTarget exportTargets[] = {
            urpg::tools::ExportTarget::Windows_x64,
            urpg::tools::ExportTarget::Linux_x64,
            urpg::tools::ExportTarget::macOS_Universal,
            urpg::tools::ExportTarget::Web_WASM,
        };
        ImGui::TextDisabled("Run native export preflight for this project; this view never creates a package.");
        ImGui::Combo("Export Target", &exportTargetIndex, exportTargetLabels, IM_ARRAYSIZE(exportTargetLabels));
        const auto previewOutputDir = runtime.project_root / "build" / "export-preview";
        ImGui::TextDisabled("Preview output: %s", previewOutputDir.generic_string().c_str());
        if (ImGui::Button("Run Export Preflight")) {
            urpg::tools::ExportConfig config{};
            config.target = exportTargets[exportTargetIndex];
            config.mode = urpg::tools::ExportMode::DevBootstrap;
            config.outputDir = previewOutputDir.string();
            runtime.export_diagnostics_panel.setExportConfig(config);
            runtime.export_diagnostics_panel.render();
            runtime.map_save_status = "Export preflight completed; no package was generated.";
        }
        const auto& exportSnapshot = runtime.export_diagnostics_panel.lastRenderSnapshot();
        if (exportSnapshot.value("panel", "") == "export_diagnostics") {
            const bool preflightPassed = exportSnapshot.value("validationPassed", false);
            ImGui::Text("Preflight: %s", preflightPassed ? "passed" : "blocked");
            ImGui::TextDisabled("This is staging validation, not package or release evidence.");
            for (const auto& error : exportSnapshot.value("errors", nlohmann::json::array())) {
                ImGui::BulletText("%s", error.get<std::string>().c_str());
            }
            if (exportSnapshot.value("postExportValidationPassed", false)) {
                ImGui::TextDisabled("Existing emitted output passed post-export inspection.");
            } else {
                ImGui::TextDisabled("No validated emitted package is present at the preview output.");
            }
        } else {
            ImGui::TextDisabled("Run preflight to inspect this project's export staging configuration.");
        }
    }
    if (ImGui::CollapsingHeader("Save and Load Preview State")) {
        static int saveSlot = 1;
        ImGui::TextDisabled("Persists authoritative Perspective 2D runtime state through the native save coordinator.");
        ImGui::InputInt("Save Slot", &saveSlot);
        saveSlot = std::max(1, saveSlot);
        const auto& runtimeSnapshot =
            runtime.perspective_2d_workspace.lastRenderSnapshot().last_perspective_2d_runtime;
        const bool canSaveRuntime = runtimeSnapshot.success && !runtimeSnapshot.serialized_runtime_state_json.empty() &&
                                    runtime.map_runtime_save_session != nullptr;
        if (!canSaveRuntime) ImGui::BeginDisabled();
        if (ImGui::Button("Save Current Preview State")) {
            const auto savesDirectory = runtime.project_root / "saves";
            const auto prefix = "map_preview_slot_" + std::to_string(saveSlot);
            nlohmann::json payload = {
                {"schema", "urpg.map_preview_save.v1"},
                {"perspective_runtime", nlohmann::json::parse(runtimeSnapshot.serialized_runtime_state_json)},
                {"battle_preview_outcomes", runtime.quest_preview_world.battles},
            };
            urpg::SaveSessionSaveRequest request;
            request.slot_id = saveSlot;
            request.meta.category = urpg::SaveSlotCategory::Manual;
            request.meta.retention_class = urpg::SaveRetentionClass::Manual;
            request.meta.map_display_name = runtimeSnapshot.player_map_id.empty() ? runtimeSnapshot.map_id
                                                                                   : runtimeSnapshot.player_map_id;
            request.meta.custom_metadata["owner"] = "map_authoring_runtime";
            request.primary_save_path = savesDirectory / (prefix + ".json");
            request.metadata_path = savesDirectory / (prefix + ".meta.json");
            request.payload = payload.dump(2);
            const auto saved = runtime.map_runtime_save_session->save(request);
            runtime.map_save_status = saved.ok ? "Saved native Map preview state to slot " + std::to_string(saveSlot) + "."
                                                : "Map preview save failed: " + saved.error;
        }
        if (!canSaveRuntime) ImGui::EndDisabled();
        ImGui::SameLine();
        const bool canLoadRuntime = runtime.map_runtime_save_session != nullptr;
        if (!canLoadRuntime) ImGui::BeginDisabled();
        if (ImGui::Button("Load Preview State")) {
            const auto savesDirectory = runtime.project_root / "saves";
            const auto prefix = "map_preview_slot_" + std::to_string(saveSlot);
            urpg::SaveSessionLoadRequest request;
            request.slot_id = saveSlot;
            request.runtime_request.primary_save_path = savesDirectory / (prefix + ".json");
            request.runtime_request.metadata_path = savesDirectory / (prefix + ".meta.json");
            const auto loaded = runtime.map_runtime_save_session->load(request);
            const auto runtimePayload = urpg::RuntimeSaveLoader::Load(request.runtime_request);
            const auto payload = runtimePayload.ok ? nlohmann::json::parse(runtimePayload.payload, nullptr, false)
                                                   : nlohmann::json(nullptr);
            if (!loaded.ok) {
                runtime.map_save_status = "Map preview load failed: " + loaded.error;
            } else if (!payload.is_object() || payload.value("schema", "") != "urpg.map_preview_save.v1" ||
                       !payload.contains("perspective_runtime")) {
                runtime.map_save_status = "Map preview load rejected an incompatible save payload.";
            } else {
                const auto restored =
                    runtime.perspective_2d_workspace.RestorePerspectiveRuntimeState(payload["perspective_runtime"].dump());
                if (!restored.success) {
                    runtime.map_save_status = "Map preview state restore blocked: " + restored.message;
                } else {
                    syncQuestPreviewWorldFromPerspectiveRuntime(restored, runtime.quest_preview_world);
                    runtime.quest_preview_world.battles = payload.value("battle_preview_outcomes", std::vector<std::string>{});
                    runtime.map_save_status = "Loaded native Map preview state from slot " + std::to_string(saveSlot) + ".";
                }
            }
        }
        if (!canLoadRuntime) ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Open Save Diagnostics")) {
            (void)editorShell.openPanel("diagnostics");
            runtime.focus_workspace_next_frame = true;
            workspace.setNextActionHint("Native Save diagnostics opened from the active Map context.");
        }
        ImGui::TextDisabled("Preview save/load is not playtest or package evidence.");
    }
    static std::string questId = "restore_moonwell_lantern";
    static std::string questTitle = "Restore the Moonwell Lantern";
    static std::string objectiveId = "return_lantern_to_elder";
    static std::string conditionType = "item";
    static std::string conditionId = "moonwell_lantern";
    static std::string graphNodeId = "optional_objective";
    static std::string graphNodeType = "objective";
    static std::string graphNodeTitle = "Optional Objective";
    static std::string graphObjectiveId = "optional_objective";
    static std::string graphLocalizationKey;
    static std::string graphLinkFrom = "objective";
    static std::string graphLinkTo = "complete";
    static std::string graphConditionNodeId = "optional_objective";
    static std::string graphConditionType = "switch";
    static std::string graphConditionId = "optional_ready";
    static int graphConditionValue = 1;
    static std::string graphRewardNodeId = "optional_objective";
    static std::string graphRewardType = "gold";
    static std::string graphRewardId = "gold";
    static int graphRewardValue = 25;
    static std::string questCanvasDragNodeId;
    static int32_t questCanvasDragStartX = 0;
    static int32_t questCanvasDragStartY = 0;
    static std::string dialogueId = "moonwell_intro";
    static std::string dialogueStartNodeId = "start";
    static std::string dialogueSpeakerId = "elder_rowan";
    static std::string dialogueSpeakerName = "Elder Rowan";
    static std::string dialogueLocalizationKey = "dialogue.moonwell_intro";
    static std::string dialogueTextPreview = "The Moonwell has grown quiet.";
    static std::string dialogueNodeId = "farewell";
    static std::string dialogueNodeSpeakerId = "elder_rowan";
    static std::string dialogueNodeSpeakerName = "Elder Rowan";
    static std::string dialogueNodeLocalizationKey = "dialogue.moonwell_farewell";
    static std::string dialogueNodeTextPreview = "Travel safely, then.";
    static bool dialogueNodeEnding = true;
    static std::string dialogueNodeVoiceAssetId;
    static std::string dialogueNodeCaptionLocalizationKey;
    static std::string dialogueCanvasDragNodeId;
    static int32_t dialogueCanvasDragStartX = 0;
    static int32_t dialogueCanvasDragStartY = 0;
    static std::string dialogueChoiceSourceId = "start";
    static std::string dialogueChoiceId = "ask_moonwell";
    static std::string dialogueChoiceLabel = "Ask about the Moonwell";
    static std::string dialogueChoiceLocalizationKey;
    static std::string dialogueChoiceTargetId = "farewell";
    static std::string dialogueRuntimeLocale;
    static std::string dialogueConditionKey = "moonwell_ready";
    static std::string dialogueConditionOp = ">=";
    static int dialogueConditionValue = 1;
    static std::string dialogueEffectKey = "elder_affinity";
    static int dialogueEffectDelta = 1;
    ImGui::TextDisabled("Quest draft: %s", runtime.quest_draft_id.c_str());
    if (ImGui::CollapsingHeader("Quest Authoring", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto pick_quest_localization_key = [&runtime](const char* label, std::string& localization_key) {
            if (runtime.dialogue_localization_key_options.empty()) {
                ImGui::TextDisabled("No valid project localization bundle is available for selection.");
                return;
            }
            if (ImGui::BeginCombo(label, localization_key.empty() ? "Select project key" : localization_key.c_str())) {
                for (const auto& candidate : runtime.dialogue_localization_key_options) {
                    const bool selected = candidate == localization_key;
                    if (ImGui::Selectable(candidate.c_str(), selected)) {
                        localization_key = candidate;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        };
        ImGui::InputText("Quest ID", &questId);
        ImGui::InputText("Quest Title", &questTitle);
        ImGui::InputText("Objective ID", &objectiveId);
        ImGui::InputText("Completion Type", &conditionType);
        ImGui::InputText("Completion ID", &conditionId);
        if (ImGui::Button("Create Quest Graph")) {
            urpg::quest::QuestObjectiveGraphDocument graph;
            graph.quest_id = questId;
            graph.title = questTitle;
            graph.nodes = {
                {"start", "start", "Start", "", "", {}, {}},
                {"objective", "objective", objectiveId, objectiveId, "", {{conditionType, conditionId, 0}}, {}},
                {"complete", "complete", "Complete", "", "", {}, {}},
            };
            graph.links = {{"start", "objective"}, {"objective", "complete"}};
            if (graph.validate().empty()) {
                runtime.quest_draft = std::move(graph);
                runtime.quest_draft_id = questId;
                runtime.quest_undo_history.clear();
                runtime.quest_redo_history.clear();
                (void)runtime.dirty_state_registry.markDirty(kQuestDirtyDocumentId, true);
                runtime.map_save_status = "Quest graph authored in the active project; save it before switching context.";
            } else {
                runtime.map_save_status = "Quest graph needs a non-empty quest ID, objective ID, and completion condition.";
            }
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Runtime preview: %zu choice(s), %zu item(s), %zu battle-preview result(s)",
                            runtime.quest_preview_world.dialogue_choices.size(), runtime.quest_preview_world.items.size(),
                            runtime.quest_preview_world.battles.size());
        if (runtime.quest_draft.has_value()) {
            const auto node_exists = [&runtime](const std::string& node_id) {
                return std::any_of(runtime.quest_draft->nodes.begin(), runtime.quest_draft->nodes.end(),
                                   [&](const auto& node) { return node.id == node_id; });
            };
            ImGui::Separator();
            ImGui::Text("Quest graph: %zu node(s), %zu link(s)", runtime.quest_draft->nodes.size(),
                        runtime.quest_draft->links.size());
            if (ImGui::Button("Undo Quest Edit") && undoQuestGraphMutation(runtime)) {
                runtime.map_save_status = "Quest graph edit undone.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Redo Quest Edit") && redoQuestGraphMutation(runtime)) {
                runtime.map_save_status = "Quest graph edit redone.";
            }
            ImGui::SameLine();
            ImGui::TextDisabled("%zu undo / %zu redo", runtime.quest_undo_history.size(), runtime.quest_redo_history.size());

            ImGui::TextUnformatted("Quest Graph Canvas");
            ImGui::TextDisabled("Click a node to load its ID into the graph controls. Drag a node to persist its canvas position.");
            const ImVec2 quest_canvas_card_size{168.0f, 56.0f};
            if (ImGui::BeginChild("QuestGraphCanvas", ImVec2(0.0f, 270.0f), true,
                                  ImGuiWindowFlags_HorizontalScrollbar)) {
                const ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
                const auto add = [](const ImVec2 left, const ImVec2 right) {
                    return ImVec2(left.x + right.x, left.y + right.y);
                };
                const auto canvas_position = [](const urpg::quest::QuestGraphNode& node, const size_t index) {
                    if (node.has_canvas_position) {
                        return ImVec2(static_cast<float>(node.canvas_x), static_cast<float>(node.canvas_y));
                    }
                    return ImVec2(16.0f + static_cast<float>(index % 3U) * 184.0f,
                                  16.0f + static_cast<float>(index / 3U) * 78.0f);
                };
                const auto node_position = [&](const std::string& node_id) {
                    for (size_t index = 0; index < runtime.quest_draft->nodes.size(); ++index) {
                        const auto& node = runtime.quest_draft->nodes[index];
                        if (node.id == node_id) return canvas_position(node, index);
                    }
                    return ImVec2{};
                };
                auto* draw_list = ImGui::GetWindowDrawList();
                for (const auto& link : runtime.quest_draft->links) {
                    const auto source = add(add(canvas_origin, node_position(link.from)),
                                            ImVec2(quest_canvas_card_size.x, quest_canvas_card_size.y * 0.5f));
                    const auto target = add(add(canvas_origin, node_position(link.to)),
                                            ImVec2(0.0f, quest_canvas_card_size.y * 0.5f));
                    draw_list->AddLine(source, target, IM_COL32(115, 210, 150, 220), 2.0f);
                }
                for (size_t index = 0; index < runtime.quest_draft->nodes.size(); ++index) {
                    const auto& node = runtime.quest_draft->nodes[index];
                    bool canvas_position_changed = false;
                    const auto position = canvas_position(node, index);
                    ImGui::SetCursorScreenPos(add(canvas_origin, position));
                    ImGui::PushID(node.id.c_str());
                    const auto label = node.id + "  [" + node.type + "]";
                    if (ImGui::Button(label.c_str(), quest_canvas_card_size)) {
                        graphNodeId = node.id;
                        graphNodeType = node.type;
                        graphNodeTitle = node.title;
                        graphObjectiveId = node.objective_id;
                        graphLocalizationKey = node.localization_key;
                        graphConditionNodeId = node.id;
                        graphRewardNodeId = node.id;
                    }
                    if (ImGui::IsItemActivated()) {
                        questCanvasDragNodeId = node.id;
                        questCanvasDragStartX = static_cast<int32_t>(position.x);
                        questCanvasDragStartY = static_cast<int32_t>(position.y);
                    }
                    if (ImGui::IsItemDeactivated() && questCanvasDragNodeId == node.id) {
                        const auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                        if (delta.x != 0.0f || delta.y != 0.0f) {
                            auto next = *runtime.quest_draft;
                            const auto moved = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                                return candidate.id == node.id;
                            });
                            if (moved != next.nodes.end()) {
                                moved->canvas_x = std::max<int32_t>(0, questCanvasDragStartX + static_cast<int32_t>(delta.x));
                                moved->canvas_y = std::max<int32_t>(0, questCanvasDragStartY + static_cast<int32_t>(delta.y));
                                moved->has_canvas_position = true;
                                if (applyQuestGraphMutation(runtime, std::move(next))) {
                                    runtime.map_save_status = "Quest node canvas position updated as one undoable project edit.";
                                    canvas_position_changed = true;
                                }
                            }
                        }
                        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                        questCanvasDragNodeId.clear();
                    }
                    ImGui::PopID();
                    if (canvas_position_changed) break;
                }
                ImGui::SetCursorScreenPos(add(canvas_origin, ImVec2(0.0f, 252.0f)));
                ImGui::Dummy(ImVec2(620.0f, 1.0f));
            }
            ImGui::EndChild();

            ImGui::InputText("Graph Node ID", &graphNodeId);
            ImGui::InputText("Graph Node Type", &graphNodeType);
            ImGui::InputText("Graph Node Title", &graphNodeTitle);
            ImGui::InputText("Graph Objective ID", &graphObjectiveId);
            ImGui::InputText("Graph Localization Key", &graphLocalizationKey);
            pick_quest_localization_key("Pick Graph Localization Key", graphLocalizationKey);
            if (ImGui::Button("Add Quest Node")) {
                auto next = *runtime.quest_draft;
                if (graphNodeId.empty() || graphNodeType.empty() || node_exists(graphNodeId)) {
                    runtime.map_save_status = "Quest node requires a unique ID and a type.";
                } else if (graphNodeType == "objective" && graphObjectiveId.empty()) {
                    runtime.map_save_status = "Objective nodes require an objective ID.";
                } else {
                    next.nodes.push_back({graphNodeId, graphNodeType, graphNodeTitle,
                                          graphNodeType == "objective" ? graphObjectiveId : "", graphLocalizationKey,
                                          {}, {}});
                    if (applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest node added as one undoable project edit.";
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Update Quest Node")) {
                auto next = *runtime.quest_draft;
                const auto node = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                    return candidate.id == graphNodeId;
                });
                const auto start_count = std::count_if(next.nodes.begin(), next.nodes.end(), [](const auto& candidate) {
                    return candidate.type == "start";
                });
                if (node == next.nodes.end() || graphNodeType.empty() ||
                    (graphNodeType == "objective" && graphObjectiveId.empty())) {
                    runtime.map_save_status = "Update an existing quest node with a type and required objective ID.";
                } else if (node->type == "start" && graphNodeType != "start" && start_count <= 1) {
                    runtime.map_save_status = "A quest graph must retain at least one start node.";
                } else {
                    node->type = graphNodeType;
                    node->title = graphNodeTitle;
                    node->objective_id = graphNodeType == "objective" ? graphObjectiveId : "";
                    node->localization_key = graphLocalizationKey;
                    if (applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest node updated as one undoable project edit.";
                    } else {
                        runtime.map_save_status = "Quest node update made no change.";
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Quest Node")) {
                auto next = *runtime.quest_draft;
                const auto node = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                    return candidate.id == graphNodeId;
                });
                const auto start_count = std::count_if(next.nodes.begin(), next.nodes.end(), [](const auto& candidate) {
                    return candidate.type == "start";
                });
                if (node == next.nodes.end()) {
                    runtime.map_save_status = "Enter an existing quest node ID to remove it.";
                } else if (node->type == "start" && start_count <= 1) {
                    runtime.map_save_status = "A quest graph must retain at least one start node.";
                } else {
                    next.nodes.erase(node);
                    next.links.erase(std::remove_if(next.links.begin(), next.links.end(), [&](const auto& link) {
                        return link.from == graphNodeId || link.to == graphNodeId;
                    }), next.links.end());
                    if (applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest node and its links removed as one undoable project edit.";
                    }
                }
            }

            ImGui::InputText("Quest Link From", &graphLinkFrom);
            ImGui::InputText("Quest Link To", &graphLinkTo);
            if (ImGui::Button("Add Quest Link")) {
                auto next = *runtime.quest_draft;
                const bool duplicate = std::any_of(next.links.begin(), next.links.end(), [&](const auto& link) {
                    return link.from == graphLinkFrom && link.to == graphLinkTo;
                });
                if (!node_exists(graphLinkFrom) || !node_exists(graphLinkTo) || graphLinkFrom == graphLinkTo) {
                    runtime.map_save_status = "Quest links require two distinct existing node IDs.";
                } else if (duplicate) {
                    runtime.map_save_status = "That quest link already exists.";
                } else {
                    next.links.push_back({graphLinkFrom, graphLinkTo});
                    if (applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest link added as one undoable project edit.";
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Quest Link")) {
                auto next = *runtime.quest_draft;
                const auto previous_size = next.links.size();
                next.links.erase(std::remove_if(next.links.begin(), next.links.end(), [&](const auto& link) {
                    return link.from == graphLinkFrom && link.to == graphLinkTo;
                }), next.links.end());
                if (next.links.size() == previous_size) {
                    runtime.map_save_status = "That quest link does not exist.";
                } else if (applyQuestGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Quest link removed as one undoable project edit.";
                }
            }

            ImGui::InputText("Condition Node ID", &graphConditionNodeId);
            ImGui::InputText("Condition Type", &graphConditionType);
            ImGui::InputText("Condition ID", &graphConditionId);
            ImGui::InputInt("Condition Value", &graphConditionValue);
            if (ImGui::Button("Add Quest Condition")) {
                auto next = *runtime.quest_draft;
                const auto node = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                    return candidate.id == graphConditionNodeId;
                });
                if (node == next.nodes.end() || graphConditionType.empty() || graphConditionId.empty()) {
                    runtime.map_save_status = "Quest conditions require an existing node plus type and ID.";
                } else {
                    const urpg::quest::QuestCondition condition{graphConditionType, graphConditionId, graphConditionValue};
                    const bool duplicate = std::any_of(node->conditions.begin(), node->conditions.end(),
                                                       [&](const auto& candidate) {
                                                           return candidate.type == condition.type &&
                                                                  candidate.id == condition.id &&
                                                                  candidate.value == condition.value;
                                                       });
                    if (duplicate) {
                        runtime.map_save_status = "That quest condition already exists on the selected node.";
                    } else {
                        node->conditions.push_back(condition);
                    }
                    if (!duplicate && applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest condition added as one undoable project edit.";
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Quest Condition")) {
                auto next = *runtime.quest_draft;
                const auto node = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                    return candidate.id == graphConditionNodeId;
                });
                if (node == next.nodes.end()) {
                    runtime.map_save_status = "Enter an existing quest node ID to remove a condition.";
                } else {
                    const auto previous_size = node->conditions.size();
                    node->conditions.erase(std::remove_if(node->conditions.begin(), node->conditions.end(),
                                                          [&](const auto& candidate) {
                                                              return candidate.type == graphConditionType &&
                                                                     candidate.id == graphConditionId &&
                                                                     candidate.value == graphConditionValue;
                                                          }),
                                           node->conditions.end());
                    if (node->conditions.size() == previous_size) {
                        runtime.map_save_status = "That quest condition does not exist on the selected node.";
                    } else if (applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest condition removed as one undoable project edit.";
                    }
                }
            }

            ImGui::InputText("Reward Node ID", &graphRewardNodeId);
            ImGui::InputText("Reward Type", &graphRewardType);
            ImGui::InputText("Reward ID", &graphRewardId);
            ImGui::InputInt("Reward Value", &graphRewardValue);
            if (ImGui::Button("Add Quest Reward")) {
                auto next = *runtime.quest_draft;
                const auto node = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                    return candidate.id == graphRewardNodeId;
                });
                if (node == next.nodes.end() || graphRewardType.empty() || graphRewardId.empty()) {
                    runtime.map_save_status = "Quest rewards require an existing node plus type and ID.";
                } else {
                    const urpg::quest::QuestReward reward{graphRewardType, graphRewardId, graphRewardValue};
                    const bool duplicate = std::any_of(node->rewards.begin(), node->rewards.end(),
                                                       [&](const auto& candidate) {
                                                           return candidate.type == reward.type && candidate.id == reward.id &&
                                                                  candidate.value == reward.value;
                                                       });
                    if (duplicate) {
                        runtime.map_save_status = "That quest reward already exists on the selected node.";
                    } else {
                        node->rewards.push_back(reward);
                    }
                    if (!duplicate && applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest reward added as one undoable project edit.";
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Quest Reward")) {
                auto next = *runtime.quest_draft;
                const auto node = std::find_if(next.nodes.begin(), next.nodes.end(), [&](const auto& candidate) {
                    return candidate.id == graphRewardNodeId;
                });
                if (node == next.nodes.end()) {
                    runtime.map_save_status = "Enter an existing quest node ID to remove a reward.";
                } else {
                    const auto previous_size = node->rewards.size();
                    node->rewards.erase(std::remove_if(node->rewards.begin(), node->rewards.end(),
                                                       [&](const auto& candidate) {
                                                           return candidate.type == graphRewardType &&
                                                                  candidate.id == graphRewardId &&
                                                                  candidate.value == graphRewardValue;
                                                       }),
                                        node->rewards.end());
                    if (node->rewards.size() == previous_size) {
                        runtime.map_save_status = "That quest reward does not exist on the selected node.";
                    } else if (applyQuestGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Quest reward removed as one undoable project edit.";
                    }
                }
            }

            auto diagnostics = runtime.quest_draft->validate();
            const auto flow_diagnostics = runtime.quest_draft->analyzeFlow();
            diagnostics.insert(diagnostics.end(), flow_diagnostics.begin(), flow_diagnostics.end());
            const std::set<std::string> localization_keys(runtime.dialogue_localization_key_options.begin(),
                                                           runtime.dialogue_localization_key_options.end());
            const auto localization_diagnostics = runtime.quest_draft->validateLocalizationKeys(localization_keys);
            diagnostics.insert(diagnostics.end(), localization_diagnostics.begin(), localization_diagnostics.end());
            if (diagnostics.empty()) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Quest graph validation: ready");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "Quest graph validation: %zu issue(s)", diagnostics.size());
                for (const auto& diagnostic : diagnostics) {
                    ImGui::BulletText("%s: %s", diagnostic.code.c_str(), diagnostic.message.c_str());
                }
            }
        }
        if (ImGui::Button("Preview Quest") && runtime.quest_draft.has_value()) {
            const auto preview = runtime.quest_draft->preview(runtime.quest_preview_world);
            runtime.map_save_status = preview.diagnostics.empty()
                                          ? "Quest preview completed: " + std::to_string(preview.ready_node_ids.size()) +
                                                " ready node(s), " + std::to_string(preview.blocked_node_ids.size()) + " blocked."
                                          : "Quest preview has validation diagnostics.";
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Quest")) {
            const auto result = runtime.dirty_state_registry.save(kQuestDirtyDocumentId);
            runtime.map_save_status = result.message;
        }
    }
    ImGui::TextDisabled("Dialogue draft: %s", runtime.dialogue_draft_id.c_str());
    if (ImGui::CollapsingHeader("Dialogue Authoring", ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto pick_localization_key = [&runtime](const char* label, std::string& localization_key) {
            if (runtime.dialogue_localization_key_options.empty()) {
                ImGui::TextDisabled("No valid project localization bundle is available for selection.");
                return;
            }
            if (ImGui::BeginCombo(label, localization_key.empty() ? "Select project key" : localization_key.c_str())) {
                for (const auto& candidate : runtime.dialogue_localization_key_options) {
                    const bool selected = candidate == localization_key;
                    if (ImGui::Selectable(candidate.c_str(), selected)) {
                        localization_key = candidate;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        };
        const auto pick_speaker = [&runtime](const char* label, std::string& speaker_id, std::string& speaker_name) {
            if (runtime.dialogue_speaker_options.empty()) {
                ImGui::TextDisabled("No valid project character draft is available for speaker selection.");
                return;
            }
            if (ImGui::BeginCombo(label, speaker_id.empty() ? "Select project character" : speaker_id.c_str())) {
                for (const auto& [candidate_id, candidate_name] : runtime.dialogue_speaker_options) {
                    const bool selected = candidate_id == speaker_id;
                    const auto label_text = candidate_id + " (" + candidate_name + ")";
                    if (ImGui::Selectable(label_text.c_str(), selected)) {
                        speaker_id = candidate_id;
                        speaker_name = candidate_name;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
        };
        ImGui::InputText("Dialogue ID", &dialogueId);
        ImGui::InputText("Start Node ID", &dialogueStartNodeId);
        ImGui::InputText("Start Speaker ID", &dialogueSpeakerId);
        ImGui::InputText("Start Speaker Name", &dialogueSpeakerName);
        pick_speaker("Pick Start Speaker", dialogueSpeakerId, dialogueSpeakerName);
        ImGui::InputText("Start Localization Key", &dialogueLocalizationKey);
        pick_localization_key("Pick Start Localization Key", dialogueLocalizationKey);
        ImGui::InputText("Start Text Preview", &dialogueTextPreview);
        if (ImGui::Button("Create Dialogue Graph")) {
            if (dialogueId.empty() || dialogueStartNodeId.empty() || dialogueSpeakerId.empty() ||
                dialogueLocalizationKey.empty()) {
                runtime.map_save_status = "Dialogue requires an ID, start node, speaker ID, and localization key.";
            } else {
                urpg::dialogue::DialogueGraph graph;
                if (!graph.addNode({dialogueStartNodeId, dialogueSpeakerId, dialogueSpeakerName,
                                    dialogueLocalizationKey, dialogueTextPreview, false, {}})) {
                    runtime.map_save_status = "Dialogue start node ID is invalid or duplicated.";
                } else {
                    runtime.dialogue_draft = std::move(graph);
                    runtime.dialogue_draft_id = dialogueId;
                    runtime.dialogue_undo_history.clear();
                    runtime.dialogue_redo_history.clear();
                    runtime.dialogue_preview_values.clear();
                    runtime.dialogue_preview_node_id.clear();
                    runtime.dialogue_preview_trace.clear();
                    runtime.dialogue_preview_diagnostics.clear();
                    (void)runtime.dirty_state_registry.markDirty(kDialogueDirtyDocumentId, true);
                    runtime.map_save_status = "Dialogue graph authored in the active project; save it before switching context.";
                }
            }
        }

        if (runtime.dialogue_draft.has_value()) {
            const auto node_exists = [&runtime](const std::string& node_id) {
                return runtime.dialogue_draft->findNode(node_id) != nullptr;
            };
            ImGui::Separator();
            ImGui::Text("Dialogue graph: %zu node(s)", runtime.dialogue_draft->nodes().size());
            if (ImGui::Button("Undo Dialogue Edit") && undoDialogueGraphMutation(runtime)) {
                runtime.map_save_status = "Dialogue graph edit undone.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Redo Dialogue Edit") && redoDialogueGraphMutation(runtime)) {
                runtime.map_save_status = "Dialogue graph edit redone.";
            }
            ImGui::SameLine();
            ImGui::TextDisabled("%zu undo / %zu redo", runtime.dialogue_undo_history.size(),
                                runtime.dialogue_redo_history.size());

            ImGui::TextUnformatted("Dialogue Graph Canvas");
            ImGui::TextDisabled("Click a node to load it into the editor controls. Drag a node to persist its canvas position.");
            const ImVec2 dialogue_canvas_card_size{168.0f, 56.0f};
            if (ImGui::BeginChild("DialogueGraphCanvas", ImVec2(0.0f, 270.0f), true,
                                  ImGuiWindowFlags_HorizontalScrollbar)) {
                const ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
                const auto add = [](const ImVec2 left, const ImVec2 right) {
                    return ImVec2(left.x + right.x, left.y + right.y);
                };
                const auto canvas_position = [&](const urpg::dialogue::DialogueNode& node, const size_t index) {
                    if (node.has_canvas_position) {
                        return ImVec2(static_cast<float>(node.canvas_x), static_cast<float>(node.canvas_y));
                    }
                    return ImVec2(16.0f + static_cast<float>(index % 3U) * 184.0f,
                                  16.0f + static_cast<float>(index / 3U) * 78.0f);
                };
                const auto node_index = [&](const std::string& node_id) {
                    size_t index = 0;
                    for (const auto& [candidate_id, _] : runtime.dialogue_draft->nodes()) {
                        if (candidate_id == node_id) return index;
                        ++index;
                    }
                    return size_t{0};
                };
                const auto node_position = [&](const std::string& node_id) {
                    const auto* node = runtime.dialogue_draft->findNode(node_id);
                    return node == nullptr ? ImVec2{} : canvas_position(*node, node_index(node_id));
                };
                auto* draw_list = ImGui::GetWindowDrawList();
                size_t node_index_value = 0;
                for (const auto& [node_id, node] : runtime.dialogue_draft->nodes()) {
                    const auto source = add(add(canvas_origin, canvas_position(node, node_index_value)),
                                            ImVec2(dialogue_canvas_card_size.x, dialogue_canvas_card_size.y * 0.5f));
                    for (const auto& choice : node.choices) {
                        const auto target = add(add(canvas_origin, node_position(choice.target_node_id)),
                                                ImVec2(0.0f, dialogue_canvas_card_size.y * 0.5f));
                        draw_list->AddLine(source, target, IM_COL32(105, 165, 230, 220), 2.0f);
                    }
                    ++node_index_value;
                }
                node_index_value = 0;
                for (const auto& [node_id, node] : runtime.dialogue_draft->nodes()) {
                    bool canvas_position_changed = false;
                    const auto position = canvas_position(node, node_index_value++);
                    ImGui::SetCursorScreenPos(add(canvas_origin, position));
                    ImGui::PushID(node_id.c_str());
                    const auto label = node_id + (node.ending ? "  [end]" : "");
                    if (ImGui::Button(label.c_str(), dialogue_canvas_card_size)) {
                        dialogueNodeId = node_id;
                        dialogueNodeSpeakerId = node.speaker_id;
                        dialogueNodeSpeakerName = node.speaker_name;
                        dialogueNodeLocalizationKey = node.localization_key;
                        dialogueNodeTextPreview = node.text_preview;
                        dialogueNodeEnding = node.ending;
                        dialogueNodeVoiceAssetId = node.voice_asset_id;
                        dialogueNodeCaptionLocalizationKey = node.caption_localization_key;
                    }
                    if (ImGui::IsItemActivated()) {
                        dialogueCanvasDragNodeId = node_id;
                        dialogueCanvasDragStartX = static_cast<int32_t>(position.x);
                        dialogueCanvasDragStartY = static_cast<int32_t>(position.y);
                    }
                    if (ImGui::IsItemDeactivated() && dialogueCanvasDragNodeId == node_id) {
                        const auto delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Left);
                        if (delta.x != 0.0f || delta.y != 0.0f) {
                            auto next = *runtime.dialogue_draft;
                            const auto next_x =
                                std::max<int32_t>(0, dialogueCanvasDragStartX + static_cast<int32_t>(delta.x));
                            const auto next_y =
                                std::max<int32_t>(0, dialogueCanvasDragStartY + static_cast<int32_t>(delta.y));
                            if (next.updateNodeCanvasPosition(node_id, next_x, next_y) &&
                                applyDialogueGraphMutation(runtime, std::move(next))) {
                                runtime.map_save_status = "Dialogue node canvas position updated as one undoable project edit.";
                                canvas_position_changed = true;
                            }
                        }
                        ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
                        dialogueCanvasDragNodeId.clear();
                    }
                    ImGui::PopID();
                    if (canvas_position_changed) break;
                }
                ImGui::SetCursorScreenPos(add(canvas_origin, ImVec2(0.0f, 252.0f)));
                ImGui::Dummy(ImVec2(620.0f, 1.0f));
            }
            ImGui::EndChild();

            ImGui::InputText("Dialogue Node ID", &dialogueNodeId);
            ImGui::InputText("Node Speaker ID", &dialogueNodeSpeakerId);
            ImGui::InputText("Node Speaker Name", &dialogueNodeSpeakerName);
            pick_speaker("Pick Node Speaker", dialogueNodeSpeakerId, dialogueNodeSpeakerName);
            ImGui::InputText("Node Localization Key", &dialogueNodeLocalizationKey);
            pick_localization_key("Pick Node Localization Key", dialogueNodeLocalizationKey);
            ImGui::InputText("Node Text Preview", &dialogueNodeTextPreview);
            ImGui::Checkbox("Node Is Ending", &dialogueNodeEnding);
            if (ImGui::Button("Add Dialogue Node")) {
                auto next = *runtime.dialogue_draft;
                if (dialogueNodeId.empty() || dialogueNodeSpeakerId.empty() || dialogueNodeLocalizationKey.empty() ||
                    node_exists(dialogueNodeId)) {
                    runtime.map_save_status = "Dialogue nodes require unique IDs, speaker IDs, and localization keys.";
                } else if (next.addNode({dialogueNodeId, dialogueNodeSpeakerId, dialogueNodeSpeakerName,
                                         dialogueNodeLocalizationKey, dialogueNodeTextPreview, dialogueNodeEnding, {}}) &&
                           applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue node added as one undoable project edit.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Update Dialogue Node")) {
                auto next = *runtime.dialogue_draft;
                if (next.updateNode(dialogueNodeId, dialogueNodeSpeakerId, dialogueNodeSpeakerName,
                                    dialogueNodeLocalizationKey, dialogueNodeTextPreview, dialogueNodeEnding) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue node updated as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Update an existing dialogue node with a speaker ID and localization key.";
                }
            }
            ImGui::TextDisabled("Node voice asset: %s",
                                dialogueNodeVoiceAssetId.empty() ? "(none)" : dialogueNodeVoiceAssetId.c_str());
            ImGui::BeginChild("DialogueNodeVoiceAssetDrop", ImVec2(0.0f, 46.0f), true);
            ImGui::TextUnformatted("Drop an attached audio asset here");
            if (ImGui::BeginDragDropTarget()) {
                if (const auto* drag = ImGui::AcceptDragDropPayload("URPG_EDITOR_ASSET_V1")) {
                    const auto* begin = static_cast<const std::uint8_t*>(drag->Data);
                    std::vector<std::uint8_t> bytes(begin, begin + drag->DataSize);
                    urpg::editor::EditorAssetDragPayload asset;
                    const auto parsed = urpg::editor::deserializeEditorAssetDragPayload(bytes, &asset);
                    auto accepted = parsed.accepted && asset.mediaKind == "audio"
                                        ? urpg::editor::assessEditorAssetDrop(asset, true)
                                        : urpg::editor::EditorAssetDropDecision{
                                              false, "dialogue_voice_drop_requires_audio",
                                              "Dialogue voice accepts attached audio assets only.",
                                              "Attach an audio asset in Assets, then drag it here."};
                    if (accepted.accepted) {
                        accepted = urpg::editor::validateEditorAssetAttachmentRevision(asset, runtime.project_root);
                    }
                    if (!accepted.accepted) {
                        runtime.map_save_status = accepted.message +
                                                  (accepted.remediation.empty() ? "" : " " + accepted.remediation);
                    } else {
                        dialogueNodeVoiceAssetId = asset.assetId;
                        runtime.map_save_status = "Attached audio asset selected for the dialogue node voice reference.";
                    }
                }
                ImGui::EndDragDropTarget();
            }
            ImGui::EndChild();
            pick_localization_key("Node Caption Localization Key", dialogueNodeCaptionLocalizationKey);
            if (ImGui::Button("Load Dialogue Node Media")) {
                if (const auto* node = runtime.dialogue_draft->findNode(dialogueNodeId)) {
                    dialogueNodeVoiceAssetId = node->voice_asset_id;
                    dialogueNodeCaptionLocalizationKey = node->caption_localization_key;
                    runtime.map_save_status = "Dialogue node voice/caption references loaded into the authoring controls.";
                } else {
                    runtime.map_save_status = "Enter an existing dialogue node ID to load its media references.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Update Dialogue Node Media")) {
                auto next = *runtime.dialogue_draft;
                if (next.updateNodeMediaReferences(dialogueNodeId, dialogueNodeVoiceAssetId,
                                                   dialogueNodeCaptionLocalizationKey) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue node voice/caption references updated as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Select an existing dialogue node and change its voice or caption reference.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Dialogue Node Voice")) {
                dialogueNodeVoiceAssetId.clear();
                runtime.map_save_status = "Dialogue node voice selection cleared; update node media to save the change.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Dialogue Node Caption")) {
                dialogueNodeCaptionLocalizationKey.clear();
                runtime.map_save_status = "Dialogue node caption selection cleared; update node media to save the change.";
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Dialogue Node")) {
                auto next = *runtime.dialogue_draft;
                if (!node_exists(dialogueNodeId)) {
                    runtime.map_save_status = "Enter an existing dialogue node ID to remove it.";
                } else if (dialogueNodeId == next.startNode()) {
                    runtime.map_save_status = "Set a different start node before removing this dialogue node.";
                } else if (next.removeNode(dialogueNodeId) && applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue node and its inbound choices removed as one undoable project edit.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Set Dialogue Start")) {
                auto next = *runtime.dialogue_draft;
                if (!node_exists(dialogueNodeId)) {
                    runtime.map_save_status = "Enter an existing dialogue node ID as the start node.";
                } else {
                    next.setStartNode(dialogueNodeId);
                    if (applyDialogueGraphMutation(runtime, std::move(next))) {
                        runtime.map_save_status = "Dialogue start node changed as one undoable project edit.";
                    }
                }
            }

            ImGui::InputText("Choice Source Node", &dialogueChoiceSourceId);
            ImGui::InputText("Dialogue Choice ID", &dialogueChoiceId);
            ImGui::InputText("Dialogue Choice Label", &dialogueChoiceLabel);
            ImGui::InputText("Dialogue Choice Localization Key", &dialogueChoiceLocalizationKey);
            pick_localization_key("Pick Choice Localization Key", dialogueChoiceLocalizationKey);
            ImGui::InputText("Choice Target Node", &dialogueChoiceTargetId);
            if (ImGui::Button("Load Dialogue Choice")) {
                const auto* source = runtime.dialogue_draft->findNode(dialogueChoiceSourceId);
                if (source == nullptr) {
                    runtime.map_save_status = "Enter an existing source node and choice ID to load a dialogue choice.";
                } else if (const auto choice = std::find_if(source->choices.begin(), source->choices.end(),
                                                            [&](const auto& candidate) {
                                                                return candidate.id == dialogueChoiceId;
                                                            });
                           choice != source->choices.end()) {
                    dialogueChoiceLabel = choice->label;
                    dialogueChoiceLocalizationKey = choice->localization_key;
                    dialogueChoiceTargetId = choice->target_node_id;
                    runtime.map_save_status = "Dialogue choice loaded into the native authoring controls.";
                } else {
                    runtime.map_save_status = "Enter an existing source node and choice ID to load a dialogue choice.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Add Dialogue Choice")) {
                auto next = *runtime.dialogue_draft;
                if (!node_exists(dialogueChoiceSourceId) || !node_exists(dialogueChoiceTargetId) ||
                    dialogueChoiceId.empty() || dialogueChoiceLabel.empty()) {
                    runtime.map_save_status = "Dialogue choices require existing source/target nodes plus an ID and label.";
                } else if (next.addChoice(dialogueChoiceSourceId,
                                           {dialogueChoiceId, dialogueChoiceLabel, dialogueChoiceTargetId, {}, {},
                                            dialogueChoiceLocalizationKey}) &&
                           applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice added as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Dialogue choice ID must be unique within its source node.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Update Dialogue Choice")) {
                auto next = *runtime.dialogue_draft;
                if (next.updateChoice(dialogueChoiceSourceId, dialogueChoiceId, dialogueChoiceLabel,
                                      dialogueChoiceTargetId, dialogueChoiceLocalizationKey) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice updated as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Update an existing choice with a non-empty label and existing target node.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Dialogue Choice")) {
                auto next = *runtime.dialogue_draft;
                if (next.removeChoice(dialogueChoiceSourceId, dialogueChoiceId) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice removed as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Enter an existing source node and choice ID to remove a dialogue choice.";
                }
            }

            ImGui::InputText("Choice Condition Key", &dialogueConditionKey);
            constexpr std::array<const char*, 7> supportedDialogueConditionOperators{
                "=", "==", "!=", ">", ">=", "<", "<="};
            if (ImGui::BeginCombo("Choice Condition Operator", dialogueConditionOp.c_str())) {
                for (const auto* candidate : supportedDialogueConditionOperators) {
                    const bool selected = dialogueConditionOp == candidate;
                    if (ImGui::Selectable(candidate, selected)) {
                        dialogueConditionOp = candidate;
                    }
                    if (selected) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            ImGui::InputInt("Choice Condition Value", &dialogueConditionValue);
            if (ImGui::Button("Add Dialogue Choice Condition")) {
                auto next = *runtime.dialogue_draft;
                if (next.addChoiceCondition(dialogueChoiceSourceId, dialogueChoiceId,
                                            {dialogueConditionKey, dialogueConditionOp, dialogueConditionValue}) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice condition added as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Dialogue conditions require an existing choice plus a unique key and operator.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Dialogue Choice Condition")) {
                auto next = *runtime.dialogue_draft;
                if (next.removeChoiceCondition(dialogueChoiceSourceId, dialogueChoiceId,
                                               {dialogueConditionKey, dialogueConditionOp, dialogueConditionValue}) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice condition removed as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Enter an existing choice condition to remove it.";
                }
            }

            ImGui::InputText("Choice Effect Key", &dialogueEffectKey);
            ImGui::InputInt("Choice Effect Delta", &dialogueEffectDelta);
            if (ImGui::Button("Add Dialogue Choice Effect")) {
                auto next = *runtime.dialogue_draft;
                if (next.addChoiceEffect(dialogueChoiceSourceId, dialogueChoiceId,
                                         {dialogueEffectKey, dialogueEffectDelta}) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice effect added as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Dialogue effects require an existing choice plus a unique key and delta.";
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Remove Dialogue Choice Effect")) {
                auto next = *runtime.dialogue_draft;
                if (next.removeChoiceEffect(dialogueChoiceSourceId, dialogueChoiceId,
                                            {dialogueEffectKey, dialogueEffectDelta}) &&
                    applyDialogueGraphMutation(runtime, std::move(next))) {
                    runtime.map_save_status = "Dialogue choice effect removed as one undoable project edit.";
                } else {
                    runtime.map_save_status = "Enter an existing choice effect to remove it.";
                }
            }

            auto diagnostics = runtime.dialogue_draft->validate();
            const auto flow_diagnostics = runtime.dialogue_draft->analyzeFlow();
            diagnostics.insert(diagnostics.end(), flow_diagnostics.begin(), flow_diagnostics.end());
            const std::set<std::string> localization_keys(runtime.dialogue_localization_key_options.begin(),
                                                           runtime.dialogue_localization_key_options.end());
            const auto localization_diagnostics = runtime.dialogue_draft->validateLocalizationKeys(localization_keys);
            diagnostics.insert(diagnostics.end(), localization_diagnostics.begin(), localization_diagnostics.end());
            const std::set<std::string> voice_asset_ids(runtime.dialogue_voice_asset_options.begin(),
                                                        runtime.dialogue_voice_asset_options.end());
            const auto voice_diagnostics = runtime.dialogue_draft->validateVoiceAssetIds(voice_asset_ids);
            diagnostics.insert(diagnostics.end(), voice_diagnostics.begin(), voice_diagnostics.end());
            if (diagnostics.empty()) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Dialogue graph validation: ready");
            } else {
                ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.4f, 1.0f), "Dialogue graph validation: %zu issue(s)",
                                   diagnostics.size());
                for (const auto& diagnostic : diagnostics) {
                    ImGui::BulletText("%s: %s", diagnostic.code.c_str(), diagnostic.message.c_str());
                }
            }
            const bool dialogue_saved = !runtime.dirty_state_registry.isDirty(kDialogueDirtyDocumentId);
            ImGui::InputText("Dialogue Runtime Locale Override (optional)", &dialogueRuntimeLocale);
            if (!dialogue_saved || runtime.perspective_2d_scene == nullptr) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Run Saved Dialogue in Native MapScene")) {
                runtime.perspective_2d_scene->setDialogueLocaleCatalog(
                    loadProjectDialogueLocaleCatalog(runtime.project_root, dialogueRuntimeLocale));
                const bool started = runtime.perspective_2d_scene->startAuthoredDialogue(
                    *runtime.dialogue_draft, "editor.dialogue." + runtime.dialogue_draft_id);
                if (started) {
                    runtime.map_save_status = "Saved Dialogue Graph started through the native MapScene message runtime";
                    if (runtime.perspective_2d_scene->dialogueLocaleCode().empty()) {
                        runtime.map_save_status += "; preview text fallback is active.";
                    } else {
                        runtime.map_save_status +=
                            " with locale " + runtime.perspective_2d_scene->dialogueLocaleCode() + ".";
                    }
                } else {
                    runtime.map_save_status = "Saved Dialogue Graph could not start: " +
                                              (runtime.perspective_2d_scene->dialogueRuntimeDiagnostics().empty()
                                                   ? std::string{"runtime admission failed"}
                                                   : runtime.perspective_2d_scene->dialogueRuntimeDiagnostics().front());
                }
            }
            if (!dialogue_saved || runtime.perspective_2d_scene == nullptr) {
                ImGui::EndDisabled();
                ImGui::TextDisabled("Save the Dialogue Graph before running it through the native MapScene runtime.");
            }
        }
        if (ImGui::CollapsingHeader("Interactive Dialogue Condition Preview")) {
            static std::string previewValueKey = "guide_affinity";
            static int previewValue = 0;
            ImGui::TextDisabled("Session-local values and selected choices never modify the saved dialogue graph.");
            ImGui::InputText("Preview Value Key", &previewValueKey);
            ImGui::InputInt("Preview Value", &previewValue);
            if (ImGui::Button("Set Preview Value") && !previewValueKey.empty()) {
                runtime.dialogue_preview_values[previewValueKey] = previewValue;
                runtime.dialogue_preview_diagnostics.clear();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Preview Values")) {
                runtime.dialogue_preview_values.clear();
                runtime.dialogue_preview_diagnostics.clear();
            }
            if (ImGui::Button("Start / Reset Dialogue Preview")) {
                if (!runtime.dialogue_draft.has_value() || runtime.dialogue_draft->startNode().empty() ||
                    runtime.dialogue_draft->findNode(runtime.dialogue_draft->startNode()) == nullptr) {
                    runtime.dialogue_preview_diagnostics = {{"preview_start_node_missing",
                                                            "Select a valid start node before previewing choices.", "", ""}};
                } else {
                    runtime.dialogue_preview_node_id = runtime.dialogue_draft->startNode();
                    runtime.dialogue_preview_trace = {runtime.dialogue_preview_node_id};
                    runtime.dialogue_preview_diagnostics.clear();
                }
            }
            if (!runtime.dialogue_preview_node_id.empty() && runtime.dialogue_draft.has_value()) {
                const auto* preview_node = runtime.dialogue_draft->findNode(runtime.dialogue_preview_node_id);
                if (preview_node == nullptr) {
                    runtime.dialogue_preview_diagnostics = {{"preview_node_missing",
                                                            "The selected preview node is no longer present in the graph.",
                                                            runtime.dialogue_preview_node_id, ""}};
                    runtime.dialogue_preview_node_id.clear();
                } else {
                    ImGui::Text("Preview node: %s", preview_node->id.c_str());
                    ImGui::TextDisabled("%s", preview_node->text_preview.c_str());
                    if (preview_node->ending) {
                        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Ending node reached.");
                    }
                    for (const auto& choice : runtime.dialogue_draft->previewChoices(
                             runtime.dialogue_preview_node_id, runtime.dialogue_preview_values)) {
                        ImGui::PushID(choice.id.c_str());
                        if (!choice.enabled) ImGui::BeginDisabled();
                        const bool selected = ImGui::Button(choice.label.empty() ? choice.id.c_str() : choice.label.c_str());
                        if (!choice.enabled) ImGui::EndDisabled();
                        if (selected && choice.enabled) {
                            if (runtime.dialogue_preview_trace.size() >= 64) {
                                runtime.dialogue_preview_diagnostics = {{"preview_step_limit_reached",
                                                                        "Dialogue preview stopped after 64 selected choices.",
                                                                        runtime.dialogue_preview_node_id, choice.id}};
                            } else {
                                const auto transition = runtime.dialogue_draft->previewChoice(
                                    runtime.dialogue_preview_node_id, choice.id, runtime.dialogue_preview_values);
                                runtime.dialogue_preview_diagnostics = transition.diagnostics;
                                if (transition.applied) {
                                    runtime.dialogue_preview_values = transition.values;
                                    runtime.dialogue_preview_node_id = transition.next_node_id;
                                    runtime.dialogue_preview_trace.push_back(runtime.dialogue_preview_node_id);
                                }
                            }
                        }
                        if (!choice.localization_key.empty()) {
                            ImGui::TextDisabled("Localization key: %s", choice.localization_key.c_str());
                        }
                        for (const auto& diagnostic : choice.diagnostics) {
                            ImGui::SameLine();
                            ImGui::TextDisabled("%s", diagnostic.code.c_str());
                        }
                        ImGui::PopID();
                    }
                }
            }
            if (!runtime.dialogue_preview_trace.empty()) {
                ImGui::Text("Preview trace: %zu node(s)", runtime.dialogue_preview_trace.size());
            }
            for (const auto& [key, value] : runtime.dialogue_preview_values) {
                ImGui::TextDisabled("%s = %d", key.c_str(), value);
            }
            for (const auto& diagnostic : runtime.dialogue_preview_diagnostics) {
                ImGui::TextColored(ImVec4(1.0f, 0.58f, 0.22f, 1.0f), "%s: %s", diagnostic.code.c_str(),
                                   diagnostic.message.c_str());
            }
        }
        if (ImGui::Button("Save Dialogue")) {
            const auto result = runtime.dirty_state_registry.save(kDialogueDirtyDocumentId);
            runtime.map_save_status = result.message;
        }
    }
    if (ImGui::CollapsingHeader("Database and Vendor Authoring")) {
        static std::string itemId = "moonwell_lantern";
        static std::string itemName = "Moonwell Lantern";
        static int itemPrice = 75;
        ImGui::TextUnformatted("Project Item");
        ImGui::InputText("Item ID", &itemId);
        ImGui::InputText("Item Name", &itemName);
        ImGui::InputInt("Item Price", &itemPrice, 1, 10);
        if (ImGui::Button("Upsert Project Item")) {
            if (itemId.empty() || itemName.empty() || itemPrice < 0) {
                runtime.map_save_status = "Project items need a non-empty ID/name and a non-negative price.";
            } else {
                runtime.database_draft.upsertItem({itemId, itemName, itemPrice, {"vendor"}});
                runtime.vendor_draft.setKnownItems(databaseItemIds(runtime.database_draft));
                (void)runtime.dirty_state_registry.markDirty(kDatabaseDirtyDocumentId, true);
                runtime.map_save_status = "Project item updated; vendor stock can now reference it.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Database")) {
            const auto result = runtime.dirty_state_registry.save(kDatabaseDirtyDocumentId);
            runtime.map_save_status = result.message;
        }

        ImGui::Separator();
        static std::string vendorId = "rowan_tonics";
        static int stockQuantity = 1;
        static int buyPrice = 75;
        static int sellPrice = 35;
        ImGui::TextUnformatted("Vendor Stock");
        ImGui::InputText("Vendor ID", &vendorId);
        ImGui::InputInt("Quantity", &stockQuantity, 1, 10);
        ImGui::InputInt("Buy Price", &buyPrice, 1, 10);
        ImGui::InputInt("Sell Price", &sellPrice, 1, 10);
        if (ImGui::Button("Upsert Vendor Stock")) {
            if (!runtime.database_draft.items().contains(itemId)) {
                runtime.map_save_status = "Vendor stock requires an existing project item; add the item first.";
            } else if (!runtime.vendor_draft.upsertStockItem(vendorId, {itemId, stockQuantity, buyPrice, sellPrice, {}})) {
                runtime.map_save_status = "Vendor stock needs non-empty IDs and non-negative quantities/prices.";
            } else {
                runtime.vendor_draft_id = vendorId;
                runtime.vendor_draft.setKnownItems(databaseItemIds(runtime.database_draft));
                (void)runtime.dirty_state_registry.markDirty(kVendorDirtyDocumentId, true);
                runtime.map_save_status = "Vendor stock updated against the project database item.";
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Preview Vendor")) {
            const auto diagnostics = runtime.vendor_draft.validate();
            const auto stock = runtime.vendor_draft.refreshStock(vendorId, {});
            runtime.map_save_status = diagnostics.empty()
                                          ? "Vendor preview completed: " + std::to_string(stock.size()) + " visible stock row(s)."
                                          : "Vendor preview has database-reference diagnostics.";
        }
        ImGui::SameLine();
        if (ImGui::Button("Save Vendor")) {
            const auto result = runtime.dirty_state_registry.save(kVendorDirtyDocumentId);
            runtime.map_save_status = result.message;
        }
    }
    if (ImGui::CollapsingHeader("Map Layout")) {
        auto layout = snapshot.layout;
        bool changed = false;
        changed |= ImGui::Checkbox("Show Palette / Library", &layout.paletteVisible);
        changed |= ImGui::Checkbox("Show Inspector", &layout.inspectorVisible);
        changed |= ImGui::Checkbox("Show Diagnostics", &layout.diagnosticsVisible);
        changed |= ImGui::SliderFloat("Palette Width", &layout.paletteWidthFraction, 0.12f, 0.35f, "%.0f%%");
        changed |= ImGui::SliderFloat("Inspector Width", &layout.inspectorWidthFraction, 0.12f, 0.35f, "%.0f%%");
        changed |= ImGui::SliderFloat("Diagnostics Height", &layout.diagnosticsHeightFraction, 0.12f, 0.40f, "%.0f%%");
        if (changed) {
            workspace.setLayout(layout);
        }
        ImGui::TextDisabled("Map pane preferences are stored in local editor settings.");
    }
    ImGui::Separator();
    ImGui::Text("Mode");
    const auto activateMapModeById = [&](const std::string& modeId) {
        if (modeId == "canvas") return workspace.activateMode(urpg::editor::MapAuthoringMode::Canvas);
        if (modeId == "tiles") return workspace.activateMode(urpg::editor::MapAuthoringMode::Tiles);
        if (modeId == "parts") return workspace.activateMode(urpg::editor::MapAuthoringMode::Parts);
        if (modeId == "props") return workspace.activateMode(urpg::editor::MapAuthoringMode::Props);
        if (modeId == "events") return workspace.activateMode(urpg::editor::MapAuthoringMode::Events);
        if (modeId == "abilities") return workspace.activateMode(urpg::editor::MapAuthoringMode::Abilities);
        if (modeId == "world") return workspace.activateMode(urpg::editor::MapAuthoringMode::World);
        if (modeId == "validate") return workspace.activateMode(urpg::editor::MapAuthoringMode::Validate);
        if (modeId == "playtest") return workspace.activateMode(urpg::editor::MapAuthoringMode::Playtest);
        if (modeId == "package") return workspace.activateMode(urpg::editor::MapAuthoringMode::Package);
        return false;
    };
    const auto& io = ImGui::GetIO();
    static constexpr std::array<ImGuiKey, 10> kMapModeShortcutKeys = {
        ImGuiKey_1, ImGuiKey_2, ImGuiKey_3, ImGuiKey_4, ImGuiKey_5,
        ImGuiKey_6, ImGuiKey_7, ImGuiKey_8, ImGuiKey_9, ImGuiKey_0,
    };
    if (!io.WantTextInput && io.KeyAlt && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
        for (size_t index = 0; index < snapshot.modes.size() && index < kMapModeShortcutKeys.size(); ++index) {
            if (snapshot.modes[index].available && ImGui::IsKeyPressed(kMapModeShortcutKeys[index], false) &&
                activateMapModeById(snapshot.modes[index].id)) {
                workspace.clearNextActionHint();
                runtime.map_save_status = "Map mode activated with Alt+" +
                                          std::to_string(index == 9 ? 0 : static_cast<int>(index + 1)) + ".";
            }
        }
    }
    ImGui::TextDisabled("Keyboard: Alt+1 through Alt+0 select Canvas through Package.");
    for (const auto& mode : snapshot.modes) {
        ImGui::PushID(mode.id.c_str());
        if (!mode.available) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(mode.label.c_str(), ImVec2(94.0f, 0.0f))) {
            if (activateMapModeById(mode.id)) workspace.clearNextActionHint();
        }
        if (!mode.available) {
            ImGui::EndDisabled();
        }
        if (mode.active) {
            ImGui::SameLine();
            ImGui::TextDisabled("active");
        }
        if ((&mode - snapshot.modes.data()) % 3 != 2) ImGui::SameLine();
        ImGui::PopID();
    }
    ImGui::Separator();
    ImGui::Text("Shared state: %s history; %s", snapshot.context.canUndo ? "undo available" : "no undo",
                snapshot.context.gridPartsDirty || snapshot.context.perspective2DDirty ? "unsaved map changes" : "saved");
    const auto saveMap = [&] {
        const auto result = runtime.dirty_state_registry.save(kMapDirtyDocumentId);
        runtime.map_save_status = result.success ? result.message : "Map save failed: " + result.message;
    };
    const auto saveAll = [&] {
        const auto result = runtime.dirty_state_registry.resolveNavigation(urpg::editor::EditorNavigationDecision::Save);
        runtime.map_save_status = result.allowed ? "All registered map documents saved."
                                                  : "Save All failed: " + result.diagnostic.message;
    };
    if (!io.WantTextInput && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        if (io.KeyShift) saveAll();
        else saveMap();
    }
    if (!io.WantTextInput && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z, false)) {
        const auto result = workspace.undo();
        runtime.map_save_status = result.success ? "Undo applied to " + result.owner + "." : result.message;
    }
    if (!io.WantTextInput && io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y, false)) {
        const auto result = workspace.redo();
        runtime.map_save_status = result.success ? "Redo applied to " + result.owner + "." : result.message;
    }
    const auto startPlaytest = [&] { (void)startCurrentMapPlaytest(runtime); };
    const auto startSelectedPartPlaytest = [&] { (void)startCurrentMapPlaytest(runtime, true); };
    const bool hasSelectedPartPlaytestTarget =
        runtime.level_builder_document.findPart(levelSnapshot.inspector.selected_instance_id) != nullptr;
    if (!io.WantTextInput && ImGui::IsKeyPressed(ImGuiKey_F5, false)) {
        if (io.KeyShift) {
            startPlaytest();
        } else if (runtime.playtest_session.isActive()) {
            runtime.playtest_session.returnToEditor();
            runtime.map_save_status = runtime.playtest_session.message();
        } else {
            startPlaytest();
        }
    }
    if (ImGui::Button("Save Map")) {
        saveMap();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save All")) {
        saveAll();
    }
    ImGui::SameLine();
    if (runtime.playtest_session.isActive()) {
        if (ImGui::Button("Return to Editor")) {
            runtime.playtest_session.returnToEditor();
            runtime.map_save_status = runtime.playtest_session.message();
        }
        ImGui::SameLine();
        if (ImGui::Button("Restart Playtest")) {
            startPlaytest();
        }
    } else {
        if (ImGui::Button("Playtest Current Map")) {
            startPlaytest();
        }
        ImGui::SameLine();
        if (hasSelectedPartPlaytestTarget) {
            if (ImGui::Button("Playtest From Selected Part")) {
                startSelectedPartPlaytest();
            }
        } else {
            ImGui::TextDisabled("Select a part to playtest from it");
        }
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Ctrl+S Save  |  Ctrl+Shift+S Save All  |  Ctrl+Z/Y Undo/Redo  |  F5 Play/Stop  |  Shift+F5 Restart");
    if (!runtime.map_save_status.empty()) ImGui::TextWrapped("%s", runtime.map_save_status.c_str());
    if (!runtime.playtest_session.sessionDirectory().empty()) {
        ImGui::TextDisabled("Playtest %s | %s @ %s | %llds | exit %d | overlay %s", playtestStateLabel,
                            runtime.playtest_session.mapId().c_str(), runtime.playtest_session.spawn().c_str(),
                            static_cast<long long>(runtime.playtest_session.elapsed().count()), runtime.playtest_session.exitCode(),
                            runtime.playtest_session.sessionDirectory().generic_string().c_str());
        if (ImGui::SmallButton("Write Redacted Playtest Support Summary")) {
            const auto supportBundle = runtime.playtest_session.writeRedactedSupportBundle();
            runtime.map_save_status = supportBundle.message;
        }
        ImGui::SameLine();
        ImGui::TextDisabled("Excludes paths, process output, diagnostic messages, source paths, and object IDs.");
    }
    for (size_t runtimeDiagnosticIndex = 0;
         runtimeDiagnosticIndex < runtime.playtest_session.diagnostics().size(); ++runtimeDiagnosticIndex) {
        const auto& diagnostic = runtime.playtest_session.diagnostics()[runtimeDiagnosticIndex];
        ImGui::PushID(static_cast<int>(runtimeDiagnosticIndex));
        ImGui::TextWrapped("Runtime %s: %s", diagnostic.code.c_str(), diagnostic.message.c_str());
        if (!diagnostic.object_id.empty()) {
            const auto matchingDiagnostic = std::find_if(
                levelSnapshot.diagnostics.begin(), levelSnapshot.diagnostics.end(), [&](const auto& candidate) {
                    return candidate.instance_id == diagnostic.object_id;
                });
            if (matchingDiagnostic != levelSnapshot.diagnostics.end()) {
                ImGui::SameLine();
                if (ImGui::SmallButton("Focus Map Object")) {
                    const auto index = static_cast<size_t>(
                        std::distance(levelSnapshot.diagnostics.begin(), matchingDiagnostic));
                    const bool focused = workspace.focusGridDiagnostic(index);
                    runtime.map_save_status = focused ? "Focused the Map object reported by the runtime."
                                                      : "The runtime-reported Map object could not be focused.";
                }
            }
        }
        ImGui::PopID();
    }

    const auto renderMapDiagnostics = [&] {
        if (!snapshot.layout.diagnosticsVisible ||
            !ImGui::CollapsingHeader("Map Diagnostics", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }
        if (levelSnapshot.diagnostics.empty() && perspectiveSnapshot.perspective_2d_project.diagnostics.empty()) {
            ImGui::TextDisabled("No Map diagnostics are currently reported.");
        }
        for (size_t index = 0; index < levelSnapshot.diagnostics.size(); ++index) {
            const auto& diagnostic = levelSnapshot.diagnostics[index];
            ImGui::PushID(static_cast<int>(index));
            const auto label = diagnostic.code + ": " + diagnostic.message;
            if (ImGui::Button("Focus")) {
                runtime.map_save_status = workspace.focusGridDiagnostic(index)
                                              ? "Focused Grid Parts diagnostic target."
                                              : "This diagnostic does not have a focusable Grid Parts target.";
            }
            ImGui::SameLine();
            ImGui::TextWrapped("%s", label.c_str());
            ImGui::PopID();
        }
        for (const auto& diagnostic : perspectiveSnapshot.perspective_2d_project.diagnostics) {
            ImGui::BulletText("Perspective 2D: %s", diagnostic.c_str());
        }
    };

    ImGui::Separator();
    ImGui::TextDisabled("Drop attached project assets on the Map canvas. Tiles, Props, and image Events place immediately.");
    if (!runtime.map_asset_drop_status.empty()) ImGui::TextWrapped("%s", runtime.map_asset_drop_status.c_str());

    const auto layout = snapshot.layout;
    const float availableWidth = ImGui::GetContentRegionAvail().x;
    const float canvasHeight = std::max(180.0f, ImGui::GetContentRegionAvail().y *
                                                    (layout.diagnosticsVisible ? 1.0f - layout.diagnosticsHeightFraction
                                                                               : 1.0f));
    const float paletteWidth = layout.paletteVisible ? availableWidth * layout.paletteWidthFraction : 1.0f;
    const float inspectorWidth = layout.inspectorVisible ? availableWidth * layout.inspectorWidthFraction : 1.0f;
    if (ImGui::BeginTable("MapAuthoringCreatorLayout", 3,
                          ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp)) {
        ImGui::TableSetupColumn("Palette / Library", ImGuiTableColumnFlags_WidthFixed, paletteWidth);
        ImGui::TableSetupColumn("Canvas", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthFixed, inspectorWidth);
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        if (layout.paletteVisible) {
            ImGui::TextUnformatted("Palette / Library");
            int visibleEntries = 0;
            for (const auto& entry : levelSnapshot.palette.entries) {
                if (visibleEntries++ == 8) {
                    ImGui::TextDisabled("More parts are available in Parts mode.");
                    break;
                }
                if (ImGui::Selectable(entry.display_name.c_str(), entry.selected)) {
                    (void)runtime.level_builder_workspace.SelectGridPart(entry.part_id);
                }
            }
            if (visibleEntries == 0) ImGui::TextDisabled("No map palette entries are available.");
        } else {
            ImGui::TextDisabled("Palette hidden");
        }
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted("Canvas");
        const auto canvasOrigin = ImGui::GetCursorScreenPos();
        ImGui::BeginChild("MapAuthoringCanvas", ImVec2(0.0f, canvasHeight), true);
        // Keep the established child renderers as the source-of-truth canvas;
        // the shared workspace determines which deep editor is foregrounded.
        if (snapshot.activeMode == "parts" || snapshot.activeMode == "validate" ||
            snapshot.activeMode == "playtest" || snapshot.activeMode == "package") {
            renderLevelBuilderWorkspace(runtime);
        } else {
            renderPerspectiveWorkspace(editorShell, runtime);
        }
        if (ImGui::BeginDragDropTarget()) {
            if (const auto* drag = ImGui::AcceptDragDropPayload("URPG_EDITOR_ASSET_V1")) {
                const auto* begin = static_cast<const std::uint8_t*>(drag->Data);
                std::vector<std::uint8_t> bytes(begin, begin + drag->DataSize);
                urpg::editor::EditorAssetDragPayload asset;
                const auto parsed = urpg::editor::deserializeEditorAssetDragPayload(bytes, &asset);
                const auto mouse = ImGui::GetMousePos();
                const auto decision = parsed.accepted
                                          ? workspace.placeAssetDrop(asset, snapshot.activeMode, mouse.x - canvasOrigin.x,
                                                                     mouse.y - canvasOrigin.y)
                                          : parsed;
                runtime.map_asset_drop_status = decision.accepted
                                                    ? decision.message
                                                    : decision.message +
                                                          (decision.remediation.empty() ? "" : " " + decision.remediation);
            }
            ImGui::EndDragDropTarget();
        }
        ImGui::EndChild();
        ImGui::TableSetColumnIndex(2);
        if (layout.inspectorVisible) {
            ImGui::TextUnformatted("Inspector");
            ImGui::TextWrapped("Layer: %s", snapshot.context.selection.layerId.empty()
                                             ? "(none)" : snapshot.context.selection.layerId.c_str());
            ImGui::TextWrapped("Part: %s", snapshot.context.selection.partId.empty()
                                            ? "(none)" : snapshot.context.selection.partId.c_str());
            ImGui::TextWrapped("Object: %s", snapshot.context.selection.objectId.empty()
                                              ? "(none)" : snapshot.context.selection.objectId.c_str());
            ImGui::TextWrapped("Event: %s", snapshot.context.selection.eventId.empty()
                                             ? "(none)" : snapshot.context.selection.eventId.c_str());
            ImGui::Separator();
            ImGui::TextWrapped("Current tool: %s", snapshot.context.selection.activeTool.c_str());
        } else {
            ImGui::TextDisabled("Inspector hidden");
        }
        ImGui::EndTable();
    }
    renderMapDiagnostics();
}

void renderAbilityWorkspaceInline(EditorPanelRuntime& runtime) {
    auto& panel = runtime.ability_inspector_panel;
    panel.update(runtime.ability_runtime);
    const auto& snapshot = panel.getRenderSnapshot();

    ImGui::Text("Active Tags");
    const auto& tags = panel.getModel().getActiveTags();
    if (tags.empty()) {
        ImGui::TextDisabled("None");
    } else {
        for (const auto& tagInfo : tags) {
            ImGui::BulletText("%s (%d stack%s)", tagInfo.tag.c_str(), tagInfo.count, tagInfo.count == 1 ? "" : "s");
        }
    }

    ImGui::Separator();
    ImGui::Text("Abilities");
    const auto& abilities = panel.getModel().getAbilities();
    if (abilities.empty()) {
        ImGui::TextDisabled("No abilities are bound to this runtime.");
    } else {
        if (ImGui::BeginTable("AbilityInspectorAbilitiesInline", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Ability");
            ImGui::TableSetupColumn("Status");
            ImGui::TableSetupColumn("Cooldown");
            ImGui::TableSetupColumn("Blocking Reason");
            ImGui::TableHeadersRow();

            const auto selected_index = panel.getModel().selectedAbilityIndex();
            for (size_t index = 0; index < abilities.size(); ++index) {
                const auto& info = abilities[index];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                const bool selected = selected_index.has_value() && *selected_index == index;
                if (ImGui::Selectable(info.name.c_str(), selected, ImGuiSelectableFlags_SpanAllColumns)) {
                    panel.selectAbility(index, runtime.ability_runtime);
                }
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.can_activate ? "Ready" : "Blocked");
                ImGui::TableNextColumn();
                ImGui::Text("%.2fs", info.cooldown_remaining);
                ImGui::TableNextColumn();
                ImGui::TextUnformatted(info.blocking_reason.empty() ? "-" : info.blocking_reason.c_str());
            }
            ImGui::EndTable();
        }
    }

    ImGui::Separator();
    ImGui::Text("Actions");
    if (ImGui::Button("Preview Selected")) {
        panel.previewSelectedAbility(runtime.ability_runtime);
    }
    ImGui::SameLine();
    if (ImGui::Button("Apply Draft")) {
        panel.applyDraftToRuntime(runtime.ability_runtime);
        panel.update(runtime.ability_runtime);
    }

    if (!snapshot.diagnostic_lines.empty()) {
        ImGui::Separator();
        ImGui::Text("Diagnostics");
        for (const auto& line : snapshot.diagnostic_lines) {
            ImGui::BulletText("%s", line.c_str());
        }
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Gameplay Recipe Gallery");
    ImGui::TextDisabled("Typed native templates only. Apply and revert changes use the project save and recovery flow.");
    if (!runtime.gameplay_recipe_load_status.empty()) {
        ImGui::TextWrapped("Project recipe document was not loaded: %s", runtime.gameplay_recipe_load_status.c_str());
    }
    auto& recipePanel = runtime.gameplay_recipe_panel;
    recipePanel.render();
    const auto& recipeSnapshot = recipePanel.snapshot();
    if (!recipeSnapshot.has_recipe) {
        ImGui::TextDisabled("No native gameplay recipe template is selected.");
        return;
    }

    const auto selectedTemplate = std::find_if(runtime.gameplay_recipe_templates.begin(),
                                               runtime.gameplay_recipe_templates.end(),
                                               [&recipeSnapshot](const auto& templateRecipe) {
                                                   return templateRecipe.id == recipeSnapshot.recipe_id;
                                               });
    const auto selectedTemplateLabel = selectedTemplate == runtime.gameplay_recipe_templates.end()
                                           ? recipeSnapshot.recipe_id
                                           : selectedTemplate->display_name;
    if (ImGui::BeginCombo("Native template", selectedTemplateLabel.c_str())) {
        for (const auto& templateRecipe : runtime.gameplay_recipe_templates) {
            const bool selected = templateRecipe.id == recipeSnapshot.recipe_id;
            const auto label = templateRecipe.display_name + "##" + templateRecipe.id;
            if (ImGui::Selectable(label.c_str(), selected)) {
                recipePanel.selectRecipe(templateRecipe);
            }
            if (selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::Text("%s (v%s)", recipeSnapshot.recipe_id.c_str(), recipeSnapshot.recipe_version.c_str());
    const auto recipeParameters = recipeSnapshot.parameters;
    for (const auto& parameter : recipeParameters) {
        const auto key = parameter.value("key", std::string{"parameter"});
        const auto label = parameter.value("label", key) + "##" + key;
        if (parameter.value("kind", "string") == "integer") {
            auto value = parameter.value("value", int32_t{0});
            if (ImGui::InputInt(label.c_str(), &value)) {
                (void)recipePanel.setSelectedIntegerParameter(key, value);
            }
            if (parameter.value("has_range", false)) {
                ImGui::TextDisabled("Allowed range: %d to %d", parameter.value("minimum", value),
                                    parameter.value("maximum", value));
            }
        } else {
            auto value = parameter.value("value", std::string{});
            if (ImGui::InputText(label.c_str(), &value)) {
                (void)recipePanel.setSelectedStringParameter(key, std::move(value));
            }
        }
    }
    const auto runtimePreview = recipeSnapshot.preview.value("runtime_preview", nlohmann::json::object());
    ImGui::TextDisabled("Target: %s", runtimePreview.value("feature_id", "(none)").c_str());
    ImGui::TextDisabled("Preview rules: %zu", runtimePreview.value("active_rules", nlohmann::json::array()).size());
    ImGui::TextDisabled("Status: %s", recipeSnapshot.status.c_str());

    if (!recipeSnapshot.can_apply) ImGui::BeginDisabled();
    if (ImGui::Button("Apply Native Recipe")) {
        (void)recipePanel.applySelectedRecipe();
    }
    if (!recipeSnapshot.can_apply) ImGui::EndDisabled();
    ImGui::SameLine();
    if (!recipeSnapshot.can_revert) ImGui::BeginDisabled();
    if (ImGui::Button("Revert Recipe-Owned Target")) {
        (void)recipePanel.revertSelectedRecipe();
    }
    if (!recipeSnapshot.can_revert) ImGui::EndDisabled();
    ImGui::SameLine();
    const bool recipeDocumentDirty = recipePanel.hasUnsavedProjectDocument();
    if (!recipeDocumentDirty) ImGui::BeginDisabled();
    if (ImGui::Button("Save Recipe Document")) {
        const auto result = runtime.dirty_state_registry.save(kGameplayRecipeDirtyDocumentId);
        runtime.gameplay_recipe_save_status = result.message;
    }
    if (!recipeDocumentDirty) ImGui::EndDisabled();
    if (!runtime.gameplay_recipe_save_status.empty()) {
        ImGui::TextDisabled("Save: %s", runtime.gameplay_recipe_save_status.c_str());
    }

    const auto diagnostics = recipeSnapshot.preview.value("diagnostics", nlohmann::json::array());
    if (!diagnostics.empty()) {
        ImGui::TextUnformatted("Recipe diagnostics");
        for (const auto& diagnostic : diagnostics) {
            ImGui::BulletText("%s: %s", diagnostic.value("code", "diagnostic").c_str(),
                              diagnostic.value("message", "").c_str());
        }
    }
}

void renderCharacterCreatorWorkspaceInline(EditorPanelRuntime& runtime) {
    auto& model = runtime.character_creator_model;
    auto name = model.getIdentity().getName();
    auto classId = model.getIdentity().getClassId();
    auto portraitId = model.getIdentity().getPortraitId();
    auto bodySpriteId = model.getIdentity().getBodySpriteId();

    ImGui::TextUnformatted("Character Identity");
    ImGui::TextDisabled("Changes are protected by the shared editor save and recovery flow.");
    if (ImGui::InputText("Name", &name)) model.setName(name);
    if (ImGui::InputText("Class ID", &classId)) model.setClassId(classId);
    if (ImGui::InputText("Portrait ID", &portraitId)) model.setPortraitId(portraitId);
    if (ImGui::InputText("Body Sprite ID", &bodySpriteId)) model.setBodySpriteId(bodySpriteId);

    ImGui::Separator();
    const auto snapshot = model.buildSnapshot();
    const auto validation = snapshot.value("validation", nlohmann::json::object());
    const bool valid = validation.value("is_valid", false);
    ImGui::Text("Validation: %s", valid ? "ready" : "needs attention");
    for (const auto& issue : validation.value("issues", nlohmann::json::array())) {
        ImGui::BulletText("%s", issue.value("message", "Invalid character field.").c_str());
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Governed Appearance Asset");
    static int appearanceSlot = 0;
    static constexpr const char* appearanceSlots[] = {"portrait", "field", "battle", "layer"};
    ImGui::Combo("Drop target", &appearanceSlot, appearanceSlots, IM_ARRAYSIZE(appearanceSlots));
    ImGui::BeginChild("CharacterAppearanceAssetDrop", ImVec2(0.0f, 58.0f), true);
    ImGui::TextUnformatted("Drop an attached image asset here");
    if (ImGui::BeginDragDropTarget()) {
        if (const auto* drag = ImGui::AcceptDragDropPayload("URPG_EDITOR_ASSET_V1")) {
            const auto* begin = static_cast<const std::uint8_t*>(drag->Data);
            std::vector<std::uint8_t> bytes(begin, begin + drag->DataSize);
            urpg::editor::EditorAssetDragPayload asset;
            const auto parsed = urpg::editor::deserializeEditorAssetDragPayload(bytes, &asset);
            auto accepted = parsed.accepted && asset.mediaKind == "image"
                                ? urpg::editor::assessEditorAssetDrop(asset, true)
                                : urpg::editor::EditorAssetDropDecision{
                                      false, "character_asset_drop_requires_image",
                                      "Character Creator accepts attached image assets only.",
                                      "Attach an image in Assets, then drag it here."};
            if (accepted.accepted) {
                accepted = urpg::editor::validateEditorAssetAttachmentRevision(asset, runtime.project_root);
            }
            if (!accepted.accepted) {
                runtime.map_asset_drop_status = accepted.message +
                                                (accepted.remediation.empty() ? "" : " " + accepted.remediation);
            } else if (model.assignAttachedAppearanceAsset(asset.assetId, appearanceSlots[appearanceSlot])) {
                runtime.map_asset_drop_status = "Attached appearance asset assigned as one undoable Character Creator action.";
            } else {
                runtime.map_asset_drop_status = "Character appearance asset assignment made no change.";
            }
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::EndChild();
    if (ImGui::Button("Undo Appearance Assignment")) (void)model.undoAppearanceAssetAssignment();
    ImGui::SameLine();
    if (ImGui::Button("Redo Appearance Assignment")) (void)model.redoAppearanceAssetAssignment();
    if (!runtime.map_asset_drop_status.empty()) ImGui::TextWrapped("%s", runtime.map_asset_drop_status.c_str());

    ImGui::Separator();
    if (ImGui::Button("Save Character")) {
        const auto result = runtime.dirty_state_registry.save(kCharacterDirtyDocumentId);
        runtime.map_save_status = result.message;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Draft: %s", runtime.character_draft_id.c_str());
    if (!runtime.map_save_status.empty()) ImGui::TextWrapped("%s", runtime.map_save_status.c_str());
}

void renderPatternsWorkspaceInline(EditorPanelRuntime& runtime) {
    auto& panel = runtime.pattern_field_panel;
    auto& currentModel = runtime.pattern_field_model;
    const auto& snapshot = panel.getRenderSnapshot();

    std::string name = snapshot.name;
    if (ImGui::InputText("Name", &name)) {
        panel.setPatternName(name);
    }

    int viewport = snapshot.viewport_size;
    if (ImGui::InputInt("Viewport", &viewport)) {
        panel.resizeViewport(viewport);
    }

    if (ImGui::Button("Clear")) {
        panel.clearPattern();
    }

    const auto presets = currentModel.availablePresets();
    if (!presets.empty()) {
        ImGui::Separator();
        ImGui::Text("Presets");
        for (const auto& preset : presets) {
            ImGui::PushID(preset.id.c_str());
            if (ImGui::Button(preset.display_name.c_str())) {
                panel.applyPreset(preset.id);
            }
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::Separator();
    ImGui::Text("Grid");
    const auto bounds = currentModel.getViewportBounds();
    for (int32_t y = bounds.minY; y <= bounds.maxY; ++y) {
        for (int32_t x = bounds.minX; x <= bounds.maxX; ++x) {
            ImGui::PushID(static_cast<int>((y - bounds.minY) * snapshot.viewport_size + (x - bounds.minX)));
            const bool selected = currentModel.isPointSelected(x, y);
            const char* label = (x == 0 && y == 0) ? (selected ? "[O]" : "[.]") : (selected ? "[X]" : "[ ]");
            if (ImGui::Button(label, ImVec2(36.0f, 28.0f))) {
                panel.togglePoint(x, y);
            }
            ImGui::PopID();
            if (x < bounds.maxX) {
                ImGui::SameLine();
            }
        }
    }
}

void renderAnalyticsWorkspaceInline(urpg::editor::AnalyticsPanel& panel) {
    panel.refreshSnapshot();
    const auto snapshot = panel.lastRenderSnapshot();
    ImGui::Text("Session ID: %s", snapshot.value("sessionId", "unknown").c_str());
    ImGui::Text("Consent State: %s", snapshot.value("privacyStatus", "unknown").c_str());

    bool optIn = snapshot.value("optIn", false);
    if (ImGui::Checkbox("Opt In", &optIn)) {
        panel.setOptIn(optIn);
        panel.refreshSnapshot();
    }

    ImGui::Text("Queue size: %zu", snapshot.value("queuedEventCount", size_t{0}));
    if (ImGui::Button("Clear Queue")) {
        panel.clearQueuedEvents();
        panel.refreshSnapshot();
    }
    ImGui::SameLine();
    if (ImGui::Button("Flush Upload")) {
        panel.flushQueuedEvents();
        panel.refreshSnapshot();
    }
}

void renderEditorWorkspace(urpg::editor::EditorShell& editorShell, EditorPanelRuntime& runtime) {
    const auto snapshot = editorShell.snapshot();
    if (runtime.last_workspace_panel_id != snapshot.active_panel_id) {
        runtime.last_workspace_panel_id = snapshot.active_panel_id;
        runtime.focus_workspace_next_frame = true;
        if (snapshot.active_panel_id == "level_builder") {
            (void)runtime.map_authoring_workspace.activateMode(urpg::editor::MapAuthoringMode::Parts);
        } else if (snapshot.active_panel_id == "spatial_authoring") {
            (void)runtime.map_authoring_workspace.activateMode(urpg::editor::MapAuthoringMode::Canvas);
        }
    }
    ImGui::SetNextWindowBgAlpha(1.0f);
    const bool isMapWorkspace = snapshot.active_panel_id == "level_builder" ||
                                snapshot.active_panel_id == "spatial_authoring";
    if (isMapWorkspace) {
        const auto& display = ImGui::GetIO().DisplaySize;
        ImGui::SetNextWindowPos(ImVec2(12.0f, 12.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(std::max(480.0f, display.x - 24.0f),
                                        std::max(480.0f, display.y - 24.0f)),
                                 ImGuiCond_Always);
    } else {
        ImGui::SetNextWindowPos(ImVec2(370.0f, 12.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(560.0f, 580.0f), ImGuiCond_Always);
    }
    if (runtime.focus_workspace_next_frame) {
        ImGui::SetNextWindowFocus();
        runtime.focus_workspace_next_frame = false;
    }
    const std::string title = std::string("URPG Workspace - ") + workspaceTitleForPanelId(snapshot) +
                              "###URPG Workspace";
    if (!ImGui::Begin(title.c_str(), nullptr, ImGuiWindowFlags_NoCollapse)) {
        ImGui::End();
        return;
    }

    ImGui::Text("Panel: %s", snapshot.active_panel_id.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Return to Main Menu")) {
        if (runtime.dirty_state_registry.dirtyDocumentIds().empty()) {
            const auto closed = runtime.project_session.closeProject();
            runtime.project_session_status = closed.message;
            runtime.creator_mode = closed.success;
            runtime.main_menu_model.returnToMainMenu();
            runtime.creator_checklist_panel.setVisible(false);
        } else {
            ImGui::OpenPopup("Unsaved Project Work");
        }
    }
    if (ImGui::BeginPopupModal("Unsaved Project Work", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("This project has unsaved work. Choose how to continue before returning to the main menu.");
        const auto closeProject = [&](urpg::editor::EditorNavigationDecision decision) {
            const auto guard = runtime.dirty_state_registry.resolveNavigation(decision);
            if (!guard.allowed) {
                runtime.project_session_status = "Project remains open: " + guard.diagnostic.message;
                return;
            }
            const auto closed = runtime.project_session.closeProject();
            runtime.project_session_status = closed.success ? closed.message : "Project close failed: " + closed.message;
            if (closed.success) {
                runtime.creator_mode = true;
                runtime.main_menu_model.returnToMainMenu();
                runtime.creator_checklist_panel.setVisible(false);
                ImGui::CloseCurrentPopup();
            }
        };
        if (ImGui::Button("Save All and Return")) {
            closeProject(urpg::editor::EditorNavigationDecision::Save);
        }
        ImGui::SameLine();
        if (ImGui::Button("Discard and Return")) {
            closeProject(urpg::editor::EditorNavigationDecision::Discard);
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            runtime.project_session_status = "Project close cancelled; unsaved work remains open.";
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    if (!runtime.project_session_status.empty()) {
        ImGui::TextDisabled("%s", runtime.project_session_status.c_str());
    }
    if (!runtime.recovery_status.empty()) {
        ImGui::TextDisabled("Recovery: %s", runtime.recovery_status.c_str());
    }
    if (runtime.project_session.isOpen()) {
        const auto recoverySnapshots = runtime.recovery_service.listSnapshots(runtime.project_root);
        if (!recoverySnapshots.empty()) {
            ImGui::TextDisabled("Recovery snapshots: %zu private snapshot%s available", recoverySnapshots.size(),
                                recoverySnapshots.size() == 1 ? "" : "s");
            ImGui::SameLine();
            if (ImGui::SmallButton("Restore Latest to New Folder")) {
                const auto stamp = std::chrono::duration_cast<std::chrono::seconds>(
                                       std::chrono::system_clock::now().time_since_epoch())
                                       .count();
                const auto destination = runtime.project_root.parent_path() /
                                         (runtime.project_root.filename().string() + "_recovered_" +
                                          std::to_string(stamp));
                if (runtime.recovery_service.restoreRecoverySnapshot(recoverySnapshots.front().path, destination)) {
                    runtime.restored_recovery_project_path = destination;
                    runtime.recovery_status = "Recovery restored safely to " + destination.generic_string() +
                                              ". Review it, then open it as a project when ready.";
                } else {
                    runtime.recovery_status = "Recovery restore failed; the open project and snapshot were left unchanged.";
                }
            }
        }
        if (!runtime.restored_recovery_project_path.empty()) {
            const auto hasUnsavedWork = !runtime.dirty_state_registry.dirtyDocumentIds().empty();
            if (hasUnsavedWork) ImGui::BeginDisabled();
            if (ImGui::SmallButton("Open Recovered Project")) {
                const auto opened = runtime.project_session.openProject(runtime.restored_recovery_project_path);
                if (opened.success) {
                    editorShell.setProjectRoot(runtime.project_session.activeProject().root);
                    runtime.main_menu_model.setLastProject(runtime.project_session.activeProject().root.generic_string());
                    runtime.main_menu_model.addRecentProject(runtime.project_session.activeProject().root.generic_string());
                    runtime.recovery_status = "Opened the recovered project in a separate editor session.";
                    runtime.restored_recovery_project_path.clear();
                } else {
                    runtime.recovery_status = "Recovered project could not be opened: " + opened.message;
                }
            }
            if (hasUnsavedWork) {
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                    ImGui::SetTooltip("Save or discard current project work before opening the recovered project.");
                }
                ImGui::EndDisabled();
            }
        }
    }
    ImGui::Separator();

    if (snapshot.active_panel_id == "diagnostics") {
        renderDiagnosticsWorkspace(runtime);
    } else if (snapshot.active_panel_id == "assets") {
        renderAssetWorkspace(runtime);
    } else if (snapshot.active_panel_id == "mod") {
        renderModWorkspace(runtime);
    } else if (snapshot.active_panel_id == "level_builder") {
        renderMapAuthoringWorkspace(editorShell, runtime);
    } else if (snapshot.active_panel_id == "spatial_authoring") {
        renderMapAuthoringWorkspace(editorShell, runtime);
    } else if (snapshot.active_panel_id == "ability") {
        renderAbilityWorkspaceInline(runtime);
    } else if (snapshot.active_panel_id == "character_creator") {
        renderCharacterCreatorWorkspaceInline(runtime);
    } else if (snapshot.active_panel_id == "patterns") {
        renderPatternsWorkspaceInline(runtime);
    } else if (snapshot.active_panel_id == "analytics") {
        renderAnalyticsWorkspaceInline(runtime.analytics_panel);
    } else {
        ImGui::TextWrapped("This panel opens in its own tool window.");
    }

    ImGui::End();
}
#endif

std::string analyticsConsentToSettings(urpg::analytics::ConsentState state) {
    switch (state) {
    case urpg::analytics::ConsentState::Granted:
        return "granted";
    case urpg::analytics::ConsentState::Denied:
        return "denied";
    case urpg::analytics::ConsentState::Unknown:
        return "unknown";
    }
    return "unknown";
}

bool useWorkspaceOnlyRenderer(const urpg::editor::EditorShell& editorShell, const EditorPanelRuntime* panelRuntime) {
    if (panelRuntime == nullptr || editorShell.snapshot().headless) {
        return false;
    }

    const auto& activePanelId = editorShell.activePanelId();
    return activePanelId == "diagnostics" || activePanelId == "assets" || activePanelId == "ability" ||
           activePanelId == "character_creator" ||
           activePanelId == "patterns" || activePanelId == "mod" || activePanelId == "analytics" ||
           activePanelId == "level_builder" || activePanelId == "spatial_authoring";
}

void refreshWorkspaceOnlyPanel(EditorPanelRuntime& runtime, const std::string& activePanelId) {
    if (activePanelId == "diagnostics") {
        runtime.diagnostics_workspace.update();
    } else if (activePanelId == "assets") {
        runtime.asset_library_panel.render();
    } else if (activePanelId == "ability") {
        runtime.ability_inspector_panel.update(runtime.ability_runtime);
    } else if (activePanelId == "character_creator") {
        runtime.character_creator_panel.render();
    } else if (activePanelId == "mod") {
        runtime.mod_manager_panel.render();
    } else if (activePanelId == "analytics") {
        runtime.analytics_panel.refreshSnapshot();
    } else if (activePanelId == "level_builder" || activePanelId == "spatial_authoring") {
        runtime.map_authoring_workspace.refresh();
    }
}

void leaveCreatorModeWhenProjectOpened(urpg::editor::EditorShell& editorShell, EditorPanelRuntime& runtime) {
    if (!runtime.creator_mode || runtime.main_menu_model.route() != "editor") return;
    const auto action = runtime.main_menu_model.snapshot().value("pending_action", nlohmann::json::object());
    const auto projectPath = action.value("projectPath", "");
    const auto opened = runtime.project_session.openProject(projectPath);
    if (!opened.success) {
        runtime.main_menu_model.reportProjectOpenFailure(projectPath, opened.message);
        return;
    }
    runtime.project_root = runtime.project_session.activeProject().root;
    runtime.creator_checklist_panel.setProjectRoot(runtime.project_root);
    runtime.external_asset_library_root = runtime.new_project_wizard.snapshot().value("external_asset_library_root", "");
    runtime.main_menu_model.setExternalAssetLibraryRoot(runtime.external_asset_library_root);
    runtime.creator_mode = false;
    editorShell.setProjectRoot(runtime.project_root);
    (void)editorShell.openPanel("level_builder");
    if (action.value("playtest_starter", false)) {
        (void)startCurrentMapPlaytest(runtime);
    }
    if (action.value("action", "") == "enter_editor") {
        runtime.map_authoring_workspace.setNextActionHint(
            action.value("playtest_starter", false)
                ? "Starter-map playtest is launching with the current private overlay."
                : "Start with Parts to paint the starter map, then choose Playtest when you are ready.");
        runtime.creator_checklist_panel.setVisible(true);
    }
    runtime.focus_workspace_next_frame = true;
}

bool runEditorFrame(urpg::EngineShell& engineShell, urpg::editor::EditorShell& editorShell, bool renderAllPanels,
                    EditorPanelRuntime* panelRuntime = nullptr, double deltaSeconds = 1.0 / 60.0) {
    engineShell.tick();
#ifdef URPG_IMGUI_ENABLED
#ifndef URPG_HEADLESS
    if (!editorShell.snapshot().headless) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
    }
#endif
    ImGui::NewFrame();
#endif
    bool rendered = false;
    if (editorShell.beginFrame(deltaSeconds)) {
#ifdef URPG_IMGUI_ENABLED
        if (!editorShell.snapshot().headless) {
            renderEditorChrome(editorShell, panelRuntime);
        }
#endif
        const bool workspaceOnly = !renderAllPanels && useWorkspaceOnlyRenderer(editorShell, panelRuntime);
        if (renderAllPanels) {
            rendered = editorShell.renderVisiblePanels() > 0;
        } else if (workspaceOnly) {
            refreshWorkspaceOnlyPanel(*panelRuntime, editorShell.activePanelId());
            rendered = true;
        } else {
            rendered = editorShell.renderActivePanel();
        }
#ifdef URPG_IMGUI_ENABLED
        if (panelRuntime != nullptr && !editorShell.snapshot().headless) {
            if (panelRuntime->creator_mode) {
                panelRuntime->main_menu_panel.render();
                leaveCreatorModeWhenProjectOpened(editorShell, *panelRuntime);
            } else {
                renderEditorWorkspace(editorShell, *panelRuntime);
                panelRuntime->creator_checklist_panel.render();
            }
        }
#endif
        if (panelRuntime != nullptr) {
            if (panelRuntime->ability_dirty_surface_registered) {
                (void)panelRuntime->dirty_state_registry.markDirty(
                    "ability.draft", panelRuntime->ability_inspector_panel.hasUnsavedDraft());
            }
            if (panelRuntime->character_dirty_surface_registered) {
                (void)panelRuntime->dirty_state_registry.markDirty(
                    kCharacterDirtyDocumentId, panelRuntime->character_creator_model.hasUnsavedDraft());
            }
            if (panelRuntime->gameplay_recipe_dirty_surface_registered) {
                (void)panelRuntime->dirty_state_registry.markDirty(
                    kGameplayRecipeDirtyDocumentId, panelRuntime->gameplay_recipe_panel.hasUnsavedProjectDocument());
            }
            if (panelRuntime->menu_studio_dirty_surface_registered) {
                const auto menuStudioDirty =
                    menuStudioSerializedGraph(*panelRuntime) != panelRuntime->menu_studio_persisted_json;
                (void)panelRuntime->dirty_state_registry.markDirty(kMenuStudioDirtyDocumentId, menuStudioDirty);
            }
            panelRuntime->project_session.setDirtySurfaceSummaries(panelRuntime->dirty_state_registry.dirtyDocumentIds());
            captureScheduledRecoverySnapshot(*panelRuntime);
        }
        rendered = editorShell.endFrame() && rendered;
    }
#ifdef URPG_IMGUI_ENABLED
    ImGui::Render();
#ifndef URPG_HEADLESS
    if (!editorShell.snapshot().headless) {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (auto* platform = engineShell.getPlatform()) {
            platform->present();
        }
    }
#endif
#endif
    return rendered;
}

nlohmann::json panelSnapshotJson(const urpg::editor::EditorShell& editorShell) {
    nlohmann::json panels = nlohmann::json::array();
    for (const auto& panel : editorShell.panels()) {
        panels.push_back({
            {"id", panel.id},
            {"title", panel.title},
            {"category", panel.category},
            {"visible", panel.visible},
            {"enabled", panel.enabled},
            {"rendered_last_frame", panel.rendered_last_frame},
            {"render_count", panel.render_count},
        });
    }
    return panels;
}

int runSmokeWorkflow(urpg::EngineShell& engineShell, urpg::editor::EditorShell& editorShell,
                     const urpg::cli::EditorCliOptions& options) {
    const auto requiredPanels = urpg::editor::smokeRequiredEditorPanelIds();

    nlohmann::json report = {
        {"schema", "urpg.editor_smoke.v1"},
        {"project_root", options.project_root.generic_string()},
        {"project_root_exists", std::filesystem::is_directory(options.project_root)},
        {"opened_panels", nlohmann::json::array()},
        {"rendered_panels", nlohmann::json::array()},
        {"errors", nlohmann::json::array()},
    };

    if (!std::filesystem::is_directory(options.project_root)) {
        report["errors"].push_back("project_root_missing");
    }

    for (const auto& panelId : requiredPanels) {
        if (!editorShell.hasPanel(panelId)) {
            report["errors"].push_back(std::string("missing_panel:") + panelId);
            continue;
        }
        if (!editorShell.openPanel(panelId)) {
            report["errors"].push_back(std::string("open_panel_failed:") + panelId);
            continue;
        }
        report["opened_panels"].push_back(panelId);
        if (!runEditorFrame(engineShell, editorShell, false)) {
            report["errors"].push_back(std::string("render_panel_failed:") + panelId);
            continue;
        }
        report["rendered_panels"].push_back(panelId);
    }

    const urpg::project::ProjectSnapshotStore snapshotStore;
    std::filesystem::remove_all(options.smoke_snapshot_root / "editor_smoke_project_state");
    const auto snapshot =
        snapshotStore.createSnapshot(options.project_root, options.smoke_snapshot_root, "editor_smoke_project_state");
    report["project_snapshot"] = {
        {"success", snapshot.success},
        {"path", snapshot.snapshot_path.generic_string()},
        {"manifest", snapshot.manifest},
        {"errors", snapshot.errors},
    };
    if (!snapshot.success) {
        report["errors"].push_back("project_snapshot_failed");
    }

    const auto shellSnapshot = editorShell.snapshot();
    report["frame_index"] = shellSnapshot.frame_index;
    report["active_panel_id"] = shellSnapshot.active_panel_id;
    report["runtime_preview_id"] = shellSnapshot.runtime_preview_id;
    report["panels"] = panelSnapshotJson(editorShell);

    std::filesystem::create_directories(options.smoke_output.parent_path());
    std::ofstream out(options.smoke_output, std::ios::binary);
    if (!out) {
        std::cerr << "URPG editor smoke failed to open output '" << options.smoke_output.string() << "'.\n";
        return 1;
    }
    out << report.dump(2) << "\n";

    if (!report["errors"].empty()) {
        std::cerr << "URPG editor smoke failed: " << report["errors"].dump() << "\n";
        return 1;
    }

    std::cout << "URPG editor smoke wrote " << options.smoke_output.string() << "\n";
    return 0;
}

} // namespace

int main(int argc, char** argv) {
    try {
        const auto cli = urpg::cli::parseEditorCli(urpg::cli::argvToViews(argc, argv), defaultHeadless());
        if (!cli.ok()) {
            std::cerr << "URPG editor: " << cli.error << "\n" << urpg::cli::editorHelpText();
            return 2;
        }
        if (cli.action == urpg::cli::CliAction::Help) {
            std::cout << urpg::cli::editorHelpText();
            return 0;
        }
        if (cli.action == urpg::cli::CliAction::Version) {
            printVersion();
            return 0;
        }

        const urpg::cli::EditorCliOptions options = cli.options;
        const auto settingsPaths = urpg::settings::editorUserSettingsPaths();
        auto settingsLoad = urpg::settings::loadEditorSettings(settingsPaths.editor_settings, settingsPaths);
        for (const auto& warning : settingsLoad.report.warnings) {
            std::cerr << "URPG editor settings warning: " << warning << "\n";
        }
        if (options.width_provided) {
            settingsLoad.settings.window.width = options.width;
        }
        if (options.height_provided) {
            settingsLoad.settings.window.height = options.height;
        }

        const bool creatorMode = !options.project_root_provided && !options.smoke &&
                                 (settingsLoad.settings.last_project.empty() ||
                                  !std::filesystem::is_directory(settingsLoad.settings.last_project));
        const auto activeProjectRoot = creatorMode ? settingsPaths.root
                                                   : (options.project_root_provided ? options.project_root
                                                                                    : (settingsLoad.settings.last_project.empty()
                                                                                           ? options.project_root
                                                                                           : std::filesystem::path(settingsLoad.settings.last_project)));
        std::filesystem::create_directories(activeProjectRoot);
        if (const auto startupFailure = urpg::diagnostics::validateStartupInputs(
                "editor", activeProjectRoot, settingsLoad.settings.window.width, settingsLoad.settings.window.height,
                options.headless)) {
            const auto writeResult = urpg::diagnostics::writeStartupDiagnostic(*startupFailure);
            printStartupFailure(*startupFailure, writeResult);
            return 1;
        }

        urpg::WindowConfig config;
        config.title = "URPG Editor";
        config.width = settingsLoad.settings.window.width;
        config.height = settingsLoad.settings.window.height;
        config.fullscreen = settingsLoad.settings.window.fullscreen;
        config.resizable = settingsLoad.settings.window.resizable;

        std::unique_ptr<urpg::IPlatformSurface> surface;
        std::unique_ptr<urpg::RendererBackend> renderer;

        if (options.headless) {
            surface = std::make_unique<urpg::HeadlessSurface>();
            renderer = std::make_unique<urpg::HeadlessRenderer>();
        } else {
#ifdef URPG_HEADLESS
            std::cerr << "URPG editor was built headless; use --headless.\n";
            return 2;
#else
            surface = std::make_unique<urpg::SDLSurface>();
            renderer = std::make_unique<urpg::OpenGLRenderer>();
#endif
        }

        if (!surface->initialize(config)) {
            std::cerr << "URPG editor failed to initialize platform surface.\n";
            printRuntimeDiagnostics();
            return 1;
        }

        auto& engineShell = urpg::EngineShell::getInstance();
        if (!engineShell.startup(std::move(surface), std::move(renderer),
                                 urpg::EngineShell::StartupOptions(activeProjectRoot))) {
            std::cerr << "URPG editor startup failed.\n";
            return 1;
        }
        if (!options.headless) {
            engineShell.getRenderer()->setAutoPresent(false);
        }

        clearSceneStack();
        if (options.headless) {
            auto editorPreview = std::make_shared<urpg::scene::MapScene>("EditorPreview", 16, 12);
            editorPreview->setAssetReferences(
                urpg::scene::loadRuntimeMapAssetReferences(activeProjectRoot, "EditorPreview"));
            urpg::scene::SceneManager::getInstance().gotoScene(editorPreview);
        }

        urpg::editor::EditorShell editorShell;
        editorShell.setProjectRoot(activeProjectRoot);
        editorShell.setRuntimePreviewId("EditorPreview");
        if (!editorShell.start(options.headless)) {
            std::cerr << "URPG editor shell startup failed.\n";
            return 1;
        }

#ifdef URPG_IMGUI_ENABLED
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(config.width), static_cast<float>(config.height));
        std::filesystem::create_directories(settingsLoad.settings.imgui_ini_path.parent_path());
        const std::string imguiIniFilename = settingsLoad.settings.imgui_ini_path.string();
        ImGui::GetIO().IniFilename = imguiIniFilename.c_str();
        ImGui::GetIO().LogFilename = nullptr;
        if (options.headless) {
            unsigned char* fontPixels = nullptr;
            int fontWidth = 0;
            int fontHeight = 0;
            ImGui::GetIO().Fonts->GetTexDataAsRGBA32(&fontPixels, &fontWidth, &fontHeight);
        }
#ifndef URPG_HEADLESS
        if (!options.headless && !ImGui_ImplOpenGL3_Init("#version 330")) {
            std::cerr << "URPG editor failed to initialize ImGui OpenGL renderer.\n";
            ImGui::DestroyContext();
            editorShell.shutdown();
            engineShell.shutdown();
            clearSceneStack();
            return 1;
        }
        if (!options.headless) {
            auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform());
            if (!sdlSurface || !ImGui_ImplSDL2_InitForOpenGL(sdlSurface->getNativeWindow(),
                                                             sdlSurface->getNativeGlContext())) {
                std::cerr << "URPG editor failed to initialize ImGui SDL renderer.\n";
                ImGui_ImplOpenGL3_Shutdown();
                ImGui::DestroyContext();
                editorShell.shutdown();
                engineShell.shutdown();
                clearSceneStack();
                return 1;
            }
            sdlSurface->setEventCallback([](const void* event) {
                ImGui_ImplSDL2_ProcessEvent(static_cast<const SDL_Event*>(event));
            });
        }
#endif
#endif

        EditorPanelRuntime panelRuntime;
        panelRuntime.project_root = activeProjectRoot;
        panelRuntime.creator_mode = creatorMode;
        panelRuntime.main_menu_model.applySettings(settingsLoad.settings);
        panelRuntime.asset_library_panel.model().applyUserAssetCuration(settingsLoad.settings);
        panelRuntime.map_authoring_workspace.setLayout({
            settingsLoad.settings.map_workspace_layout.palette_width_fraction,
            settingsLoad.settings.map_workspace_layout.inspector_width_fraction,
            settingsLoad.settings.map_workspace_layout.diagnostics_height_fraction,
            settingsLoad.settings.map_workspace_layout.palette_visible,
            settingsLoad.settings.map_workspace_layout.inspector_visible,
            settingsLoad.settings.map_workspace_layout.diagnostics_visible,
        });
        panelRuntime.external_asset_library_root = settingsLoad.settings.external_asset_library_root;
        panelRuntime.new_project_wizard.setExternalAssetLibraryRoot(panelRuntime.external_asset_library_root);
        panelRuntime.main_menu_panel.bindModel(&panelRuntime.main_menu_model);
        panelRuntime.main_menu_panel.bindWizard(&panelRuntime.new_project_wizard);
        panelRuntime.project_session.addSwitchListener([&panelRuntime](const urpg::editor::EditorProjectIdentity& identity) {
            const bool unclean = panelRuntime.recovery_service.hasUncleanSessionMarker(identity.root);
            bindMapAuthoringProject(panelRuntime, identity.root);
            bindMenuStudioProject(panelRuntime, identity.root);
            if (panelRuntime.gameplay_recipe_dirty_surface_registered) {
                (void)panelRuntime.dirty_state_registry.markDirty(kGameplayRecipeDirtyDocumentId, false);
            }
            if (panelRuntime.menu_studio_dirty_surface_registered) {
                (void)panelRuntime.dirty_state_registry.markDirty(kMenuStudioDirtyDocumentId, false);
            }
            if (panelRuntime.mz_plugin_lock_dirty_surface_registered) {
                (void)panelRuntime.dirty_state_registry.markDirty(kMzPluginLockDirtyDocumentId, false);
            }
            panelRuntime.next_recovery_snapshot_at = {};
            panelRuntime.creator_checklist_panel.setProjectRoot(identity.root);
            if (panelRuntime.recovery_service.writeSessionMarker(identity.root)) {
                panelRuntime.recovery_status = unclean
                    ? "The previous editor session did not close cleanly. Recovery snapshots are available under .urpg/recovery."
                    : "Recovery session marker is active; manual saves remain separate from recovery data.";
            } else {
                panelRuntime.recovery_status = "Could not write the editor recovery session marker.";
            }
        });
        panelRuntime.project_session.addCloseListener([&panelRuntime](const urpg::editor::EditorProjectIdentity& identity) {
            (void)panelRuntime.recovery_service.clearSessionMarker(identity.root);
            panelRuntime.gameplay_recipe_panel.loadProject({});
            if (panelRuntime.gameplay_recipe_dirty_surface_registered) {
                (void)panelRuntime.dirty_state_registry.markDirty(kGameplayRecipeDirtyDocumentId, false);
            }
            panelRuntime.menu_studio_runtime.getSceneGraphMutable().clearRegisteredScenes();
            panelRuntime.menu_studio_runtime.getRegistryMutable().clear();
            panelRuntime.diagnostics_workspace.clearMenuRuntime();
            panelRuntime.menu_studio_persisted_json.clear();
            panelRuntime.menu_studio_load_status.clear();
            panelRuntime.menu_studio_save_status.clear();
            if (panelRuntime.menu_studio_dirty_surface_registered) {
                (void)panelRuntime.dirty_state_registry.markDirty(kMenuStudioDirtyDocumentId, false);
            }
            panelRuntime.mz_plugin_lock_draft = nlohmann::json::object();
            panelRuntime.mz_plugin_lock_source_current = false;
            panelRuntime.mz_plugin_lock_status.clear();
            if (panelRuntime.mz_plugin_lock_dirty_surface_registered) {
                (void)panelRuntime.dirty_state_registry.markDirty(kMzPluginLockDirtyDocumentId, false);
            }
        });
        // A supplied/recent path is not editor state until the session accepts
        // its manifest. This prevents a stale directory from becoming an
        // implicit global project merely because the engine shell could start.
        if (!panelRuntime.creator_mode && !options.smoke) {
            const auto opened = panelRuntime.project_session.openProject(activeProjectRoot);
            if (opened.success) {
                panelRuntime.main_menu_model.setLastProject(panelRuntime.project_session.activeProject().root.generic_string());
                panelRuntime.main_menu_model.addRecentProject(panelRuntime.project_session.activeProject().root.generic_string());
                editorShell.setProjectRoot(panelRuntime.project_session.activeProject().root);
            } else {
                panelRuntime.creator_mode = true;
                panelRuntime.project_root = settingsPaths.root;
                panelRuntime.main_menu_model.markProjectMissing(activeProjectRoot.generic_string());
                editorShell.setProjectRoot(settingsPaths.root);
            }
        }
        panelRuntime.creator_checklist_panel.setProjectRoot(panelRuntime.project_root);
        panelRuntime.creator_checklist_panel.setVisible(false);
        const auto analyticsConsent = analyticsConsentFromSettings(settingsLoad.settings.analytics_consent_state);
        panelRuntime.analytics_privacy_controller.recordConsentDecision(analyticsConsent);
        panelRuntime.analytics_dispatcher.setOptIn(analyticsConsent == urpg::analytics::ConsentState::Granted &&
                                                   settingsLoad.settings.analytics_upload_enabled);
        if (!registerEditorPanels(editorShell, panelRuntime)) {
            std::cerr << "URPG editor failed to register required panels.\n";
#ifdef URPG_IMGUI_ENABLED
#ifndef URPG_HEADLESS
            if (!options.headless) {
                if (auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform())) {
                    sdlSurface->setEventCallback(nullptr);
                }
                ImGui_ImplSDL2_Shutdown();
                ImGui_ImplOpenGL3_Shutdown();
            }
#endif
            ImGui::DestroyContext();
#endif
            editorShell.shutdown();
            engineShell.shutdown();
            clearSceneStack();
            return 1;
        }

        if (options.open_panel_id.has_value() && !editorShell.openPanel(*options.open_panel_id)) {
            std::cerr << "URPG editor has no reachable panel with id '" << *options.open_panel_id << "'.\n";
#ifdef URPG_IMGUI_ENABLED
#ifndef URPG_HEADLESS
            if (!options.headless) {
                if (auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform())) {
                    sdlSurface->setEventCallback(nullptr);
                }
                ImGui_ImplSDL2_Shutdown();
                ImGui_ImplOpenGL3_Shutdown();
            }
#endif
            ImGui::DestroyContext();
#endif
            editorShell.shutdown();
            engineShell.shutdown();
            clearSceneStack();
            return 2;
        }

        if (options.list_panels) {
            printPanelList(editorShell);
        }

        if (options.smoke) {
            const int smokeResult = runSmokeWorkflow(engineShell, editorShell, options);
            editorShell.shutdown();
#ifdef URPG_IMGUI_ENABLED
            ImGui::DestroyContext();
#endif
            engineShell.shutdown();
            clearSceneStack();
            return smokeResult;
        }

        int frame = 0;
        while (engineShell.isRunning() && editorShell.isRunning() && (options.frames < 0 || frame < options.frames)) {
            (void)runEditorFrame(engineShell, editorShell, options.render_all_panels, &panelRuntime);
            ++frame;
            if (options.headless) {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }

        editorShell.shutdown();
#ifdef URPG_IMGUI_ENABLED
#ifndef URPG_HEADLESS
        if (!options.headless) {
            if (auto* sdlSurface = dynamic_cast<urpg::SDLSurface*>(engineShell.getPlatform())) {
                sdlSurface->setEventCallback(nullptr);
            }
            ImGui_ImplSDL2_Shutdown();
            ImGui_ImplOpenGL3_Shutdown();
        }
#endif
        ImGui::DestroyContext();
#endif
        engineShell.shutdown();
        clearSceneStack();

        settingsLoad.settings.window.width = config.width;
        settingsLoad.settings.window.height = config.height;
        settingsLoad.settings.window.fullscreen = config.fullscreen;
        settingsLoad.settings.window.resizable = config.resizable;
        settingsLoad.settings.analytics_consent_state =
            analyticsConsentToSettings(panelRuntime.analytics_privacy_controller.getConsentState());
        settingsLoad.settings.analytics_upload_enabled = panelRuntime.analytics_dispatcher.isOptIn();
        panelRuntime.main_menu_model.writeSettings(&settingsLoad.settings);
        panelRuntime.asset_library_panel.model().writeUserAssetCuration(&settingsLoad.settings);
        const auto& mapLayout = panelRuntime.map_authoring_workspace.snapshot().layout;
        settingsLoad.settings.map_workspace_layout.palette_width_fraction = mapLayout.paletteWidthFraction;
        settingsLoad.settings.map_workspace_layout.inspector_width_fraction = mapLayout.inspectorWidthFraction;
        settingsLoad.settings.map_workspace_layout.diagnostics_height_fraction = mapLayout.diagnosticsHeightFraction;
        settingsLoad.settings.map_workspace_layout.palette_visible = mapLayout.paletteVisible;
        settingsLoad.settings.map_workspace_layout.inspector_visible = mapLayout.inspectorVisible;
        settingsLoad.settings.map_workspace_layout.diagnostics_visible = mapLayout.diagnosticsVisible;
        std::string settingsError;
        if (!urpg::settings::saveEditorSettings(settingsPaths.editor_settings, settingsLoad.settings, &settingsError)) {
            std::cerr << "URPG editor failed to save settings: " << settingsError << "\n";
        }

        std::cout << "URPG editor exited after " << frame << " frame(s).\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "URPG editor exception: " << ex.what() << "\n";
        return 2;
    }
}
