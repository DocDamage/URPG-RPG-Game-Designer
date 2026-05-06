#include "editor/spatial/level_builder_workspace.h"

#include "engine/core/render/asset_loader.h"
#include "engine/core/map/grid_part_runtime_compiler.h"
#include "engine/core/map/grid_part_serializer.h"
#include "engine/core/map/grid_part_validator.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

namespace {

size_t countBlockingDiagnostics(const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    return static_cast<size_t>(std::count_if(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.severity == urpg::map::GridPartSeverity::Blocker ||
               diagnostic.severity == urpg::map::GridPartSeverity::Error;
    }));
}

void appendBlockingCodes(std::vector<std::string>& codes,
                         const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic.severity == urpg::map::GridPartSeverity::Blocker ||
            diagnostic.severity == urpg::map::GridPartSeverity::Error) {
            codes.push_back(diagnostic.code);
        }
    }
}

const char* severityName(urpg::map::GridPartSeverity severity) {
    switch (severity) {
    case urpg::map::GridPartSeverity::Info:
        return "info";
    case urpg::map::GridPartSeverity::Warning:
        return "warning";
    case urpg::map::GridPartSeverity::Error:
        return "error";
    case urpg::map::GridPartSeverity::Blocker:
        return "blocker";
    }
    return "error";
}

bool isBlocking(urpg::map::GridPartSeverity severity) {
    return severity == urpg::map::GridPartSeverity::Blocker || severity == urpg::map::GridPartSeverity::Error;
}

void appendDiagnosticSummaries(std::vector<LevelBuilderWorkspace::DiagnosticSummary>& target,
                               const std::string& source,
                               const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    for (const auto& diagnostic : diagnostics) {
        LevelBuilderWorkspace::DiagnosticSummary summary;
        summary.source = source;
        summary.severity = severityName(diagnostic.severity);
        summary.code = diagnostic.code;
        summary.message = diagnostic.message;
        summary.instance_id = diagnostic.instance_id;
        summary.part_id = diagnostic.part_id;
        summary.target = diagnostic.target;
        summary.x = diagnostic.x;
        summary.y = diagnostic.y;
        summary.blocking = isBlocking(diagnostic.severity);
        target.push_back(std::move(summary));
    }
}

#ifdef URPG_IMGUI_ENABLED
ImVec4 categoryColor(urpg::map::GridPartCategory category, bool selected) {
    if (selected) {
        return ImVec4(0.36f, 0.58f, 0.92f, 1.0f);
    }

    switch (category) {
    case urpg::map::GridPartCategory::Tile:
        return ImVec4(0.20f, 0.39f, 0.22f, 1.0f);
    case urpg::map::GridPartCategory::Wall:
    case urpg::map::GridPartCategory::LevelBlock:
        return ImVec4(0.34f, 0.34f, 0.38f, 1.0f);
    case urpg::map::GridPartCategory::Door:
        return ImVec4(0.45f, 0.30f, 0.18f, 1.0f);
    case urpg::map::GridPartCategory::Enemy:
    case urpg::map::GridPartCategory::Hazard:
        return ImVec4(0.55f, 0.18f, 0.18f, 1.0f);
    case urpg::map::GridPartCategory::Npc:
        return ImVec4(0.24f, 0.36f, 0.62f, 1.0f);
    case urpg::map::GridPartCategory::TreasureChest:
    case urpg::map::GridPartCategory::QuestItem:
        return ImVec4(0.55f, 0.43f, 0.15f, 1.0f);
    case urpg::map::GridPartCategory::SavePoint:
        return ImVec4(0.20f, 0.48f, 0.48f, 1.0f);
    case urpg::map::GridPartCategory::Trigger:
    case urpg::map::GridPartCategory::CutsceneZone:
    case urpg::map::GridPartCategory::Shop:
    case urpg::map::GridPartCategory::Prop:
    case urpg::map::GridPartCategory::Platform:
        return ImVec4(0.22f, 0.30f, 0.38f, 1.0f);
    }
    return ImVec4(0.22f, 0.30f, 0.38f, 1.0f);
}

ImVec4 hoverColor(ImVec4 color) {
    return ImVec4(std::min(color.x + 0.08f, 1.0f), std::min(color.y + 0.08f, 1.0f),
                  std::min(color.z + 0.08f, 1.0f), color.w);
}

ImVec4 palettePreviewColor(const std::string& category, bool selected) {
    if (category == "tile") {
        return categoryColor(urpg::map::GridPartCategory::Tile, selected);
    }
    if (category == "wall") {
        return categoryColor(urpg::map::GridPartCategory::Wall, selected);
    }
    if (category == "door") {
        return categoryColor(urpg::map::GridPartCategory::Door, selected);
    }
    if (category == "enemy") {
        return categoryColor(urpg::map::GridPartCategory::Enemy, selected);
    }
    if (category == "npc") {
        return categoryColor(urpg::map::GridPartCategory::Npc, selected);
    }
    if (category == "hazard") {
        return categoryColor(urpg::map::GridPartCategory::Hazard, selected);
    }
    if (category == "treasure_chest" || category == "quest_item") {
        return categoryColor(urpg::map::GridPartCategory::TreasureChest, selected);
    }
    if (category == "save_point") {
        return categoryColor(urpg::map::GridPartCategory::SavePoint, selected);
    }
    return categoryColor(urpg::map::GridPartCategory::Prop, selected);
}

char paletteGlyph(const GridPartPalettePanel::EntrySnapshot& entry) {
    if (entry.category == "tile") {
        return '.';
    }
    if (entry.category == "wall") {
        return '#';
    }
    if (entry.category == "door") {
        return 'D';
    }
    if (entry.category == "enemy") {
        return 'E';
    }
    if (entry.category == "npc") {
        return 'N';
    }
    if (entry.category == "hazard") {
        return '!';
    }
    if (entry.category == "treasure_chest") {
        return '$';
    }
    if (entry.category == "save_point") {
        return 'S';
    }
    if (!entry.part_id.empty()) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(entry.part_id.front())));
    }
    return '?';
}

std::shared_ptr<urpg::Texture> palettePreviewTexture(const std::string& path) {
    if (path.empty() || ImGui::GetIO().BackendRendererName == nullptr) {
        return nullptr;
    }

    static std::unordered_map<std::string, std::shared_ptr<urpg::Texture>> textures;
    const auto found = textures.find(path);
    if (found != textures.end()) {
        return found->second;
    }

    auto texture = urpg::AssetLoader::loadTexture(path);
    textures.emplace(path, texture);
    return texture;
}

bool renderPalettePreviewButton(const GridPartPalettePanel::EntrySnapshot& entry) {
    if (const auto texture = palettePreviewTexture(entry.preview_path); texture != nullptr && texture->getId() != 0) {
        const auto id = static_cast<ImTextureID>(texture->getId());
        return ImGui::ImageButton(("##preview_" + entry.part_id).c_str(), id, ImVec2(42.0f, 30.0f));
    }

    const auto color = palettePreviewColor(entry.category, entry.selected);
    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor(color));
    const std::string previewLabel = std::string(1, paletteGlyph(entry)) + "##preview_" + entry.part_id;
    const bool clicked = ImGui::Button(previewLabel.c_str(), ImVec2(42.0f, 30.0f));
    ImGui::PopStyleColor(2);
    return clicked;
}

const std::string* previewPathForPart(const urpg::map::PlacedPartInstance& part) {
    const auto found = part.properties.find("previewPath");
    if (found == part.properties.end() || found->second.empty()) {
        return nullptr;
    }
    return &found->second;
}

bool renderGridCellButton(const urpg::map::PlacedPartInstance* topPart, bool isSelected, const std::string& label,
                          float cell) {
    if (topPart != nullptr) {
        if (const auto* previewPath = previewPathForPart(*topPart); previewPath != nullptr) {
            if (const auto texture = palettePreviewTexture(*previewPath); texture != nullptr && texture->getId() != 0) {
                const auto tint = isSelected ? ImVec4(0.78f, 0.88f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                return ImGui::ImageButton(label.c_str(), static_cast<ImTextureID>(texture->getId()),
                                          ImVec2(cell - 4.0f, cell - 4.0f), ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f),
                                          ImVec4(0.08f, 0.09f, 0.10f, 1.0f), tint);
            }
        }
    }

    if (topPart != nullptr) {
        const auto color = categoryColor(topPart->category, isSelected);
        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor(color));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.14f, 0.15f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.18f, 0.23f, 0.27f, 1.0f));
    }
    const bool clicked = ImGui::Button(label.c_str(), ImVec2(cell, cell));
    ImGui::PopStyleColor(2);
    return clicked;
}

void renderReadinessFlag(const char* label, bool value) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextUnformatted(label);
    ImGui::TableSetColumnIndex(1);
    if (value) {
        ImGui::TextColored(ImVec4(0.35f, 0.85f, 0.55f, 1.0f), "pass");
    } else {
        ImGui::TextDisabled("missing");
    }
}

size_t categoryCount(const GridPartPalettePanel::RenderSnapshot& snapshot, const std::string& category) {
    const auto found = snapshot.category_counts.find(category);
    return found == snapshot.category_counts.end() ? 0 : found->second;
}

bool paletteCategoryButton(LevelBuilderWorkspace& workspace, const GridPartPalettePanel::RenderSnapshot& snapshot,
                           const char* label, const char* categoryName,
                           std::optional<urpg::map::GridPartCategory> category) {
    const bool active = snapshot.active_category == categoryName;
    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.42f, 0.70f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.31f, 0.52f, 0.84f, 1.0f));
    }
    const bool clicked = ImGui::Button(label);
    if (active) {
        ImGui::PopStyleColor(2);
    }
    if (clicked) {
        workspace.palettePanel().SetCategoryFilter(category);
        return true;
    }
    return false;
}

std::vector<std::pair<std::string, size_t>> sortedSourceCounts(const GridPartPalettePanel::RenderSnapshot& snapshot) {
    std::vector<std::pair<std::string, size_t>> sources(snapshot.source_counts.begin(), snapshot.source_counts.end());
    std::sort(sources.begin(), sources.end(), [](const auto& left, const auto& right) {
        if (left.second != right.second) {
            return left.second > right.second;
        }
        return left.first < right.first;
    });
    return sources;
}

void renderSourceFilter(LevelBuilderWorkspace& workspace, const GridPartPalettePanel::RenderSnapshot& snapshot) {
    const auto sources = sortedSourceCounts(snapshot);
    const std::string current = snapshot.active_source == "all" ? "All sources" : snapshot.active_source;
    ImGui::SetNextItemWidth(-1.0f);
    if (!ImGui::BeginCombo("##PaletteSource", current.c_str())) {
        return;
    }

    const bool allSelected = snapshot.active_source == "all";
    if (ImGui::Selectable("All sources", allSelected)) {
        workspace.palettePanel().SetSourceFilter("all");
    }
    if (allSelected) {
        ImGui::SetItemDefaultFocus();
    }
    for (const auto& [source, count] : sources) {
        const std::string label = source + " (" + std::to_string(count) + ")";
        const bool selected = snapshot.active_source == source;
        if (ImGui::Selectable(label.c_str(), selected)) {
            workspace.palettePanel().SetSourceFilter(source);
        }
        if (selected) {
            ImGui::SetItemDefaultFocus();
        }
    }
    ImGui::EndCombo();
}

void renderPaletteSnapshot(LevelBuilderWorkspace& workspace, const GridPartPalettePanel::RenderSnapshot& snapshot) {
    ImGui::TextUnformatted("Parts");
    ImGui::SameLine();
    ImGui::TextDisabled("%zu", snapshot.part_count);
    if (snapshot.entry_limit_reached) {
        ImGui::SameLine();
        ImGui::TextDisabled("showing first %zu", snapshot.entries.size());
    }
    if (snapshot.selected_part_id.empty()) {
        ImGui::TextDisabled("Choose a part, then paint on the grid.");
    } else {
        ImGui::Text("Painting: %s", snapshot.selected_part_id.c_str());
    }
    ImGui::Separator();

    static char searchBuffer[128] = {};
    if (snapshot.search_query.empty() && searchBuffer[0] != '\0') {
        searchBuffer[0] = '\0';
    }
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::InputTextWithHint("##PaletteSearch", "Search parts", searchBuffer, sizeof(searchBuffer))) {
        workspace.palettePanel().SetSearchQuery(searchBuffer);
    }
    renderSourceFilter(workspace, snapshot);

    paletteCategoryButton(workspace, snapshot, "All", "all", std::nullopt);
    ImGui::SameLine();
    paletteCategoryButton(workspace, snapshot, ("Tiles " + std::to_string(categoryCount(snapshot, "tile"))).c_str(),
                          "tile", urpg::map::GridPartCategory::Tile);
    ImGui::SameLine();
    paletteCategoryButton(workspace, snapshot, ("Walls " + std::to_string(categoryCount(snapshot, "wall"))).c_str(),
                          "wall", urpg::map::GridPartCategory::Wall);
    ImGui::SameLine();
    paletteCategoryButton(workspace, snapshot, ("Props " + std::to_string(categoryCount(snapshot, "prop"))).c_str(),
                          "prop", urpg::map::GridPartCategory::Prop);

    paletteCategoryButton(workspace, snapshot, "NPC", "npc", urpg::map::GridPartCategory::Npc);
    ImGui::SameLine();
    paletteCategoryButton(workspace, snapshot, "Enemy", "enemy", urpg::map::GridPartCategory::Enemy);
    ImGui::SameLine();
    paletteCategoryButton(workspace, snapshot, "Door", "door", urpg::map::GridPartCategory::Door);
    ImGui::SameLine();
    paletteCategoryButton(workspace, snapshot, "Hazard", "hazard", urpg::map::GridPartCategory::Hazard);

    ImGui::Separator();

    if (!ImGui::BeginTable("LevelBuilderPalette", 3,
                           ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                           ImVec2(0.0f, 0.0f))) {
        return;
    }
    ImGui::TableSetupColumn("Preview", ImGuiTableColumnFlags_WidthFixed, 54.0f);
    ImGui::TableSetupColumn("Name");
    ImGui::TableSetupColumn("Category");
    ImGui::TableHeadersRow();
    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(snapshot.entries.size()));
    while (clipper.Step()) {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row) {
            const auto& entry = snapshot.entries[static_cast<size_t>(row)];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            if (renderPalettePreviewButton(entry)) {
                workspace.SelectGridPart(entry.part_id);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("asset=%s\npreview=%s\nsource=%s\nrect=%d,%d %dx%d\ntile=%d",
                                  entry.asset_id.empty() ? "-" : entry.asset_id.c_str(),
                                  entry.preview_path.empty() ? "-" : entry.preview_path.c_str(),
                                  entry.source_image_path.empty() ? "-" : entry.source_image_path.c_str(),
                                  entry.atlas_x, entry.atlas_y, entry.atlas_width, entry.atlas_height, entry.tile_id);
            }
            ImGui::TableSetColumnIndex(1);
            if (entry.selected) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.58f, 0.75f, 1.0f, 1.0f));
            }
            if (ImGui::Selectable(entry.display_name.c_str(), entry.selected, ImGuiSelectableFlags_SpanAllColumns)) {
                workspace.SelectGridPart(entry.part_id);
            }
            if (entry.selected) {
                ImGui::PopStyleColor();
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", entry.part_id.c_str());
            }
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(entry.category.c_str());
        }
    }
    ImGui::EndTable();
}

char gridCellGlyph(const urpg::map::PlacedPartInstance& part) {
    if (part.category == urpg::map::GridPartCategory::Tile) {
        return '.';
    }
    if (part.category == urpg::map::GridPartCategory::Wall) {
        return '#';
    }
    if (part.category == urpg::map::GridPartCategory::Door) {
        return 'D';
    }
    if (part.category == urpg::map::GridPartCategory::Enemy) {
        return 'E';
    }
    if (part.category == urpg::map::GridPartCategory::Npc) {
        return 'N';
    }
    if (part.category == urpg::map::GridPartCategory::TreasureChest) {
        return '$';
    }
    if (part.category == urpg::map::GridPartCategory::SavePoint) {
        return 'S';
    }
    if (part.category == urpg::map::GridPartCategory::Hazard) {
        return '!';
    }
    if (!part.part_id.empty()) {
        return static_cast<char>(std::toupper(static_cast<unsigned char>(part.part_id.front())));
    }
    return '?';
}

void renderMakerGrid(LevelBuilderWorkspace& workspace, urpg::map::GridPartDocument* document,
                     const LevelBuilderWorkspace::RenderSnapshot& snapshot) {
    if (document == nullptr) {
        ImGui::TextDisabled("No level document is bound.");
        return;
    }

    ImGui::Text("%s", document->mapId().c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("%dx%d  %zu parts", document->width(), document->height(), document->parts().size());
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 118.0f);
    if (ImGui::Button("Undo")) {
        workspace.UndoLastEdit();
    }
    ImGui::SameLine();
    if (ImGui::Button("Redo")) {
        workspace.RedoLastEdit();
    }

    ImGui::Separator();

    const float cell = std::clamp((ImGui::GetContentRegionAvail().x - 24.0f) / static_cast<float>(document->width()),
                                  28.0f, 44.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(1.0f, 1.0f));
    for (int32_t y = 0; y < document->height(); ++y) {
        for (int32_t x = 0; x < document->width(); ++x) {
            const auto parts = document->partsAt(x, y);
            const auto* topPart = parts.empty() ? nullptr : parts.back();
            const bool isSelected = topPart != nullptr && topPart->instance_id == snapshot.inspector.selected_instance_id;
            std::string label;
            if (topPart == nullptr) {
                label = " ";
            } else {
                label = std::string(1, gridCellGlyph(*topPart));
            }
            label += "##cell_" + std::to_string(x) + "_" + std::to_string(y);

            bool mutated = false;
            bool erased = false;
            if (renderGridCellButton(topPart, isSelected, label, cell)) {
                const auto selectedPart = workspace.palettePanel().selectedPartId();
                if (!selectedPart.empty() && parts.empty()) {
                    const bool placed = workspace.placementPanel().PlaceSelectedPartAtGrid(x, y);
                    if (placed && !document->parts().empty()) {
                        (void)workspace.inspectorPanel().SelectInstance(document->parts().back().instance_id);
                        mutated = true;
                    }
                } else if (topPart != nullptr) {
                    (void)workspace.inspectorPanel().SelectInstance(topPart->instance_id);
                }
            }
            const bool hovered = ImGui::IsItemHovered();
            if (!mutated && hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                const auto selectedPart = workspace.palettePanel().selectedPartId();
                if (!selectedPart.empty() && topPart == nullptr) {
                    const bool placed = workspace.placementPanel().PlaceSelectedPartAtGrid(x, y);
                    if (placed && !document->parts().empty()) {
                        (void)workspace.inspectorPanel().SelectInstance(document->parts().back().instance_id);
                    }
                }
            }
            if (topPart != nullptr && hovered &&
                (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsMouseDown(ImGuiMouseButton_Right))) {
                erased = workspace.placementPanel().RemoveTopPartAtGrid(x, y);
            }
            if (hovered) {
                if (erased) {
                    ImGui::SetTooltip("(%d,%d) erased", x, y);
                } else if (topPart == nullptr) {
                    ImGui::SetTooltip("(%d,%d) empty\nClick or drag: paint selected part", x, y);
                } else {
                    ImGui::SetTooltip("(%d,%d) %s\nClick: inspect\nRight-click or right-drag: erase", x, y,
                                      topPart->part_id.c_str());
                }
            }

            if (x + 1 < document->width()) {
                ImGui::SameLine();
            }
        }
    }
    ImGui::PopStyleVar();

    if (snapshot.palette.selected_part_id.empty()) {
        ImGui::TextDisabled("Pick a part from the palette. Click or drag empty cells to paint. Right-click to erase.");
    } else {
        ImGui::Text("Painting %s. Click-drag to paint empty cells; right-drag to erase.",
                    snapshot.palette.selected_part_id.c_str());
    }
}

bool workspaceActionButton(LevelBuilderWorkspace& workspace, const char* label, const char* actionId, bool enabled,
                           bool primary = false) {
    if (!enabled) {
        ImGui::BeginDisabled();
    }
    if (primary) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.42f, 0.70f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.31f, 0.52f, 0.84f, 1.0f));
    }
    const bool clicked = ImGui::Button(label);
    if (primary) {
        ImGui::PopStyleColor(2);
    }
    if (!enabled) {
        ImGui::EndDisabled();
    }
    if (clicked) {
        return workspace.ActivateToolbarAction(actionId);
    }
    return false;
}

void renderLevelBuilderWindow(LevelBuilderWorkspace& workspace, urpg::map::GridPartDocument* document,
                              const LevelBuilderWorkspace::RenderSnapshot& snapshot) {
    const auto& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(96.0f, 56.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(std::max(900.0f, io.DisplaySize.x - 128.0f),
                                    std::max(660.0f, io.DisplaySize.y - 88.0f)),
                             ImGuiCond_Always);

    if (!ImGui::Begin("Level Builder - Maker Workspace")) {
        ImGui::End();
        return;
    }

    ImGui::TextUnformatted("Level Builder");
    ImGui::SameLine();
    ImGui::TextDisabled("%s  %s", snapshot.status.c_str(), snapshot.has_unsaved_changes ? "unsaved" : "saved");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 320.0f);
    workspaceActionButton(workspace, "Playtest", "playtest_start", snapshot.can_playtest, true);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Validate", "validate", true);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Package", "package", true);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Save", "save_level_draft", snapshot.has_document);

    ImGui::TextDisabled("%s", snapshot.message.c_str());
    workspaceActionButton(workspace, "Build", "build", true, snapshot.active_mode == "build");
    ImGui::SameLine();
    workspaceActionButton(workspace, "Set Spawn", "mark_player_spawn", snapshot.inspector.has_selection);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Set Objective", "set_reach_exit_objective", snapshot.inspector.has_selection);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Export Checks", "mark_target_export_checks_passed", true);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Accessibility", "mark_accessibility_checks_passed", true);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Performance", "mark_performance_budget_passed", true);
    ImGui::SameLine();
    workspaceActionButton(workspace, "Review", "mark_human_review_passed", true);
    ImGui::NewLine();
    ImGui::Separator();

    if (ImGui::BeginTable("MakerWorkspace", 3, ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("Palette", ImGuiTableColumnFlags_WidthFixed, 260.0f);
        ImGui::TableSetupColumn("Canvas", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Inspector", ImGuiTableColumnFlags_WidthFixed, 300.0f);
        ImGui::TableNextRow();

        ImGui::TableSetColumnIndex(0);
        ImGui::BeginChild("MakerPalette", ImVec2(0.0f, 0.0f), false);
        renderPaletteSnapshot(workspace, snapshot.palette);
        ImGui::EndChild();

        ImGui::TableSetColumnIndex(1);
        ImGui::BeginChild("MakerCanvas", ImVec2(0.0f, 0.0f), false);
        renderMakerGrid(workspace, document, snapshot);
        ImGui::EndChild();

        ImGui::TableSetColumnIndex(2);
        ImGui::BeginChild("MakerInspector", ImVec2(0.0f, 0.0f), false);
        const auto& inspector = snapshot.inspector;
        ImGui::TextUnformatted("Inspector");
        ImGui::Separator();
        ImGui::TextWrapped("%s", inspector.has_selection ? inspector.selected_instance_id.c_str() : "No part selected.");
        if (inspector.has_selection) {
            ImGui::Text("Part: %s", inspector.selected_part_id.c_str());
            ImGui::Text("Position: %d,%d", inspector.grid_x, inspector.grid_y);
            ImGui::Text("Size: %dx%d", inspector.width, inspector.height);
            if (ImGui::Button("Mark Spawn")) {
                workspace.MarkSelectedInstanceAsPlayerSpawn();
            }
            ImGui::SameLine();
            if (ImGui::Button("Set Exit")) {
                workspace.SetSelectedInstanceAsReachExitObjective();
            }
            if (inspector.properties.empty()) {
                ImGui::TextDisabled("No custom properties.");
            } else {
                for (const auto& [key, value] : inspector.properties) {
                    ImGui::BulletText("%s = %s", key.c_str(), value.c_str());
                }
            }
        }
        ImGui::Separator();
        if (ImGui::BeginTabBar("MakerDetails")) {
            if (ImGui::BeginTabItem("Build")) {
            const auto& placement = snapshot.placement;
            ImGui::Text("Placement target: document=%s catalog=%s overlay=%s", placement.has_document ? "yes" : "no",
                        placement.has_catalog ? "yes" : "no", placement.has_spatial_overlay ? "yes" : "no");
            ImGui::Text("Placed: %zu  Diagnostics: %zu", placement.placed_count, placement.diagnostic_count);
            ImGui::Text("Hover: %s (%d,%d) %s", placement.hover_active ? "active" : "idle", placement.hover_x,
                        placement.hover_y, placement.hover_reason.c_str());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Validate")) {
            ImGui::Text("Document: %s", snapshot.validation.document_ok ? "ok" : "needs work");
            ImGui::Text("Ruleset: %s", snapshot.validation.ruleset_ok ? "ok" : "needs work");
            ImGui::Text("Objective: %s", snapshot.validation.objective_ok ? "ok" : "needs work");
            ImGui::Text("Diagnostics: %zu blocking %zu", snapshot.validation.diagnostic_count,
                        snapshot.validation.blocking_count);
            if (ImGui::BeginTable("LevelDiagnostics", 5,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
                                      ImGuiTableFlags_ScrollY,
                                  ImVec2(0.0f, 220.0f))) {
                ImGui::TableSetupColumn("Severity");
                ImGui::TableSetupColumn("Code");
                ImGui::TableSetupColumn("Message");
                ImGui::TableSetupColumn("Part");
                ImGui::TableSetupColumn("Target");
                ImGui::TableHeadersRow();
                for (const auto& diagnostic : snapshot.diagnostics) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(diagnostic.severity.c_str());
                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(diagnostic.code.c_str());
                    ImGui::TableSetColumnIndex(2);
                    ImGui::TextWrapped("%s", diagnostic.message.c_str());
                    ImGui::TableSetColumnIndex(3);
                    ImGui::TextUnformatted(diagnostic.part_id.c_str());
                    ImGui::TableSetColumnIndex(4);
                    ImGui::TextUnformatted(diagnostic.target.c_str());
                }
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Playtest")) {
            const auto& playtest = snapshot.playtest;
            ImGui::Text("Can launch: %s", playtest.can_launch ? "yes" : "no");
            ImGui::Text("Running: %s", playtest.running ? "yes" : "no");
            ImGui::Text("Returned to editor: %s", playtest.returned_to_editor ? "yes" : "no");
            ImGui::Separator();
            ImGui::Text("Objective complete: %s", playtest.latest_result.completed_objective ? "yes" : "no");
            ImGui::Text("Softlocked: %s", playtest.latest_result.softlocked ? "yes" : "no");
            ImGui::Text("Visited: %zu", playtest.latest_result.visited_instance_ids.size());
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Package")) {
            ImGui::Text("Readiness: %s", snapshot.package.readiness.c_str());
            ImGui::Text("Can export: %s", snapshot.package.can_export ? "yes" : "no");
            ImGui::Text("Can publish: %s", snapshot.package.can_publish ? "yes" : "no");
            ImGui::Text("Dependencies: %zu", snapshot.package.dependency_count);
            if (ImGui::BeginTable("LevelReadiness", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                renderReadinessFlag("Player spawn", snapshot.readiness_evidence.has_player_spawn);
                renderReadinessFlag("Objective", snapshot.readiness_evidence.has_objective);
                renderReadinessFlag("Reachability", snapshot.readiness_evidence.reachability_passed);
                renderReadinessFlag("Export checks", snapshot.readiness_evidence.target_export_checks_passed);
                renderReadinessFlag("Accessibility", snapshot.readiness_evidence.accessibility_checks_passed);
                renderReadinessFlag("Performance", snapshot.readiness_evidence.performance_budget_passed);
                renderReadinessFlag("Human review", !snapshot.readiness_evidence.human_review_required ||
                                                        snapshot.readiness_evidence.human_review_passed);
                ImGui::EndTable();
            }
            ImGui::EndTabItem();
        }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();
        ImGui::EndTable();
    }

    ImGui::End();
}
#endif

} // namespace

const char* LevelBuilderWorkspace::modeName(WorkflowMode mode) {
    switch (mode) {
    case WorkflowMode::Build:
        return "build";
    case WorkflowMode::Validate:
        return "validate";
    case WorkflowMode::Playtest:
        return "playtest";
    case WorkflowMode::Package:
        return "package";
    case WorkflowMode::SupportingSpatial:
        return "supporting_spatial";
    }
    return "build";
}

const char* LevelBuilderWorkspace::readinessName(urpg::map::GridPartReadinessLevel readiness) {
    switch (readiness) {
    case urpg::map::GridPartReadinessLevel::Draft:
        return "draft";
    case urpg::map::GridPartReadinessLevel::Playable:
        return "playable";
    case urpg::map::GridPartReadinessLevel::Validated:
        return "validated";
    case urpg::map::GridPartReadinessLevel::Publishable:
        return "publishable";
    case urpg::map::GridPartReadinessLevel::Exportable:
        return "exportable";
    case urpg::map::GridPartReadinessLevel::Certified:
        return "certified";
    }
    return "draft";
}

void LevelBuilderWorkspace::syncPanelVisibility() {
    const bool show_build = active_mode_ == WorkflowMode::Build;
    palette_panel_.SetVisible(show_build);
    placement_panel_.SetVisible(show_build);
    inspector_panel_.SetVisible(show_build || active_mode_ == WorkflowMode::Validate ||
                                active_mode_ == WorkflowMode::Package);
    playtest_panel_.SetVisible(active_mode_ == WorkflowMode::Playtest);
    supporting_spatial_workspace_.SetVisible(active_mode_ == WorkflowMode::SupportingSpatial);
}

void LevelBuilderWorkspace::Render(const urpg::FrameContext& context) {
    if (!m_visible) {
        return;
    }

    syncPanelVisibility();
    palette_panel_.Render(context);
    placement_panel_.Render(context);
    inspector_panel_.Render(context);
    playtest_panel_.Render(context);
    supporting_spatial_workspace_.Render(context);
    captureRenderSnapshot();
#ifdef URPG_IMGUI_ENABLED
    const auto stable_snapshot = last_render_snapshot_;
    renderLevelBuilderWindow(*this, document_, stable_snapshot);
#endif
}

void LevelBuilderWorkspace::SetTargets(urpg::map::GridPartDocument* document,
                                       const urpg::map::GridPartCatalog* catalog,
                                       urpg::presentation::SpatialMapOverlay* overlay,
                                       urpg::scene::MapScene* scene) {
    document_ = document;
    catalog_ = catalog;
    overlay_ = overlay;
    scene_ = scene;

    palette_panel_.SetCatalog(catalog);
    placement_panel_.SetTargets(document, catalog, overlay);
    inspector_panel_.SetTargets(document, catalog);
    playtest_panel_.SetTargets(document, catalog);
    supporting_spatial_workspace_.SetTargets(scene, overlay);
    supporting_spatial_workspace_.SetGridPartTargets(document, catalog);
    syncPanelVisibility();
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetProjectionSettings(const PropPlacementPanel::ScreenProjectionSettings& settings) {
    placement_panel_.SetProjectionSettings(settings);
    supporting_spatial_workspace_.SetProjectionSettings(settings);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetRulesetProfile(urpg::map::GridRulesetProfile ruleset) {
    ruleset_ = std::move(ruleset);
    playtest_panel_.SetRulesetProfile(ruleset_);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetObjective(urpg::map::MapObjective objective) {
    objective_ = std::move(objective);
    playtest_panel_.SetObjective(objective_);
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetPackageManifest(urpg::map::GridPartPackageManifest manifest) {
    package_manifest_ = std::move(manifest);
    has_custom_package_manifest_ = true;
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetReadinessEvidence(urpg::map::GridPartReadinessEvidence evidence) {
    readiness_evidence_ = evidence;
    captureRenderSnapshot();
}

void LevelBuilderWorkspace::SetActiveMode(WorkflowMode mode) {
    if (active_mode_ == mode) {
        return;
    }
    active_mode_ = mode;
    syncPanelVisibility();
    captureRenderSnapshot();
}

bool LevelBuilderWorkspace::ActivateToolbarAction(const std::string& action_id) {
    if (action_id == "build") {
        SetActiveMode(WorkflowMode::Build);
        return true;
    }
    if (action_id == "validate") {
        SetActiveMode(WorkflowMode::Validate);
        return true;
    }
    if (action_id == "playtest") {
        SetActiveMode(WorkflowMode::Playtest);
        return true;
    }
    if (action_id == "package") {
        SetActiveMode(WorkflowMode::Package);
        return true;
    }
    if (action_id == "undo") {
        return UndoLastEdit().success;
    }
    if (action_id == "redo") {
        return RedoLastEdit().success;
    }
    if (action_id == "mark_player_spawn") {
        return MarkSelectedInstanceAsPlayerSpawn().success;
    }
    if (action_id == "set_reach_exit_objective") {
        return SetSelectedInstanceAsReachExitObjective().success;
    }
    if (action_id == "mark_target_export_checks_passed") {
        return MarkTargetExportChecksPassed().success;
    }
    if (action_id == "mark_accessibility_checks_passed") {
        return MarkAccessibilityChecksPassed().success;
    }
    if (action_id == "mark_performance_budget_passed") {
        return MarkPerformanceBudgetPassed().success;
    }
    if (action_id == "mark_human_review_passed") {
        return MarkHumanReviewPassed().success;
    }
    if (action_id == "save_level_draft") {
        return SaveLevelDraft().success;
    }
    if (action_id == "export_current_level") {
        return ExportCurrentLevel().success;
    }
    if (action_id == "supporting_spatial") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return true;
    }
    if (action_id == "supporting_elevation") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("elevation");
    }
    if (action_id == "supporting_props") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("props");
    }
    if (action_id == "supporting_abilities") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("abilities");
    }
    if (action_id == "supporting_composite") {
        SetActiveMode(WorkflowMode::SupportingSpatial);
        return supporting_spatial_workspace_.ActivateToolbarAction("composite");
    }
    if (action_id == "playtest_start") {
        SetActiveMode(WorkflowMode::Playtest);
        const bool launched = playtest_panel_.PlaytestFromStart();
        const auto& playtest_snapshot = playtest_panel_.lastRenderSnapshot();
        if (launched && playtest_snapshot.latest_result.completed_objective &&
            !playtest_snapshot.latest_result.softlocked) {
            readiness_evidence_.reachability_passed = true;
        }
        captureRenderSnapshot();
        return launched;
    }
    if (action_id == "playtest_return") {
        const bool returned = playtest_panel_.ReturnToEditor();
        captureRenderSnapshot();
        return returned;
    }
    return false;
}

LevelBuilderWorkspace::EditHistoryResult LevelBuilderWorkspace::UndoLastEdit() {
    EditHistoryResult result;
    result.command_id = "undo";
    result.message = "No edit history is available to undo.";

    if (inspector_panel_.lastRenderSnapshot().can_undo) {
        result.success = inspector_panel_.Undo();
        result.source = "inspector";
    } else if (placement_panel_.lastRenderSnapshot().can_undo) {
        result.success = placement_panel_.Undo();
        result.source = "placement";
    }

    if (result.success) {
        result.message = "Undo applied.";
    }
    last_edit_history_result_ = result;
    captureRenderSnapshot();
    return last_edit_history_result_;
}

LevelBuilderWorkspace::EditHistoryResult LevelBuilderWorkspace::RedoLastEdit() {
    EditHistoryResult result;
    result.command_id = "redo";
    result.message = "No edit history is available to redo.";

    if (inspector_panel_.lastRenderSnapshot().can_redo) {
        result.success = inspector_panel_.Redo();
        result.source = "inspector";
    } else if (placement_panel_.lastRenderSnapshot().can_redo) {
        result.success = placement_panel_.Redo();
        result.source = "placement";
    }

    if (result.success) {
        result.message = "Redo applied.";
    }
    last_edit_history_result_ = result;
    captureRenderSnapshot();
    return last_edit_history_result_;
}

LevelBuilderWorkspace::FocusDiagnosticResult LevelBuilderWorkspace::FocusDiagnostic(size_t diagnostic_index) {
    FocusDiagnosticResult result;
    result.diagnostic_index = diagnostic_index;

    captureRenderSnapshot();
    if (diagnostic_index >= last_render_snapshot_.diagnostics.size()) {
        result.message = "Diagnostic index is out of range.";
        last_focus_diagnostic_result_ = result;
        captureRenderSnapshot();
        return last_focus_diagnostic_result_;
    }

    const auto& diagnostic = last_render_snapshot_.diagnostics[diagnostic_index];
    result.source = diagnostic.source;
    result.instance_id = diagnostic.instance_id;
    if (diagnostic.instance_id.empty()) {
        result.message = "Diagnostic has no instance target to focus.";
        last_focus_diagnostic_result_ = result;
        captureRenderSnapshot();
        return last_focus_diagnostic_result_;
    }

    if (!inspector_panel_.SelectInstance(diagnostic.instance_id)) {
        result.message = "Diagnostic target instance could not be selected.";
        last_focus_diagnostic_result_ = result;
        captureRenderSnapshot();
        return last_focus_diagnostic_result_;
    }

    result.success = true;
    result.message = "Diagnostic target selected.";
    SetActiveMode(WorkflowMode::Validate);
    last_focus_diagnostic_result_ = result;
    captureRenderSnapshot();
    return last_focus_diagnostic_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkSelectedInstanceAsPlayerSpawn() {
    AuthoringCommandResult result;
    result.command_id = "mark_player_spawn";

    const auto selected_instance_id = inspector_panel_.lastRenderSnapshot().selected_instance_id;
    result.instance_id = selected_instance_id;
    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before marking a spawn.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }
    if (selected_instance_id.empty() || document_->findPart(selected_instance_id) == nullptr) {
        result.message = "Select a placed grid part before marking a player spawn.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }

    if (!inspector_panel_.SetProperty("role", "player_spawn")) {
        result.message = "Selected part could not be marked as a player spawn.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }

    readiness_evidence_.has_player_spawn = true;
    result.success = true;
    result.message = "Selected part marked as player spawn.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult
LevelBuilderWorkspace::SetSelectedInstanceAsReachExitObjective(const std::string& objective_id) {
    AuthoringCommandResult result;
    result.command_id = "set_reach_exit_objective";

    const auto selected_instance_id = inspector_panel_.lastRenderSnapshot().selected_instance_id;
    result.instance_id = selected_instance_id;
    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before setting an objective.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }
    if (selected_instance_id.empty() || document_->findPart(selected_instance_id) == nullptr) {
        result.message = "Select a placed grid part before setting the objective.";
        last_authoring_command_result_ = result;
        captureRenderSnapshot();
        return last_authoring_command_result_;
    }

    urpg::map::MapObjective objective;
    objective.type = urpg::map::MapObjectiveType::ReachExit;
    objective.objective_id = objective_id.empty() ? "reach_exit" : objective_id;
    objective.target_instance_id = selected_instance_id;
    SetObjective(std::move(objective));
    readiness_evidence_.has_objective = true;
    (void)inspector_panel_.SetProperty("role", "exit");

    result.success = true;
    result.message = "Selected part set as reach-exit objective.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkTargetExportChecksPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_target_export_checks_passed";
    readiness_evidence_.target_export_checks_passed = true;
    result.success = true;
    result.message = "Target export checks marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkAccessibilityChecksPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_accessibility_checks_passed";
    readiness_evidence_.accessibility_checks_passed = true;
    result.success = true;
    result.message = "Accessibility checks marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkPerformanceBudgetPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_performance_budget_passed";
    readiness_evidence_.performance_budget_passed = true;
    result.success = true;
    result.message = "Performance budget marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::AuthoringCommandResult LevelBuilderWorkspace::MarkHumanReviewPassed() {
    AuthoringCommandResult result;
    result.command_id = "mark_human_review_passed";
    readiness_evidence_.human_review_required = true;
    readiness_evidence_.human_review_passed = true;
    result.success = true;
    result.message = "Human review marked as passed.";
    last_authoring_command_result_ = result;
    captureRenderSnapshot();
    return last_authoring_command_result_;
}

LevelBuilderWorkspace::SaveDraftResult LevelBuilderWorkspace::SaveLevelDraft() {
    SaveDraftResult result;

    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before saving.";
        last_save_result_ = result;
        captureRenderSnapshot();
        return last_save_result_;
    }

    result.map_id = document_->mapId();
    result.saved_part_count = document_->parts().size();
    const auto document_validation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
    result.diagnostic_count = document_validation.diagnostics.size();
    appendBlockingCodes(result.blocker_codes, document_validation.diagnostics);

    if (!result.blocker_codes.empty()) {
        result.message = "Current level draft has blocking document errors.";
        last_save_result_ = std::move(result);
        captureRenderSnapshot();
        return last_save_result_;
    }

    result.success = true;
    result.message = "Current level draft saved.";
    result.serialized_document_json = urpg::map::GridPartDocumentToJson(*document_).dump(2);
    document_->clearDirtyChunks();
    last_save_result_ = std::move(result);
    captureRenderSnapshot();
    return last_save_result_;
}

LevelBuilderWorkspace::LoadDraftResult
LevelBuilderWorkspace::LoadLevelDraft(const std::string& serialized_document_json) {
    LoadDraftResult result;

    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before loading.";
        last_load_result_ = result;
        captureRenderSnapshot();
        return last_load_result_;
    }

    nlohmann::json json;
    try {
        json = nlohmann::json::parse(serialized_document_json);
    } catch (const nlohmann::json::exception&) {
        result.message = "Level draft JSON could not be parsed.";
        result.blocker_codes.push_back("draft_json_parse_failed");
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    auto parsed = urpg::map::GridPartDocumentFromJson(json);
    if (!parsed.has_value()) {
        result.message = "Level draft JSON is not a valid grid-part document.";
        result.blocker_codes.push_back("draft_document_invalid");
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    result.map_id = parsed->mapId();
    if (!document_->mapId().empty() && parsed->mapId() != document_->mapId()) {
        result.message = "Level draft map id does not match the bound document.";
        result.blocker_codes.push_back("draft_map_id_mismatch");
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    const auto validation = urpg::map::ValidateGridPartDocument(*parsed, *catalog_);
    result.diagnostic_count = validation.diagnostics.size();
    appendBlockingCodes(result.blocker_codes, validation.diagnostics);
    if (!result.blocker_codes.empty()) {
        result.message = "Level draft has blocking document errors.";
        last_load_result_ = std::move(result);
        captureRenderSnapshot();
        return last_load_result_;
    }

    result.success = true;
    result.message = "Level draft loaded.";
    result.loaded_part_count = parsed->parts().size();
    *document_ = std::move(*parsed);
    placement_panel_.SetTargets(document_, catalog_, overlay_);
    inspector_panel_.SetTargets(document_, catalog_);
    playtest_panel_.SetTargets(document_, catalog_);
    supporting_spatial_workspace_.SetGridPartTargets(document_, catalog_);
    last_load_result_ = std::move(result);
    SetActiveMode(WorkflowMode::Build);
    captureRenderSnapshot();
    return last_load_result_;
}

LevelBuilderWorkspace::ExportResult LevelBuilderWorkspace::ExportCurrentLevel() {
    ExportResult result;

    if (document_ == nullptr || catalog_ == nullptr) {
        result.message = "Bind a GridPartDocument and GridPartCatalog before exporting.";
        last_export_result_ = result;
        captureRenderSnapshot();
        return last_export_result_;
    }

    result.map_id = document_->mapId();
    const auto document_validation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
    const auto ruleset_validation = urpg::map::ValidateGridPartRuleset(*document_, *catalog_, ruleset_);
    const auto objective_validation = urpg::map::ValidateMapObjective(*document_, *catalog_, objective_);
    const auto manifest = has_custom_package_manifest_ ? package_manifest_
                                                       : urpg::map::BuildGridPartPackageManifest(*document_, *catalog_);
    const auto package = urpg::map::ValidateGridPartPackageGovernance(*document_, *catalog_, manifest,
                                                                      readiness_evidence_);

    result.readiness = readinessName(package.readiness);
    result.diagnostic_count = document_validation.diagnostics.size() + ruleset_validation.diagnostics.size() +
                              objective_validation.diagnostics.size() + package.diagnostics.size();
    appendBlockingCodes(result.blocker_codes, document_validation.diagnostics);
    appendBlockingCodes(result.blocker_codes, ruleset_validation.diagnostics);
    appendBlockingCodes(result.blocker_codes, objective_validation.diagnostics);
    appendBlockingCodes(result.blocker_codes, package.diagnostics);

    if (!result.blocker_codes.empty() || !package.canExport()) {
        result.message = "Current level is not exportable.";
        last_export_result_ = std::move(result);
        SetActiveMode(WorkflowMode::Package);
        captureRenderSnapshot();
        return last_export_result_;
    }

    result.success = true;
    result.message = "Current level is exportable.";
    result.serialized_document_json = urpg::map::GridPartDocumentToJson(*document_).dump(2);
    last_export_result_ = std::move(result);
    SetActiveMode(WorkflowMode::Package);
    captureRenderSnapshot();
    return last_export_result_;
}

bool LevelBuilderWorkspace::SelectGridPart(const std::string& part_id) {
    const bool palette_selected = palette_panel_.SelectPart(part_id);
    const bool placement_selected = placement_panel_.SetSelectedPartId(part_id);
    const bool supporting_selected = supporting_spatial_workspace_.SelectGridPart(part_id);
    captureRenderSnapshot();
    return palette_selected && placement_selected && supporting_selected;
}

bool LevelBuilderWorkspace::SelectAssetBrowserRecord(const nlohmann::json& asset_record) {
    if (!asset_record.is_object()) {
        captureRenderSnapshot();
        return false;
    }
    const auto partId = asset_record.value("stableId", "");
    if (partId.empty()) {
        captureRenderSnapshot();
        return false;
    }
    return SelectGridPart(partId);
}

bool LevelBuilderWorkspace::RouteCanvasHover(float screen_x, float screen_y) {
    if (active_mode_ == WorkflowMode::SupportingSpatial) {
        const bool handled = supporting_spatial_workspace_.RouteCanvasHover(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool handled = placement_panel_.HoverSelectedPartFromScreen(screen_x, screen_y);
    captureRenderSnapshot();
    return handled;
}

bool LevelBuilderWorkspace::RouteCanvasPrimaryAction(float screen_x, float screen_y) {
    if (active_mode_ == WorkflowMode::SupportingSpatial) {
        const bool handled = supporting_spatial_workspace_.RouteCanvasPrimaryAction(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool placed = placement_panel_.PlaceSelectedPartFromScreen(screen_x, screen_y);
    if (placed && document_ != nullptr) {
        const auto& parts = document_->parts();
        if (!parts.empty()) {
            (void)inspector_panel_.SelectInstance(parts.back().instance_id);
        }
    }
    captureRenderSnapshot();
    return placed;
}

bool LevelBuilderWorkspace::RouteCanvasSecondaryAction(float screen_x, float screen_y) {
    if (active_mode_ == WorkflowMode::SupportingSpatial) {
        const bool handled = supporting_spatial_workspace_.RouteCanvasSecondaryAction(screen_x, screen_y);
        captureRenderSnapshot();
        return handled;
    }
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool undone = placement_panel_.Undo();
    captureRenderSnapshot();
    return undone;
}

bool LevelBuilderWorkspace::PaintSelectedGridRectangle(int32_t min_x, int32_t min_y, int32_t max_x, int32_t max_y) {
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool painted = placement_panel_.FillSelectedPartRectangle(min_x, min_y, max_x, max_y);
    if (painted && document_ != nullptr && !document_->parts().empty()) {
        (void)inspector_panel_.SelectInstance(document_->parts().back().instance_id);
    }
    captureRenderSnapshot();
    return painted;
}

bool LevelBuilderWorkspace::EraseTopPartAtGrid(int32_t grid_x, int32_t grid_y) {
    if (active_mode_ != WorkflowMode::Build) {
        captureRenderSnapshot();
        return false;
    }
    const bool erased = placement_panel_.RemoveTopPartAtGrid(grid_x, grid_y);
    captureRenderSnapshot();
    return erased;
}

void LevelBuilderWorkspace::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.active_mode = modeName(active_mode_);
    last_render_snapshot_.has_document = document_ != nullptr;
    last_render_snapshot_.has_catalog = catalog_ != nullptr;
    last_render_snapshot_.has_spatial_overlay = overlay_ != nullptr;
    last_render_snapshot_.has_target_scene = scene_ != nullptr;
    last_render_snapshot_.can_author = document_ != nullptr && catalog_ != nullptr && overlay_ != nullptr;
    last_render_snapshot_.can_playtest = document_ != nullptr && catalog_ != nullptr;
    last_render_snapshot_.has_unsaved_changes = document_ != nullptr && !document_->dirtyChunks().empty();
    last_render_snapshot_.can_undo =
        placement_panel_.lastRenderSnapshot().can_undo || inspector_panel_.lastRenderSnapshot().can_undo;
    last_render_snapshot_.can_redo =
        placement_panel_.lastRenderSnapshot().can_redo || inspector_panel_.lastRenderSnapshot().can_redo;
    last_render_snapshot_.last_authoring_command = last_authoring_command_result_;
    last_render_snapshot_.last_focus_diagnostic = last_focus_diagnostic_result_;
    last_render_snapshot_.last_edit_history = last_edit_history_result_;
    last_render_snapshot_.last_load = last_load_result_;
    last_render_snapshot_.last_save = last_save_result_;
    last_render_snapshot_.last_export = last_export_result_;

    if (document_ == nullptr || catalog_ == nullptr) {
        last_render_snapshot_.status = "disabled";
        last_render_snapshot_.message = "Bind a GridPartDocument and GridPartCatalog to use the native Level Builder.";
    } else if (overlay_ == nullptr) {
        last_render_snapshot_.status = "ready_without_canvas";
        last_render_snapshot_.message =
            "Level Builder is bound; attach a SpatialMapOverlay to enable canvas placement.";
    } else {
        last_render_snapshot_.status = "ready";
        last_render_snapshot_.message = "Grid-part Level Builder is the canonical map authoring surface.";
    }

    last_render_snapshot_.palette = palette_panel_.lastRenderSnapshot();
    last_render_snapshot_.placement = placement_panel_.lastRenderSnapshot();
    last_render_snapshot_.inspector = inspector_panel_.lastRenderSnapshot();
    last_render_snapshot_.playtest = playtest_panel_.lastRenderSnapshot();
    last_render_snapshot_.supporting_spatial = supporting_spatial_workspace_.lastRenderSnapshot();
    last_render_snapshot_.supporting_spatial.visible = active_mode_ == WorkflowMode::SupportingSpatial;
    last_render_snapshot_.readiness_evidence.has_player_spawn = readiness_evidence_.has_player_spawn;
    last_render_snapshot_.readiness_evidence.has_objective = readiness_evidence_.has_objective;
    last_render_snapshot_.readiness_evidence.reachability_passed = readiness_evidence_.reachability_passed;
    last_render_snapshot_.readiness_evidence.target_export_checks_passed =
        readiness_evidence_.target_export_checks_passed;
    last_render_snapshot_.readiness_evidence.accessibility_checks_passed =
        readiness_evidence_.accessibility_checks_passed;
    last_render_snapshot_.readiness_evidence.performance_budget_passed =
        readiness_evidence_.performance_budget_passed;
    last_render_snapshot_.readiness_evidence.human_review_required = readiness_evidence_.human_review_required;
    last_render_snapshot_.readiness_evidence.human_review_passed = readiness_evidence_.human_review_passed;

    if (document_ != nullptr && catalog_ != nullptr) {
        const auto document_validation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
        const auto ruleset_validation = urpg::map::ValidateGridPartRuleset(*document_, *catalog_, ruleset_);
        const auto objective_validation = urpg::map::ValidateMapObjective(*document_, *catalog_, objective_);

        last_render_snapshot_.validation.document_ok = document_validation.ok;
        last_render_snapshot_.validation.ruleset_ok = ruleset_validation.ok;
        last_render_snapshot_.validation.objective_ok = objective_validation.ok;
        last_render_snapshot_.validation.diagnostic_count = document_validation.diagnostics.size() +
                                                            ruleset_validation.diagnostics.size() +
                                                            objective_validation.diagnostics.size();
        last_render_snapshot_.validation.blocking_count =
            countBlockingDiagnostics(document_validation.diagnostics) +
            countBlockingDiagnostics(ruleset_validation.diagnostics) +
            countBlockingDiagnostics(objective_validation.diagnostics);
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "document", document_validation.diagnostics);
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "ruleset", ruleset_validation.diagnostics);
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "objective", objective_validation.diagnostics);

        const auto manifest = has_custom_package_manifest_
                                  ? package_manifest_
                                  : urpg::map::BuildGridPartPackageManifest(*document_, *catalog_);
        const auto package = urpg::map::ValidateGridPartPackageGovernance(*document_, *catalog_, manifest,
                                                                          readiness_evidence_);
        last_render_snapshot_.package.readiness = readinessName(package.readiness);
        last_render_snapshot_.package.can_publish = package.canPublish();
        last_render_snapshot_.package.can_export = package.canExport();
        last_render_snapshot_.package.dependency_count = package.dependencies.size();
        last_render_snapshot_.package.diagnostic_count = package.diagnostics.size();
        appendDiagnosticSummaries(last_render_snapshot_.diagnostics, "package", package.diagnostics);
        last_render_snapshot_.can_export_current_level =
            last_render_snapshot_.validation.blocking_count == 0 && package.canExport();
    }

    last_render_snapshot_.actions = {
        {"build", "Build", active_mode_ == WorkflowMode::Build, last_render_snapshot_.has_document},
        {"undo", "Undo", false, last_render_snapshot_.can_undo},
        {"redo", "Redo", false, last_render_snapshot_.can_redo},
        {"mark_player_spawn", "Spawn", false, last_render_snapshot_.inspector.has_selection},
        {"set_reach_exit_objective", "Objective", false, last_render_snapshot_.inspector.has_selection},
        {"mark_target_export_checks_passed", "Export Checks", false, last_render_snapshot_.has_document},
        {"mark_accessibility_checks_passed", "Accessibility", false, last_render_snapshot_.has_document},
        {"mark_performance_budget_passed", "Performance", false, last_render_snapshot_.has_document},
        {"mark_human_review_passed", "Review", false, last_render_snapshot_.has_document},
        {"validate", "Validate", active_mode_ == WorkflowMode::Validate, last_render_snapshot_.has_document},
        {"playtest", "Playtest", active_mode_ == WorkflowMode::Playtest, last_render_snapshot_.can_playtest},
        {"playtest_start", "Run", false, last_render_snapshot_.can_playtest},
        {"playtest_return", "Return", false, last_render_snapshot_.playtest.running},
        {"package", "Package", active_mode_ == WorkflowMode::Package, last_render_snapshot_.has_document},
        {"save_level_draft", "Save", false, last_render_snapshot_.has_document},
        {"export_current_level", "Export", false, last_render_snapshot_.can_export_current_level},
        {"supporting_spatial", "Supporting Spatial", active_mode_ == WorkflowMode::SupportingSpatial,
         last_render_snapshot_.has_spatial_overlay || last_render_snapshot_.has_target_scene},
        {"supporting_elevation", "Elevation", active_mode_ == WorkflowMode::SupportingSpatial &&
                                               supporting_spatial_workspace_.activeMode() ==
                                                   SpatialAuthoringWorkspace::ToolMode::Elevation,
         last_render_snapshot_.has_spatial_overlay},
        {"supporting_props", "Props", active_mode_ == WorkflowMode::SupportingSpatial &&
                                      supporting_spatial_workspace_.activeMode() ==
                                          SpatialAuthoringWorkspace::ToolMode::Props,
         last_render_snapshot_.has_spatial_overlay},
        {"supporting_abilities", "Abilities", active_mode_ == WorkflowMode::SupportingSpatial &&
                                              supporting_spatial_workspace_.activeMode() ==
                                                  SpatialAuthoringWorkspace::ToolMode::Abilities,
         last_render_snapshot_.has_spatial_overlay || last_render_snapshot_.has_target_scene},
    };
}

} // namespace urpg::editor
