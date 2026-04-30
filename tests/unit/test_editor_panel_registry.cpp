#include "engine/core/editor/editor_panel_registry.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace {

bool ContainsId(const std::vector<std::string>& ids, const std::string& id) {
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

const std::vector<std::string>& CanonicalReleasePanelIds() {
    static const std::vector<std::string> ids = {"diagnostics", "assets",  "ability", "patterns",
                                                 "mod",         "analytics", "level_builder"};
    return ids;
}

std::string normalizePath(const std::filesystem::path& path) {
    return path.generic_string();
}

const std::map<std::string, std::string>& CompiledPanelRegistryOwners() {
    static const std::map<std::string, std::string> owners = {
        {"editor/ability/ability_inspector_panel.cpp", "ability"},
        {"editor/ability/ability_orchestration_panel.cpp", "ability"},
        {"editor/ability/ability_sandbox_panel.cpp", "ability"},
        {"editor/ability/pattern_field_panel.cpp", "patterns"},
        {"editor/accessibility/accessibility_assistant_panel.cpp", "accessibility_assistant"},
        {"editor/accessibility/accessibility_panel.cpp", "accessibility"},
        {"editor/achievement/achievement_panel.cpp", "achievement"},
        {"editor/action/controller_binding_panel.cpp", "controller_binding"},
        {"editor/ai/ai_assistant_panel.cpp", "ai_assistant"},
        {"editor/ai/creator_command_panel.cpp", "ai_assistant"},
        {"editor/analytics/analytics_panel.cpp", "analytics"},
        {"editor/assets/asset_library_panel.cpp", "assets"},
        {"editor/audio/audio_mix_panel.cpp", "audio_mix"},
        {"editor/balance/balance_panel.cpp", "balance"},
        {"editor/balance/encounter_designer_panel.cpp", "encounter_designer"},
        {"editor/battle/battle_inspector_panel.cpp", "battle_inspector"},
        {"editor/battle/battle_presentation_panel.cpp", "battle_presentation"},
        {"editor/battle/battle_preview_panel.cpp", "battle_preview"},
        {"editor/battle/battle_vfx_timeline_panel.cpp", "battle_presentation"},
        {"editor/battle/boss_designer_panel.cpp", "boss_designer"},
        {"editor/battle/formula_debugger_panel.cpp", "formula_debugger"},
        {"editor/capture/capture_panel.cpp", "capture"},
        {"editor/character/character_creator_panel.cpp", "character_creator"},
        {"editor/codex/codex_panel.cpp", "codex"},
        {"editor/collaboration/local_review_panel.cpp", "local_review"},
        {"editor/community/community_wysiwyg_panel.cpp", "smart_event_workflow"},
        {"editor/compat/compat_report_panel.cpp", "compat_report"},
        {"editor/crafting/crafting_loop_panel.cpp", "crafting"},
        {"editor/crafting/crafting_panel.cpp", "crafting"},
        {"editor/database/database_panel.cpp", "database"},
        {"editor/diagnostics/diagnostics_bundle_panel.cpp", "diagnostics_bundle"},
        {"editor/diagnostics/diagnostics_workspace.cpp", "diagnostics"},
        {"editor/diagnostics/event_authority_panel.cpp", "event_authority"},
        {"editor/diagnostics/project_audit_panel.cpp", "project_audit"},
        {"editor/diagnostics/project_health_panel.cpp", "project_health"},
        {"editor/dialogue/dialogue_graph_panel.cpp", "dialogue_graph"},
        {"editor/events/event_authoring_panel.cpp", "event_authoring"},
        {"editor/events/event_command_graph_panel.cpp", "event_authoring"},
        {"editor/export/export_diagnostics_panel.cpp", "export_diagnostics"},
        {"editor/export/export_preview_panel.cpp", "export_diagnostics"},
        {"editor/gameplay/gameplay_wysiwyg_panel.cpp", "status_effect_designer"},
        {"editor/input/input_remap_panel.cpp", "input_remap"},
        {"editor/items/loot_generator_panel.cpp", "loot_generator"},
        {"editor/localization/localization_workspace_panel.cpp", "localization_workspace"},
        {"editor/maker/maker_wysiwyg_panel.cpp", "project_search_everywhere"},
        {"editor/message/dialogue_preview_panel.cpp", "message_inspector"},
        {"editor/message/message_inspector_panel.cpp", "message_inspector"},
        {"editor/message/visual_novel_pacing_panel.cpp", "message_inspector"},
        {"editor/metroidvania/ability_gate_panel.cpp", "metroidvania_gates"},
        {"editor/mod/mod_manager_panel.cpp", "mod"},
        {"editor/mod/mod_sdk_panel.cpp", "mod_sdk"},
        {"editor/monster/monster_collection_panel.cpp", "monster_collection"},
        {"editor/narrative/narrative_continuity_panel.cpp", "narrative_continuity"},
        {"editor/npc/npc_panel.cpp", "npc"},
        {"editor/perf/perf_diagnostics_panel.cpp", "perf_diagnostics"},
        {"editor/platform/device_profile_panel.cpp", "device_profile"},
        {"editor/plugin/plugin_inspector_panel.cpp", "plugin_inspector"},
        {"editor/presentation/photo_mode_panel.cpp", "photo_mode"},
        {"editor/progression/skill_tree_panel.cpp", "skill_tree"},
        {"editor/progression/stat_allocation_panel.cpp", "skill_tree"},
        {"editor/project/new_project_wizard_panel.cpp", "new_project_wizard"},
        {"editor/puzzle/puzzle_panel.cpp", "puzzle"},
        {"editor/quest/quest_panel.cpp", "quest"},
        {"editor/relationship/relationship_panel.cpp", "relationship"},
        {"editor/replay/replay_panel.cpp", "replay"},
        {"editor/save/save_debugger_panel.cpp", "save_debugger"},
        {"editor/save/save_inspector_panel.cpp", "save_inspector"},
        {"editor/save/save_load_preview_lab_panel.cpp", "save_inspector"},
        {"editor/save/save_migration_preview_panel.cpp", "save_migration_preview"},
        {"editor/shop/vendor_panel.cpp", "vendor"},
        {"editor/spatial/dungeon3d_world_panel.cpp", "3d_dungeon_world"},
        {"editor/spatial/elevation_brush_panel.cpp", "elevation_brush"},
        {"editor/spatial/grid_part_inspector_panel.cpp", "level_builder"},
        {"editor/spatial/grid_part_palette_panel.cpp", "level_builder"},
        {"editor/spatial/grid_part_placement_panel.cpp", "level_builder"},
        {"editor/spatial/grid_part_playtest_panel.cpp", "level_builder"},
        {"editor/spatial/level_builder_workspace.cpp", "level_builder"},
        {"editor/spatial/map_ability_binding_panel.cpp", "map_ability_binding"},
        {"editor/spatial/map_environment_preview_panel.cpp", "spatial_authoring"},
        {"editor/spatial/procedural_map_panel.cpp", "procedural_map"},
        {"editor/spatial/prop_placement_panel.cpp", "prop_placement"},
        {"editor/spatial/region_rules_panel.cpp", "region_rules"},
        {"editor/spatial/spatial_ability_canvas_panel.cpp", "spatial_ability_canvas"},
        {"editor/spatial/spatial_authoring_workspace.cpp", "spatial_authoring"},
        {"editor/spatial/terrain_brush_panel.cpp", "terrain_brush"},
        {"editor/sprite/sprite_animation_preview_panel.cpp", "sprite_animation_preview"},
        {"editor/time/calendar_panel.cpp", "calendar"},
        {"editor/timeline/cutscene_timeline_panel.cpp", "cutscene_timeline"},
        {"editor/timeline/timeline_panel.cpp", "timeline"},
        {"editor/ui/menu_inspector_panel.cpp", "menu_inspector"},
        {"editor/ui/menu_preview_panel.cpp", "menu_preview"},
        {"editor/ui/theme_builder_panel.cpp", "theme_builder"},
        {"editor/world/world_panel.cpp", "world"},
    };
    return owners;
}

bool isAllowedReleasePlaceholderTerm(const std::string& source, const std::string& line,
                                     const std::string& term) {
    struct Allowance {
        const char* source;
        const char* needle;
        const char* term;
        const char* reason;
    };

    static constexpr Allowance allowances[] = {
        {"editor/diagnostics/diagnostics_workspace_export.cpp", "stubCount", "stub",
         "Compatibility reports expose stubCount as an explicit diagnostic status bucket, not production behavior."},
    };

    for (const auto& allowance : allowances) {
        if (source == allowance.source && term == allowance.term && line.find(allowance.needle) != std::string::npos &&
            std::string_view(allowance.reason).size() > 0) {
            return true;
        }
    }
    return false;
}

std::vector<std::filesystem::path> sourceFilesUnder(const std::filesystem::path& ownerPath) {
    std::vector<std::filesystem::path> files;
    if (std::filesystem::is_regular_file(ownerPath)) {
        files.push_back(ownerPath);
        return files;
    }

    for (const auto& entry : std::filesystem::recursive_directory_iterator(ownerPath)) {
        if (!entry.is_regular_file()) {
            continue;
        }

        const auto extension = entry.path().extension().string();
        if (extension == ".cpp" || extension == ".h" || extension == ".hpp") {
            files.push_back(entry.path());
        }
    }
    return files;
}

} // namespace

TEST_CASE("Editor panel registry exposes canonical top-level panels", "[editor][panel][registry]") {
    const auto ids = urpg::editor::requiredTopLevelPanelIds();

    REQUIRE(ids == CanonicalReleasePanelIds());
    REQUIRE(ContainsId(ids, "diagnostics"));
    REQUIRE(ContainsId(ids, "assets"));
    REQUIRE(ContainsId(ids, "ability"));
    REQUIRE(ContainsId(ids, "patterns"));
    REQUIRE(ContainsId(ids, "mod"));
    REQUIRE(ContainsId(ids, "analytics"));
    REQUIRE(ContainsId(ids, "level_builder"));

    const auto* patterns = urpg::editor::findEditorPanelRegistryEntry("patterns");
    REQUIRE(patterns != nullptr);
    REQUIRE(patterns->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);
    REQUIRE(patterns->title == "Pattern Field Editor");
}

TEST_CASE("Editor panel registry documents every hidden compiled panel", "[editor][panel][registry]") {
    REQUIRE(urpg::editor::hiddenEditorPanelEntriesHaveReasons());

    std::set<std::string> seen;
    for (const auto& entry : urpg::editor::editorPanelRegistry()) {
        REQUIRE_FALSE(entry.id.empty());
        REQUIRE_FALSE(entry.title.empty());
        REQUIRE_FALSE(entry.category.empty());
        REQUIRE_FALSE(entry.owner.empty());
        REQUIRE(seen.insert(entry.id).second);

        const auto isKnownExposure =
            entry.exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel ||
            entry.exposure == urpg::editor::EditorPanelExposure::Nested ||
            entry.exposure == urpg::editor::EditorPanelExposure::DevOnly ||
            entry.exposure == urpg::editor::EditorPanelExposure::Deferred;
        REQUIRE(isKnownExposure);

        if (entry.exposure != urpg::editor::EditorPanelExposure::ReleaseTopLevel) {
            REQUIRE_FALSE(entry.reason.empty());
        }
    }
}

TEST_CASE("Editor panel registry assigns every compiled editor panel one exposure decision",
          "[editor][panel][registry]") {
    const auto sourceRoot = std::filesystem::path(URPG_SOURCE_DIR);
    const auto editorRoot = sourceRoot / "editor";

    std::set<std::string> compiledPanelSources;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(editorRoot)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".cpp") {
            continue;
        }

        const auto filename = entry.path().filename().string();
        if (filename.ends_with("_panel.cpp") || filename.ends_with("_workspace.cpp")) {
            compiledPanelSources.insert(normalizePath(std::filesystem::relative(entry.path(), sourceRoot)));
        }
    }

    std::set<std::string> classifiedPanelSources;
    for (const auto& [source, registryId] : CompiledPanelRegistryOwners()) {
        INFO(source);
        classifiedPanelSources.insert(source);
        REQUIRE(urpg::editor::findEditorPanelRegistryEntry(registryId) != nullptr);
    }

    REQUIRE(classifiedPanelSources == compiledPanelSources);
}

TEST_CASE("Editor panel registry owner paths exist", "[editor][panel][registry]") {
    const auto sourceRoot = std::filesystem::path(URPG_SOURCE_DIR);

    for (const auto& entry : urpg::editor::editorPanelRegistry()) {
        INFO(entry.id);
        REQUIRE(std::filesystem::exists(sourceRoot / entry.owner));
    }
}

TEST_CASE("Editor panel registry release top-level owners do not expose ungoverned placeholder terms",
          "[editor][panel][registry][release]") {
    static const std::vector<std::string> bannedTerms = {
        "TODO", "FIXME", "stub", "placeholder", "fake", "mock", "For testing", "dummy", "dev bootstrap",
    };

    const auto sourceRoot = std::filesystem::path(URPG_SOURCE_DIR);
    std::set<std::filesystem::path> releaseOwnerPaths;
    for (const auto& panel : urpg::editor::topLevelEditorPanels()) {
        releaseOwnerPaths.insert(sourceRoot / panel.owner);
    }

    for (const auto& ownerPath : releaseOwnerPaths) {
        for (const auto& sourcePath : sourceFilesUnder(ownerPath)) {
            const auto relativeSource = normalizePath(std::filesystem::relative(sourcePath, sourceRoot));
            std::ifstream input(sourcePath);
            REQUIRE(input.good());

            std::string line;
            size_t lineNumber = 0;
            while (std::getline(input, line)) {
                ++lineNumber;
                for (const auto& term : bannedTerms) {
                    if (line.find(term) == std::string::npos) {
                        continue;
                    }

                    INFO(relativeSource << ":" << lineNumber << " contains " << term);
                    REQUIRE(isAllowedReleasePlaceholderTerm(relativeSource, line, term));
                }
            }
        }
    }
}

TEST_CASE("Editor panel registry classifies diagnostics and incubating workspaces", "[editor][panel][registry]") {
    const auto* saveInspector = urpg::editor::findEditorPanelRegistryEntry("save_inspector");
    REQUIRE(saveInspector != nullptr);
    REQUIRE(saveInspector->exposure == urpg::editor::EditorPanelExposure::Nested);

    const auto* levelBuilder = urpg::editor::findEditorPanelRegistryEntry("level_builder");
    REQUIRE(levelBuilder != nullptr);
    REQUIRE(levelBuilder->exposure == urpg::editor::EditorPanelExposure::ReleaseTopLevel);

    const auto* spatialAuthoring = urpg::editor::findEditorPanelRegistryEntry("spatial_authoring");
    REQUIRE(spatialAuthoring != nullptr);
    REQUIRE(spatialAuthoring->exposure == urpg::editor::EditorPanelExposure::Nested);

    const auto* modSdk = urpg::editor::findEditorPanelRegistryEntry("mod_sdk");
    REQUIRE(modSdk != nullptr);
    REQUIRE(modSdk->exposure == urpg::editor::EditorPanelExposure::DevOnly);

    const auto* developerDebugOverlay = urpg::editor::findEditorPanelRegistryEntry("developer_debug_overlay");
    REQUIRE(developerDebugOverlay != nullptr);
    REQUIRE(developerDebugOverlay->exposure == urpg::editor::EditorPanelExposure::DevOnly);
    REQUIRE_FALSE(ContainsId(urpg::editor::requiredTopLevelPanelIds(), "developer_debug_overlay"));

    const auto topLevel = urpg::editor::topLevelEditorPanels();
    REQUIRE(topLevel.size() == urpg::editor::requiredTopLevelPanelIds().size());
}

TEST_CASE("Editor smoke coverage follows every registered top-level panel", "[editor][panel][registry][smoke]") {
    const auto topLevelIds = urpg::editor::requiredTopLevelPanelIds();
    const auto smokeIds = urpg::editor::smokeRequiredEditorPanelIds();

    REQUIRE(smokeIds == topLevelIds);
    REQUIRE(smokeIds == CanonicalReleasePanelIds());
}
