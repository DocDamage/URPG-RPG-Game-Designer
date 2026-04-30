#include "editor/spatial/grid_part_playtest_panel.h"

#include "engine/core/map/grid_part_validator.h"

#include <algorithm>
#include <utility>

namespace urpg::editor {

namespace {

bool hasPropertyValue(const urpg::map::PlacedPartInstance& instance, const std::string& key, const std::string& value) {
    const auto found = instance.properties.find(key);
    return found != instance.properties.end() && found->second == value;
}

bool isSpawnPart(const urpg::map::PlacedPartInstance& instance) {
    return hasPropertyValue(instance, "role", "player_spawn") || hasPropertyValue(instance, "spawn", "player") ||
           instance.part_id.find("player_spawn") != std::string::npos ||
           instance.part_id.find("spawn.player") != std::string::npos;
}

bool containsCell(const std::vector<std::pair<int32_t, int32_t>>& cells, int32_t x, int32_t y) {
    return std::find(cells.begin(), cells.end(), std::pair<int32_t, int32_t>{x, y}) != cells.end();
}

bool partTouchesCells(const urpg::map::PlacedPartInstance& instance,
                      const std::vector<std::pair<int32_t, int32_t>>& cells) {
    for (int32_t y = instance.grid_y; y < instance.grid_y + instance.height; ++y) {
        for (int32_t x = instance.grid_x; x < instance.grid_x + instance.width; ++x) {
            if (containsCell(cells, x, y)) {
                return true;
            }
        }
    }
    return false;
}

void appendDiagnostics(std::vector<urpg::map::GridPartDiagnostic>& target,
                       const std::vector<urpg::map::GridPartDiagnostic>& source) {
    target.insert(target.end(), source.begin(), source.end());
}

bool hasBlockingDiagnostics(const std::vector<urpg::map::GridPartDiagnostic>& diagnostics) {
    return std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.severity == urpg::map::GridPartSeverity::Error ||
               diagnostic.severity == urpg::map::GridPartSeverity::Blocker;
    });
}

} // namespace

void GridPartPlaytestPanel::Render(const urpg::FrameContext& context) {
    (void)context;
    if (!m_visible) {
        return;
    }

    captureRenderSnapshot();
}

void GridPartPlaytestPanel::SetTargets(urpg::map::GridPartDocument* document,
                                       const urpg::map::GridPartCatalog* catalog) {
    document_ = document;
    catalog_ = catalog;
    compiled_runtime_.reset();
    running_ = false;
    returned_to_editor_ = false;
    captureRenderSnapshot();
}

void GridPartPlaytestPanel::SetRulesetProfile(urpg::map::GridRulesetProfile ruleset) {
    ruleset_ = std::move(ruleset);
    captureRenderSnapshot();
}

void GridPartPlaytestPanel::SetObjective(urpg::map::MapObjective objective) {
    objective_ = std::move(objective);
    captureRenderSnapshot();
}

bool GridPartPlaytestPanel::PlaytestFromStart() {
    const auto spawn = findSpawnPoint();
    if (!spawn.has_value()) {
        return launchPlaytest(0, 0, true);
    }
    return launchPlaytest(spawn->first, spawn->second, true);
}

bool GridPartPlaytestPanel::PlaytestFromHere(int32_t start_x, int32_t start_y) {
    return launchPlaytest(start_x, start_y, true);
}

bool GridPartPlaytestPanel::PlaytestObjectivePath() {
    return PlaytestFromStart();
}

bool GridPartPlaytestPanel::ReturnToEditor() {
    if (!running_) {
        captureRenderSnapshot();
        return false;
    }

    running_ = false;
    returned_to_editor_ = true;
    captureRenderSnapshot();
    return true;
}

bool GridPartPlaytestPanel::launchPlaytest(int32_t start_x, int32_t start_y, bool require_objective_path) {
    latest_result_ = {};
    compiled_runtime_.reset();
    running_ = false;
    returned_to_editor_ = false;

    if (document_ == nullptr || catalog_ == nullptr) {
        latest_result_.softlocked = true;
        captureRenderSnapshot();
        return false;
    }

    latest_result_.map_id = document_->mapId();
    latest_result_.start_x = start_x;
    latest_result_.start_y = start_y;
    latest_result_.end_x = start_x;
    latest_result_.end_y = start_y;

    const auto documentValidation = urpg::map::ValidateGridPartDocument(*document_, *catalog_);
    appendDiagnostics(latest_result_.diagnostics, documentValidation.diagnostics);

    const auto rulesetValidation = urpg::map::ValidateGridPartRuleset(*document_, *catalog_, ruleset_);
    appendDiagnostics(latest_result_.diagnostics, rulesetValidation.diagnostics);

    const auto objectiveValidation = urpg::map::ValidateMapObjective(*document_, *catalog_, objective_);
    appendDiagnostics(latest_result_.diagnostics, objectiveValidation.diagnostics);

    const auto reachability = urpg::map::ValidateReachability(*document_, *catalog_, ruleset_, objective_);
    appendDiagnostics(latest_result_.diagnostics, reachability.diagnostics);

    compiled_runtime_ = urpg::map::CompileGridPartRuntime(*document_, *catalog_);
    appendDiagnostics(latest_result_.diagnostics, compiled_runtime_->diagnostics);

    latest_result_.visited_instance_ids = collectVisitedInstanceIds(reachability.reachable_cells);

    if (hasBlockingDiagnostics(latest_result_.diagnostics) || !compiled_runtime_->ok) {
        latest_result_.softlocked = true;
        captureRenderSnapshot();
        return false;
    }

    if (const auto objectivePoint = findObjectivePoint(); objectivePoint.has_value()) {
        latest_result_.end_x = objectivePoint->first;
        latest_result_.end_y = objectivePoint->second;
        latest_result_.completed_objective =
            !require_objective_path ||
            containsCell(reachability.reachable_cells, objectivePoint->first, objectivePoint->second);
    } else {
        latest_result_.completed_objective = !require_objective_path;
    }

    latest_result_.softlocked = require_objective_path && !latest_result_.completed_objective;
    latest_result_.elapsed_seconds =
        static_cast<float>(
            std::max<int32_t>(1, std::abs(latest_result_.end_x - start_x) + std::abs(latest_result_.end_y - start_y))) *
        0.25f;
    running_ = !latest_result_.softlocked;
    captureRenderSnapshot();
    return running_;
}

std::optional<std::pair<int32_t, int32_t>> GridPartPlaytestPanel::findSpawnPoint() const {
    if (document_ == nullptr) {
        return std::nullopt;
    }

    for (const auto& part : document_->parts()) {
        if (isSpawnPart(part)) {
            return std::pair<int32_t, int32_t>{part.grid_x, part.grid_y};
        }
    }
    return std::nullopt;
}

std::optional<std::pair<int32_t, int32_t>> GridPartPlaytestPanel::findObjectivePoint() const {
    if (document_ == nullptr || objective_.target_instance_id.empty()) {
        return std::nullopt;
    }

    const auto* target = document_->findPart(objective_.target_instance_id);
    if (target == nullptr) {
        return std::nullopt;
    }
    return std::pair<int32_t, int32_t>{target->grid_x, target->grid_y};
}

std::vector<std::string>
GridPartPlaytestPanel::collectVisitedInstanceIds(const std::vector<std::pair<int32_t, int32_t>>& cells) const {
    std::vector<std::string> visited;
    if (document_ == nullptr) {
        return visited;
    }

    for (const auto& part : document_->parts()) {
        if (partTouchesCells(part, cells)) {
            visited.push_back(part.instance_id);
        }
    }
    std::sort(visited.begin(), visited.end());
    return visited;
}

void GridPartPlaytestPanel::captureRenderSnapshot() {
    last_render_snapshot_ = {};
    last_render_snapshot_.visible = m_visible;
    last_render_snapshot_.has_document = document_ != nullptr;
    last_render_snapshot_.has_catalog = catalog_ != nullptr;
    last_render_snapshot_.can_launch =
        document_ != nullptr && catalog_ != nullptr && latest_result_.diagnostics.empty();
    last_render_snapshot_.running = running_;
    last_render_snapshot_.returned_to_editor = returned_to_editor_;
    last_render_snapshot_.has_runtime = compiled_runtime_.has_value() && compiled_runtime_->ok;
    last_render_snapshot_.diagnostic_count = latest_result_.diagnostics.size();
    last_render_snapshot_.latest_result = latest_result_;
}

} // namespace urpg::editor
