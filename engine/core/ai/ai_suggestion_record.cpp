#include "engine/core/ai/ai_suggestion_record.h"

#include <algorithm>
#include <cctype>

namespace {

std::string stateName(urpg::ai::AiSuggestionReviewState state) {
    using urpg::ai::AiSuggestionReviewState;
    switch (state) {
    case AiSuggestionReviewState::Draft:
        return "draft";
    case AiSuggestionReviewState::NeedsReview:
        return "needs_review";
    case AiSuggestionReviewState::Approved:
        return "approved";
    case AiSuggestionReviewState::Rejected:
        return "rejected";
    case AiSuggestionReviewState::Applied:
        return "applied";
    }
    return "unknown";
}

} // namespace

namespace urpg::ai {

bool AiSuggestionPolicy::canApply(const AiSuggestionRecord& record) const {
    return record.reviewState == AiSuggestionReviewState::Approved && !isProtectedRuntimeStatusDoc(record.targetPath);
}

nlohmann::json AiSuggestionPolicy::toJson(const AiSuggestionRecord& record) const {
    return {
        {"id", record.id},
        {"target_path", record.targetPath},
        {"summary", record.summary},
        {"provenance", record.provenance},
        {"generated_content", record.generatedContent},
        {"review_state", stateName(record.reviewState)},
        {"source_notes", record.sourceNotes},
        {"can_apply", canApply(record)},
    };
}

bool AiSuggestionPolicy::isProtectedRuntimeStatusDoc(const std::string& path) const {
    auto lowered = path;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return lowered.find("program_completion_status.md") != std::string::npos ||
           lowered.find("technical_debt_remediation_plan.md") != std::string::npos;
}

} // namespace urpg::ai
