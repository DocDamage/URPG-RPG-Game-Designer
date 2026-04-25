#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::ai {

enum class AiSuggestionReviewState {
    Draft,
    NeedsReview,
    Approved,
    Rejected,
    Applied
};

struct AiSuggestionRecord {
    std::string id;
    std::string targetPath;
    std::string summary;
    std::string provenance;
    bool generatedContent = true;
    AiSuggestionReviewState reviewState = AiSuggestionReviewState::NeedsReview;
    std::vector<std::string> sourceNotes;
};

class AiSuggestionPolicy {
public:
    bool canApply(const AiSuggestionRecord& record) const;
    nlohmann::json toJson(const AiSuggestionRecord& record) const;

private:
    bool isProtectedRuntimeStatusDoc(const std::string& path) const;
};

} // namespace urpg::ai
