#include "editor/ui/menu_preview_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <algorithm>
#include <utility>

/**
 * NOTE: ImGui headers are expected to be provided by the editor environment.
 */
#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace urpg::editor {

MenuPreviewPanel::MenuPreviewPanel() : EditorPanel("Menu Preview") {}

void MenuPreviewPanel::bindRuntime(urpg::ui::MenuSceneGraph& scene_graph) {
    scene_graph_ = &scene_graph;
}

void MenuPreviewPanel::setLayoutChangeHandler(LayoutChangeHandler handler) {
    layout_change_handler_ = std::move(handler);
}

void MenuPreviewPanel::clearRuntime() {
    scene_graph_ = nullptr;
    layout_change_handler_ = {};
    drag_state_.reset();
    has_rendered_frame_ = false;
    last_render_snapshot_ = {};
}

void MenuPreviewPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible)
        return;

    captureRenderSnapshot();

#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    if (!ImGui::Begin(m_title.c_str(), &m_visible)) {
        ImGui::End();
        return;
    }

    if (!scene_graph_) {
        ImGui::TextDisabled("No scene graph bound.");
        ImGui::End();
        return;
    }

    auto activeScene = scene_graph_->getActiveScene();
    if (!activeScene) {
        ImGui::TextDisabled("No active scene in stack.");
        ImGui::End();
        return;
    }

    const auto& canvas = activeScene->getDesignCanvas();
    ImGui::Text("Active Scene: %s", activeScene->getId().c_str());
    ImGui::TextDisabled("Native canvas: %d x %d", canvas.width, canvas.height);
    if (ImGui::SliderInt("Snap grid", &snap_grid_size_, 4, 64, "%d px")) {
        snap_grid_size_ = std::clamp(snap_grid_size_, 4, 64);
    }
    ImGui::TextDisabled("Drag a pane to move it, or its lower-right handle to resize it.");
    ImGui::TextDisabled("Focus badges show the authored pane traversal; runtime rules can still disable a pane.");
    ImGui::Separator();

    if (!canvas.isValid()) {
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Invalid native canvas layout.");
        ImGui::End();
        return;
    }

    if (ImGui::BeginChild("MenuLayoutCanvas", ImVec2(0.0f, 0.0f), true, ImGuiWindowFlags_HorizontalScrollbar)) {
        const ImVec2 canvas_origin = ImGui::GetCursorScreenPos();
        const ImVec2 canvas_size(static_cast<float>(canvas.width), static_cast<float>(canvas.height));
        ImGui::Dummy(canvas_size);
        const ImVec2 canvas_end(canvas_origin.x + canvas_size.x, canvas_origin.y + canvas_size.y);
        auto* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(canvas_origin, canvas_end, IM_COL32(24, 29, 38, 255));
        draw_list->AddRect(canvas_origin, canvas_end, IM_COL32(130, 150, 180, 255));
        for (int coordinate = 0; coordinate <= canvas.width; coordinate += snap_grid_size_) {
            draw_list->AddLine(ImVec2(canvas_origin.x + static_cast<float>(coordinate), canvas_origin.y),
                               ImVec2(canvas_origin.x + static_cast<float>(coordinate), canvas_end.y),
                               IM_COL32(68, 84, 104, 130));
        }
        for (int coordinate = 0; coordinate <= canvas.height; coordinate += snap_grid_size_) {
            draw_list->AddLine(ImVec2(canvas_origin.x, canvas_origin.y + static_cast<float>(coordinate)),
                               ImVec2(canvas_end.x, canvas_origin.y + static_cast<float>(coordinate)),
                               IM_COL32(68, 84, 104, 130));
        }

        struct VisiblePane {
            const urpg::ui::MenuPane* pane = nullptr;
            size_t index = 0;
        };
        std::vector<VisiblePane> visible_panes;
        const auto& panes = activeScene->getPanes();
        for (size_t pane_index = 0; pane_index < panes.size(); ++pane_index) {
            const auto& pane = panes[pane_index];
            if (pane.isVisible) {
                visible_panes.push_back({&pane, pane_index});
            }
        }
        std::stable_sort(visible_panes.begin(), visible_panes.end(), [](const auto& left, const auto& right) {
            return left.pane->layout.z_order < right.pane->layout.z_order;
        });

        if (drag_state_.has_value() &&
            (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left))) {
            auto& drag = *drag_state_;
            const auto mouse = ImGui::GetIO().MousePos;
            const auto snap = [this](int value) {
                return ((value + snap_grid_size_ / 2) / snap_grid_size_) * snap_grid_size_;
            };
            if (drag.mode == DragMode::Move) {
                const int max_x = std::max(0, canvas.width - drag.initial_layout.width);
                const int max_y = std::max(0, canvas.height - drag.initial_layout.height);
                drag.preview_layout.x = std::clamp(
                    snap(drag.initial_layout.x + static_cast<int>(mouse.x - drag.start_mouse_x)), 0, max_x);
                drag.preview_layout.y = std::clamp(
                    snap(drag.initial_layout.y + static_cast<int>(mouse.y - drag.start_mouse_y)), 0, max_y);
                draw_list->AddLine(ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x), canvas_origin.y),
                                   ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x), canvas_end.y),
                                   IM_COL32(244, 198, 74, 210), 2.0f);
                draw_list->AddLine(ImVec2(canvas_origin.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y)),
                                   ImVec2(canvas_end.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y)),
                                   IM_COL32(244, 198, 74, 210), 2.0f);
            } else {
                const int max_width = std::max(1, canvas.width - drag.initial_layout.x);
                const int max_height = std::max(1, canvas.height - drag.initial_layout.y);
                drag.preview_layout.width = std::clamp(
                    snap(drag.initial_layout.width + static_cast<int>(mouse.x - drag.start_mouse_x)),
                    32, max_width);
                drag.preview_layout.height = std::clamp(
                    snap(drag.initial_layout.height + static_cast<int>(mouse.y - drag.start_mouse_y)),
                    32, max_height);
                draw_list->AddLine(
                    ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x + drag.preview_layout.width), canvas_origin.y),
                    ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x + drag.preview_layout.width), canvas_end.y),
                    IM_COL32(244, 198, 74, 210), 2.0f);
                draw_list->AddLine(
                    ImVec2(canvas_origin.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y + drag.preview_layout.height)),
                    ImVec2(canvas_end.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y + drag.preview_layout.height)),
                    IM_COL32(244, 198, 74, 210), 2.0f);
            }
        }

        for (const auto& visible_pane : visible_panes) {
            const auto* pane = visible_pane.pane;
            const auto layout = drag_state_.has_value() && drag_state_->pane_index == visible_pane.index
                ? drag_state_->preview_layout
                : pane->layout;
            if (!layout.isValid()) {
                continue;
            }
            const ImVec2 pane_min(canvas_origin.x + static_cast<float>(layout.x),
                                  canvas_origin.y + static_cast<float>(layout.y));
            const ImVec2 pane_max(pane_min.x + static_cast<float>(layout.width),
                                  pane_min.y + static_cast<float>(layout.height));
            const ImU32 border = pane->isActive ? IM_COL32(245, 210, 90, 255) : IM_COL32(105, 160, 220, 255);
            draw_list->AddRectFilled(pane_min, pane_max, IM_COL32(37, 57, 79, 235));
            draw_list->AddRect(pane_min, pane_max, border, 0.0f, 0, pane->isActive ? 2.0f : 1.0f);
            const std::string title = pane->displayName.empty() ? pane->id : pane->displayName;
            draw_list->AddText(ImVec2(pane_min.x + 8.0f, pane_min.y + 8.0f), border, title.c_str());
            for (size_t index = 0; index < pane->commands.size(); ++index) {
                const auto& command = pane->commands[index];
                const std::string label = command.label.empty() ? command.id : command.label;
                const ImU32 color = pane->selectedCommandIndex >= 0 && index == static_cast<size_t>(pane->selectedCommandIndex)
                    ? IM_COL32(255, 238, 148, 255)
                    : IM_COL32(225, 230, 238, 255);
                draw_list->AddText(
                    ImVec2(pane_min.x + 12.0f, pane_min.y + 32.0f + static_cast<float>(index) * 20.0f),
                    color, label.c_str());
            }
            constexpr float handle_size = 12.0f;
            const bool can_resize = layout.x >= 0 && layout.y >= 0 &&
                                    layout.x <= canvas.width - 32 && layout.y <= canvas.height - 32;
            if (can_resize) {
                const ImVec2 handle_min(pane_max.x - handle_size, pane_max.y - handle_size);
                draw_list->AddRectFilled(handle_min, pane_max, border);
                draw_list->AddRect(handle_min, pane_max, IM_COL32(24, 29, 38, 255));
            }
        }

        std::vector<VisiblePane> focus_panes;
        for (const auto& visible_pane : visible_panes) {
            if (!visible_pane.pane->commands.empty()) {
                focus_panes.push_back(visible_pane);
            }
        }
        std::stable_sort(focus_panes.begin(), focus_panes.end(), [](const auto& left, const auto& right) {
            const int left_order = left.pane->layout.focus_order;
            const int right_order = right.pane->layout.focus_order;
            if (left_order < 0 && right_order < 0) {
                return false;
            }
            if (left_order < 0) {
                return false;
            }
            if (right_order < 0) {
                return true;
            }
            return left_order < right_order;
        });
        const auto center_for_pane = [&canvas_origin, this](const VisiblePane& visible_pane) {
            const auto layout = drag_state_.has_value() && drag_state_->pane_index == visible_pane.index
                ? drag_state_->preview_layout
                : visible_pane.pane->layout;
            return ImVec2(canvas_origin.x + static_cast<float>(layout.x) + static_cast<float>(layout.width) / 2.0f,
                          canvas_origin.y + static_cast<float>(layout.y) + static_cast<float>(layout.height) / 2.0f);
        };
        for (size_t focus_index = 0; focus_index < focus_panes.size(); ++focus_index) {
            const ImVec2 center = center_for_pane(focus_panes[focus_index]);
            if (focus_index + 1 < focus_panes.size()) {
                const ImVec2 next_center = center_for_pane(focus_panes[focus_index + 1]);
                draw_list->AddLine(center, next_center, IM_COL32(164, 228, 255, 180), 2.0f);
            }
            const std::string badge = "Focus " + std::to_string(focus_index + 1);
            draw_list->AddText(ImVec2(center.x - 24.0f, center.y - 8.0f), IM_COL32(164, 228, 255, 255), badge.c_str());
        }

        // Hit-test in reverse draw order so a higher-z pane owns any overlap.
        for (auto pane_it = visible_panes.rbegin(); pane_it != visible_panes.rend(); ++pane_it) {
            const auto* pane = pane_it->pane;
            const auto layout = drag_state_.has_value() && drag_state_->pane_index == pane_it->index
                ? drag_state_->preview_layout
                : pane->layout;
            if (!layout.isValid()) {
                continue;
            }
            const ImVec2 pane_min(canvas_origin.x + static_cast<float>(layout.x),
                                  canvas_origin.y + static_cast<float>(layout.y));
            const ImVec2 pane_max(pane_min.x + static_cast<float>(layout.width),
                                  pane_min.y + static_cast<float>(layout.height));
            constexpr float handle_size = 12.0f;
            const ImVec2 handle_min(pane_max.x - handle_size, pane_max.y - handle_size);

            ImGui::PushID(static_cast<int>(pane_it->index));
            const bool can_resize = layout.x >= 0 && layout.y >= 0 &&
                                    layout.x <= canvas.width - 32 && layout.y <= canvas.height - 32;
            if (can_resize) {
                ImGui::SetCursorScreenPos(handle_min);
                ImGui::InvisibleButton("pane_resize", ImVec2(handle_size, handle_size));
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !drag_state_.has_value()) {
                    const auto mouse = ImGui::GetIO().MousePos;
                    drag_state_ = DragState{
                        pane_it->index, DragMode::Resize, pane->layout, pane->layout, mouse.x, mouse.y};
                }
            }

            ImGui::SetCursorScreenPos(pane_min);
            ImGui::InvisibleButton("pane_drag", ImVec2(static_cast<float>(layout.width), static_cast<float>(layout.height)));
            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !drag_state_.has_value()) {
                const auto mouse = ImGui::GetIO().MousePos;
                drag_state_ = DragState{
                    pane_it->index, DragMode::Move, pane->layout, pane->layout, mouse.x, mouse.y};
            }
            ImGui::PopID();
        }

        if (drag_state_.has_value() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
            const auto completed_drag = std::move(*drag_state_);
            drag_state_.reset();
            if ((completed_drag.initial_layout.x != completed_drag.preview_layout.x ||
                 completed_drag.initial_layout.y != completed_drag.preview_layout.y ||
                 completed_drag.initial_layout.width != completed_drag.preview_layout.width ||
                 completed_drag.initial_layout.height != completed_drag.preview_layout.height) && layout_change_handler_) {
                (void)layout_change_handler_(completed_drag.pane_index, completed_drag.preview_layout);
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();
#endif
}

void MenuPreviewPanel::refresh() {
    if (!m_visible || !scene_graph_) {
        return;
    }

    captureRenderSnapshot();
}

void MenuPreviewPanel::update() {
    refresh();
}

void MenuPreviewPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    if (!scene_graph_) {
        has_rendered_frame_ = false;
        return;
    }

    const auto activeScene = scene_graph_->getActiveScene();
    if (!activeScene) {
        has_rendered_frame_ = false;
        return;
    }

    last_render_snapshot_.active_scene_id = activeScene->getId();
    last_render_snapshot_.design_canvas = activeScene->getDesignCanvas();
    last_render_snapshot_.last_blocked_command_id = scene_graph_->getLastBlockedCommandId();
    last_render_snapshot_.last_blocked_reason = scene_graph_->getLastBlockedReason();

    const auto& panes = activeScene->getPanes();
    for (size_t pane_index = 0; pane_index < panes.size(); ++pane_index) {
        const auto& pane = panes[pane_index];
        if (!pane.isVisible) {
            continue;
        }

        PaneSnapshot snapshot;
        snapshot.pane_index = pane_index;
        snapshot.pane_id = pane.id;
        snapshot.pane_label = pane.displayName;
        snapshot.pane_active = pane.isActive;
        snapshot.layout = pane.layout;

        for (const auto& cmd : pane.commands) {
            snapshot.command_ids.push_back(cmd.id);
            snapshot.command_labels.push_back(cmd.label.empty() ? cmd.id : cmd.label);
            snapshot.command_enabled.push_back(true);
        }

        if (pane.selectedCommandIndex >= 0 && static_cast<size_t>(pane.selectedCommandIndex) < pane.commands.size()) {
            snapshot.selected_command_id = pane.commands[pane.selectedCommandIndex].id;
        }

        last_render_snapshot_.visible_panes.push_back(std::move(snapshot));
    }

    last_render_snapshot_.has_data =
        !last_render_snapshot_.active_scene_id.empty() || !last_render_snapshot_.visible_panes.empty();
    has_rendered_frame_ = true;
}

} // namespace urpg::editor
