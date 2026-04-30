#include "editor/spatial/grid_part_inspector_panel.h"

#include <memory>

namespace urpg::editor {

void GridPartInspectorPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void GridPartInspectorPanel::SetTargets(urpg::map::GridPartDocument* document,
                                        const urpg::map::GridPartCatalog* catalog) {
    document_ = document;
    catalog_ = catalog;
    if (document_ == nullptr ||
        (!selected_instance_id_.empty() && document_->findPart(selected_instance_id_) == nullptr)) {
        selected_instance_id_.clear();
    }
    captureRenderSnapshot();
}

bool GridPartInspectorPanel::SelectInstance(const std::string& instance_id) {
    if (document_ == nullptr || document_->findPart(instance_id) == nullptr) {
        return false;
    }

    selected_instance_id_ = instance_id;
    captureRenderSnapshot();
    return true;
}

bool GridPartInspectorPanel::SetProperty(const std::string& key, const std::string& value) {
    if (document_ == nullptr || selected_instance_id_.empty()) {
        return false;
    }

    auto command = std::make_unique<urpg::map::ChangePartPropertyCommand>(selected_instance_id_, key, value);
    const bool changed = history_.execute(*document_, std::move(command));
    captureRenderSnapshot();
    return changed;
}

bool GridPartInspectorPanel::ClearProperty(const std::string& key) {
    if (document_ == nullptr || selected_instance_id_.empty()) {
        return false;
    }

    auto command = std::make_unique<urpg::map::ChangePartPropertyCommand>(selected_instance_id_, key, std::nullopt);
    const bool changed = history_.execute(*document_, std::move(command));
    captureRenderSnapshot();
    return changed;
}

bool GridPartInspectorPanel::Undo() {
    if (document_ == nullptr) {
        return false;
    }

    const bool undone = history_.undo(*document_);
    captureRenderSnapshot();
    return undone;
}

bool GridPartInspectorPanel::Redo() {
    if (document_ == nullptr) {
        return false;
    }

    const bool redone = history_.redo(*document_);
    captureRenderSnapshot();
    return redone;
}

void GridPartInspectorPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_document = document_ != nullptr;
    last_render_snapshot_.has_catalog = catalog_ != nullptr;
    last_render_snapshot_.selected_instance_id = selected_instance_id_;
    last_render_snapshot_.can_undo = history_.canUndo();
    last_render_snapshot_.can_redo = history_.canRedo();

    if (document_ == nullptr || selected_instance_id_.empty()) {
        return;
    }

    const auto* selected = document_->findPart(selected_instance_id_);
    if (selected == nullptr) {
        last_render_snapshot_.selected_instance_id.clear();
        return;
    }

    last_render_snapshot_.has_selection = true;
    last_render_snapshot_.can_edit = !selected->locked;
    last_render_snapshot_.selected_part_id = selected->part_id;
    last_render_snapshot_.grid_x = selected->grid_x;
    last_render_snapshot_.grid_y = selected->grid_y;
    last_render_snapshot_.width = selected->width;
    last_render_snapshot_.height = selected->height;
    last_render_snapshot_.locked = selected->locked;
    last_render_snapshot_.hidden = selected->hidden;
    last_render_snapshot_.properties = selected->properties;
}

} // namespace urpg::editor
