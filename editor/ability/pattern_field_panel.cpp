#include "editor/ability/pattern_field_panel.h"

#include <string_view>

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>
#endif

namespace urpg::editor {

void PatternFieldPanel::bindModel(PatternFieldModel& model) {
    m_bound_model = &model;
    rebuildSnapshot();
}

void PatternFieldPanel::clearBinding() {
    if (m_bound_model) {
        m_model = *m_bound_model;
    }
    m_bound_model = nullptr;
    rebuildSnapshot();
}

void PatternFieldPanel::update(const PatternFieldModel& model) {
    if (m_bound_model == nullptr) {
        m_model = model;
    } else {
        *m_bound_model = model;
    }
    rebuildSnapshot();
}

void PatternFieldPanel::render() {
    if (!m_visible)
        return;

    m_snapshot.visible = m_visible;
    m_snapshot.has_rendered_frame = true;
    rebuildControlState();

#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    if (!ImGui::Begin("Pattern Field Editor", &m_visible)) {
        ImGui::End();
        return;
    }

    auto& currentModel = model();
    const auto controlById = [this](std::string_view id) -> const RenderSnapshot::ControlState* {
        for (const auto& control : m_snapshot.controls) {
            if (control.id == id) {
                return &control;
            }
        }
        return nullptr;
    };

    const auto disabledTooltip = [](const RenderSnapshot::ControlState* control) {
        if (control != nullptr && !control->disabled_reason.empty() &&
            ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            ImGui::SetTooltip("%s", control->disabled_reason.c_str());
        }
    };

    std::string name = m_snapshot.name;
    if (ImGui::InputText("Name", &name)) {
        (void)setPatternName(name);
    }

    int viewport = m_snapshot.viewport_size;
    if (ImGui::InputInt("Viewport", &viewport)) {
        (void)resizeViewport(viewport);
    }

    const auto* clearControl = controlById("clear_pattern");
    const bool canClear = clearControl == nullptr || clearControl->enabled;
    if (!canClear) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Clear") && canClear) {
        (void)clearPattern();
    }
    if (!canClear) {
        disabledTooltip(clearControl);
        ImGui::EndDisabled();
    }

    const auto presets = currentModel.availablePresets();
    if (!presets.empty()) {
        ImGui::Separator();
        ImGui::Text("Presets");
        for (const auto& preset : presets) {
            ImGui::PushID(preset.id.c_str());
            if (ImGui::Button(preset.display_name.c_str())) {
                (void)applyPreset(preset.id);
            }
            ImGui::PopID();
            ImGui::SameLine();
        }
        ImGui::NewLine();
    }

    ImGui::Separator();
    ImGui::Text("Grid");
    const auto* toggleControl = controlById("toggle_point");
    const bool canTogglePoint = toggleControl == nullptr || toggleControl->enabled;
    const auto bounds = currentModel.getViewportBounds();
    for (int32_t y = bounds.minY; y <= bounds.maxY; ++y) {
        for (int32_t x = bounds.minX; x <= bounds.maxX; ++x) {
            ImGui::PushID(static_cast<int>((y - bounds.minY) * m_snapshot.viewport_size + (x - bounds.minX)));
            const bool selected = currentModel.isPointSelected(x, y);
            const char* label = (x == 0 && y == 0) ? (selected ? "[O]" : "[.]") : (selected ? "[X]" : "[ ]");
            if (!canTogglePoint) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button(label, ImVec2(36.0f, 28.0f)) && canTogglePoint) {
                (void)togglePoint(x, y);
            }
            if (!canTogglePoint) {
                disabledTooltip(toggleControl);
                ImGui::EndDisabled();
            }
            ImGui::PopID();
            if (x < bounds.maxX) {
                ImGui::SameLine();
            }
        }
    }

    if (!m_snapshot.is_valid) {
        ImGui::Separator();
        ImGui::Text("Validation");
        for (const auto& issue : m_snapshot.issues) {
            ImGui::BulletText("%s", issue.c_str());
        }
    }

    ImGui::End();
#endif
}

bool PatternFieldPanel::setPatternName(const std::string& name) {
    const auto current = model().getCurrentPattern();
    if (current && current->getName() == name) {
        return false;
    }

    model().setName(name);
    rebuildSnapshot();
    return true;
}

bool PatternFieldPanel::togglePoint(int32_t x, int32_t y) {
    const bool before = model().isPointSelected(x, y);
    model().togglePoint(x, y);
    const bool changed = before != model().isPointSelected(x, y);
    rebuildSnapshot();
    return changed;
}

bool PatternFieldPanel::clearPattern() {
    const auto before = model().buildPreviewSnapshot().grid_rows;
    model().clearPattern();
    const bool changed = before != model().buildPreviewSnapshot().grid_rows;
    rebuildSnapshot();
    return changed;
}

bool PatternFieldPanel::resizeViewport(int32_t viewport_size) {
    const int32_t before = model().getViewportSize();
    model().resizeViewport(viewport_size);
    const bool changed = before != model().getViewportSize();
    rebuildSnapshot();
    return changed;
}

bool PatternFieldPanel::applyPreset(const std::string& preset_id) {
    const auto before = model().buildPreviewSnapshot().grid_rows;
    model().applyPreset(preset_id);
    const bool changed = before != model().buildPreviewSnapshot().grid_rows;
    rebuildSnapshot();
    return changed;
}

PatternFieldModel& PatternFieldPanel::model() {
    return m_bound_model ? *m_bound_model : m_model;
}

const PatternFieldModel& PatternFieldPanel::model() const {
    return m_bound_model ? *m_bound_model : m_model;
}

void PatternFieldPanel::rebuildSnapshot() {
    const auto preview = model().buildPreviewSnapshot();
    m_snapshot = {};
    m_snapshot.visible = m_visible;
    m_snapshot.name = preview.name;
    m_snapshot.is_valid = preview.is_valid;
    m_snapshot.issues = preview.issues;
    m_snapshot.grid_rows = preview.grid_rows;
    m_snapshot.viewport_bounds = model().getViewportBounds();
    m_snapshot.viewport_size = model().getViewportSize();
    if (const auto pattern = model().getCurrentPattern()) {
        m_snapshot.active_point_count = pattern->getPoints().size();
    }
    rebuildControlState();
}

void PatternFieldPanel::rebuildControlState() {
    const bool hasPattern = model().getCurrentPattern() != nullptr;
    const bool hasPoints = m_snapshot.active_point_count > 0;
    const bool hasPresets = !model().availablePresets().empty();
    m_snapshot.controls = {
        {"set_name", "Set Name", hasPattern, hasPattern ? "" : "No pattern is loaded."},
        {"toggle_point", "Toggle Point", hasPattern, hasPattern ? "" : "No pattern is loaded."},
        {"clear_pattern", "Clear Pattern", hasPoints, hasPoints ? "" : "No selected pattern points to clear."},
        {"resize_viewport", "Resize Viewport", true, ""},
        {"apply_preset", "Apply Preset", hasPresets, hasPresets ? "" : "No pattern presets are available."},
    };
}

} // namespace urpg::editor
