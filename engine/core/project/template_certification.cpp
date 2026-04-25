#include "engine/core/project/template_certification.h"

#include <algorithm>
#include <array>

namespace {

std::string severityName(urpg::project::CertificationSeverity severity) {
    return severity == urpg::project::CertificationSeverity::Error ? "error" : "advisory";
}

const std::vector<urpg::project::TemplateCertificationSuite>& suites() {
    using urpg::project::CertificationRequirement;
    using urpg::project::TemplateCertificationSuite;

    static const std::vector<TemplateCertificationSuite> kSuites = {
        {"jrpg", "READY", {{"battle_loop", "Map-to-battle-to-reward loop"}, {"save_loop", "Save/load loop"}}},
        {"visual_novel", "READY", {{"dialogue_loop", "Dialogue choice loop"}, {"save_loop", "Save/load loop"}}},
        {"turn_based_rpg", "READY", {{"turn_based_battle_loop", "Turn-based battle loop"}, {"save_loop", "Save/load loop"}}},
        {"tactics_rpg", "EXPERIMENTAL", {{"tactical_battle_loop", "Grid tactics loop"}, {"save_loop", "Save/load loop"}}},
        {"arpg", "EXPERIMENTAL", {{"action_combat_loop", "Action combat loop"}, {"save_loop", "Save/load loop"}}},
        {"monster_collector_rpg", "PLANNED", {{"capture_loop", "Monster capture loop"}, {"save_loop", "Save/load loop"}}},
        {"cozy_life_rpg", "PLANNED", {{"daily_life_loop", "Daily life schedule loop"}, {"save_loop", "Save/load loop"}}},
        {"metroidvania_lite", "PLANNED", {{"ability_gate_loop", "Ability-gated exploration loop"}, {"save_loop", "Save/load loop"}}},
        {"2_5d_rpg", "PLANNED", {{"spatial_navigation_loop", "2.5D spatial navigation loop"}, {"save_loop", "Save/load loop"}}},
    };
    return kSuites;
}

} // namespace

namespace urpg::project {

nlohmann::json CertificationReport::toJson() const {
    nlohmann::json issuesJson = nlohmann::json::array();
    for (const auto& issue : issues) {
        issuesJson.push_back({
            {"severity", severityName(issue.severity)},
            {"code", issue.code},
            {"detail", issue.detail},
        });
    }

    return {
        {"schema", "urpg.template_certification.v1"},
        {"templateId", templateId},
        {"passed", passed},
        {"issues", issuesJson},
    };
}

std::vector<TemplateCertificationSuite> TemplateCertification::defaultSuites() const {
    return suites();
}

CertificationReport TemplateCertification::certify(const nlohmann::json& projectDocument,
                                                   const std::string& templateId) const {
    CertificationReport report;
    report.templateId = templateId;
    const auto* suite = findSuite(templateId);
    if (!suite) {
        report.issues.push_back({CertificationSeverity::Error, "unknown_template", "No certification suite exists for template."});
        return report;
    }

    for (const auto& requirement : suite->requirements) {
        if (requirement.optional && projectDisablesOptionalFeature(projectDocument, requirement.id)) {
            continue;
        }

        if (!projectHasLoop(projectDocument, requirement.id)) {
            report.issues.push_back({
                requirement.optional ? CertificationSeverity::Advisory : CertificationSeverity::Error,
                "missing_required_loop",
                "Template '" + templateId + "' is missing loop '" + requirement.id + "'.",
            });
        }
    }

    report.passed = std::none_of(report.issues.begin(), report.issues.end(), [](const CertificationIssue& issue) {
        return issue.severity == CertificationSeverity::Error;
    });
    return report;
}

const TemplateCertificationSuite* TemplateCertification::findSuite(const std::string& templateId) const {
    const auto& allSuites = suites();
    const auto it = std::find_if(allSuites.begin(), allSuites.end(), [&](const TemplateCertificationSuite& suite) {
        return suite.templateId == templateId;
    });
    return it == allSuites.end() ? nullptr : &*it;
}

bool TemplateCertification::projectHasLoop(const nlohmann::json& projectDocument, const std::string& loopId) const {
    if (!projectDocument.contains("loops") || !projectDocument["loops"].is_array()) {
        return false;
    }

    for (const auto& loop : projectDocument["loops"]) {
        if (loop.is_string() && loop.get<std::string>() == loopId) {
            return true;
        }
        if (loop.is_object() && loop.value("id", "") == loopId && loop.value("implemented", true)) {
            return true;
        }
    }
    return false;
}

bool TemplateCertification::projectDisablesOptionalFeature(const nlohmann::json& projectDocument,
                                                           const std::string& featureId) const {
    if (!projectDocument.contains("disabledOptionalFeatures") || !projectDocument["disabledOptionalFeatures"].is_array()) {
        return false;
    }

    for (const auto& feature : projectDocument["disabledOptionalFeatures"]) {
        if (feature.is_string() && feature.get<std::string>() == featureId) {
            return true;
        }
    }
    return false;
}

} // namespace urpg::project
