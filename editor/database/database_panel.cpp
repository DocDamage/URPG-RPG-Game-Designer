#include "editor/database/database_panel.h"

#include <charconv>
#include <set>
#include <utility>

namespace urpg::editor {

void DatabasePanel::setDatabase(database::RpgDatabase database) {
    database_ = std::move(database);
    authority_ = &database_;
    refreshFromAuthority();
}

void DatabasePanel::bindDatabase(database::RpgDatabase& database) {
    authority_ = &database;
    refreshFromAuthority();
}

void DatabasePanel::refreshFromAuthority() {
    tables_.project(*authority_);
}

bool DatabasePanel::synchronizeRows(const DatabaseTableKind kind, const std::vector<DatabaseTableRow>& rows,
                                    database::RpgDatabase& output, std::string* diagnostic) const {
    if (kind != DatabaseTableKind::Actors && kind != DatabaseTableKind::Items) {
        if (diagnostic) *diagnostic = "database_batch_owner_unavailable";
        return false;
    }
    const auto parseInteger = [](const DatabaseTableRow& row, const std::string& field, int32_t& value) {
        const auto found = row.fields.find(field);
        if (found == row.fields.end()) return false;
        const auto* begin = found->second.data();
        const auto* end = begin + found->second.size();
        const auto parsed = std::from_chars(begin, end, value);
        return parsed.ec == std::errc{} && parsed.ptr == end;
    };
    std::set<std::string> row_ids;
    for (const auto& row : rows) {
        if (row.id.empty() || row.name.empty() || !row_ids.insert(row.id).second) {
            if (diagnostic) *diagnostic = "database_batch_row_invalid:" + row.id;
            return false;
        }
        if (kind == DatabaseTableKind::Actors) {
            int32_t max_hp = 0;
            int32_t attack = 0;
            const auto class_id = row.fields.find("class_id");
            if (class_id == row.fields.end() || class_id->second.empty() || !parseInteger(row, "max_hp", max_hp) ||
                !parseInteger(row, "attack", attack) || max_hp < 1 || attack < 1) {
                if (diagnostic) *diagnostic = "database_batch_actor_invalid:" + row.id;
                return false;
            }
            output.upsertActor({row.id, row.name, class_id->second, max_hp, attack});
        } else {
            int32_t price = -1;
            if (!parseInteger(row, "price", price) || price < 0) {
                if (diagnostic) *diagnostic = "database_batch_item_invalid:" + row.id;
                return false;
            }
            const auto existing = output.items().find(row.id);
            const auto tags = existing == output.items().end() ? std::set<std::string>{} : existing->second.tags;
            output.upsertItem({row.id, row.name, price, tags});
        }
    }
    if (kind == DatabaseTableKind::Actors) {
        std::vector<std::string> removed;
        for (const auto& [id, _] : output.actors()) if (!row_ids.contains(id)) removed.push_back(id);
        for (const auto& id : removed) (void)output.eraseActor(id);
    } else {
        std::vector<std::string> removed;
        for (const auto& [id, _] : output.items()) if (!row_ids.contains(id)) removed.push_back(id);
        for (const auto& id : removed) (void)output.eraseItem(id);
    }
    return true;
}

bool DatabasePanel::applyBatch(DatabaseBatchEditPlan& plan, std::string* diagnostic) {
    auto next = *authority_;
    if (!synchronizeRows(plan.kind, plan.after, next, diagnostic)) return false;
    if (!DatabaseBatchEditService::apply(tables_.table(plan.kind), plan)) {
        if (diagnostic) *diagnostic = "database_batch_apply_rejected";
        return false;
    }
    *authority_ = std::move(next);
    if (diagnostic) diagnostic->clear();
    return true;
}

bool DatabasePanel::undoBatch(DatabaseBatchEditPlan& plan, std::string* diagnostic) {
    auto previous = *authority_;
    if (!synchronizeRows(plan.kind, plan.before, previous, diagnostic)) return false;
    if (!DatabaseBatchEditService::undo(tables_.table(plan.kind), plan)) {
        if (diagnostic) *diagnostic = "database_batch_undo_rejected";
        return false;
    }
    *authority_ = std::move(previous);
    if (diagnostic) diagnostic->clear();
    return true;
}

bool DatabasePanel::redoBatch(DatabaseBatchEditPlan& plan, std::string* diagnostic) {
    auto next = *authority_;
    if (!synchronizeRows(plan.kind, plan.after, next, diagnostic)) return false;
    if (!DatabaseBatchEditService::redo(tables_.table(plan.kind), plan)) {
        if (diagnostic) *diagnostic = "database_batch_redo_rejected";
        return false;
    }
    *authority_ = std::move(next);
    if (diagnostic) diagnostic->clear();
    return true;
}

void DatabasePanel::bindReferenceIndex(const project::ProjectReferenceIndex* index) {
    reference_index_ = index;
    pending_record_operation_.reset();
}

DatabaseRecordOperationPreview DatabasePanel::previewRecordOperation(DatabaseRecordOperationRequest request) {
    if (reference_index_ == nullptr) {
        pending_record_operation_.reset();
        return {false, false, "database_reference_index_not_bound",
                "Bind the project reference index before reviewing a database record operation.", {},
                std::move(request), {}};
    }
    auto preview = record_operations_.preview(tables_, *reference_index_, request);
    pending_record_operation_ = preview;
    return preview;
}

project::ProjectOperationResult DatabasePanel::applyReviewedRecordOperation(
    std::vector<project::ProjectOperationParticipant> participants) {
    if (!pending_record_operation_) {
        return {false, false, "database_record_operation_review_missing",
                "Preview the database record impact before applying it.", {}};
    }
    auto result = record_operations_.execute(*pending_record_operation_, std::move(participants));
    if (result.success) {
        pending_record_operation_.reset();
        refreshFromAuthority();
    }
    return result;
}

project::ProjectOperationResult DatabasePanel::undoRecordOperation() {
    auto result = record_operations_.undoLast();
    if (result.success) refreshFromAuthority();
    return result;
}

project::ProjectOperationResult DatabasePanel::redoRecordOperation() {
    auto result = record_operations_.redoLast();
    if (result.success) refreshFromAuthority();
    return result;
}

DatabasePanelSnapshot DatabasePanel::snapshot() const {
    DatabasePanelSnapshot result;
    result.actor_count = authority_->actors().size();
    result.item_count = authority_->items().size();
    result.diagnostic_count = authority_->validate().size();
    result.table_count = tables_.tableCount();
    if (pending_record_operation_) {
        result.reference_review_pending = true;
        result.reference_review_can_apply = pending_record_operation_->applicable;
        result.reference_review_code = pending_record_operation_->code;
        result.reference_review_use_count = pending_record_operation_->reference_plan.updates.size() +
                                            pending_record_operation_->reference_plan.blocked_references.size();
        result.reference_review_package_impact_count =
            pending_record_operation_->reference_plan.package_impact.size();
    }
    return result;
}

void DatabasePanel::render() {
    last_render_snapshot_ = snapshot();
}

} // namespace urpg::editor
