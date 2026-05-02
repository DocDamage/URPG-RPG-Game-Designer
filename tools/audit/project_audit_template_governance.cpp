#include "tools/audit/project_audit_template_governance.h"

#include "tools/audit/project_audit_template_spec.h"

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace urpg::tools::audit {
void addTemplateSpecArtifactGovernance(const TemplateContext& templateContext,
                                       std::vector<AuditIssue>& issues,
                                       std::size_t& templateSpecArtifactIssueCount,
                                       std::size_t& releaseBlockerCount,
                                       std::size_t& exportBlockerCount,
                                       json& governanceReport) {
    const bool enabled = templateContext.id != "unknown" && templateContext.data.is_object() && !templateContext.data.empty();

    // Templates that are READY candidates must fail closed on spec governance gaps.
    static const std::vector<std::string> kFailClosedTemplates = {
        "jrpg",
        "visual_novel",
        "turn_based_rpg",
        "tactics_rpg",
        "arpg",
        "monster_collector_rpg",
        "cozy_life_rpg",
        "metroidvania_lite",
        "2_5d_rpg",
    };
    const bool failClosed = enabled && std::find(
        kFailClosedTemplates.begin(), kFailClosedTemplates.end(), templateContext.id
    ) != kFailClosedTemplates.end();

    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "template_spec_artifact.missing",
            "Canonical template spec artifact missing",
            "Template spec",
            fs::path("docs") / "templates" / (templateContext.id + "_spec.md"),
        },
    };

    json section = json::object();
    section["enabled"] = enabled;
    section["dependency"] = "template-spec governance";
    section["issueCount"] = 0;
    section["expectedArtifacts"] = json::array();
    section["summary"] = enabled
        ? "Checking canonical template-spec artifacts and conservative readiness parity for selected template " + templateContext.id + "."
        : "Selected template " + templateContext.id + " does not currently depend on template-spec governance.";

    for (const auto& artifact : artifacts) {
        const bool exists = fs::exists(artifact.path);
        const bool regularFile = exists && fs::is_regular_file(artifact.path);
        const bool required = enabled;
        std::string status = !enabled ? "not_checked" : (regularFile ? "present" : (exists ? "invalid" : "missing"));
        json artifactEntry = {
            {"code", artifact.code},
            {"title", artifact.title},
            {"path", artifact.path.string()},
            {"required", required},
            {"exists", exists},
            {"isRegularFile", regularFile},
            {"status", status},
        };

        if (!enabled) {
            section["expectedArtifacts"].push_back(std::move(artifactEntry));
            continue;
        }

        if (!regularFile) {
            ++templateSpecArtifactIssueCount;
            if (failClosed) {
                ++releaseBlockerCount;
                ++exportBlockerCount;
            }
            issues.push_back({
                artifact.code,
                artifact.title,
                artifact.detailPrefix + " canonical artifact expected at " + artifact.path.string() +
                    " is " + (exists ? "present but not a regular file" : "missing") +
                    (failClosed
                        ? "; this template is a READY candidate — missing spec is a release blocker."
                        : "; this is a governance gap, not proof the feature is absent."),
                failClosed ? "error" : "warning",
                failClosed,
                failClosed,
            });
            section["expectedArtifacts"].push_back(std::move(artifactEntry));
            continue;
        }

        try {
            const std::string text = readFile(artifact.path);
            const std::string expectedAuthority = "Authority: canonical template spec for `" + templateContext.id + "`";
            const bool templateIdMatches = text.find(expectedAuthority) != std::string::npos;
            artifactEntry["templateIdMatches"] = templateIdMatches;

            const std::vector<std::string> expectedSubsystems = getStringArray(templateContext.data, "requiredSubsystems");
            std::vector<std::string> specSubsystems = extractTemplateSpecRequiredSubsystems(text);
            std::sort(specSubsystems.begin(), specSubsystems.end());

            std::vector<std::string> sortedExpectedSubsystems = expectedSubsystems;
            std::sort(sortedExpectedSubsystems.begin(), sortedExpectedSubsystems.end());

            std::vector<std::string> missingSubsystems;
            std::vector<std::string> unexpectedSubsystems;
            std::set_difference(sortedExpectedSubsystems.begin(),
                                sortedExpectedSubsystems.end(),
                                specSubsystems.begin(),
                                specSubsystems.end(),
                                std::back_inserter(missingSubsystems));
            std::set_difference(specSubsystems.begin(),
                                specSubsystems.end(),
                                sortedExpectedSubsystems.begin(),
                                sortedExpectedSubsystems.end(),
                                std::back_inserter(unexpectedSubsystems));

            const bool requiredSubsystemsMatch = missingSubsystems.empty() && unexpectedSubsystems.empty();
            artifactEntry["requiredSubsystemsMatch"] = requiredSubsystemsMatch;
            if (!missingSubsystems.empty()) {
                artifactEntry["missingRequiredSubsystems"] = missingSubsystems;
            }
            if (!unexpectedSubsystems.empty()) {
                artifactEntry["unexpectedRequiredSubsystems"] = unexpectedSubsystems;
            }

            const json specBars = extractTemplateSpecBars(text);
            json barMismatches = json::array();
            bool barsMatch = true;
            if (templateContext.data.contains("bars") && templateContext.data.at("bars").is_object()) {
                for (const auto& [barName, barValue] : templateContext.data.at("bars").items()) {
                    if (!barValue.is_string()) {
                        continue;
                    }

                    const std::string expectedStatus = barValue.get<std::string>();
                    const std::string specStatus = specBars.contains(barName) && specBars.at(barName).is_string()
                        ? specBars.at(barName).get<std::string>()
                        : "missing";
                    if (specStatus != expectedStatus) {
                        barsMatch = false;
                        barMismatches.push_back({
                            {"bar", barName},
                            {"label", templateBarDisplayName(barName)},
                            {"expectedStatus", expectedStatus},
                            {"specStatus", specStatus},
                        });
                    }
                }
            }

            artifactEntry["barsMatch"] = barsMatch;
            if (!barMismatches.empty()) {
                artifactEntry["barMismatches"] = barMismatches;
            }

            if (!templateIdMatches) {
                ++templateSpecArtifactIssueCount;
                if (failClosed) {
                    ++releaseBlockerCount;
                    ++exportBlockerCount;
                }
                issues.push_back({
                    "template_spec_artifact.template_id_mismatch",
                    "Canonical template spec artifact names the wrong template",
                    artifact.detailPrefix + " canonical artifact at " + artifact.path.string() +
                        " does not contain the expected authority line for template " + templateContext.id +
                        (failClosed
                            ? "; this template is a READY candidate — authority mismatch blocks release."
                            : "; keep template-facing docs aligned with the selected readiness context."),
                    failClosed ? "error" : "warning",
                    failClosed,
                    failClosed,
                });
            }

            if (!requiredSubsystemsMatch) {
                ++templateSpecArtifactIssueCount;
                if (failClosed) {
                    ++releaseBlockerCount;
                    ++exportBlockerCount;
                }
                issues.push_back({
                    "template_spec_artifact.required_subsystems_mismatch",
                    "Canonical template spec required subsystems drift from readiness",
                    artifact.detailPrefix + " canonical artifact at " + artifact.path.string() +
                        " does not match readiness requiredSubsystems for template " + templateContext.id +
                        ". Missing from spec: [" + joinItems(missingSubsystems) + "]. Unexpected in spec: [" +
                        joinItems(unexpectedSubsystems) + "].",
                    failClosed ? "error" : "warning",
                    failClosed,
                    failClosed,
                });
            }

            if (!barsMatch) {
                std::ostringstream detail;
                detail << artifact.detailPrefix << " canonical artifact at " << artifact.path.string()
                       << " does not match readiness cross-cutting bar statuses for template " << templateContext.id
                       << ".";
                for (const auto& mismatch : barMismatches) {
                    detail << " " << mismatch.at("label").get<std::string>() << " expected "
                           << mismatch.at("expectedStatus").get<std::string>() << " but spec shows "
                           << mismatch.at("specStatus").get<std::string>() << ".";
                }

                ++templateSpecArtifactIssueCount;
                if (failClosed) {
                    ++releaseBlockerCount;
                    ++exportBlockerCount;
                }
                issues.push_back({
                    "template_spec_artifact.bars_mismatch",
                    "Canonical template spec bar statuses drift from readiness",
                    detail.str(),
                    failClosed ? "error" : "warning",
                    failClosed,
                    failClosed,
                });
            }

            if (!templateIdMatches || !requiredSubsystemsMatch || !barsMatch) {
                status = "parity_mismatch";
                artifactEntry["status"] = status;
            }
        } catch (const std::exception&) {
            ++templateSpecArtifactIssueCount;
            if (failClosed) {
                ++releaseBlockerCount;
                ++exportBlockerCount;
            }
            artifactEntry["status"] = "unreadable";
            issues.push_back({
                "template_spec_artifact.unreadable",
                "Canonical template spec artifact unreadable",
                artifact.detailPrefix + " canonical artifact at " + artifact.path.string() +
                    " could not be read for template-governance parity checks.",
                failClosed ? "error" : "warning",
                failClosed,
                failClosed,
            });
        }

        section["expectedArtifacts"].push_back(std::move(artifactEntry));
    }

    section["issueCount"] = templateSpecArtifactIssueCount;
    governanceReport["templateSpecArtifacts"] = std::move(section);
}

void addSignoffArtifactGovernance(const json& readiness,
                                  std::vector<AuditIssue>& issues,
                                  std::size_t& signoffArtifactIssueCount,
                                  std::size_t& releaseBlockerCount,
                                  json& governanceReport) {
    std::vector<SignoffArtifactSpec> artifacts;
    if (readiness.contains("subsystems") && readiness.at("subsystems").is_array()) {
        for (const auto& subsystem : readiness.at("subsystems")) {
            if (!subsystem.is_object()) {
                continue;
            }

            const std::string subsystemId = getString(subsystem, "id", "");
            const bool ready = getString(subsystem, "status", "") == "READY";
            const bool hasSignoff = subsystem.contains("signoff") && subsystem.at("signoff").is_object();
            const bool signoffRequired = hasSignoff &&
                subsystem.at("signoff").contains("required") &&
                subsystem.at("signoff").at("required").is_boolean() &&
                subsystem.at("signoff").at("required").get<bool>();

            if (!ready && !signoffRequired) {
                continue;
            }

            const std::string artifactPath = hasSignoff ? getString(subsystem.at("signoff"), "artifactPath", "") : "";
            const std::string issuePrefix = subsystemId.empty() ? "unknown" : subsystemId;
            artifacts.push_back({
                subsystemId,
                "signoff_artifact." + issuePrefix + "_missing",
                "signoff_artifact." + issuePrefix + "_wording_mismatch",
                "signoff_artifact." + issuePrefix + "_contract_mismatch",
                "Subsystem signoff artifact missing or non-conservative",
                "Subsystem signoff",
                artifactPath.empty() ? fs::path() : fs::path(artifactPath),
                {"Human review is required", "residual gaps", "PARTIAL"},
                {"Status:** `READY`", "approved by release-owner review"},
            });
        }
    }

    json section = json::object();
    section["enabled"] = true;
    section["dependency"] = "human-review-gated subsystem signoff artifacts";
    section["issueCount"] = 0;
    section["summary"] =
        "Checking required subsystem signoff artifacts, conservative wording, and structured human-review signoff contracts for governed lanes.";
    section["expectedArtifacts"] = json::array();

    const std::string statusDate = getString(readiness, "statusDate", "");
    for (const auto& artifact : artifacts) {
        const json* subsystem = findSubsystemById(readiness, artifact.subsystemId);
        const bool pathProvided = !artifact.path.empty();
        const bool exists = pathProvided && fs::exists(artifact.path);
        const bool regularFile = exists && fs::is_regular_file(artifact.path);
        bool wordingOk = false;
        bool contractOk = false;
        bool readyCompletenessOk = false;
        json missingPhrases = json::array();
        json missingContractFields = json::array();
        std::string status = !pathProvided ? "missing_contract" : (regularFile ? "present" : (exists ? "invalid" : "missing"));
        json contractEntry = json::object();
        const bool subsystemReady = subsystem != nullptr && getString(*subsystem, "status", "") == "READY";

        if (subsystem != nullptr && subsystem->contains("signoff") && subsystem->at("signoff").is_object()) {
            const auto& signoff = subsystem->at("signoff");
            const bool required = signoff.contains("required") && signoff.at("required").is_boolean() &&
                signoff.at("required").get<bool>();
            const std::string artifactPath = getString(signoff, "artifactPath", "");
            const bool promotionRequiresHumanReview =
                signoff.contains("promotionRequiresHumanReview") &&
                signoff.at("promotionRequiresHumanReview").is_boolean() &&
                signoff.at("promotionRequiresHumanReview").get<bool>();
            const std::string workflow = getString(signoff, "workflow", "");
            const std::string reviewStatus = getString(signoff, "reviewStatus", "");
            const std::string reviewedBy = getString(signoff, "reviewedBy", "");
            const std::string reviewedDate = getString(signoff, "reviewedDate", "");
            const std::string verificationCommand = getString(signoff, "verificationCommand", "");
            const std::string evidenceCommandResult = getString(signoff, "evidenceCommandResult", "");
            const bool docsAligned = subsystem->contains("evidence") && subsystem->at("evidence").is_object() &&
                subsystem->at("evidence").contains("docsAligned") &&
                subsystem->at("evidence").at("docsAligned").is_boolean() &&
                subsystem->at("evidence").at("docsAligned").get<bool>();

            if (subsystemReady) {
                if (reviewedBy.empty()) {
                    missingContractFields.push_back("reviewedBy");
                }
                if (reviewedDate.empty()) {
                    missingContractFields.push_back("reviewedDate");
                }
                if (verificationCommand.empty()) {
                    missingContractFields.push_back("verificationCommand");
                }
                if (evidenceCommandResult != "PASS") {
                    missingContractFields.push_back("evidenceCommandResult");
                }
                if (!docsAligned) {
                    missingContractFields.push_back("evidence.docsAligned");
                }
                if (!statusDate.empty() && !reviewedDate.empty() && reviewedDate < statusDate) {
                    missingContractFields.push_back("reviewedDate.current");
                }
            }
            readyCompletenessOk = !subsystemReady || missingContractFields.empty();

            contractOk = required && artifactPath == artifact.path.generic_string() &&
                workflow == (fs::path("docs") / "RELEASE_SIGNOFF_WORKFLOW.md").generic_string() &&
                (subsystemReady ? (!promotionRequiresHumanReview && reviewStatus == "APPROVED")
                                : promotionRequiresHumanReview) &&
                readyCompletenessOk;

            contractEntry = {
                {"required", required},
                {"artifactPath", artifactPath},
                {"promotionRequiresHumanReview", promotionRequiresHumanReview},
                {"reviewStatus", reviewStatus},
                {"reviewedBy", reviewedBy},
                {"reviewedDate", reviewedDate},
                {"verificationCommand", verificationCommand},
                {"evidenceCommandResult", evidenceCommandResult},
                {"docsAligned", docsAligned},
                {"workflow", workflow},
                {"contractOk", contractOk},
            };
            if (!missingContractFields.empty()) {
                contractEntry["missingFields"] = missingContractFields;
            }
        } else {
            contractEntry = {
                {"required", false},
                {"artifactPath", ""},
                {"promotionRequiresHumanReview", false},
                {"reviewStatus", ""},
                {"reviewedBy", ""},
                {"reviewedDate", ""},
                {"verificationCommand", ""},
                {"evidenceCommandResult", ""},
                {"docsAligned", false},
                {"workflow", ""},
                {"contractOk", false},
            };
        }

        if (regularFile) {
            try {
                const std::string text = readFile(artifact.path);
                wordingOk = true;
                const auto& requiredPhrases = subsystemReady ? artifact.readyPhrases : artifact.pendingPhrases;
                for (const auto& phrase : requiredPhrases) {
                    if (text.find(phrase) == std::string::npos) {
                        wordingOk = false;
                        missingPhrases.push_back(phrase);
                    }
                }
                if (!wordingOk) {
                    status = "wording_mismatch";
                }
            } catch (const std::exception&) {
                wordingOk = false;
                status = "unreadable";
            }
        }

        json artifactEntry = {
            {"subsystemId", artifact.subsystemId},
            {"title", artifact.title},
            {"path", artifact.path.string()},
            {"required", true},
            {"exists", exists},
            {"isRegularFile", regularFile},
            {"status", status},
            {"signoffContract", contractEntry},
        };
        if (regularFile) {
            artifactEntry["wordingOk"] = wordingOk;
        }
        if (!missingPhrases.empty()) {
            artifactEntry["missingPhrases"] = std::move(missingPhrases);
        }

        if (!pathProvided || !exists || !regularFile) {
            ++signoffArtifactIssueCount;
            if (subsystemReady) {
                ++releaseBlockerCount;
            }
            issues.push_back({
                subsystemReady ? "signoff_artifact.ready_missing_contract" : artifact.missingCode,
                artifact.title,
                subsystemReady
                    ? artifact.subsystemId +
                        " is marked READY but lacks a usable signoff artifact path; READY rows require artifact, reviewer, date, verification command, PASS result, and docs alignment."
                    : artifact.detailPrefix + " canonical artifact expected at " + artifact.path.string() +
                        " is " + (exists ? "present but not a regular file" : "missing") +
                        "; this is a governance gap, not proof the subsystem is absent.",
                subsystemReady ? "error" : "warning",
                subsystemReady,
                false,
            });
            section["expectedArtifacts"].push_back(std::move(artifactEntry));
            continue;
        }

        if (!wordingOk) {
            ++signoffArtifactIssueCount;
            issues.push_back({
                artifact.wordingCode,
                artifact.title,
                artifact.detailPrefix + " canonical artifact at " + artifact.path.string() +
                    " is missing one or more required conservative signoff phrases; update the wording to keep the audit below promotion language.",
                "warning",
                false,
                false,
            });
        }

        if (!contractOk) {
            ++signoffArtifactIssueCount;
            if (subsystemReady) {
                ++releaseBlockerCount;
            }
            if (status == "present") {
                status = "contract_mismatch";
                artifactEntry["status"] = status;
            }
            issues.push_back({
                subsystemReady ? "signoff_artifact.ready_contract_incomplete" : artifact.contractCode,
                artifact.title,
                subsystemReady
                    ? artifact.subsystemId +
                        " is marked READY but its structured signoff contract is incomplete; required fields are artifactPath, reviewedBy, reviewedDate, verificationCommand, evidenceCommandResult=PASS, docsAligned=true, reviewStatus=APPROVED, and workflow."
                    : artifact.detailPrefix + " readiness record is missing the expected structured signoff contract for " +
                        artifact.subsystemId + "; keep the artifact path, workflow path, and human-review requirement aligned.",
                subsystemReady ? "error" : "warning",
                subsystemReady,
                false,
            });
            if (subsystemReady && !readyCompletenessOk) {
                issues.push_back({
                    "signoff_artifact.ready_docs_alignment_missing",
                    artifact.title,
                    artifact.subsystemId +
                        " READY signoff completeness check found missing or stale evidence fields: " +
                        missingContractFields.dump() + ".",
                    "error",
                    true,
                    false,
                });
            }
        }

        section["expectedArtifacts"].push_back(std::move(artifactEntry));
    }

    section["issueCount"] = signoffArtifactIssueCount;
    governanceReport["signoffArtifacts"] = std::move(section);
}

} // namespace urpg::tools::audit
