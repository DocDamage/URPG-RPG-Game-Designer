#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::presentation {

enum class PacingFrictionCategory : uint8_t {
    TimeToControl,
    MenuDepth,
    TutorialInterruption,
    RepeatedConfirmation,
    RewardCadence,
    LoadMasking,
    IdleWait
};

struct PacingObservation {
    std::string id;
    std::string route;
    PacingFrictionCategory category = PacingFrictionCategory::TimeToControl;
    uint32_t measured_ms = 0;
    uint32_t count = 0;
    bool blocks_control = false;
    bool load_masked = false;
    bool avoidable = false;
    bool resolved = false;
    std::string annotation;
    std::string resolution;
};

struct PacingReviewIssue {
    std::string observation_id;
    std::string code;
    std::string message;
};

struct PacingReviewResult {
    bool complete = false;
    uint32_t observation_count = 0;
    uint32_t friction_count = 0;
    uint32_t resolved_avoidable_count = 0;
    std::vector<PacingReviewIssue> issues;
    std::string annotated_playthrough;
};

struct PacingBudgets {
    uint32_t time_to_control_ms = 5000;
    uint32_t maximum_menu_depth = 3;
    uint32_t tutorial_block_ms = 250;
    uint32_t maximum_confirmations = 1;
    uint32_t reward_gap_ms = 180000;
    uint32_t unmasked_load_ms = 250;
    uint32_t idle_wait_ms = 500;
};

class VerticalSlicePacingReview {
public:
    explicit VerticalSlicePacingReview(PacingBudgets budgets = {});

    PacingReviewResult review(std::vector<PacingObservation> observations) const;
    const PacingBudgets& budgets() const { return budgets_; }

private:
    PacingBudgets budgets_;
};

const char* pacingFrictionCategoryName(PacingFrictionCategory category);

} // namespace urpg::presentation
