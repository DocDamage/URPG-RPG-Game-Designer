#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::project {

enum class CertificationSeverity {
    Advisory,
    Error
};

struct CertificationRequirement {
    std::string id;
    std::string description;
    bool optional = false;
};

struct TemplateCertificationSuite {
    std::string templateId;
    std::string status;
    std::vector<CertificationRequirement> requirements;
};

struct CertificationIssue {
    CertificationSeverity severity = CertificationSeverity::Error;
    std::string code;
    std::string detail;
};

struct CertificationReport {
    bool passed = false;
    std::string templateId;
    std::vector<CertificationIssue> issues;
    nlohmann::json toJson() const;
};

class TemplateCertification {
public:
    std::vector<TemplateCertificationSuite> defaultSuites() const;
    CertificationReport certify(const nlohmann::json& projectDocument, const std::string& templateId) const;

private:
    const TemplateCertificationSuite* findSuite(const std::string& templateId) const;
    bool projectHasLoop(const nlohmann::json& projectDocument, const std::string& loopId) const;
    bool projectDisablesOptionalFeature(const nlohmann::json& projectDocument, const std::string& featureId) const;
};

} // namespace urpg::project
