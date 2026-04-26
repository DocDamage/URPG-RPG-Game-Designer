#include "engine/core/editor/editor_panel_registry.h"

#include <algorithm>

namespace urpg::editor {

namespace {

const std::vector<EditorPanelRegistryEntry> kRegistry = {
    {"diagnostics", "Diagnostics", "System", EditorPanelExposure::TopLevel, "editor/diagnostics",
     "Top-level diagnostics workspace and nested validation panels."},
    {"assets", "Assets", "Content", EditorPanelExposure::TopLevel, "editor/assets",
     "Top-level asset library and hygiene surface."},
    {"ability", "Ability Inspector", "Gameplay", EditorPanelExposure::TopLevel, "editor/ability",
     "Top-level ability runtime and authoring inspector."},
    {"patterns", "Pattern Field Editor", "Gameplay", EditorPanelExposure::TopLevel, "editor/ability",
     "Top-level pattern authoring surface required by editor smoke."},
    {"mod", "Mod Manager", "Runtime", EditorPanelExposure::TopLevel, "editor/mod",
     "Top-level mod registry, loader, and validation surface."},
    {"analytics", "Analytics", "Runtime", EditorPanelExposure::TopLevel, "editor/analytics",
     "Top-level telemetry privacy, validation, and upload surface."},

    {"compat_report", "Compatibility Report", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Compat tab."},
    {"save_inspector", "Save Inspector", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Save tab."},
    {"event_authority", "Event Authority", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Event Authority tab."},
    {"message_inspector", "Message Inspector", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Message Text tab."},
    {"battle_inspector", "Battle Inspector", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Battle tab."},
    {"menu_inspector", "Menu Inspector", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Menu tab."},
    {"menu_preview", "Menu Preview", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Menu preview surface."},
    {"audio_inspector", "Audio Inspector", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Audio tab."},
    {"migration_wizard", "Migration Wizard", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Migration Wizard tab."},
    {"project_audit", "Project Audit", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Project Audit tab."},
    {"project_health", "Project Health", "Diagnostics", EditorPanelExposure::NestedWorkspace,
     "editor/diagnostics", "Rendered inside DiagnosticsWorkspace as the Project Health tab."},

    {"diagnostics_bundle", "Diagnostics Bundle", "Diagnostics", EditorPanelExposure::Internal,
     "editor/diagnostics", "Headless export panel used by tests and bundle tooling, not shell navigation."},
    {"event_authoring", "Event Authoring", "Gameplay", EditorPanelExposure::Internal, "editor/events",
     "Compiled authoring surface with direct panel tests; awaiting shell workflow wiring."},
    {"plugin_inspector", "Plugin Inspector", "Runtime", EditorPanelExposure::Internal, "editor/plugin",
     "Compiled compatibility inspector with direct panel tests; awaiting shell workflow wiring."},
    {"new_project_wizard", "New Project Wizard", "Project", EditorPanelExposure::Internal, "editor/project",
     "Project creation wizard is invoked by project flow, not persistent top-level navigation."},
    {"quest", "Quest", "Narrative", EditorPanelExposure::Internal, "editor/quest",
     "Compiled roadmap panel with snapshot tests; awaiting shell workflow wiring."},
    {"dialogue_graph", "Dialogue Graph", "Narrative", EditorPanelExposure::Internal, "editor/dialogue",
     "Compiled roadmap panel with snapshot tests; awaiting shell workflow wiring."},
    {"narrative_continuity", "Narrative Continuity", "Narrative", EditorPanelExposure::Internal,
     "editor/narrative", "Compiled roadmap panel with snapshot tests; awaiting shell workflow wiring."},
    {"relationship", "Relationship", "Narrative", EditorPanelExposure::Internal, "editor/relationship",
     "Compiled roadmap panel with snapshot tests; awaiting shell workflow wiring."},
    {"localization_workspace", "Localization Workspace", "Content", EditorPanelExposure::Internal,
     "editor/localization", "Compiled workspace panel; awaiting shell workflow wiring."},
    {"timeline", "Timeline", "Presentation", EditorPanelExposure::Internal, "editor/timeline",
     "Compiled timeline panel with direct tests; awaiting shell workflow wiring."},
    {"replay", "Replay", "Presentation", EditorPanelExposure::Internal, "editor/replay",
     "Compiled replay panel with direct tests; awaiting shell workflow wiring."},
    {"capture", "Capture", "Presentation", EditorPanelExposure::Internal, "editor/capture",
     "Compiled capture panel; awaiting shell workflow wiring."},
    {"photo_mode", "Photo Mode", "Presentation", EditorPanelExposure::Internal, "editor/presentation",
     "Compiled photo mode panel; awaiting shell workflow wiring."},
    {"database", "Database", "Content", EditorPanelExposure::Internal, "editor/database",
     "Compiled database panel with direct tests; awaiting shell workflow wiring."},
    {"balance", "Balance", "Gameplay", EditorPanelExposure::Internal, "editor/balance",
     "Compiled balance panel with direct tests; awaiting shell workflow wiring."},
    {"vendor", "Vendor", "Gameplay", EditorPanelExposure::Internal, "editor/shop",
     "Compiled vendor panel with direct tests; awaiting shell workflow wiring."},
    {"world", "World", "Worldbuilding", EditorPanelExposure::Internal, "editor/world",
     "Compiled world panel; awaiting shell workflow wiring."},
    {"crafting", "Crafting", "Gameplay", EditorPanelExposure::Internal, "editor/crafting",
     "Compiled crafting panel; awaiting shell workflow wiring."},
    {"codex", "Codex", "Content", EditorPanelExposure::Internal, "editor/codex",
     "Compiled codex panel; awaiting shell workflow wiring."},
    {"calendar", "Calendar", "Worldbuilding", EditorPanelExposure::Internal, "editor/time",
     "Compiled calendar panel; awaiting shell workflow wiring."},
    {"npc", "NPC", "Worldbuilding", EditorPanelExposure::Internal, "editor/npc",
     "Compiled NPC panel; awaiting shell workflow wiring."},
    {"puzzle", "Puzzle", "Gameplay", EditorPanelExposure::Internal, "editor/puzzle",
     "Compiled puzzle panel; awaiting shell workflow wiring."},
    {"export_diagnostics", "Export Diagnostics", "Release", EditorPanelExposure::Internal, "editor/export",
     "Compiled export validation panel with direct tests; awaiting shell workflow wiring."},
    {"character_creator", "Character Creator", "Gameplay", EditorPanelExposure::Internal, "editor/character",
     "Compiled character creator panel with direct tests; awaiting shell workflow wiring."},
    {"achievement", "Achievement", "Gameplay", EditorPanelExposure::Internal, "editor/achievement",
     "Compiled achievement panel with direct tests; awaiting shell workflow wiring."},
    {"controller_binding", "Controller Binding", "Input", EditorPanelExposure::Internal, "editor/action",
     "Compiled input binding panel with direct tests; awaiting shell workflow wiring."},
    {"save_debugger", "Save Debugger", "Diagnostics", EditorPanelExposure::Internal, "editor/save",
     "Compiled save debugger; awaiting shell workflow wiring."},
    {"save_migration_preview", "Save Migration Preview", "Diagnostics", EditorPanelExposure::Internal,
     "editor/save", "Compiled save migration preview; awaiting shell workflow wiring."},
    {"battle_presentation", "Battle Presentation", "Battle", EditorPanelExposure::Internal, "editor/battle",
     "Compiled battle presentation panel; awaiting shell workflow wiring."},
    {"boss_designer", "Boss Designer", "Battle", EditorPanelExposure::Internal, "editor/battle",
     "Compiled boss designer panel; awaiting shell workflow wiring."},
    {"formula_debugger", "Formula Debugger", "Battle", EditorPanelExposure::Internal, "editor/battle",
     "Compiled formula debugger; awaiting shell workflow wiring."},
    {"battle_preview", "Battle Preview", "Battle", EditorPanelExposure::Internal, "editor/battle",
     "Compiled battle preview panel with direct tests; awaiting shell workflow wiring."},
    {"perf_diagnostics", "Performance Diagnostics", "Diagnostics", EditorPanelExposure::Internal, "editor/perf",
     "Compiled performance diagnostics panel with direct tests; awaiting shell workflow wiring."},
    {"spatial_authoring", "Spatial Authoring", "Spatial", EditorPanelExposure::Internal, "editor/spatial",
     "Compiled spatial workspace; awaiting top-level shell workflow wiring."},
    {"elevation_brush", "Elevation Brush", "Spatial", EditorPanelExposure::NestedWorkspace, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"terrain_brush", "Terrain Brush", "Spatial", EditorPanelExposure::NestedWorkspace, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"region_rules", "Region Rules", "Spatial", EditorPanelExposure::NestedWorkspace, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"procedural_map", "Procedural Map", "Spatial", EditorPanelExposure::NestedWorkspace, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"prop_placement", "Prop Placement", "Spatial", EditorPanelExposure::NestedWorkspace, "editor/spatial",
     "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"map_ability_binding", "Map Ability Binding", "Spatial", EditorPanelExposure::NestedWorkspace,
     "editor/spatial", "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"spatial_ability_canvas", "Spatial Ability Canvas", "Spatial", EditorPanelExposure::NestedWorkspace,
     "editor/spatial", "Rendered through SpatialAuthoringWorkspace when spatial authoring is wired."},
    {"sprite_animation_preview", "Sprite Animation Preview", "Content", EditorPanelExposure::Internal,
     "editor/sprite", "Compiled sprite preview panel with direct tests; awaiting shell workflow wiring."},
    {"accessibility", "Accessibility", "Quality", EditorPanelExposure::Internal, "editor/accessibility",
     "Compiled accessibility panel with direct tests; awaiting shell workflow wiring."},
    {"accessibility_assistant", "Accessibility Assistant", "Quality", EditorPanelExposure::Internal,
     "editor/accessibility", "Compiled assistant panel; awaiting shell workflow wiring."},
    {"audio_mix", "Audio Mix", "Audio", EditorPanelExposure::Internal, "editor/audio",
     "Compiled audio mix panel; awaiting shell workflow wiring."},
    {"input_remap", "Input Remap", "Input", EditorPanelExposure::Internal, "editor/input",
     "Compiled remap panel; awaiting shell workflow wiring."},
    {"device_profile", "Device Profile", "Platform", EditorPanelExposure::Internal, "editor/platform",
     "Compiled device profile panel; awaiting shell workflow wiring."},
    {"theme_builder", "Theme Builder", "UI", EditorPanelExposure::Internal, "editor/ui",
     "Compiled theme builder panel; awaiting shell workflow wiring."},
    {"ai_assistant", "AI Assistant", "Productivity", EditorPanelExposure::Internal, "editor/ai",
     "Compiled AI assistant panel with direct tests; awaiting shell workflow wiring."},
    {"local_review", "Local Review", "Collaboration", EditorPanelExposure::Internal, "editor/collaboration",
     "Compiled review panel with direct tests; awaiting shell workflow wiring."},
    {"mod_sdk", "Mod SDK", "Runtime", EditorPanelExposure::Internal, "editor/mod",
     "Compiled SDK panel with direct tests; awaiting shell workflow wiring."},
    {"core_asset_browser", "Core Asset Browser", "Core Editor", EditorPanelExposure::Internal,
     "engine/core/editor/panels", "Legacy core editor panel API; not part of the current shell navigation."},
    {"core_hierarchy", "Core Hierarchy", "Core Editor", EditorPanelExposure::Internal,
     "engine/core/editor/panels", "Legacy core editor panel API; not part of the current shell navigation."},
    {"core_property_inspector", "Core Property Inspector", "Core Editor", EditorPanelExposure::Internal,
     "engine/core/editor/panels", "Legacy core editor panel API; not part of the current shell navigation."},
};

} // namespace

const std::vector<EditorPanelRegistryEntry>& editorPanelRegistry() {
    return kRegistry;
}

std::vector<EditorPanelRegistryEntry> topLevelEditorPanels() {
    std::vector<EditorPanelRegistryEntry> panels;
    for (const auto& entry : kRegistry) {
        if (entry.exposure == EditorPanelExposure::TopLevel) {
            panels.push_back(entry);
        }
    }
    return panels;
}

std::vector<std::string> requiredTopLevelPanelIds() {
    std::vector<std::string> ids;
    for (const auto& entry : kRegistry) {
        if (entry.exposure == EditorPanelExposure::TopLevel) {
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
        return entry.exposure == EditorPanelExposure::TopLevel || !entry.reason.empty();
    });
}

} // namespace urpg::editor
