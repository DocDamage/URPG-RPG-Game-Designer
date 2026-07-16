#include "engine/core/presentation/vertical_slice_pacing_review.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

using Category = urpg::presentation::PacingFrictionCategory;
using Observation = urpg::presentation::PacingObservation;

std::vector<Observation> resolvedPlaythrough() {
    return {
        {"boot_control", "startup -> title -> gameplay", Category::TimeToControl, 4200, 0, false, false,
         false, false, {}, {}},
        {"pause_depth", "gameplay -> pause -> settings", Category::MenuDepth, 0, 3,
         false, false, false, false, {}, {}},
        {"tutorial_prompt", "gameplay first interaction", Category::TutorialInterruption, 200, 0, true,
         false, false, false, {}, {}},
        {"quit_confirm", "pause -> quit", Category::RepeatedConfirmation, 0, 1,
         false, false, false, false, {}, {}},
        {"first_reward", "gameplay -> first reward", Category::RewardCadence, 195000, 0, false, false, true, true,
         "The first reward arrived 15 seconds beyond the cadence budget.",
         "Moved the guaranteed discovery reward to the first optional interaction."},
        {"map_transfer", "exploration map transfer", Category::LoadMasking, 480, 0, true, false, false, false,
         "Streaming exceeds the unmasked-load budget on minimum hardware.",
         "Keep control and transition animation active until the destination is ready."},
        {"battle_results", "battle victory -> results", Category::IdleWait, 780, 0, true, false, true, true,
         "A fixed flourish blocked result confirmation after rewards were readable.",
         "Made the flourish skippable immediately after the reward summary appears."},
    };
}

bool hasIssue(const urpg::presentation::PacingReviewResult& result, const char* code) {
    return std::any_of(result.issues.begin(), result.issues.end(), [&](const auto& issue) { return issue.code == code; });
}

} // namespace

TEST_CASE("Holistic pacing review covers all seven routes and resolves avoidable friction",
          "[presentation][pacing][pcq607]") {
    const urpg::presentation::VerticalSlicePacingReview review;
    const auto result = review.review(resolvedPlaythrough());

    REQUIRE(result.complete);
    REQUIRE(result.observation_count == 7);
    REQUIRE(result.friction_count == 3);
    REQUIRE(result.resolved_avoidable_count == 2);
    REQUIRE(result.issues.empty());
    for (const auto* category : {"time_to_control", "menu_depth", "tutorial_interruption",
                                 "repeated_confirmation", "reward_cadence", "load_masking", "idle_wait"}) {
        REQUIRE(result.annotated_playthrough.find(category) != std::string::npos);
    }
    REQUIRE(result.annotated_playthrough.find("Moved the guaranteed discovery reward") != std::string::npos);
    REQUIRE(result.annotated_playthrough.find("Keep control and transition animation active") != std::string::npos);
}

TEST_CASE("Holistic pacing review rejects every unresolved avoidable friction point",
          "[presentation][pacing][pcq607]") {
    auto observations = resolvedPlaythrough();
    auto& reward = observations[4];
    reward.resolved = false;
    reward.resolution.clear();

    const auto result = urpg::presentation::VerticalSlicePacingReview{}.review(std::move(observations));
    REQUIRE_FALSE(result.complete);
    REQUIRE(hasIssue(result, "pacing_avoidable_unresolved"));
}

TEST_CASE("Holistic pacing review requires complete annotated category coverage",
          "[presentation][pacing][pcq607]") {
    auto observations = resolvedPlaythrough();
    observations.pop_back();
    observations[0].id.clear();
    observations[5].annotation.clear();
    observations[5].resolution.clear();

    const auto result = urpg::presentation::VerticalSlicePacingReview{}.review(std::move(observations));
    REQUIRE_FALSE(result.complete);
    REQUIRE(hasIssue(result, "pacing_identity_missing"));
    REQUIRE(hasIssue(result, "pacing_friction_unannotated"));
    REQUIRE(hasIssue(result, "pacing_unavoidable_unmitigated"));
    REQUIRE(hasIssue(result, "pacing_category_missing"));
}
