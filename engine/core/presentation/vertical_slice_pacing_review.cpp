#include "engine/core/presentation/vertical_slice_pacing_review.h"

#include <algorithm>
#include <array>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

namespace urpg::presentation {
namespace {

constexpr size_t kCategoryCount = 7;

size_t categoryIndex(const PacingFrictionCategory category) {
    return static_cast<size_t>(category);
}

bool isFriction(const PacingObservation& row, const PacingBudgets& budgets) {
    switch (row.category) {
    case PacingFrictionCategory::TimeToControl:
        return row.measured_ms > budgets.time_to_control_ms;
    case PacingFrictionCategory::MenuDepth:
        return row.count > budgets.maximum_menu_depth;
    case PacingFrictionCategory::TutorialInterruption:
        return row.blocks_control && row.measured_ms > budgets.tutorial_block_ms;
    case PacingFrictionCategory::RepeatedConfirmation:
        return row.count > budgets.maximum_confirmations;
    case PacingFrictionCategory::RewardCadence:
        return row.measured_ms > budgets.reward_gap_ms;
    case PacingFrictionCategory::LoadMasking:
        return row.measured_ms > budgets.unmasked_load_ms && !row.load_masked;
    case PacingFrictionCategory::IdleWait:
        return row.measured_ms > budgets.idle_wait_ms && row.blocks_control;
    }
    return true;
}

void addIssue(PacingReviewResult& result, const PacingObservation& row, std::string code, std::string message) {
    result.issues.push_back({row.id, std::move(code), std::move(message)});
}

} // namespace

const char* pacingFrictionCategoryName(const PacingFrictionCategory category) {
    switch (category) {
    case PacingFrictionCategory::TimeToControl: return "time_to_control";
    case PacingFrictionCategory::MenuDepth: return "menu_depth";
    case PacingFrictionCategory::TutorialInterruption: return "tutorial_interruption";
    case PacingFrictionCategory::RepeatedConfirmation: return "repeated_confirmation";
    case PacingFrictionCategory::RewardCadence: return "reward_cadence";
    case PacingFrictionCategory::LoadMasking: return "load_masking";
    case PacingFrictionCategory::IdleWait: return "idle_wait";
    }
    return "unknown";
}

VerticalSlicePacingReview::VerticalSlicePacingReview(PacingBudgets budgets) : budgets_(budgets) {}

PacingReviewResult VerticalSlicePacingReview::review(std::vector<PacingObservation> observations) const {
    PacingReviewResult result;
    result.observation_count = static_cast<uint32_t>(observations.size());
    std::stable_sort(observations.begin(), observations.end(), [](const auto& left, const auto& right) {
        return std::tie(left.route, left.id) < std::tie(right.route, right.id);
    });

    std::set<std::string> ids;
    std::array<bool, kCategoryCount> covered{};
    for (const auto& row : observations) {
        if (row.id.empty() || row.route.empty()) {
            addIssue(result, row, "pacing_identity_missing", "Every observation requires a stable ID and route.");
        } else if (!ids.insert(row.id).second) {
            addIssue(result, row, "pacing_id_duplicate", "Observation IDs must be unique.");
        }
        const auto index = categoryIndex(row.category);
        if (index >= covered.size()) {
            addIssue(result, row, "pacing_category_invalid", "Observation category is invalid.");
            continue;
        }
        covered[index] = true;
        const bool friction = isFriction(row, budgets_);
        if (friction) {
            ++result.friction_count;
            if (row.annotation.empty()) {
                addIssue(result, row, "pacing_friction_unannotated", "Every budget breach requires an annotation.");
            }
            if (row.avoidable) {
                if (!row.resolved || row.resolution.empty()) {
                    addIssue(result, row, "pacing_avoidable_unresolved",
                             "Every avoidable friction point requires an implemented resolution.");
                } else {
                    ++result.resolved_avoidable_count;
                }
            } else if (row.resolution.empty()) {
                addIssue(result, row, "pacing_unavoidable_unmitigated",
                         "Unavoidable friction requires a documented mitigation or masking treatment.");
            }
        } else if (row.avoidable || row.resolved) {
            addIssue(result, row, "pacing_disposition_inaccurate",
                     "An observation within budget cannot be classified as avoidable or resolved friction.");
        }
    }

    for (size_t index = 0; index < covered.size(); ++index) {
        if (!covered[index]) {
            const auto category = static_cast<PacingFrictionCategory>(index);
            result.issues.push_back({{}, "pacing_category_missing",
                                     std::string("Playthrough does not cover ") + pacingFrictionCategoryName(category) + "."});
        }
    }

    std::ostringstream markdown;
    markdown << "# Vertical-slice pacing review\n\n"
             << "| Route | Checkpoint | Category | Measurement | Finding | Resolution |\n"
             << "| --- | --- | --- | ---: | --- | --- |\n";
    for (const auto& row : observations) {
        const bool friction = isFriction(row, budgets_);
        const std::string measurement = row.category == PacingFrictionCategory::MenuDepth ||
                                                row.category == PacingFrictionCategory::RepeatedConfirmation
                                            ? std::to_string(row.count)
                                            : std::to_string(row.measured_ms) + " ms";
        markdown << "| " << row.route << " | " << row.id << " | " << pacingFrictionCategoryName(row.category)
                 << " | " << measurement << " | "
                 << (friction ? row.annotation : "Within budget") << " | "
                 << (friction ? row.resolution : "None required") << " |\n";
    }
    result.annotated_playthrough = markdown.str();
    result.complete = result.issues.empty() && std::all_of(covered.begin(), covered.end(), [](const bool value) {
        return value;
    });
    return result;
}

} // namespace urpg::presentation
