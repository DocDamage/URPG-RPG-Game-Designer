#pragma once

#include "editor/ui/editor_panel.h"
#include "engine/core/map/grid_part_catalog.h"
#include "engine/core/map/grid_part_objective.h"
#include "engine/core/map/grid_part_reachability.h"
#include "engine/core/map/grid_part_ruleset.h"
#include "engine/core/map/grid_part_runtime_compiler.h"

#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

struct GridPartPlaytestResult {
    std::string map_id;
    bool completed_objective = false;
    bool player_died = false;
    bool softlocked = false;

    float elapsed_seconds = 0.0f;

    int32_t start_x = 0;
    int32_t start_y = 0;
    int32_t end_x = 0;
    int32_t end_y = 0;

    std::vector<std::string> visited_instance_ids;
    std::vector<urpg::map::GridPartDiagnostic> diagnostics;
};

class GridPartPlaytestPanel : public EditorPanel {
  public:
    struct RenderSnapshot {
        bool visible = true;
        bool has_document = false;
        bool has_catalog = false;
        bool can_launch = false;
        bool running = false;
        bool returned_to_editor = false;
        bool has_runtime = false;
        size_t diagnostic_count = 0;
        GridPartPlaytestResult latest_result;
    };

    GridPartPlaytestPanel() : EditorPanel("Grid Part Playtest") {}

    void Render(const urpg::FrameContext& context) override;

    void SetTargets(urpg::map::GridPartDocument* document, const urpg::map::GridPartCatalog* catalog);
    void SetRulesetProfile(urpg::map::GridRulesetProfile ruleset);
    void SetObjective(urpg::map::MapObjective objective);

    bool PlaytestFromStart();
    bool PlaytestFromHere(int32_t start_x, int32_t start_y);
    bool PlaytestObjectivePath();
    bool ReturnToEditor();

    const RenderSnapshot& lastRenderSnapshot() const { return last_render_snapshot_; }

  private:
    bool launchPlaytest(int32_t start_x, int32_t start_y, bool require_objective_path);
    std::optional<std::pair<int32_t, int32_t>> findSpawnPoint() const;
    std::optional<std::pair<int32_t, int32_t>> findObjectivePoint() const;
    std::vector<std::string> collectVisitedInstanceIds(const std::vector<std::pair<int32_t, int32_t>>& cells) const;
    void captureRenderSnapshot();

    urpg::map::GridPartDocument* document_ = nullptr;
    const urpg::map::GridPartCatalog* catalog_ = nullptr;
    urpg::map::GridRulesetProfile ruleset_ =
        urpg::map::MakeDefaultGridRulesetProfile(urpg::map::GridPartRuleset::TopDownJRPG);
    urpg::map::MapObjective objective_;
    std::optional<urpg::map::GridPartRuntimeCompileResult> compiled_runtime_;
    bool running_ = false;
    bool returned_to_editor_ = false;
    GridPartPlaytestResult latest_result_;
    RenderSnapshot last_render_snapshot_;
};

} // namespace urpg::editor
