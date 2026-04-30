#pragma once

#include "engine/core/map/grid_part_document.h"

#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace urpg::map {

class IGridPartCommand {
  public:
    virtual ~IGridPartCommand() = default;
    virtual bool apply(GridPartDocument& document) = 0;
    virtual bool undo(GridPartDocument& document) = 0;
    virtual std::string label() const = 0;
};

class PlacePartCommand final : public IGridPartCommand {
  public:
    explicit PlacePartCommand(PlacedPartInstance instance);

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    PlacedPartInstance instance_;
    bool applied_ = false;
};

class RemovePartCommand final : public IGridPartCommand {
  public:
    explicit RemovePartCommand(std::string instance_id);

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    std::string instance_id_;
    std::optional<PlacedPartInstance> removed_;
    bool applied_ = false;
};

class MovePartCommand final : public IGridPartCommand {
  public:
    MovePartCommand(std::string instance_id, int32_t next_x, int32_t next_y, int32_t next_z = 0);

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    std::string instance_id_;
    int32_t next_x_ = 0;
    int32_t next_y_ = 0;
    int32_t next_z_ = 0;
    int32_t previous_x_ = 0;
    int32_t previous_y_ = 0;
    int32_t previous_z_ = 0;
    bool applied_ = false;
};

class ResizePartCommand final : public IGridPartCommand {
  public:
    ResizePartCommand(std::string instance_id, int32_t next_width, int32_t next_height);

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    std::string instance_id_;
    int32_t next_width_ = 1;
    int32_t next_height_ = 1;
    int32_t previous_width_ = 1;
    int32_t previous_height_ = 1;
    bool applied_ = false;
};

class ReplacePartCommand final : public IGridPartCommand {
  public:
    explicit ReplacePartCommand(PlacedPartInstance replacement);

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    PlacedPartInstance replacement_;
    std::optional<PlacedPartInstance> previous_;
    bool applied_ = false;
};

class ChangePartPropertyCommand final : public IGridPartCommand {
  public:
    ChangePartPropertyCommand(std::string instance_id, std::string key, std::optional<std::string> next_value);

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    std::string instance_id_;
    std::string key_;
    std::optional<std::string> next_value_;
    std::optional<std::string> previous_value_;
    bool had_previous_value_ = false;
    bool applied_ = false;
};

class BulkGridPartCommand final : public IGridPartCommand {
  public:
    explicit BulkGridPartCommand(std::vector<std::unique_ptr<IGridPartCommand>> commands,
                                 std::string label = "Bulk Grid Part Edit");

    bool apply(GridPartDocument& document) override;
    bool undo(GridPartDocument& document) override;
    std::string label() const override;

  private:
    std::vector<std::unique_ptr<IGridPartCommand>> commands_;
    std::string label_;
    size_t applied_count_ = 0;
};

class GridPartCommandHistory {
  public:
    bool execute(GridPartDocument& document, std::unique_ptr<IGridPartCommand> command);
    bool undo(GridPartDocument& document);
    bool redo(GridPartDocument& document);

    bool canUndo() const;
    bool canRedo() const;
    std::string undoLabel() const;
    std::string redoLabel() const;
    void clear();

  private:
    std::vector<std::unique_ptr<IGridPartCommand>> undo_stack_;
    std::vector<std::unique_ptr<IGridPartCommand>> redo_stack_;
};

} // namespace urpg::map
