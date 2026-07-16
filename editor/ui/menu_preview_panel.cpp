#include "editor/ui/menu_preview_panel.h"
#include "engine/core/engine_context.h"
#include "engine/core/ui/menu_scene_graph.h"

#include <algorithm>
#include <cstdlib>
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
    authored_preview_graph_.reset();
    authored_nodes_.clear();
    authored_disabled_command_ids_.clear();
    scene_graph_ = &scene_graph;
}

bool MenuPreviewPanel::bindAuthoringDocument(const urpg::ui::MenuAuthoringDocument& document,
                                             std::string scene_id,
                                             std::vector<std::string>* diagnostics) {
    return bindAuthoringDocumentInternal(document, std::move(scene_id), nullptr, diagnostics);
}

bool MenuPreviewPanel::bindAuthoringDocument(const urpg::ui::MenuAuthoringDocument& document,
                                             std::string scene_id,
                                             const urpg::ui::MenuBindingContext& binding_context,
                                             std::vector<std::string>* diagnostics) {
    return bindAuthoringDocumentInternal(document, std::move(scene_id), &binding_context, diagnostics);
}

bool MenuPreviewPanel::bindAuthoringDocumentInternal(
    const urpg::ui::MenuAuthoringDocument& document, std::string scene_id,
    const urpg::ui::MenuBindingContext* binding_context,
    std::vector<std::string>* diagnostics) {
    auto materialized = urpg::ui::materializeMenuAuthoringDocument(
        document, std::move(scene_id), binding_context);
    if (diagnostics) *diagnostics = materialized.diagnostics;
    if (!materialized.scene) return false;
    auto graph = std::make_unique<urpg::ui::MenuSceneGraph>();
    const auto active_id = materialized.scene->getId();
    graph->registerScene(std::move(materialized.scene));
    graph->pushScene(active_id);
    authored_nodes_.clear();
    for (const auto& node : document.nodes()) authored_nodes_.emplace(node.id, node);
    authored_disabled_command_ids_.clear();
    authored_disabled_command_ids_.insert(materialized.disabled_command_ids.begin(),
                                          materialized.disabled_command_ids.end());
    const auto disabled = authored_disabled_command_ids_;
    graph->setCommandEnabledEvaluator([disabled](const urpg::MenuCommandMeta& command) {
        return !disabled.contains(command.id);
    });
    authored_preview_graph_ = std::move(graph);
    scene_graph_ = authored_preview_graph_.get();
    captureRenderSnapshot();
    return true;
}

void MenuPreviewPanel::setPreviewAccessibilityPolicy(bool reduced_motion, bool audio_enabled) {
    reduced_motion_preview_ = reduced_motion;
    audio_enabled_preview_ = audio_enabled;
    captureRenderSnapshot();
}

void MenuPreviewPanel::setLayoutChangeHandler(LayoutChangeHandler handler) {
    layout_change_handler_ = std::move(handler);
}

void MenuPreviewPanel::setLayoutBatchChangeHandler(LayoutBatchChangeHandler handler) {
    layout_batch_change_handler_ = std::move(handler);
}

bool MenuPreviewPanel::setPreviewTargetCanvas(urpg::ui::MenuDesignCanvas canvas) {
    if (!canvas.isValid()) {
        return false;
    }
    preview_target_canvas_ = canvas;
    drag_state_.reset();
    captureRenderSnapshot();
    return true;
}

void MenuPreviewPanel::clearPreviewTargetCanvas() {
    preview_target_canvas_.reset();
    drag_state_.reset();
    captureRenderSnapshot();
}

void MenuPreviewPanel::clearRuntime() {
    scene_graph_ = nullptr;
    authored_preview_graph_.reset();
    authored_nodes_.clear();
    authored_disabled_command_ids_.clear();
    layout_change_handler_ = {};
    layout_batch_change_handler_ = {};
    preview_target_canvas_.reset();
    drag_state_.reset();
    selected_pane_indices_.clear();
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

    const auto& design_canvas = activeScene->getDesignCanvas();
    const auto canvas = preview_target_canvas_.value_or(design_canvas);
    const bool previewing_target_canvas = canvas.width != design_canvas.width || canvas.height != design_canvas.height;
    ImGui::Text("Active Scene: %s", activeScene->getId().c_str());
    ImGui::TextDisabled("Authored canvas: %d x %d", design_canvas.width, design_canvas.height);
    int preview_size[] = {canvas.width, canvas.height};
    if (ImGui::InputInt2("Preview target", preview_size)) {
        (void)setPreviewTargetCanvas({preview_size[0], preview_size[1]});
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset Preview Target")) {
        clearPreviewTargetCanvas();
    }
    if (previewing_target_canvas) {
        ImGui::TextDisabled("Responsive target preview: %d x %d. Drag, resize, and distribution are disabled.",
                            canvas.width, canvas.height);
        for (const auto& diagnostic : last_render_snapshot_.responsive_diagnostics) {
            ImGui::TextColored(ImVec4(1.0f, 0.58f, 0.22f, 1.0f), "%s", diagnostic.c_str());
        }
    }
    if (ImGui::SliderInt("Snap grid", &snap_grid_size_, 4, 64, "%d px")) {
        snap_grid_size_ = std::clamp(snap_grid_size_, 4, 64);
    }
    ImGui::TextDisabled(previewing_target_canvas
                            ? "Responsive pane rectangles are resolved from the authored canvas."
                            : "Drag a pane to move it, or its lower-right handle to resize it.");
    const auto& active_panes = activeScene->getPanes();
    selected_pane_indices_.erase(
        std::remove_if(selected_pane_indices_.begin(), selected_pane_indices_.end(),
                       [&](const size_t pane_index) {
                           return pane_index >= active_panes.size() || !active_panes[pane_index].isVisible ||
                                  !active_panes[pane_index].layout.isValid();
                       }),
        selected_pane_indices_.end());
    const auto distribute_selected = [&](const bool horizontal) {
        if (!layout_batch_change_handler_) return false;
        const auto& panes = activeScene->getPanes();
        std::vector<size_t> selected;
        for (const auto pane_index : selected_pane_indices_) {
            if (pane_index < panes.size() && panes[pane_index].isVisible && panes[pane_index].layout.isValid()) {
                selected.push_back(pane_index);
            }
        }
        if (selected.size() < 2) return false;
        std::sort(selected.begin(), selected.end(), [&](const size_t left, const size_t right) {
            return horizontal ? panes[left].layout.x < panes[right].layout.x : panes[left].layout.y < panes[right].layout.y;
        });
        const int first_position = horizontal ? panes[selected.front()].layout.x : panes[selected.front()].layout.y;
        const auto& last_layout = panes[selected.back()].layout;
        const int last_end = horizontal ? last_layout.x + last_layout.width : last_layout.y + last_layout.height;
        int total_size = 0;
        for (const auto pane_index : selected) {
            total_size += horizontal ? panes[pane_index].layout.width : panes[pane_index].layout.height;
        }
        const int gap_space = last_end - first_position - total_size;
        if (gap_space < 0) return false;
        const int gap = gap_space / static_cast<int>(selected.size() - 1);
        int next_position = first_position;
        std::vector<std::pair<size_t, urpg::ui::MenuPaneLayout>> layouts;
        layouts.reserve(selected.size());
        for (const auto pane_index : selected) {
            auto layout = panes[pane_index].layout;
            if (horizontal) layout.x = next_position;
            else layout.y = next_position;
            layouts.push_back({pane_index, layout});
            next_position += (horizontal ? layout.width : layout.height) + gap;
        }
        return layout_batch_change_handler_(layouts);
    };
    if (!previewing_target_canvas && selected_pane_indices_.size() >= 2) {
        if (ImGui::Button("Distribute Selected Horizontally")) (void)distribute_selected(true);
        ImGui::SameLine();
        if (ImGui::Button("Distribute Selected Vertically")) (void)distribute_selected(false);
        ImGui::TextDisabled("Ctrl+click visible panes to add or remove them from the distribution selection.");
    }
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

        std::optional<int> vertical_alignment_guide;
        std::optional<int> horizontal_alignment_guide;
        if (!previewing_target_canvas && drag_state_.has_value() &&
            (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseReleased(ImGuiMouseButton_Left))) {
            auto& drag = *drag_state_;
            const auto mouse = ImGui::GetIO().MousePos;
            const auto snap = [this](int value) {
                return ((value + snap_grid_size_ / 2) / snap_grid_size_) * snap_grid_size_;
            };
            constexpr int kAlignmentGuideSnapDistance = 8;
            const auto snapMoveAxis = [&](const int position, const int size, const int maximum,
                                          const bool horizontal, std::optional<int>& guide) {
                int snapped_position = position;
                int closest_distance = kAlignmentGuideSnapDistance + 1;
                for (const auto& candidate : visible_panes) {
                    if (candidate.index == drag.pane_index) continue;
                    const auto& layout = candidate.pane->layout;
                    if (!layout.isValid()) continue;
                    const int origin = horizontal ? layout.x : layout.y;
                    const int extent = horizontal ? layout.width : layout.height;
                    const int candidate_guides[] = {origin, origin + extent / 2, origin + extent};
                    for (const int candidate_guide : candidate_guides) {
                        const int positions[] = {candidate_guide, candidate_guide - size / 2,
                                                 candidate_guide - size};
                        for (const int proposed_position : positions) {
                            if (proposed_position < 0 || proposed_position > maximum) continue;
                            const int distance = std::abs(proposed_position - position);
                            if (distance < closest_distance) {
                                closest_distance = distance;
                                snapped_position = proposed_position;
                                guide = candidate_guide;
                            }
                        }
                    }
                }
                return snapped_position;
            };
            const auto snapResizeAxis = [&](const int size, const int origin, const int maximum,
                                            const bool horizontal, std::optional<int>& guide) {
                int snapped_size = size;
                int closest_distance = kAlignmentGuideSnapDistance + 1;
                for (const auto& candidate : visible_panes) {
                    if (candidate.index == drag.pane_index) continue;
                    const auto& layout = candidate.pane->layout;
                    if (!layout.isValid()) continue;
                    const int candidate_origin = horizontal ? layout.x : layout.y;
                    const int candidate_extent = horizontal ? layout.width : layout.height;
                    const int candidate_guides[] = {candidate_origin, candidate_origin + candidate_extent / 2,
                                                     candidate_origin + candidate_extent};
                    for (const int candidate_guide : candidate_guides) {
                        const int proposed_size = candidate_guide - origin;
                        if (proposed_size < 32 || proposed_size > maximum) continue;
                        const int distance = std::abs(proposed_size - size);
                        if (distance < closest_distance) {
                            closest_distance = distance;
                            snapped_size = proposed_size;
                            guide = candidate_guide;
                        }
                    }
                }
                return snapped_size;
            };
            if (drag.mode == DragMode::Move) {
                const int max_x = std::max(0, canvas.width - drag.initial_layout.width);
                const int max_y = std::max(0, canvas.height - drag.initial_layout.height);
                const int grid_x = std::clamp(
                    snap(drag.initial_layout.x + static_cast<int>(mouse.x - drag.start_mouse_x)), 0, max_x);
                const int grid_y = std::clamp(
                    snap(drag.initial_layout.y + static_cast<int>(mouse.y - drag.start_mouse_y)), 0, max_y);
                drag.preview_layout.x = snapMoveAxis(
                    grid_x, drag.initial_layout.width, max_x, true, vertical_alignment_guide);
                drag.preview_layout.y = snapMoveAxis(
                    grid_y, drag.initial_layout.height, max_y, false, horizontal_alignment_guide);
                draw_list->AddLine(ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x), canvas_origin.y),
                                   ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x), canvas_end.y),
                                   IM_COL32(244, 198, 74, 210), 2.0f);
                draw_list->AddLine(ImVec2(canvas_origin.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y)),
                                   ImVec2(canvas_end.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y)),
                                   IM_COL32(244, 198, 74, 210), 2.0f);
            } else {
                const int max_width = std::max(1, canvas.width - drag.initial_layout.x);
                const int max_height = std::max(1, canvas.height - drag.initial_layout.y);
                const int grid_width = std::clamp(
                    snap(drag.initial_layout.width + static_cast<int>(mouse.x - drag.start_mouse_x)),
                    32, max_width);
                const int grid_height = std::clamp(
                    snap(drag.initial_layout.height + static_cast<int>(mouse.y - drag.start_mouse_y)),
                    32, max_height);
                drag.preview_layout.width = snapResizeAxis(
                    grid_width, drag.initial_layout.x, max_width, true, vertical_alignment_guide);
                drag.preview_layout.height = snapResizeAxis(
                    grid_height, drag.initial_layout.y, max_height, false, horizontal_alignment_guide);
                draw_list->AddLine(
                    ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x + drag.preview_layout.width), canvas_origin.y),
                    ImVec2(canvas_origin.x + static_cast<float>(drag.preview_layout.x + drag.preview_layout.width), canvas_end.y),
                    IM_COL32(244, 198, 74, 210), 2.0f);
                draw_list->AddLine(
                    ImVec2(canvas_origin.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y + drag.preview_layout.height)),
                                   ImVec2(canvas_end.x, canvas_origin.y + static_cast<float>(drag.preview_layout.y + drag.preview_layout.height)),
                                   IM_COL32(244, 198, 74, 210), 2.0f);
            }
            if (vertical_alignment_guide.has_value()) {
                draw_list->AddLine(
                    ImVec2(canvas_origin.x + static_cast<float>(*vertical_alignment_guide), canvas_origin.y),
                    ImVec2(canvas_origin.x + static_cast<float>(*vertical_alignment_guide), canvas_end.y),
                    IM_COL32(102, 235, 171, 230), 2.0f);
            }
            if (horizontal_alignment_guide.has_value()) {
                draw_list->AddLine(
                    ImVec2(canvas_origin.x, canvas_origin.y + static_cast<float>(*horizontal_alignment_guide)),
                    ImVec2(canvas_end.x, canvas_origin.y + static_cast<float>(*horizontal_alignment_guide)),
                    IM_COL32(102, 235, 171, 230), 2.0f);
            }
        }

        for (const auto& visible_pane : visible_panes) {
            const auto* pane = visible_pane.pane;
            const auto layout = previewing_target_canvas
                                    ? urpg::ui::resolveMenuPaneLayoutForCanvas(
                                          pane->layout, design_canvas, canvas)
                                    : drag_state_.has_value() && drag_state_->pane_index == visible_pane.index
                                          ? drag_state_->preview_layout
                                          : pane->layout;
            if (!layout.isValid()) {
                continue;
            }
            const ImVec2 pane_min(canvas_origin.x + static_cast<float>(layout.x),
                                  canvas_origin.y + static_cast<float>(layout.y));
            const ImVec2 pane_max(pane_min.x + static_cast<float>(layout.width),
                                  pane_min.y + static_cast<float>(layout.height));
            const bool selected_for_distribution =
                std::find(selected_pane_indices_.begin(), selected_pane_indices_.end(), visible_pane.index) !=
                selected_pane_indices_.end();
            const ImU32 border = selected_for_distribution ? IM_COL32(102, 235, 171, 255)
                : pane->isActive ? IM_COL32(245, 210, 90, 255) : IM_COL32(105, 160, 220, 255);
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
            if (!previewing_target_canvas && can_resize) {
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
        const auto center_for_pane = [&canvas_origin, &design_canvas, &canvas, previewing_target_canvas, this](
                                         const VisiblePane& visible_pane) {
            const auto layout = previewing_target_canvas
                                    ? urpg::ui::resolveMenuPaneLayoutForCanvas(
                                          visible_pane.pane->layout, design_canvas, canvas)
                                    : drag_state_.has_value() && drag_state_->pane_index == visible_pane.index
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
        if (!previewing_target_canvas) {
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
                if (ImGui::GetIO().KeyCtrl) {
                    const auto selected = std::find(selected_pane_indices_.begin(), selected_pane_indices_.end(), pane_it->index);
                    if (selected == selected_pane_indices_.end()) selected_pane_indices_.push_back(pane_it->index);
                    else selected_pane_indices_.erase(selected);
                    ImGui::PopID();
                    continue;
                }
                if (std::find(selected_pane_indices_.begin(), selected_pane_indices_.end(), pane_it->index) ==
                    selected_pane_indices_.end()) {
                    selected_pane_indices_ = {pane_it->index};
                }
                const auto mouse = ImGui::GetIO().MousePos;
                drag_state_ = DragState{
                    pane_it->index, DragMode::Move, pane->layout, pane->layout, mouse.x, mouse.y};
            }
            ImGui::PopID();
        }
        }

        if (!previewing_target_canvas && drag_state_.has_value() && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
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

std::vector<urpg::accessibility::InclusiveAuditIssue> MenuPreviewPanel::auditInclusiveSnapshot(
    bool touch_declared, bool reduced_motion) const {
    std::vector<urpg::accessibility::InclusiveUiSnapshot> elements;
    for (const auto& pane : last_render_snapshot_.visible_panes) {
        for (size_t index = 0; index < pane.command_ids.size(); ++index) {
            const auto authored = authored_nodes_.find(pane.command_ids[index]);
            if (authored == authored_nodes_.end()) continue;
            float contrast = 7.0f;
            if (index < pane.command_state_properties.size()) {
                const auto value = pane.command_state_properties[index].find("contrast_ratio");
                if (value != pane.command_state_properties[index].end()) {
                    try { contrast = std::stof(value->second); } catch (...) { contrast = 0.0f; }
                }
            }
            const auto& node = authored->second;
            const bool clipped = node.layout.x < 0 || node.layout.y < 0 ||
                node.layout.x + node.layout.width > last_render_snapshot_.design_canvas.width ||
                node.layout.y + node.layout.height > last_render_snapshot_.design_canvas.height;
            const std::string rendered_label = index < pane.command_labels.size() ? pane.command_labels[index] : node.label;
            const bool overflow = static_cast<float>(rendered_label.size()) * 8.0f > node.layout.width;
            const uint32_t motion = index < pane.command_transition_duration_ms.size()
                ? pane.command_transition_duration_ms[index] : 0;
            const auto essential = node.instance_overrides.find("motion_essential");
            elements.push_back({node.id, node.accessible_label, node.focusable, node.layout.focus_order,
                contrast, node.layout.width, node.layout.height, clipped, overflow, motion,
                essential != node.instance_overrides.end() && essential->second == "true"});
        }
    }
    return urpg::accessibility::auditInclusiveUi(elements, touch_declared, reduced_motion);
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
    last_render_snapshot_.preview_canvas = preview_target_canvas_.value_or(last_render_snapshot_.design_canvas);
    last_render_snapshot_.previewing_target_canvas =
        last_render_snapshot_.preview_canvas.width != last_render_snapshot_.design_canvas.width ||
        last_render_snapshot_.preview_canvas.height != last_render_snapshot_.design_canvas.height;
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
        snapshot.layout = urpg::ui::resolveMenuPaneLayoutForCanvas(
            pane.layout, last_render_snapshot_.design_canvas, last_render_snapshot_.preview_canvas);
        const auto& layout = snapshot.layout;
        const auto& canvas = last_render_snapshot_.preview_canvas;
        const std::string pane_label = pane.id.empty() ? std::to_string(pane_index) : pane.id;
        if (!layout.isValid()) {
            last_render_snapshot_.responsive_diagnostics.push_back(
                "Pane '" + pane_label + "' resolves outside supported native layout bounds at this target.");
        } else if (layout.x < 0 || layout.y < 0 || layout.x + layout.width > canvas.width ||
                   layout.y + layout.height > canvas.height) {
            last_render_snapshot_.responsive_diagnostics.push_back(
                "Pane '" + pane_label + "' overflows the responsive target canvas.");
        }

        for (size_t command_index = 0; command_index < pane.commands.size(); ++command_index) {
            const auto& cmd = pane.commands[command_index];
            snapshot.command_ids.push_back(cmd.id);
            snapshot.command_labels.push_back(cmd.label.empty() ? cmd.id : cmd.label);
            const bool enabled = !authored_disabled_command_ids_.contains(cmd.id);
            snapshot.command_enabled.push_back(enabled);
            const bool focused = pane.isActive && pane.selectedCommandIndex >= 0 &&
                                 static_cast<size_t>(pane.selectedCommandIndex) == command_index;
            const auto state = !enabled ? urpg::ui::MenuVisualState::Disabled
                                       : focused ? urpg::ui::MenuVisualState::Focus
                                                 : urpg::ui::MenuVisualState::Default;
            snapshot.command_visual_states.push_back(state);
            std::map<std::string, std::string> properties;
            uint32_t transition_duration = 0;
            std::string audio_hook;
            if (const auto authored = authored_nodes_.find(cmd.id); authored != authored_nodes_.end()) {
                const auto style = std::find_if(authored->second.state_styles.begin(),
                    authored->second.state_styles.end(), [&](const auto& item) { return item.state == state; });
                if (style != authored->second.state_styles.end()) properties = style->properties;
                else if (const auto fallback = std::find_if(authored->second.state_styles.begin(),
                    authored->second.state_styles.end(), [](const auto& item) {
                        return item.state == urpg::ui::MenuVisualState::Default;
                    }); fallback != authored->second.state_styles.end()) properties = fallback->properties;
                if (state != urpg::ui::MenuVisualState::Default) {
                    const auto transition = urpg::ui::resolveMenuTransition(
                        authored->second, urpg::ui::MenuVisualState::Default, state,
                        reduced_motion_preview_, audio_enabled_preview_);
                    transition_duration = transition.duration_ms;
                    audio_hook = transition.audio_hook;
                }
            }
            snapshot.command_state_properties.push_back(std::move(properties));
            snapshot.command_transition_duration_ms.push_back(transition_duration);
            snapshot.command_audio_hooks.push_back(std::move(audio_hook));
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
