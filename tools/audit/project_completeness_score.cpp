#include "tools/audit/project_completeness_score.h"

#include <algorithm>

namespace {

bool arrayContainsString(const nlohmann::json& value, const std::string& expected) {
    if (!value.is_array()) {
        return false;
    }

    for (const auto& entry : value) {
        if (entry.is_string() && entry.get<std::string>() == expected) {
            return true;
        }
    }
    return false;
}

bool isDisabledOptional(const nlohmann::json& projectDocument, const std::string& id) {
    return projectDocument.contains("disabledOptionalFeatures") &&
           arrayContainsString(projectDocument["disabledOptionalFeatures"], id);
}

bool isSatisfied(const nlohmann::json& projectDocument, const std::string& id) {
    if (projectDocument.contains("implementedFeatures") &&
        arrayContainsString(projectDocument["implementedFeatures"], id)) {
        return true;
    }
    if (projectDocument.contains("loops") && arrayContainsString(projectDocument["loops"], id)) {
        return true;
    }
    return false;
}

const nlohmann::json* findSubsystem(const nlohmann::json& readiness, const std::string& id) {
    if (!readiness.contains("subsystems") || !readiness["subsystems"].is_array()) {
        return nullptr;
    }
    for (const auto& subsystem : readiness["subsystems"]) {
        if (subsystem.value("id", "") == id) {
            return &subsystem;
        }
    }
    return nullptr;
}

} // namespace

namespace urpg::tools::audit {

nlohmann::json ProjectCompletenessScore::toJson() const {
    return {
        {"schema", "urpg.project_completeness_score.v1"},
        {"advisoryOnly", advisoryOnly},
        {"score", score},
        {"satisfied", satisfied},
        {"applicable", applicable},
        {"missingRequired", missingRequired},
    };
}

ProjectCompletenessScore scoreProjectCompleteness(const nlohmann::json& projectDocument,
                                                  const std::vector<CompletenessRequirement>& requirements) {
    ProjectCompletenessScore result;
    result.satisfied = 0;
    result.applicable = 0;

    for (const auto& requirement : requirements) {
        if (requirement.optional && isDisabledOptional(projectDocument, requirement.id)) {
            continue;
        }

        ++result.applicable;
        if (isSatisfied(projectDocument, requirement.id)) {
            ++result.satisfied;
        } else if (!requirement.optional) {
            result.missingRequired.push_back(requirement.id);
        }
    }

    result.score = result.applicable == 0 ? 1.0 : static_cast<double>(result.satisfied) / result.applicable;
    return result;
}

nlohmann::json scoreTemplateReadinessCompleteness(const nlohmann::json& readiness,
                                                  const nlohmann::json& templateRecord) {
    nlohmann::json missing = nlohmann::json::array();
    int applicable = 0;
    int satisfied = 0;

    if (templateRecord.contains("requiredSubsystems") && templateRecord["requiredSubsystems"].is_array()) {
        for (const auto& required : templateRecord["requiredSubsystems"]) {
            if (!required.is_string()) {
                continue;
            }

            ++applicable;
            const auto* subsystem = findSubsystem(readiness, required.get<std::string>());
            if (subsystem && (subsystem->value("status", "") == "READY" || subsystem->value("status", "") == "PARTIAL")) {
                ++satisfied;
            } else {
                missing.push_back(required.get<std::string>());
            }
        }
    }

    const double score = applicable == 0 ? 1.0 : static_cast<double>(satisfied) / applicable;
    return {
        {"schema", "urpg.project_completeness_score.v1"},
        {"advisoryOnly", true},
        {"nonAuthoritative", true},
        {"score", score},
        {"satisfied", satisfied},
        {"applicable", applicable},
        {"missingRequiredSubsystems", missing},
    };
}

} // namespace urpg::tools::audit
