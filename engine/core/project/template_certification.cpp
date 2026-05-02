#include "engine/core/project/template_certification.h"

#include "engine/core/project/template_runtime_profile.h"

#include <algorithm>
#include <cctype>

namespace {

std::string severityName(urpg::project::CertificationSeverity severity) {
    return severity == urpg::project::CertificationSeverity::Error ? "error" : "advisory";
}

const std::vector<urpg::project::TemplateCertificationSuite>& suites() {
    using urpg::project::CertificationRequirement;
    using urpg::project::TemplateCertificationSuite;

    static const std::vector<TemplateCertificationSuite> kSuites = [] {
        std::vector<TemplateCertificationSuite> generated;
        for (const auto& profile : urpg::project::allTemplateRuntimeProfiles()) {
            std::vector<CertificationRequirement> requirements;
            for (const auto& loop : profile.loops) {
                std::string label = loop;
                for (char& c : label) {
                    if (c == '_') {
                        c = ' ';
                    }
                }
                if (!label.empty()) {
                    label[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(label[0])));
                }
                requirements.push_back({loop, label, false});
            }
            generated.push_back({profile.id, profile.status, std::move(requirements)});
        }
        return generated;
    }();
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
        report.issues.push_back(
            {CertificationSeverity::Error, "unknown_template", "No certification suite exists for template."});
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
        report.issues.push_back(
            {CertificationSeverity::Error, "missing_runtime_profile", "Template runtime profile is missing."});
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
    if (!projectDocument.contains("disabledOptionalFeatures") ||
        !projectDocument["disabledOptionalFeatures"].is_array()) {
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
