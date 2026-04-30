#include "engine/core/map/grid_part_commands.h"

#include <algorithm>
#include <utility>

namespace urpg::map {

PlacePartCommand::PlacePartCommand(PlacedPartInstance instance) : instance_(std::move(instance)) {}

bool PlacePartCommand::apply(GridPartDocument& document) {
    if (!document.placePart(instance_)) {
        return false;
    }
    applied_ = true;
    return true;
}

bool PlacePartCommand::undo(GridPartDocument& document) {
    if (!applied_ || !document.removePart(instance_.instance_id)) {
        return false;
    }
    applied_ = false;
    return true;
}

std::string PlacePartCommand::label() const {
    return "Place Part";
}

RemovePartCommand::RemovePartCommand(std::string instance_id) : instance_id_(std::move(instance_id)) {}

bool RemovePartCommand::apply(GridPartDocument& document) {
    const auto* existing = document.findPart(instance_id_);
    if (existing == nullptr) {
        return false;
    }

    removed_ = *existing;
    if (!document.removePart(instance_id_)) {
        removed_.reset();
        return false;
    }

    applied_ = true;
    return true;
}

bool RemovePartCommand::undo(GridPartDocument& document) {
    if (!applied_ || !removed_.has_value() || !document.placePart(*removed_)) {
        return false;
    }

    applied_ = false;
    return true;
}

std::string RemovePartCommand::label() const {
    return "Remove Part";
}

MovePartCommand::MovePartCommand(std::string instance_id, int32_t next_x, int32_t next_y, int32_t next_z)
    : instance_id_(std::move(instance_id)), next_x_(next_x), next_y_(next_y), next_z_(next_z) {}

bool MovePartCommand::apply(GridPartDocument& document) {
    const auto* existing = document.findPart(instance_id_);
    if (existing == nullptr) {
        return false;
    }

    previous_x_ = existing->grid_x;
    previous_y_ = existing->grid_y;
    previous_z_ = existing->grid_z;
    if (!document.movePart(instance_id_, next_x_, next_y_, next_z_)) {
        return false;
    }

    applied_ = true;
    return true;
}

bool MovePartCommand::undo(GridPartDocument& document) {
    if (!applied_ || !document.movePart(instance_id_, previous_x_, previous_y_, previous_z_)) {
        return false;
    }

    applied_ = false;
    return true;
}

std::string MovePartCommand::label() const {
    return "Move Part";
}

ResizePartCommand::ResizePartCommand(std::string instance_id, int32_t next_width, int32_t next_height)
    : instance_id_(std::move(instance_id)), next_width_(next_width), next_height_(next_height) {}

bool ResizePartCommand::apply(GridPartDocument& document) {
    const auto* existing = document.findPart(instance_id_);
    if (existing == nullptr) {
        return false;
    }

    previous_width_ = existing->width;
    previous_height_ = existing->height;
    if (!document.resizePart(instance_id_, next_width_, next_height_)) {
        return false;
    }

    applied_ = true;
    return true;
}

bool ResizePartCommand::undo(GridPartDocument& document) {
    if (!applied_ || !document.resizePart(instance_id_, previous_width_, previous_height_)) {
        return false;
    }

    applied_ = false;
    return true;
}

std::string ResizePartCommand::label() const {
    return "Resize Part";
}

ReplacePartCommand::ReplacePartCommand(PlacedPartInstance replacement) : replacement_(std::move(replacement)) {}

bool ReplacePartCommand::apply(GridPartDocument& document) {
    const auto* existing = document.findPart(replacement_.instance_id);
    if (existing == nullptr) {
        return false;
    }

    previous_ = *existing;
    if (!document.replacePart(replacement_)) {
        previous_.reset();
        return false;
    }

    applied_ = true;
    return true;
}

bool ReplacePartCommand::undo(GridPartDocument& document) {
    if (!applied_ || !previous_.has_value() || !document.replacePart(*previous_)) {
        return false;
    }

    applied_ = false;
    return true;
}

std::string ReplacePartCommand::label() const {
    return "Replace Part";
}

ChangePartPropertyCommand::ChangePartPropertyCommand(std::string instance_id, std::string key,
                                                     std::optional<std::string> next_value)
    : instance_id_(std::move(instance_id)), key_(std::move(key)), next_value_(std::move(next_value)) {}

bool ChangePartPropertyCommand::apply(GridPartDocument& document) {
    auto* part = document.findPartMutable(instance_id_);
    if (part == nullptr || part->locked || key_.empty()) {
        return false;
    }

    const auto previous = part->properties.find(key_);
    had_previous_value_ = previous != part->properties.end();
    previous_value_ = had_previous_value_ ? std::make_optional(previous->second) : std::nullopt;

    if (next_value_.has_value()) {
        part->properties[key_] = *next_value_;
    } else {
        part->properties.erase(key_);
    }

    (void)document.markPartDirty(instance_id_);
    applied_ = true;
    return true;
}

bool ChangePartPropertyCommand::undo(GridPartDocument& document) {
    auto* part = document.findPartMutable(instance_id_);
    if (!applied_ || part == nullptr) {
        return false;
    }

    if (had_previous_value_ && previous_value_.has_value()) {
        part->properties[key_] = *previous_value_;
    } else {
        part->properties.erase(key_);
    }

    (void)document.markPartDirty(instance_id_);
    applied_ = false;
    return true;
}

std::string ChangePartPropertyCommand::label() const {
    return "Change Part Property";
}

BulkGridPartCommand::BulkGridPartCommand(std::vector<std::unique_ptr<IGridPartCommand>> commands, std::string label)
    : commands_(std::move(commands)), label_(std::move(label)) {}

bool BulkGridPartCommand::apply(GridPartDocument& document) {
    applied_count_ = 0;
    for (auto& command : commands_) {
        if (command == nullptr || !command->apply(document)) {
            for (size_t index = applied_count_; index > 0; --index) {
                (void)commands_[index - 1]->undo(document);
            }
            applied_count_ = 0;
            return false;
        }
        ++applied_count_;
    }
    return true;
}

bool BulkGridPartCommand::undo(GridPartDocument& document) {
    if (applied_count_ != commands_.size()) {
        return false;
    }

    bool ok = true;
    for (size_t index = applied_count_; index > 0; --index) {
        ok = commands_[index - 1]->undo(document) && ok;
    }
    applied_count_ = 0;
    return ok;
}

std::string BulkGridPartCommand::label() const {
    return label_;
}

bool GridPartCommandHistory::execute(GridPartDocument& document, std::unique_ptr<IGridPartCommand> command) {
    if (command == nullptr || !command->apply(document)) {
        return false;
    }

    undo_stack_.push_back(std::move(command));
    redo_stack_.clear();
    return true;
}

bool GridPartCommandHistory::undo(GridPartDocument& document) {
    if (undo_stack_.empty()) {
        return false;
    }

    auto command = std::move(undo_stack_.back());
    undo_stack_.pop_back();
    if (!command->undo(document)) {
        undo_stack_.push_back(std::move(command));
        return false;
    }

    redo_stack_.push_back(std::move(command));
    return true;
}

bool GridPartCommandHistory::redo(GridPartDocument& document) {
    if (redo_stack_.empty()) {
        return false;
    }

    auto command = std::move(redo_stack_.back());
    redo_stack_.pop_back();
    if (!command->apply(document)) {
        redo_stack_.push_back(std::move(command));
        return false;
    }

    undo_stack_.push_back(std::move(command));
    return true;
}

bool GridPartCommandHistory::canUndo() const {
    return !undo_stack_.empty();
}

bool GridPartCommandHistory::canRedo() const {
    return !redo_stack_.empty();
}

std::string GridPartCommandHistory::undoLabel() const {
    return undo_stack_.empty() ? std::string{} : undo_stack_.back()->label();
}

std::string GridPartCommandHistory::redoLabel() const {
    return redo_stack_.empty() ? std::string{} : redo_stack_.back()->label();
}

void GridPartCommandHistory::clear() {
    undo_stack_.clear();
    redo_stack_.clear();
}

} // namespace urpg::map
