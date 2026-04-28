#include "engine/core/project/template_certification.h"

#include "engine/core/project/template_runtime_profile.h"

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
        {"tactics_rpg", "READY", {{"tactical_battle_loop", "Grid tactics loop"}, {"scenario_authoring_loop", "Scenario authoring loop"}, {"save_loop", "Save/load loop"}}},
        {"arpg", "READY", {{"action_combat_loop", "Action combat loop"}, {"growth_loop", "Growth loop"}, {"save_loop", "Save/load loop"}}},
        {"monster_collector_rpg", "READY", {{"capture_loop", "Monster capture loop"}, {"party_assembly_loop", "Party assembly loop"}, {"battle_loop", "Battle loop"}, {"save_loop", "Save/load loop"}}},
        {"cozy_life_rpg", "READY", {{"daily_life_loop", "Daily life schedule loop"}, {"relationship_loop", "Relationship loop"}, {"crafting_loop", "Crafting loop"}, {"economy_loop", "Economy loop"}, {"save_loop", "Save/load loop"}}},
        {"metroidvania_lite", "READY", {{"ability_gate_loop", "Ability-gated exploration loop"}, {"map_unlock_loop", "Map unlock loop"}, {"traversal_loop", "Traversal loop"}, {"save_loop", "Save/load loop"}}},
        {"2_5d_rpg", "READY", {{"spatial_navigation_loop", "2.5D spatial navigation loop"}, {"raycast_authoring_loop", "Raycast authoring loop"}, {"save_loop", "Save/load loop"}}},
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

    const auto profile = findTemplateRuntimeProfile(templateId);
    if (!profile.has_value()) {
        report.issues.push_back({CertificationSeverity::Error, "missing_runtime_profile",
                                 "Template runtime profile is missing."});
    } else {
        for (const auto& issue : validateTemplateRuntimeProfile(*profile)) {
            report.issues.push_back({CertificationSeverity::Error, "invalid_runtime_profile", issue});
        }
        if (projectDocument.contains("template_bars") && projectDocument["template_bars"].is_object()) {
            for (const auto& [bar, value] : profile->bars.items()) {
                if (!projectDocument["template_bars"].contains(bar) ||
                    projectDocument["template_bars"][bar].value("status", "") != "READY") {
                    report.issues.push_back({CertificationSeverity::Error, "template_bar_not_ready",
                                             "Template '" + templateId + "' has non-ready bar '" + bar + "'."});
                }
            }
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
