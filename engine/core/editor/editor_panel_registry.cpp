#include "engine/core/editor/editor_panel_registry.h"

#include <algorithm>

namespace urpg::editor {

namespace {

const std::vector<EditorPanelRegistryEntry> kRegistry = {
    {"diagnostics", "Diagnostics", "System", EditorPanelExposure::ReleaseTopLevel, "editor/diagnostics",
     "Top-level diagnostics workspace and nested validation panels."},
    {"assets", "Assets", "Content", EditorPanelExposure::ReleaseTopLevel, "editor/assets",
     "Top-level asset library and hygiene surface."},
    {"ability", "Ability Inspector", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/ability",
     "Top-level ability runtime and authoring inspector."},
    {"patterns", "Pattern Field Editor", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/ability",
     "Top-level pattern authoring surface required by editor smoke."},
    {"mod", "Mod Manager", "Runtime", EditorPanelExposure::ReleaseTopLevel, "editor/mod",
     "Top-level mod registry, loader, and validation surface."},
    {"analytics", "Analytics", "Runtime", EditorPanelExposure::ReleaseTopLevel, "editor/analytics",
     "Top-level telemetry privacy, validation, and upload surface."},

    {"compat_report", "Compatibility Report", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Compat tab."},
    {"save_inspector", "Save Inspector", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Save tab."},
    {"event_authority", "Event Authority", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Event Authority tab."},
    {"message_inspector", "Message Inspector", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Message Text tab."},
    {"battle_inspector", "Battle Inspector", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Battle tab."},
    {"menu_inspector", "Menu Inspector", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Menu tab."},
    {"menu_preview", "Menu Preview", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Menu preview surface."},
    {"audio_inspector", "Audio Inspector", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Audio tab."},
    {"migration_wizard", "Migration Wizard", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Migration Wizard tab."},
    {"project_audit", "Project Audit", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Project Audit tab."},
    {"project_health", "Project Health", "Diagnostics", EditorPanelExposure::Nested,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Project Health tab."},

    {"diagnostics_bundle", "Diagnostics Bundle", "Diagnostics", EditorPanelExposure::DevOnly,
     "editor/diagnostics", "Dev-only diagnostics export tooling for support bundles; excluded from release navigation."},
    {"event_authoring", "Event Authoring", "Gameplay", EditorPanelExposure::Deferred, "editor/events",
     "Deferred until the event authoring workflow is selected for release shell registration."},
    {"plugin_inspector", "Plugin Inspector", "Runtime", EditorPanelExposure::Deferred, "editor/plugin",
     "Deferred until plugin compatibility triage is selected for release shell registration."},
    {"new_project_wizard", "New Project Wizard", "Project", EditorPanelExposure::Deferred, "editor/project",
     "Project creation wizard is invoked by project flow, not persistent top-level navigation."},
    {"quest", "Quest", "Narrative", EditorPanelExposure::ReleaseTopLevel, "editor/quest",
     "Quest objective graph authoring, live preview, runtime application, diagnostics, and project-data saving."},
    {"skill_tree", "Skill Tree", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/progression",
     "Skill tree/class progression authoring, unlock preview, runtime ability grants, diagnostics, and project-data saving."},
    {"dialogue_graph", "Dialogue Graph", "Narrative", EditorPanelExposure::Deferred, "editor/dialogue",
     "Deferred roadmap panel; dialogue graph editing is outside the current release shell."},
    {"narrative_continuity", "Narrative Continuity", "Narrative", EditorPanelExposure::Deferred,
     "editor/narrative", "Deferred roadmap panel; continuity authoring is outside the current release shell."},
    {"relationship", "Relationship", "Narrative", EditorPanelExposure::ReleaseTopLevel, "editor/relationship",
     "Relationship affinity authoring, event preview, runtime affinity application, diagnostics, and project-data saving."},
    {"localization_workspace", "Localization Workspace", "Content", EditorPanelExposure::Deferred,
     "editor/localization", "Deferred until localization import/export ownership is selected for release shell registration."},
    {"timeline", "Timeline", "Presentation", EditorPanelExposure::ReleaseTopLevel, "editor/timeline",
     "Timeline authoring and runtime playback preview surface."},
    {"cutscene_timeline", "Cutscene Timeline", "Presentation", EditorPanelExposure::ReleaseTopLevel, "editor/timeline",
     "Cutscene track authoring, localized cue preview, runtime timeline playback, diagnostics, and project-data saving."},
    {"replay", "Replay", "Presentation", EditorPanelExposure::Deferred, "editor/replay",
     "Deferred until replay review is selected for release shell registration."},
    {"capture", "Capture", "Presentation", EditorPanelExposure::Deferred, "editor/capture",
     "Deferred until capture tooling is selected for release shell registration."},
    {"photo_mode", "Photo Mode", "Presentation", EditorPanelExposure::Deferred, "editor/presentation",
     "Deferred until presentation photo tooling is selected for release shell registration."},
    {"database", "Database", "Content", EditorPanelExposure::Deferred, "editor/database",
     "Deferred until database editing ownership is selected for release shell registration."},
    {"balance", "Balance", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/balance",
     "Balance editing and simulation surface."},
    {"encounter_designer", "Encounter Designer", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/balance",
     "Encounter region/pool authoring, seeded preview, runtime table export, diagnostics, and project-data saving."},
    {"loot_generator", "Loot Generator", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/items",
     "Loot table and affix authoring, deterministic generated-item preview, diagnostics, and project-data saving."},
    {"vendor", "Vendor", "Gameplay", EditorPanelExposure::Deferred, "editor/shop",
     "Deferred until shop/vendor authoring is selected for release shell registration."},
    {"world", "World", "Worldbuilding", EditorPanelExposure::Deferred, "editor/world",
     "Deferred roadmap panel; worldbuilding workflow is outside the current release shell."},
    {"crafting", "Crafting", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/crafting",
     "Crafting/gathering/economy authoring, live loop preview, runtime application, diagnostics, and project-data saving."},
    {"codex", "Codex", "Content", EditorPanelExposure::Deferred, "editor/codex",
     "Deferred roadmap panel; codex authoring is outside the current release shell."},
    {"calendar", "Calendar", "Worldbuilding", EditorPanelExposure::Deferred, "editor/time",
     "Deferred roadmap panel; calendar authoring is outside the current release shell."},
    {"monster_collection", "Monster Collection", "Gameplay", EditorPanelExposure::ReleaseTopLevel, "editor/monster",
     "Monster capture, party/storage, evolution authoring, deterministic preview, diagnostics, and project-data saving."},
    {"npc", "NPC", "Worldbuilding", EditorPanelExposure::ReleaseTopLevel, "editor/npc",
     "NPC schedule/daily routine authoring, time preview, runtime resolution, diagnostics, and project-data saving."},
    {"metroidvania_gates", "Metroidvania Gates", "Worldbuilding", EditorPanelExposure::ReleaseTopLevel,
     "editor/metroidvania", "Ability-gated region authoring, reachability preview, diagnostics, and project-data saving."},
    {"puzzle", "Puzzle", "Gameplay", EditorPanelExposure::Deferred, "editor/puzzle",
     "Deferred roadmap panel; puzzle authoring is outside the current release shell."},
    {"export_diagnostics", "Export Diagnostics", "Release", EditorPanelExposure::Deferred, "editor/export",
     "Deferred until export diagnostics are promoted from validation tooling into release navigation."},
    {"character_creator", "Character Creator", "Gameplay", EditorPanelExposure::Deferred, "editor/character",
     "Deferred until character creation workflow is selected for release shell registration."},
    {"achievement", "Achievement", "Gameplay", EditorPanelExposure::Deferred, "editor/achievement",
     "Deferred until achievement authoring is selected for release shell registration."},
    {"controller_binding", "Controller Binding", "Input", EditorPanelExposure::Deferred, "editor/action",
     "Deferred until controller binding UX is selected for release shell registration."},
    {"save_debugger", "Save Debugger", "Diagnostics", EditorPanelExposure::Deferred, "editor/save",
     "Deferred diagnostics panel; save debugging remains outside release navigation."},
    {"save_migration_preview", "Save Migration Preview", "Diagnostics", EditorPanelExposure::Deferred,
     "editor/save", "Deferred diagnostics panel; save migration preview remains outside release navigation."},
    {"battle_presentation", "Battle Presentation", "Battle", EditorPanelExposure::Deferred, "editor/battle",
     "Deferred until battle presentation authoring is selected for release shell registration."},
    {"boss_designer", "Boss Designer", "Battle", EditorPanelExposure::Deferred, "editor/battle",
     "Deferred until boss design workflow is selected for release shell registration."},
    {"formula_debugger", "Formula Debugger", "Battle", EditorPanelExposure::Deferred, "editor/battle",
     "Deferred diagnostics panel; formula debugging remains outside release navigation."},
    {"battle_preview", "Battle Preview", "Battle", EditorPanelExposure::Deferred, "editor/battle",
     "Deferred until battle preview workflow is selected for release shell registration."},
    {"perf_diagnostics", "Performance Diagnostics", "Diagnostics", EditorPanelExposure::Deferred, "editor/perf",
     "Deferred diagnostics panel; performance triage remains outside release navigation."},
    {"spatial_authoring", "Spatial Authoring", "Spatial", EditorPanelExposure::Deferred, "editor/spatial",
     "Deferred until spatial authoring graduates from incubating workspace to release navigation."},
    {"elevation_brush", "Elevation Brush", "Spatial", EditorPanelExposure::Nested, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"terrain_brush", "Terrain Brush", "Spatial", EditorPanelExposure::Nested, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"region_rules", "Region Rules", "Spatial", EditorPanelExposure::Nested, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"procedural_map", "Procedural Map", "Spatial", EditorPanelExposure::Nested, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"prop_placement", "Prop Placement", "Spatial", EditorPanelExposure::Nested, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"map_ability_binding", "Map Ability Binding", "Spatial", EditorPanelExposure::Nested,
     "editor/spatial", "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"spatial_ability_canvas", "Spatial Ability Canvas", "Spatial", EditorPanelExposure::Nested,
     "editor/spatial", "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"sprite_animation_preview", "Sprite Animation Preview", "Content", EditorPanelExposure::Deferred,
     "editor/sprite", "Deferred until sprite animation preview is selected for release shell registration."},
    {"accessibility", "Accessibility", "Quality", EditorPanelExposure::Deferred, "editor/accessibility",
     "Deferred until accessibility settings graduate into release navigation or runtime options."},
    {"accessibility_assistant", "Accessibility Assistant", "Quality", EditorPanelExposure::Deferred,
     "editor/accessibility", "Deferred assistant workflow; accessibility automation is outside release navigation."},
    {"audio_mix", "Audio Mix", "Audio", EditorPanelExposure::Deferred, "editor/audio",
     "Deferred until audio mixing workflow is selected for release shell registration."},
    {"input_remap", "Input Remap", "Input", EditorPanelExposure::Deferred, "editor/input",
     "Deferred until editor input remapping is selected for release shell registration."},
    {"device_profile", "Device Profile", "Platform", EditorPanelExposure::Deferred, "editor/platform",
     "Deferred until platform device profiles are selected for release shell registration."},
    {"theme_builder", "Theme Builder", "UI", EditorPanelExposure::Deferred, "editor/ui",
     "Deferred until UI theme authoring is selected for release shell registration."},
    {"ai_assistant", "AI Assistant", "Productivity", EditorPanelExposure::DevOnly, "editor/ai",
     "Dev-only productivity assistant; excluded from release navigation."},
    {"local_review", "Local Review", "Collaboration", EditorPanelExposure::DevOnly, "editor/collaboration",
     "Dev-only local review tooling; excluded from release navigation."},
    {"mod_sdk", "Mod SDK", "Runtime", EditorPanelExposure::DevOnly, "editor/mod",
     "Dev-only SDK tooling for extension authors; excluded from release navigation."},
    {"core_asset_browser", "Core Asset Browser", "Core Editor", EditorPanelExposure::DevOnly,
     "engine/core/editor/panels", "Dev-only legacy core editor panel API; excluded from release navigation."},
    {"core_hierarchy", "Core Hierarchy", "Core Editor", EditorPanelExposure::DevOnly,
     "engine/core/editor/panels", "Dev-only legacy core editor panel API; excluded from release navigation."},
    {"core_property_inspector", "Core Property Inspector", "Core Editor", EditorPanelExposure::DevOnly,
     "engine/core/editor/panels", "Dev-only legacy core editor panel API; excluded from release navigation."},
};

} // namespace

const std::vector<EditorPanelRegistryEntry>& editorPanelRegistry() {
    return kRegistry;
}

std::vector<EditorPanelRegistryEntry> topLevelEditorPanels() {
    std::vector<EditorPanelRegistryEntry> panels;
    for (const auto& entry : kRegistry) {
        if (entry.exposure == EditorPanelExposure::ReleaseTopLevel) {
            panels.push_back(entry);
        }
    }
    return panels;
}

std::vector<std::string> requiredTopLevelPanelIds() {
    std::vector<std::string> ids;
    for (const auto& entry : kRegistry) {
        if (entry.exposure == EditorPanelExposure::ReleaseTopLevel) {
            ids.push_back(entry.id);
        }
    }
    return ids;
}

std::vector<std::string> smokeRequiredEditorPanelIds() {
    return requiredTopLevelPanelIds();
}

const EditorPanelRegistryEntry* findEditorPanelRegistryEntry(std::string_view id) {
    const auto it = std::find_if(kRegistry.begin(), kRegistry.end(),
                                 [id](const EditorPanelRegistryEntry& entry) { return entry.id == id; });
    return it == kRegistry.end() ? nullptr : &(*it);
}

bool hiddenEditorPanelEntriesHaveReasons() {
    return std::all_of(kRegistry.begin(), kRegistry.end(), [](const EditorPanelRegistryEntry& entry) {
        return entry.exposure == EditorPanelExposure::ReleaseTopLevel || !entry.reason.empty();
    });
}

} // namespace urpg::editor
