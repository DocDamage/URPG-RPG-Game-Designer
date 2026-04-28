#include "tools/audit/project_audit_artifact_governance.h"

#include <filesystem>
#include <string>
#include <utility>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace urpg::tools::audit {

namespace {
void addCanonicalArtifactSection(const TemplateContext& templateContext,
                                 const std::string& sectionName,
                                 const std::string& dependencyLabel,
                                 bool enabled,
                                 const std::vector<CanonicalArtifactSpec>& artifacts,
                                 std::vector<AuditIssue>& issues,
                                 std::size_t& sectionIssueCount,
                                 json& governanceReport) {
    json section = json::object();
    section["enabled"] = enabled;
    section["dependency"] = dependencyLabel;
    section["issueCount"] = 0;
    section["expectedArtifacts"] = json::array();
    section["summary"] = enabled
        ? "Checking canonical " + dependencyLabel + " artifacts for selected template " + templateContext.id + "."
        : "Selected template " + templateContext.id + " does not currently depend on " + dependencyLabel + ".";

    for (const auto& artifact : artifacts) {
        const bool exists = fs::exists(artifact.path);
        const bool regularFile = exists && fs::is_regular_file(artifact.path);
        const bool required = enabled;
        const std::string status = !enabled ? "not_checked" : (regularFile ? "present" : (exists ? "invalid" : "missing"));

        section["expectedArtifacts"].push_back({
            {"code", artifact.code},
            {"title", artifact.title},
            {"path", artifact.path.string()},
            {"required", required},
            {"exists", exists},
            {"isRegularFile", regularFile},
            {"status", status},
        });

        if (!enabled || regularFile) {
            continue;
        }

        ++sectionIssueCount;
        issues.push_back({
            artifact.code,
            artifact.title,
            artifact.detailPrefix + " canonical artifact expected at " + artifact.path.string() +
                " is " + (exists ? "present but not a regular file" : "missing") +
                "; this is a governance gap, not proof the feature is absent.",
            "warning",
            false,
            false,
        });
    }

    section["issueCount"] = sectionIssueCount;
    governanceReport[sectionName] = section;
}

} // namespace

void addLocalizationArtifactGovernance(const TemplateContext& templateContext,
                                       std::vector<AuditIssue>& issues,
                                       std::size_t& localizationArtifactIssueCount,
                                       json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "localization");
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "localization_artifact.bundle_schema_missing",
            "Canonical localization bundle schema missing",
            "Localization",
            fs::path("content") / "schemas" / "localization_bundle.schema.json",
        },
        {
            "localization_artifact.locale_catalog_runtime_missing",
            "Canonical localization catalog runtime missing",
            "Localization",
            fs::path("engine") / "core" / "localization" / "locale_catalog.cpp",
        },
        {
            "localization_artifact.completeness_checker_missing",
            "Canonical localization completeness checker missing",
            "Localization",
            fs::path("engine") / "core" / "localization" / "completeness_checker.cpp",
        },
        {
            "localization_artifact.ci_gate_missing",
            "Canonical localization consistency gate missing",
            "Localization",
            fs::path("tools") / "ci" / "check_localization_consistency.ps1",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "localizationArtifacts",
        "localization governance",
        enabled,
        artifacts,
        issues,
        localizationArtifactIssueCount,
        governanceReport);
}

void addExportArtifactGovernance(const TemplateContext& templateContext,
                                 std::vector<AuditIssue>& issues,
                                 std::size_t& exportArtifactIssueCount,
                                 json& governanceReport) {
    const bool enabled = !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "export_artifact.packager_missing",
            "Canonical export packager missing",
            "Export",
            fs::path("engine") / "core" / "tools" / "export_packager.cpp",
        },
        {
            "export_artifact.cli_missing",
            "Canonical export packaging CLI missing",
            "Export",
            fs::path("tools") / "pack" / "pack_cli.cpp",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "exportArtifacts",
        "export governance",
        enabled,
        artifacts,
        issues,
        exportArtifactIssueCount,
        governanceReport);
}

void addInputArtifactGovernance(const TemplateContext& templateContext,
                                std::vector<AuditIssue>& issues,
                                std::size_t& inputArtifactIssueCount,
                                json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "input") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "input_artifact.remap_header_missing",
            "Canonical input remap store header missing",
            "Input",
            fs::path("engine") / "core" / "input" / "input_remap_store.h",
        },
        {
            "input_artifact.remap_source_missing",
            "Canonical input remap store source missing",
            "Input",
            fs::path("engine") / "core" / "input" / "input_remap_store.cpp",
        },
        {
            "input_artifact.runtime_header_missing",
            "Canonical controller binding runtime header missing",
            "Input",
            fs::path("engine") / "core" / "action" / "controller_binding_runtime.h",
        },
        {
            "input_artifact.runtime_source_missing",
            "Canonical controller binding runtime source missing",
            "Input",
            fs::path("engine") / "core" / "action" / "controller_binding_runtime.cpp",
        },
        {
            "input_artifact.panel_header_missing",
            "Canonical controller binding panel header missing",
            "Input",
            fs::path("editor") / "action" / "controller_binding_panel.h",
        },
        {
            "input_artifact.panel_source_missing",
            "Canonical controller binding panel source missing",
            "Input",
            fs::path("editor") / "action" / "controller_binding_panel.cpp",
        },
        {
            "input_artifact.schema_missing",
            "Canonical input bindings schema missing",
            "Input",
            fs::path("content") / "schemas" / "input_bindings.schema.json",
        },
        {
            "input_artifact.controller_schema_missing",
            "Canonical controller bindings schema missing",
            "Input",
            fs::path("content") / "schemas" / "controller_bindings.schema.json",
        },
        {
            "input_artifact.governance_script_missing",
            "Canonical input governance script missing",
            "Input",
            fs::path("tools") / "ci" / "check_input_governance.ps1",
        },
        {
            "input_artifact.fixture_missing",
            "Canonical input bindings fixture missing",
            "Input",
            fs::path("content") / "fixtures" / "input_bindings_fixture.json",
        },
        {
            "input_artifact.controller_fixture_missing",
            "Canonical controller bindings fixture missing",
            "Input",
            fs::path("content") / "fixtures" / "controller_bindings_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "inputArtifacts",
        "input governance",
        enabled,
        artifacts,
        issues,
        inputArtifactIssueCount,
        governanceReport);
}

void addAccessibilityArtifactGovernance(const TemplateContext& templateContext,
                                        std::vector<AuditIssue>& issues,
                                        std::size_t& accessibilityArtifactIssueCount,
                                        json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "accessibility") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "accessibility_artifact.schema_missing",
            "Canonical accessibility report schema missing",
            "Accessibility",
            fs::path("content") / "schemas" / "accessibility_report.schema.json",
        },
        {
            "accessibility_artifact.runtime_header_missing",
            "Canonical accessibility auditor header missing",
            "Accessibility",
            fs::path("engine") / "core" / "accessibility" / "accessibility_auditor.h",
        },
        {
            "accessibility_artifact.runtime_source_missing",
            "Canonical accessibility auditor source missing",
            "Accessibility",
            fs::path("engine") / "core" / "accessibility" / "accessibility_auditor.cpp",
        },
        {
            "accessibility_artifact.panel_header_missing",
            "Canonical accessibility panel header missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_panel.h",
        },
        {
            "accessibility_artifact.panel_source_missing",
            "Canonical accessibility panel source missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_panel.cpp",
        },
        {
            "accessibility_artifact.menu_adapter_header_missing",
            "Canonical accessibility menu adapter header missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_menu_adapter.h",
        },
        {
            "accessibility_artifact.menu_adapter_source_missing",
            "Canonical accessibility menu adapter source missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_menu_adapter.cpp",
        },
        {
            "accessibility_artifact.spatial_adapter_header_missing",
            "Canonical accessibility spatial adapter header missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_spatial_adapter.h",
        },
        {
            "accessibility_artifact.spatial_adapter_source_missing",
            "Canonical accessibility spatial adapter source missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_spatial_adapter.cpp",
        },
        {
            "accessibility_artifact.audio_adapter_header_missing",
            "Canonical accessibility audio adapter header missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_audio_adapter.h",
        },
        {
            "accessibility_artifact.audio_adapter_source_missing",
            "Canonical accessibility audio adapter source missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_audio_adapter.cpp",
        },
        {
            "accessibility_artifact.battle_adapter_header_missing",
            "Canonical accessibility battle adapter header missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_battle_adapter.h",
        },
        {
            "accessibility_artifact.battle_adapter_source_missing",
            "Canonical accessibility battle adapter source missing",
            "Accessibility",
            fs::path("editor") / "accessibility" / "accessibility_battle_adapter.cpp",
        },
        {
            "accessibility_artifact.governance_script_missing",
            "Canonical accessibility governance script missing",
            "Accessibility",
            fs::path("tools") / "ci" / "check_accessibility_governance.ps1",
        },
        {
            "accessibility_artifact.fixture_missing",
            "Canonical accessibility report fixture missing",
            "Accessibility",
            fs::path("content") / "fixtures" / "accessibility_report_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "accessibilityArtifacts",
        "accessibility governance",
        enabled,
        artifacts,
        issues,
        accessibilityArtifactIssueCount,
        governanceReport);
}

void addAudioArtifactGovernance(const TemplateContext& templateContext,
                                std::vector<AuditIssue>& issues,
                                std::size_t& audioArtifactIssueCount,
                                json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "audio") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "audio_artifact.schema_missing",
            "Canonical audio mix schema missing",
            "Audio",
            fs::path("content") / "schemas" / "audio_mix_presets.schema.json",
        },
        {
            "audio_artifact.runtime_header_missing",
            "Canonical audio mix preset runtime header missing",
            "Audio",
            fs::path("engine") / "core" / "audio" / "audio_mix_presets.h",
        },
        {
            "audio_artifact.runtime_source_missing",
            "Canonical audio mix preset runtime source missing",
            "Audio",
            fs::path("engine") / "core" / "audio" / "audio_mix_presets.cpp",
        },
        {
            "audio_artifact.panel_header_missing",
            "Canonical audio mix panel header missing",
            "Audio",
            fs::path("editor") / "audio" / "audio_mix_panel.h",
        },
        {
            "audio_artifact.panel_source_missing",
            "Canonical audio mix panel source missing",
            "Audio",
            fs::path("editor") / "audio" / "audio_mix_panel.cpp",
        },
        {
            "audio_artifact.validator_header_missing",
            "Canonical audio mix validator header missing",
            "Audio",
            fs::path("engine") / "core" / "audio" / "audio_mix_validator.h",
        },
        {
            "audio_artifact.validator_source_missing",
            "Canonical audio mix validator source missing",
            "Audio",
            fs::path("engine") / "core" / "audio" / "audio_mix_validator.cpp",
        },
        {
            "audio_artifact.governance_script_missing",
            "Canonical audio governance script missing",
            "Audio",
            fs::path("tools") / "ci" / "check_audio_governance.ps1",
        },
        {
            "audio_artifact.fixture_missing",
            "Canonical audio mix preset fixture missing",
            "Audio",
            fs::path("content") / "fixtures" / "audio_mix_presets_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "audioArtifacts",
        "audio governance",
        enabled,
        artifacts,
        issues,
        audioArtifactIssueCount,
        governanceReport);
}

void addAchievementArtifactGovernance(const TemplateContext& templateContext,
                                      std::vector<AuditIssue>& issues,
                                      std::size_t& achievementArtifactIssueCount,
                                      json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "achievement") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "achievement_artifact.schema_missing",
            "Canonical achievement schema missing",
            "Achievement",
            fs::path("content") / "schemas" / "achievements.schema.json",
        },
        {
            "achievement_artifact.runtime_header_missing",
            "Canonical achievement registry header missing",
            "Achievement",
            fs::path("engine") / "core" / "achievement" / "achievement_registry.h",
        },
        {
            "achievement_artifact.runtime_source_missing",
            "Canonical achievement registry source missing",
            "Achievement",
            fs::path("engine") / "core" / "achievement" / "achievement_registry.cpp",
        },
        {
            "achievement_artifact.validator_header_missing",
            "Canonical achievement validator header missing",
            "Achievement",
            fs::path("engine") / "core" / "achievement" / "achievement_validator.h",
        },
        {
            "achievement_artifact.validator_source_missing",
            "Canonical achievement validator source missing",
            "Achievement",
            fs::path("engine") / "core" / "achievement" / "achievement_validator.cpp",
        },
        {
            "achievement_artifact.platform_backend_header_missing",
            "Canonical achievement platform backend header missing",
            "Achievement",
            fs::path("engine") / "core" / "achievement" / "achievement_platform_backend.h",
        },
        {
            "achievement_artifact.platform_backend_source_missing",
            "Canonical achievement platform backend source missing",
            "Achievement",
            fs::path("engine") / "core" / "achievement" / "achievement_platform_backend.cpp",
        },
        {
            "achievement_artifact.panel_header_missing",
            "Canonical achievement panel header missing",
            "Achievement",
            fs::path("editor") / "achievement" / "achievement_panel.h",
        },
        {
            "achievement_artifact.panel_source_missing",
            "Canonical achievement panel source missing",
            "Achievement",
            fs::path("editor") / "achievement" / "achievement_panel.cpp",
        },
        {
            "achievement_artifact.governance_script_missing",
            "Canonical achievement governance script missing",
            "Achievement",
            fs::path("tools") / "ci" / "check_achievement_governance.ps1",
        },
        {
            "achievement_artifact.fixture_missing",
            "Canonical achievement registry fixture missing",
            "Achievement",
            fs::path("content") / "fixtures" / "achievement_registry_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "achievementArtifacts",
        "achievement governance",
        enabled,
        artifacts,
        issues,
        achievementArtifactIssueCount,
        governanceReport);
}

void addCharacterArtifactGovernance(const TemplateContext& templateContext,
                                    std::vector<AuditIssue>& issues,
                                    std::size_t& characterArtifactIssueCount,
                                    json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "character") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "character_artifact.schema_missing",
            "Canonical character identity schema missing",
            "Character",
            fs::path("content") / "schemas" / "character_identity.schema.json",
        },
        {
            "character_artifact.runtime_header_missing",
            "Canonical character identity header missing",
            "Character",
            fs::path("engine") / "core" / "character" / "character_identity.h",
        },
        {
            "character_artifact.runtime_source_missing",
            "Canonical character identity source missing",
            "Character",
            fs::path("engine") / "core" / "character" / "character_identity.cpp",
        },
        {
            "character_artifact.validator_header_missing",
            "Canonical character identity validator header missing",
            "Character",
            fs::path("engine") / "core" / "character" / "character_identity_validator.h",
        },
        {
            "character_artifact.validator_source_missing",
            "Canonical character identity validator source missing",
            "Character",
            fs::path("engine") / "core" / "character" / "character_identity_validator.cpp",
        },
        {
            "character_artifact.model_header_missing",
            "Canonical character creator model header missing",
            "Character",
            fs::path("editor") / "character" / "character_creator_model.h",
        },
        {
            "character_artifact.model_source_missing",
            "Canonical character creator model source missing",
            "Character",
            fs::path("editor") / "character" / "character_creator_model.cpp",
        },
        {
            "character_artifact.panel_header_missing",
            "Canonical character creator panel header missing",
            "Character",
            fs::path("editor") / "character" / "character_creator_panel.h",
        },
        {
            "character_artifact.panel_source_missing",
            "Canonical character creator panel source missing",
            "Character",
            fs::path("editor") / "character" / "character_creator_panel.cpp",
        },
        {
            "character_artifact.governance_script_missing",
            "Canonical character governance script missing",
            "Character",
            fs::path("tools") / "ci" / "check_character_governance.ps1",
        },
        {
            "character_artifact.fixture_missing",
            "Canonical character identity fixture missing",
            "Character",
            fs::path("content") / "fixtures" / "character_identity_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "characterArtifacts",
        "character governance",
        enabled,
        artifacts,
        issues,
        characterArtifactIssueCount,
        governanceReport);
}

void addModArtifactGovernance(const TemplateContext& templateContext,
                              std::vector<AuditIssue>& issues,
                              std::size_t& modArtifactIssueCount,
                              json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "mod") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "mod_artifact.schema_missing",
            "Canonical mod manifest schema missing",
            "Mod",
            fs::path("content") / "schemas" / "mod_manifest.schema.json",
        },
        {
            "mod_artifact.runtime_header_missing",
            "Canonical mod registry header missing",
            "Mod",
            fs::path("engine") / "core" / "mod" / "mod_registry.h",
        },
        {
            "mod_artifact.runtime_source_missing",
            "Canonical mod registry source missing",
            "Mod",
            fs::path("engine") / "core" / "mod" / "mod_registry.cpp",
        },
        {
            "mod_artifact.validator_header_missing",
            "Canonical mod registry validator header missing",
            "Mod",
            fs::path("engine") / "core" / "mod" / "mod_registry_validator.h",
        },
        {
            "mod_artifact.validator_source_missing",
            "Canonical mod registry validator source missing",
            "Mod",
            fs::path("engine") / "core" / "mod" / "mod_registry_validator.cpp",
        },
        {
            "mod_artifact.panel_header_missing",
            "Canonical mod manager panel header missing",
            "Mod",
            fs::path("editor") / "mod" / "mod_manager_panel.h",
        },
        {
            "mod_artifact.panel_source_missing",
            "Canonical mod manager panel source missing",
            "Mod",
            fs::path("editor") / "mod" / "mod_manager_panel.cpp",
        },
        {
            "mod_artifact.governance_script_missing",
            "Canonical mod governance script missing",
            "Mod",
            fs::path("tools") / "ci" / "check_mod_governance.ps1",
        },
        {
            "mod_artifact.fixture_missing",
            "Canonical mod manifest fixture missing",
            "Mod",
            fs::path("content") / "fixtures" / "mod_manifest_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "modArtifacts",
        "mod governance",
        enabled,
        artifacts,
        issues,
        modArtifactIssueCount,
        governanceReport);
}

void addAnalyticsArtifactGovernance(const TemplateContext& templateContext,
                                    std::vector<AuditIssue>& issues,
                                    std::size_t& analyticsArtifactIssueCount,
                                    json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "analytics") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "analytics_artifact.schema_missing",
            "Canonical analytics config schema missing",
            "Analytics",
            fs::path("content") / "schemas" / "analytics_config.schema.json",
        },
        {
            "analytics_artifact.runtime_header_missing",
            "Canonical analytics dispatcher header missing",
            "Analytics",
            fs::path("engine") / "core" / "analytics" / "analytics_dispatcher.h",
        },
        {
            "analytics_artifact.runtime_source_missing",
            "Canonical analytics dispatcher source missing",
            "Analytics",
            fs::path("engine") / "core" / "analytics" / "analytics_dispatcher.cpp",
        },
        {
            "analytics_artifact.event_header_missing",
            "Canonical analytics event header missing",
            "Analytics",
            fs::path("engine") / "core" / "analytics" / "analytics_event.h",
        },
        {
            "analytics_artifact.validator_header_missing",
            "Canonical analytics validator header missing",
            "Analytics",
            fs::path("engine") / "core" / "analytics" / "analytics_dispatcher_validator.h",
        },
        {
            "analytics_artifact.validator_source_missing",
            "Canonical analytics validator source missing",
            "Analytics",
            fs::path("engine") / "core" / "analytics" / "analytics_dispatcher_validator.cpp",
        },
        {
            "analytics_artifact.panel_header_missing",
            "Canonical analytics panel header missing",
            "Analytics",
            fs::path("editor") / "analytics" / "analytics_panel.h",
        },
        {
            "analytics_artifact.panel_source_missing",
            "Canonical analytics panel source missing",
            "Analytics",
            fs::path("editor") / "analytics" / "analytics_panel.cpp",
        },
        {
            "analytics_artifact.governance_script_missing",
            "Canonical analytics governance script missing",
            "Analytics",
            fs::path("tools") / "ci" / "check_analytics_governance.ps1",
        },
        {
            "analytics_artifact.fixture_missing",
            "Canonical analytics fixture missing",
            "Analytics",
            fs::path("content") / "fixtures" / "analytics_fixture.json",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "analyticsArtifacts",
        "analytics governance",
        enabled,
        artifacts,
        issues,
        analyticsArtifactIssueCount,
        governanceReport);
}

void addPerformanceArtifactGovernance(const TemplateContext& templateContext,
                                      std::vector<AuditIssue>& issues,
                                      std::size_t& performanceArtifactIssueCount,
                                      json& governanceReport) {
    const bool enabled = templateBarNeedsProjectArtifact(templateContext, "performance") || !isReadyStatus(templateContext.status);
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "performance_artifact.runtime_header_missing",
            "Canonical performance profiler header missing",
            "Performance",
            fs::path("engine") / "core" / "perf" / "perf_profiler.h",
        },
        {
            "performance_artifact.runtime_source_missing",
            "Canonical performance profiler source missing",
            "Performance",
            fs::path("engine") / "core" / "perf" / "perf_profiler.cpp",
        },
        {
            "performance_artifact.panel_header_missing",
            "Canonical performance diagnostics panel header missing",
            "Performance",
            fs::path("editor") / "perf" / "perf_diagnostics_panel.h",
        },
        {
            "performance_artifact.panel_source_missing",
            "Canonical performance diagnostics panel source missing",
            "Performance",
            fs::path("editor") / "perf" / "perf_diagnostics_panel.cpp",
        },
        {
            "performance_artifact.budget_doc_missing",
            "Canonical performance budget doc missing",
            "Performance",
            fs::path("docs") / "presentation" / "performance_budgets.md",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "performanceArtifacts",
        "performance governance",
        enabled,
        artifacts,
        issues,
        performanceArtifactIssueCount,
        governanceReport);
}

void addReleaseSignoffWorkflowGovernance(const TemplateContext& templateContext,
                                         std::vector<AuditIssue>& issues,
                                         std::size_t& releaseSignoffWorkflowIssueCount,
                                         json& governanceReport) {
    const std::vector<CanonicalArtifactSpec> artifacts = {
        {
            "release_signoff_workflow.missing",
            "Canonical release signoff workflow artifact missing",
            "Release-signoff workflow",
            fs::path("docs") / "RELEASE_SIGNOFF_WORKFLOW.md",
        },
    };

    addCanonicalArtifactSection(
        templateContext,
        "releaseSignoffWorkflow",
        "release-signoff workflow governance",
        true,
        artifacts,
        issues,
        releaseSignoffWorkflowIssueCount,
        governanceReport);
}


} // namespace urpg::tools::audit
