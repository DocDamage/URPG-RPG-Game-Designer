#include "apps/editor/editor_app_panels.h"
#include "editor/ability/ability_inspector_panel.h"
#include "editor/ability/pattern_field_panel.h"
#include "editor/analytics/analytics_panel.h"
#include "editor/assets/asset_library_panel.h"
#include "editor/assets/editor_asset_drag_payload.h"
#include "editor/assets/editor_thumbnail_cache.h"
#include "editor/character/character_creator_model.h"
#include "editor/character/character_creator_panel.h"
#include "editor/project/creator_checklist_panel.h"
#include "editor/project/main_menu_panel.h"
#include "editor/project/new_project_wizard_model.h"
#include "editor/project/editor_project_session.h"
#include "editor/project/editor_dirty_state_registry.h"
#include "editor/project/editor_recovery_service.h"
#include "editor/playtest/playtest_session_controller.h"
#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/mod/mod_manager_panel.h"
#include "editor/spatial/level_builder_workspace.h"
#include "editor/spatial/map_authoring_workspace.h"
#include "editor/spatial/map_authoring_persistence.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/analytics/analytics_dispatcher.h"
#include "engine/core/analytics/analytics_privacy_controller.h"
#include "engine/core/analytics/analytics_uploader.h"
#include "engine/core/app_cli.h"

#include <type_traits>
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/diagnostics/startup_diagnostics.h"
#include "engine/core/editor/editor_panel_registry.h"
#include "engine/core/editor/editor_shell.h"
#include "engine/core/engine_context.h"
#include "engine/core/engine_shell.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_document.h"
#include "engine/core/map/grid_part_ruleset.h"
#include "engine/core/map/grid_part_serializer.h"
#include "engine/core/mod/mod_loader.h"
#include "engine/core/mod/mod_registry.h"
#include "engine/core/platform/headless_renderer.h"
#include "engine/core/platform/headless_surface.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/project/project_snapshot_store.h"
#include "engine/core/scene/map_scene.h"
#include "engine/core/scene/scene_manager.h"
#include "engine/core/settings/app_settings_store.h"
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
#include <string>
#include <thread>
#include <unordered_map>
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
    urpg::editor::CharacterCreatorModel character_creator_model;
    urpg::editor::CharacterCreatorPanel character_creator_panel;
    urpg::editor::PatternFieldModel pattern_field_model;
    urpg::editor::PatternFieldPanel pattern_field_panel;
    urpg::editor::ModManagerPanel mod_manager_panel;
    urpg::editor::AnalyticsPanel analytics_panel;
    urpg::editor::LevelBuilderWorkspace level_builder_workspace;
    urpg::editor::SpatialAuthoringWorkspace perspective_2d_workspace;
    // The two release routes remain available, but this shared coordinator owns
    // their creator-facing mode, selection, history, and project context.
    urpg::editor::MapAuthoringWorkspace map_authoring_workspace;
    urpg::editor::PlaytestSessionController playtest_session;
    urpg::ability::AbilitySystemComponent ability_runtime;
    urpg::map::GridPartDocument level_builder_document{"EditorPreview", 16, 12};
    urpg::map::GridPartCatalog level_builder_catalog;
    urpg::presentation::SpatialMapOverlay level_builder_overlay;
    std::unique_ptr<urpg::scene::MapScene> perspective_2d_scene =
        std::make_unique<urpg::scene::MapScene>("EditorPreview", 16, 12);
    urpg::mod::ModRegistry mod_registry;
    std::unique_ptr<urpg::mod::ModLoader> mod_loader;
    urpg::analytics::AnalyticsDispatcher analytics_dispatcher;
    urpg::analytics::AnalyticsUploader analytics_uploader;
    urpg::analytics::AnalyticsPrivacyController analytics_privacy_controller;
    std::filesystem::path project_root;
    std::filesystem::path external_asset_library_root;
    std::string character_draft_id = "protagonist";
    bool creator_mode = false;
    bool focus_workspace_next_frame = true;
    std::string last_workspace_panel_id;
    std::string map_save_status;
    std::string map_asset_drop_status;
    std::string project_session_status;
    std::string recovery_status;
    std::filesystem::path restored_recovery_project_path;
    std::chrono::steady_clock::time_point next_recovery_snapshot_at{};
    bool map_dirty_surface_registered = false;
    bool perspective_2d_dirty_surface_registered = false;
    bool ability_dirty_surface_registered = false;
    bool character_dirty_surface_registered = false;
};

constexpr const char* kMapDirtyDocumentId = "map.grid_parts";
constexpr const char* kPerspective2DDirtyDocumentId = "map.perspective_2d";
constexpr const char* kCharacterDirtyDocumentId = "character.creator";

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

urpg::map::GridPartCategory gridPartCategoryFromString(const std::string& value) {
    using Category = urpg::map::GridPartCategory;
    static const std::unordered_map<std::string, Category> categories = {
        {"Tile", Category::Tile},
        {"Wall", Category::Wall},
        {"Platform", Category::Platform},
        {"Hazard", Category::Hazard},
        {"Door", Category::Door},
        {"Npc", Category::Npc},
        {"Enemy", Category::Enemy},
        {"TreasureChest", Category::TreasureChest},
        {"SavePoint", Category::SavePoint},
        {"Trigger", Category::Trigger},
        {"CutsceneZone", Category::CutsceneZone},
        {"Shop", Category::Shop},
        {"QuestItem", Category::QuestItem},
        {"Prop", Category::Prop},
        {"LevelBlock", Category::LevelBlock},
    };

    const auto found = categories.find(value);
    return found == categories.end() ? Category::Prop : found->second;
}

urpg::map::GridPartLayer gridPartLayerFromString(const std::string& value) {
    using Layer = urpg::map::GridPartLayer;
    static const std::unordered_map<std::string, Layer> layers = {
        {"Terrain", Layer::Terrain},     {"Decoration", Layer::Decoration}, {"Collision", Layer::Collision},
        {"Object", Layer::Object},       {"Actor", Layer::Actor},           {"Trigger", Layer::Trigger},
        {"Region", Layer::Region},       {"Overlay", Layer::Overlay},
    };

    const auto found = layers.find(value);
    return found == layers.end() ? Layer::Object : found->second;
}

urpg::map::GridPartCollisionPolicy gridPartCollisionPolicyFromString(const std::string& value) {
    using Policy = urpg::map::GridPartCollisionPolicy;
    static const std::unordered_map<std::string, Policy> policies = {
        {"None", Policy::None},
        {"Solid", Policy::Solid},
        {"Hazard", Policy::Hazard},
        {"TriggerOnly", Policy::TriggerOnly},
        {"Custom", Policy::Custom},
    };

    const auto found = policies.find(value);
    return found == policies.end() ? Policy::None : found->second;
}

urpg::map::GridPartRuleset gridPartRulesetFromString(const std::string& value) {
    using Ruleset = urpg::map::GridPartRuleset;
    static const std::unordered_map<std::string, Ruleset> rulesets = {
        {"TopDownJRPG", Ruleset::TopDownJRPG},
        {"SideScrollerAction", Ruleset::SideScrollerAction},
        {"TacticalGrid", Ruleset::TacticalGrid},
        {"DungeonRoomBuilder", Ruleset::DungeonRoomBuilder},
        {"WorldMap", Ruleset::WorldMap},
        {"TownHub", Ruleset::TownHub},
        {"BattleArena", Ruleset::BattleArena},
        {"CutsceneStage", Ruleset::CutsceneStage},
    };

    const auto found = rulesets.find(value);
    return found == rulesets.end() ? Ruleset::TopDownJRPG : found->second;
}

bool loadGridPartCatalog(const std::filesystem::path& projectRoot, urpg::map::GridPartCatalog& catalog) {
    const auto catalogPath = projectRoot / "content" / "part_catalogs" / "base_jrpg_parts.json";
    std::ifstream stream(catalogPath, std::ios::binary);
    if (!stream) {
        return false;
    }

    nlohmann::json payload;
    try {
        payload = nlohmann::json::parse(stream);
    } catch (const nlohmann::json::exception&) {
        return false;
    }

    if (!payload.contains("parts") || !payload["parts"].is_array()) {
        return false;
    }

    urpg::map::GridPartCatalog loaded;
    for (const auto& part : payload["parts"]) {
        if (!part.is_object() || !part.contains("partId") || !part["partId"].is_string()) {
            return false;
        }

        urpg::map::GridPartDefinition definition;
        definition.part_id = part["partId"].get<std::string>();
        definition.display_name = part.value("displayName", definition.part_id);
        definition.description = part.value("description", "");
        definition.category = gridPartCategoryFromString(part.value("category", "Prop"));
        definition.default_layer = gridPartLayerFromString(part.value("defaultLayer", "Object"));
        definition.collision_policy = gridPartCollisionPolicyFromString(part.value("collisionPolicy", "None"));
        definition.asset_id = part.value("assetId", "");
        definition.prefab_path = part.value("prefabPath", "");
        definition.tile_id = part.value("tileId", 0);

        const auto footprint = part.value("footprint", nlohmann::json::object());
        definition.footprint.width = footprint.value("width", 1);
        definition.footprint.height = footprint.value("height", 1);
        definition.footprint.allow_overlap = footprint.value("allowOverlap", false);
        definition.footprint.blocks_navigation = footprint.value("blocksNavigation", false);

        for (const auto& ruleset : part.value("supportedRulesets", nlohmann::json::array())) {
            if (ruleset.is_string()) {
                definition.supported_rulesets.push_back(gridPartRulesetFromString(ruleset.get<std::string>()));
            }
        }
        if (definition.supported_rulesets.empty()) {
            definition.supported_rulesets.push_back(urpg::map::GridPartRuleset::TopDownJRPG);
        }

        for (const auto& tag : part.value("tags", nlohmann::json::array())) {
            if (tag.is_string()) {
                definition.tags.push_back(tag.get<std::string>());
            }
        }

        const auto properties = part.value("defaultProperties", nlohmann::json::object());
        for (const auto& [key, value] : properties.items()) {
            if (value.is_string()) {
                definition.default_properties[key] = value.get<std::string>();
            }
        }

        if (!loaded.addDefinition(std::move(definition))) {
            return false;
        }
    }

    catalog = std::move(loaded);
    return catalog.size() > 0;
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

void captureRecoverySnapshot(EditorPanelRuntime& runtime) {
    if (!runtime.project_session.isOpen() || runtime.project_root.empty()) return;
    const auto mapDirty = runtime.dirty_state_registry.isDirty(kMapDirtyDocumentId) ||
                          runtime.dirty_state_registry.isDirty(kPerspective2DDirtyDocumentId);
    const auto abilityDirty = runtime.dirty_state_registry.isDirty("ability.draft");
    const auto characterDirty = runtime.dirty_state_registry.isDirty(kCharacterDirtyDocumentId);
    if (!mapDirty && !abilityDirty && !characterDirty) return;

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

void bindMapAuthoringProject(EditorPanelRuntime& runtime, const std::filesystem::path& projectRoot) {
    runtime.project_root = projectRoot;
    runtime.character_draft_id = "protagonist";
    {
        std::ifstream manifestInput(projectRoot / "project.json", std::ios::binary);
        const auto manifest = nlohmann::json::parse(manifestInput, nullptr, false);
        if (manifest.is_object() && manifest.contains("creator") && manifest["creator"].is_object() &&
            manifest["creator"].value("vertical_slice_seed", "") == "lantern_of_the_willow_draft") {
            runtime.character_draft_id = "willow_hero";
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
    const auto mapId = starterMapIdForProject(projectRoot);
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
    static std::string assetWorkflowStatus;
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
        ImGui::Combo("If attachment conflicts", &attachmentConflictPolicy, conflictLabels, IM_ARRAYSIZE(conflictLabels));
        const auto policy = static_cast<urpg::assets::ProjectAssetAttachmentConflictPolicy>(attachmentConflictPolicy);
        const auto& actionRows = panel.lastRenderSnapshot().asset_action_rows;
        for (const auto& row : actionRows) {
            const auto attach = row.value("attach_button", nlohmann::json::object());
            const bool projectAttached = row.value("project_attached", false);
            if (!attach.value("enabled", false) && !projectAttached) continue;
            const auto path = row.value("path", "");
            ImGui::PushID(row.value("asset_id", path).c_str());
            ImGui::Text("%s: %s", projectAttached ? "Attached" : "Ready", row.value("asset_id", "asset").c_str());
            if (attach.value("enabled", false)) {
                ImGui::SameLine();
                if (ImGui::Button("Attach To Project")) {
                    const auto result = panel.attachSelectedPromotedAssetsToProject({path}, runtime.project_root, policy);
                    assetWorkflowStatus = result.value("message", "Attachment did not return a status.");
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
}

void renderPerspectiveWorkspace(EditorPanelRuntime& runtime) {
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
    ImGui::Separator();
    ImGui::TextUnformatted("Contextual Authoring");
    ImGui::TextDisabled("Current character draft: %s", runtime.character_draft_id.c_str());
    if (ImGui::Button("Edit Character for This Map")) {
        (void)editorShell.openPanel("character_creator");
        runtime.focus_workspace_next_frame = true;
        workspace.setNextActionHint("Character authoring opened from the active Map context.");
    }
    ImGui::SameLine();
    ImGui::TextDisabled("Keeps the active project and Map context intact.");
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
    for (const auto& mode : snapshot.modes) {
        ImGui::PushID(mode.id.c_str());
        if (!mode.available) {
            ImGui::BeginDisabled();
        }
        if (ImGui::Button(mode.label.c_str(), ImVec2(94.0f, 0.0f))) {
            if (mode.id == "canvas") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Canvas);
            else if (mode.id == "tiles") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Tiles);
            else if (mode.id == "parts") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Parts);
            else if (mode.id == "props") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Props);
            else if (mode.id == "events") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Events);
            else if (mode.id == "abilities") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Abilities);
            else if (mode.id == "world") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::World);
            else if (mode.id == "validate") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Validate);
            else if (mode.id == "playtest") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Playtest);
            else if (mode.id == "package") (void)workspace.activateMode(urpg::editor::MapAuthoringMode::Package);
            workspace.clearNextActionHint();
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
    const auto& io = ImGui::GetIO();
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
    ImGui::TextDisabled("Drop attached project assets on the Map canvas. Tiles place immediately; Props enroll in their palette.");
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
            renderPerspectiveWorkspace(runtime);
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
            const auto accepted = parsed.accepted && asset.mediaKind == "image"
                                      ? urpg::editor::assessEditorAssetDrop(asset, true)
                                      : urpg::editor::EditorAssetDropDecision{
                                            false, "character_asset_drop_requires_image",
                                            "Character Creator accepts attached image assets only.",
                                            "Attach an image in Assets, then drag it here."};
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
