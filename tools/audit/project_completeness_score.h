#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::tools::audit {

struct CompletenessRequirement {
    std::string id;
    bool optional = false;
};

struct ProjectCompletenessScore {
    double score = 1.0;
    int satisfied = 0;
    int applicable = 0;
    bool advisoryOnly = true;
    std::vector<std::string> missingRequired;
    nlohmann::json toJson() const;
};

ProjectCompletenessScore scoreProjectCompleteness(const nlohmann::json& projectDocument,
                                                  const std::vector<CompletenessRequirement>& requirements);

nlohmann::json scoreTemplateReadinessCompleteness(const nlohmann::json& readiness,
                                                  const nlohmann::json& templateRecord);

} // namespace urpg::tools::audit
