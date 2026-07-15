#include "editor/ui/menu_inspector_panel.h"
#include "engine/core/engine_context.h"
#include <algorithm>
#include <utility>

/**
 * NOTE: ImGui headers are expected to be provided by the editor environment.
 */
#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

MenuInspectorPanel::MenuInspectorPanel(std::shared_ptr<MenuInspectorModel> model)
    : EditorPanel("Menu Inspector"), model_(std::move(model)) {}

void MenuInspectorPanel::refresh() {
    if (!m_visible || !model_) {
        return;
    }

    CaptureRenderSnapshot();
}

void MenuInspectorPanel::update() {
    refresh();
}

void MenuInspectorPanel::setApplyChangesHandler(std::function<bool()> handler) {
    apply_changes_handler_ = std::move(handler);
}

void MenuInspectorPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible)
        return;

    if (!model_) {
        last_render_snapshot_ = {};
        has_rendered_frame_ = false;
        return;
    }

    CaptureRenderSnapshot();

#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    if (!ImGui::Begin(m_title.c_str(), &m_visible)) {
        ImGui::End();
        return;
    }

    if (!model_) {
        ImGui::TextDisabled("No model bound.");
        ImGui::End();
        return;
    }

    const auto& summary = model_->Summary();

    // Header Section - Summary
    if (ImGui::CollapsingHeader("Summary", 32)) { // ImGuiTreeNodeFlags_DefaultOpen
        ImGui::Columns(2);
        ImGui::Text("Active Scene:");
        ImGui::NextColumn();
        ImGui::Text("%s", summary.active_scene_id.empty() ? "(none)" : summary.active_scene_id.c_str());
        ImGui::NextColumn();
        ImGui::Text("Stack Depth:");
        ImGui::NextColumn();
        ImGui::Text("%zu", summary.stack_depth);
        ImGui::NextColumn();
        ImGui::Text("Total Panes:");
        ImGui::NextColumn();
        ImGui::Text("%zu", summary.total_panes);
        ImGui::NextColumn();
        ImGui::Text("Total Commands:");
        ImGui::NextColumn();
        ImGui::Text("%zu", summary.total_commands);
        ImGui::NextColumn();

        if (summary.issue_count > 0) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Total Issues:");
            ImGui::NextColumn();
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%zu", summary.issue_count);
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }

    ImGui::Separator();

    // Secondary Sections
    if (ImGui::TreeNodeEx("Registry & State", 32)) {
        RenderCommandRegistry();
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Scene Graph", 32)) {
        RenderSceneGraphState();
        ImGui::TreePop();
    }

    if (ImGui::TreeNodeEx("Selected Command", 32)) {
        RenderSelectedCommandDetails();
        ImGui::TreePop();
    }

    // Command List Table
    ImGui::Text("Command Audit:");
    if (ImGui::BeginTable("MenuAuditTable", 6, 1 | 2 | 8 | 16)) { // Borders | RowBg | Resizable | ScrollY
        ImGui::TableSetupColumn("Pane", 0, 0.15f);
        ImGui::TableSetupColumn("Command ID", 0, 0.25f);
        ImGui::TableSetupColumn("Label", 0, 0.20f);
        ImGui::TableSetupColumn("Route", 0, 0.15f);
        ImGui::TableSetupColumn("Status", 0, 0.10f);
        ImGui::TableSetupColumn("Issues", 0, 0.15f);
        ImGui::TableHeadersRow();

        const auto selected_command_id = model_->SelectedCommandId();
        const auto& rows = model_->VisibleRows();
        for (size_t index = 0; index < rows.size(); ++index) {
            const auto& row = rows[index];
            ImGui::TableNextRow();

            ImGui::TableNextColumn();
            const bool is_selected = selected_command_id.has_value() && row.command_id == *selected_command_id;
            if (ImGui::Selectable((row.pane_id + "##menu-pane-" + std::to_string(index)).c_str(), is_selected,
                                  1 | 2)) { // SpanAllColumns | AllowItemOverlap
                model_->SelectRow(index);
                CaptureRenderSnapshot();
            }

            ImGui::TableNextColumn();
            ImGui::Text("%s", row.command_id.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.command_label.c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", row.route_label.c_str());

            ImGui::TableNextColumn();
            if (row.command_enabled) {
                ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Enabled");
            } else if (row.command_visible) {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.4f, 1.0f), "Disabled");
            } else {
                ImGui::TextDisabled("Hidden");
            }

            ImGui::TableNextColumn();
            if (row.issue_count > 0) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%zu", row.issue_count);
            } else {
                ImGui::TextDisabled("-");
            }
        }
        ImGui::EndTable();
    }

    ImGui::End();
#endif
}

void MenuInspectorPanel::CaptureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.summary = model_->Summary();
    last_render_snapshot_.visible_rows = model_->VisibleRows();
    last_render_snapshot_.issues = model_->Issues();
    last_render_snapshot_.selected_command_id = model_->SelectedCommandId();
    last_render_snapshot_.selected_row = model_->SelectedRow();
    last_render_snapshot_.command_id_filter =
        model_->CommandIdFilter().has_value() ? *model_->CommandIdFilter() : std::string{};
    last_render_snapshot_.show_issues_only = model_->ShowIssuesOnly();
    last_render_snapshot_.has_data =
        !last_render_snapshot_.summary.active_scene_id.empty() || last_render_snapshot_.summary.total_panes > 0 ||
        !last_render_snapshot_.visible_rows.empty() || !last_render_snapshot_.issues.empty();

    has_rendered_frame_ = true;
}

void MenuInspectorPanel::RenderCommandRegistry() {
#ifdef URPG_IMGUI_ENABLED
    const auto& summary = last_render_snapshot_.summary;
    ImGui::Text("Missing registry entries: %zu", summary.missing_registry_entries);
    ImGui::Text("Duplicate command ids: %zu", summary.duplicate_command_ids);
    ImGui::Text("Route binding issues: %zu", summary.route_binding_issues);
    ImGui::Text("Rule validation issues: %zu", summary.rule_validation_issues);
    ImGui::Text("Total inspector issues: %zu", last_render_snapshot_.issues.size());
#endif
}

void MenuInspectorPanel::RenderSceneGraphState() {
#ifdef URPG_IMGUI_ENABLED
    const auto& summary = last_render_snapshot_.summary;
    ImGui::Text("Visible panes: %zu / %zu", summary.visible_panes, summary.total_panes);
    ImGui::Text("Native canvas: %d x %d", summary.design_canvas.width, summary.design_canvas.height);
    ImGui::Text("Layout diagnostics: %zu", summary.layout_issues);
    int canvas_size[] = {summary.design_canvas.width, summary.design_canvas.height};
    if (ImGui::InputInt2("Design canvas", canvas_size)) {
        if (model_->UpdateDesignCanvas({canvas_size[0], canvas_size[1]})) {
            if (apply_changes_handler_) {
                (void)apply_changes_handler_();
            }
            CaptureRenderSnapshot();
            return;
        }
    }
    ImGui::TextDisabled("Target presets preserve pane rectangles and report any resulting overflow.");
    const auto apply_canvas_preset = [this](int width, int height) {
        if (!model_->UpdateDesignCanvas({width, height})) {
            return false;
        }
        if (apply_changes_handler_) {
            (void)apply_changes_handler_();
        }
        CaptureRenderSnapshot();
        return true;
    };
    if (ImGui::Button("1280 x 720")) {
        if (apply_canvas_preset(1280, 720)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("1920 x 1080")) {
        if (apply_canvas_preset(1920, 1080)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("800 x 600")) {
        if (apply_canvas_preset(800, 600)) {
            return;
        }
    }
    const bool can_undo = model_->CanUndo();
    const bool can_redo = model_->CanRedo();
    if (!can_undo) ImGui::BeginDisabled();
    if (ImGui::Button("Undo Menu Edit") && model_->Undo()) {
        if (apply_changes_handler_) {
            (void)apply_changes_handler_();
        }
        CaptureRenderSnapshot();
        if (!can_undo) ImGui::EndDisabled();
        return;
    }
    if (!can_undo) ImGui::EndDisabled();
    ImGui::SameLine();
    if (!can_redo) ImGui::BeginDisabled();
    if (ImGui::Button("Redo Menu Edit") && model_->Redo()) {
        if (apply_changes_handler_) {
            (void)apply_changes_handler_();
        }
        CaptureRenderSnapshot();
        if (!can_redo) ImGui::EndDisabled();
        return;
    }
    if (!can_redo) ImGui::EndDisabled();
    ImGui::Text("Active panes: %zu", summary.active_panes);
    ImGui::Text("Navigable panes: %zu", summary.navigable_panes);
    ImGui::Text("Visible commands: %zu / %zu", summary.visible_commands, summary.total_commands);
    ImGui::Text("Enabled commands: %zu", summary.enabled_commands);
    ImGui::Text("Blocked commands: %zu", summary.blocked_commands);
    ImGui::Text("Command id filter: %s", last_render_snapshot_.command_id_filter.empty()
                                             ? "(none)"
                                             : last_render_snapshot_.command_id_filter.c_str());
    ImGui::Text("Issues only: %s", last_render_snapshot_.show_issues_only ? "yes" : "no");
#endif
}

void MenuInspectorPanel::RenderSelectedCommandDetails() {
#ifdef URPG_IMGUI_ENABLED
    if (!last_render_snapshot_.selected_row.has_value()) {
        ImGui::TextDisabled("No command selected.");
        return;
    }

    const auto& row = *last_render_snapshot_.selected_row;
    ImGui::Text("Command: %s", row.command_id.c_str());
    ImGui::Text("Label: %s", row.command_label.c_str());
    ImGui::Text("Pane: %s", row.pane_label.c_str());
    ImGui::Text("Pane rectangle: %d, %d, %d x %d", row.pane_layout.x, row.pane_layout.y,
                row.pane_layout.width, row.pane_layout.height);
    ImGui::Text("Layer / focus order: %d / %d", row.pane_layout.z_order, row.pane_layout.focus_order);
    int rectangle[] = {row.pane_layout.x, row.pane_layout.y, row.pane_layout.width, row.pane_layout.height};
    int layer = row.pane_layout.z_order;
    int focus_order = row.pane_layout.focus_order;
    bool anchors[] = {row.pane_layout.anchor_left, row.pane_layout.anchor_top,
                      row.pane_layout.anchor_right, row.pane_layout.anchor_bottom};
    int minimum_size[] = {row.pane_layout.min_width, row.pane_layout.min_height};
    const bool rectangle_changed = ImGui::InputInt4("Pane rectangle (x y w h)", rectangle);
    const bool layer_changed = ImGui::InputInt("Pane layer", &layer);
    const bool focus_changed = ImGui::InputInt("Pane focus order (-1 uses insertion order)", &focus_order);
    ImGui::Text("Responsive anchors:");
    const bool left_anchor_changed = ImGui::Checkbox("Left", &anchors[0]);
    ImGui::SameLine();
    const bool top_anchor_changed = ImGui::Checkbox("Top", &anchors[1]);
    ImGui::SameLine();
    const bool right_anchor_changed = ImGui::Checkbox("Right", &anchors[2]);
    ImGui::SameLine();
    const bool bottom_anchor_changed = ImGui::Checkbox("Bottom", &anchors[3]);
    const bool anchors_changed = left_anchor_changed || top_anchor_changed || right_anchor_changed ||
                                 bottom_anchor_changed;
    const bool minimum_changed = ImGui::InputInt2("Minimum pane size", minimum_size);
    ImGui::TextDisabled("Opposite anchors preserve margins and stretch; a trailing-only anchor follows the target edge.");
    if (rectangle_changed || layer_changed || focus_changed || anchors_changed || minimum_changed) {
        auto layout = row.pane_layout;
        layout.x = rectangle[0];
        layout.y = rectangle[1];
        layout.width = rectangle[2];
        layout.height = rectangle[3];
        layout.z_order = layer;
        layout.focus_order = focus_order;
        layout.anchor_left = anchors[0];
        layout.anchor_top = anchors[1];
        layout.anchor_right = anchors[2];
        layout.anchor_bottom = anchors[3];
        layout.min_width = minimum_size[0];
        layout.min_height = minimum_size[1];
        if (model_->UpdatePaneLayout(row.pane_index, layout)) {
            if (apply_changes_handler_) {
                (void)apply_changes_handler_();
            }
            CaptureRenderSnapshot();
            return;
        }
    }
    const auto& canvas = last_render_snapshot_.summary.design_canvas;
    const auto apply_alignment = [this, &row](int x, int y, bool align_x, bool align_y) {
        auto layout = row.pane_layout;
        if (align_x) {
            layout.x = x;
        }
        if (align_y) {
            layout.y = y;
        }
        if (!model_->UpdatePaneLayout(row.pane_index, layout)) {
            return false;
        }
        if (apply_changes_handler_) {
            (void)apply_changes_handler_();
        }
        CaptureRenderSnapshot();
        return true;
    };
    ImGui::Text("Align selected pane to native canvas:");
    if (ImGui::Button("Align Left")) {
        if (apply_alignment(0, 0, true, false)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Align Center X")) {
        if (apply_alignment(std::max(0, (canvas.width - row.pane_layout.width) / 2), 0, true, false)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Align Right")) {
        if (apply_alignment(std::max(0, canvas.width - row.pane_layout.width), 0, true, false)) {
            return;
        }
    }
    if (ImGui::Button("Align Top")) {
        if (apply_alignment(0, 0, false, true)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Align Center Y")) {
        if (apply_alignment(0, std::max(0, (canvas.height - row.pane_layout.height) / 2), false, true)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Align Bottom")) {
        if (apply_alignment(0, std::max(0, canvas.height - row.pane_layout.height), false, true)) {
            return;
        }
    }
    ImGui::TextDisabled("Oversized panes stay origin-aligned and remain visible to layout diagnostics.");
    const auto apply_layout_template = [this, &row](MenuPaneLayoutTemplate layout_template) {
        if (!model_->ApplyPaneLayoutTemplate(row.pane_index, layout_template)) {
            return false;
        }
        if (apply_changes_handler_) {
            (void)apply_changes_handler_();
        }
        CaptureRenderSnapshot();
        return true;
    };
    ImGui::Text("Apply native pane template:");
    if (ImGui::Button("Compact List")) {
        if (apply_layout_template(MenuPaneLayoutTemplate::CompactList)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Centered Dialog")) {
        if (apply_layout_template(MenuPaneLayoutTemplate::CenteredDialog)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Bottom Overlay")) {
        if (apply_layout_template(MenuPaneLayoutTemplate::BottomOverlay)) {
            return;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Full Canvas")) {
        if (apply_layout_template(MenuPaneLayoutTemplate::FullCanvas)) {
            return;
        }
    }
    ImGui::TextDisabled("Templates keep the pane's layer, focus order, commands, and identity intact.");
    ImGui::TextDisabled("Edits update the native runtime graph; durable project save remains a separate owner.");
    ImGui::Text("Route: %s", row.route_label.c_str());
    ImGui::Text("Summary: %s", row.summary.c_str());
    ImGui::Text("Visible / Enabled / Navigable: %s / %s / %s", row.command_visible ? "yes" : "no",
                row.command_enabled ? "yes" : "no", row.row_navigable ? "yes" : "no");
    ImGui::Text("Issues on row: %zu", row.issue_count);
#endif
}

} // namespace urpg::editor
