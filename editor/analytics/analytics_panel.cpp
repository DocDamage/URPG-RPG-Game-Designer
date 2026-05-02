#include "analytics_panel.h"

#ifdef URPG_IMGUI_ENABLED
#include <imgui.h>
#endif

namespace {

std::string issueCategoryToString(urpg::analytics::AnalyticsValidationCategory category) {
    using urpg::analytics::AnalyticsValidationCategory;

    switch (category) {
    case AnalyticsValidationCategory::EmptyEventName:
        return "empty_event_name";
    case AnalyticsValidationCategory::EmptyCategory:
        return "empty_category";
    case AnalyticsValidationCategory::DisallowedCategory:
        return "disallowed_category";
    case AnalyticsValidationCategory::EmptyParameterKey:
        return "empty_parameter_key";
    case AnalyticsValidationCategory::EmptyParameterValue:
        return "empty_parameter_value";
    case AnalyticsValidationCategory::ExcessiveParameterCount:
        return "excessive_parameter_count";
    }

    return "unknown";
}

std::string issueSeverityToString(urpg::analytics::AnalyticsValidationSeverity severity) {
    return severity == urpg::analytics::AnalyticsValidationSeverity::Error ? "error" : "warning";
}

} // namespace

namespace urpg::editor {

void AnalyticsPanel::bindDispatcher(urpg::analytics::AnalyticsDispatcher* dispatcher) {
    m_dispatcher = dispatcher;
}

void AnalyticsPanel::bindUploader(urpg::analytics::AnalyticsUploader* uploader) {
    m_uploader = uploader;
}

void AnalyticsPanel::bindPrivacyController(urpg::analytics::AnalyticsPrivacyController* privacyController) {
    m_privacyController = privacyController;
}

void AnalyticsPanel::bindEndpointProfile(const urpg::analytics::AnalyticsEndpointProfile* endpointProfile) {
    m_endpointProfile = endpointProfile;
}

void AnalyticsPanel::setSessionId(std::string sessionId) {
    m_sessionId = std::move(sessionId);
}

void AnalyticsPanel::render() {
    rebuildSnapshot();

#ifdef URPG_IMGUI_ENABLED
    if (ImGui::GetCurrentContext() == nullptr) {
        return;
    }

    if (!ImGui::Begin("Analytics", nullptr)) {
        ImGui::End();
        return;
    }

    bool optIn = m_dispatcher != nullptr && m_dispatcher->isOptIn();
    const auto actionEnabled = [this](const char* id) {
        if (m_lastSnapshot.contains("actionDetails") && m_lastSnapshot["actionDetails"].contains(id)) {
            return m_lastSnapshot["actionDetails"][id].value("enabled", false);
        }
        return m_lastSnapshot.contains("actions") && m_lastSnapshot["actions"].value(id, false);
    };
    const auto disabledTooltip = [this](const char* id) {
        if (m_lastSnapshot.contains("actionDetails") && m_lastSnapshot["actionDetails"].contains(id)) {
            const auto reason = m_lastSnapshot["actionDetails"][id].value("disabledReason", std::string{});
            if (!reason.empty() && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
                ImGui::SetTooltip("%s", reason.c_str());
            }
        }
    };

    const bool canSetOptIn = actionEnabled("setOptIn");
    if (!canSetOptIn) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Checkbox("Opt In", &optIn) && canSetOptIn) {
        (void)setOptIn(optIn);
    }
    if (!canSetOptIn) {
        disabledTooltip("setOptIn");
        ImGui::EndDisabled();
    }

    const bool canClearQueue = actionEnabled("clearQueue");
    if (!canClearQueue) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Clear Queue") && canClearQueue) {
        (void)clearQueuedEvents();
    }
    if (!canClearQueue) {
        disabledTooltip("clearQueue");
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    const bool canApplyEndpointProfile = actionEnabled("applyEndpointProfile");
    if (!canApplyEndpointProfile) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Apply Endpoint Profile") && canApplyEndpointProfile) {
        (void)applyEndpointProfile();
    }
    if (!canApplyEndpointProfile) {
        disabledTooltip("applyEndpointProfile");
        ImGui::EndDisabled();
    }
    ImGui::SameLine();
    const bool canFlushUpload = actionEnabled("flushUpload");
    if (!canFlushUpload) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Flush Upload") && canFlushUpload) {
        (void)flushQueuedEvents();
    }
    if (!canFlushUpload) {
        disabledTooltip("flushUpload");
        ImGui::EndDisabled();
    }

    ImGui::Text("Privacy: %s", m_lastSnapshot.value("privacyStatus", "unknown").c_str());
    ImGui::Text("Upload: %s", m_lastSnapshot.value("uploadStatus", "unknown").c_str());
    ImGui::Text("Queued Events: %zu", m_lastSnapshot.value("queuedEventCount", size_t{0}));
    ImGui::Text("Retention: max age %llu ticks, max events %zu",
                static_cast<unsigned long long>(m_lastSnapshot.value("retentionMaxAgeTicks", uint64_t{0})),
                m_lastSnapshot.value("retentionMaxEventCount", size_t{0}));

    if (m_lastSnapshot.contains("disabledUploadMessage") &&
        !m_lastSnapshot["disabledUploadMessage"].get<std::string>().empty()) {
        ImGui::TextDisabled("%s", m_lastSnapshot["disabledUploadMessage"].get<std::string>().c_str());
    }

    ImGui::End();
#endif
}

bool AnalyticsPanel::setOptIn(bool enabled) {
    if (!m_dispatcher) {
        recordAction("set_opt_in", false, "No analytics dispatcher is bound.");
        rebuildSnapshot();
        return false;
    }

    m_dispatcher->setOptIn(enabled);
    if (m_privacyController) {
        m_privacyController->recordConsentDecision(enabled ? urpg::analytics::ConsentState::Granted
                                                           : urpg::analytics::ConsentState::Denied);
    }
    recordAction("set_opt_in", true, enabled ? "Analytics collection enabled." : "Analytics collection disabled.");
    rebuildSnapshot();
    return true;
}

bool AnalyticsPanel::applyEndpointProfile() {
    if (!m_uploader) {
        recordAction("apply_endpoint_profile", false, "No analytics uploader is bound.");
        rebuildSnapshot();
        return false;
    }
    if (!m_endpointProfile) {
        recordAction("apply_endpoint_profile", false, "No analytics endpoint profile is bound.");
        rebuildSnapshot();
        return false;
    }

    const auto result = urpg::analytics::applyAnalyticsEndpointProfile(*m_uploader, *m_endpointProfile);
    if (!result.applied) {
        const std::string message = result.diagnostics.empty()
                                        ? "Analytics endpoint profile was not applied."
                                        : result.diagnostics.front().message;
        recordAction("apply_endpoint_profile", false, message, result.diagnostics.size());
        rebuildSnapshot();
        return false;
    }

    recordAction("apply_endpoint_profile", true, "Analytics endpoint profile applied.");
    rebuildSnapshot();
    return true;
}

size_t AnalyticsPanel::clearQueuedEvents() {
    if (!m_dispatcher) {
        recordAction("clear_queue", false, "No analytics dispatcher is bound.");
        rebuildSnapshot();
        return 0;
    }

    const size_t cleared = m_dispatcher->clearQueuedEvents();
    recordAction("clear_queue", true, "Queued analytics events cleared.", cleared);
    rebuildSnapshot();
    return cleared;
}

bool AnalyticsPanel::flushQueuedEvents() {
    if (!m_dispatcher) {
        recordAction("flush_upload", false, "No analytics dispatcher is bound.");
        rebuildSnapshot();
        return false;
    }
    if (!m_uploader) {
        recordAction("flush_upload", false, "No analytics uploader is bound.");
        rebuildSnapshot();
        return false;
    }
    if (!m_uploader->hasUploadHandler()) {
        recordAction("flush_upload", false, "Upload disabled: no upload handler configured.");
        rebuildSnapshot();
        return false;
    }
    if (m_privacyController && !m_privacyController->isAnalyticsPermitted()) {
        recordAction("flush_upload", false, "Upload disabled: analytics consent is not granted.");
        rebuildSnapshot();
        return false;
    }
    if (!m_dispatcher->isOptIn()) {
        recordAction("flush_upload", false, "Upload disabled: analytics opt-in is off.");
        rebuildSnapshot();
        return false;
    }

    const auto events = m_dispatcher->snapshotEvents();
    const auto result = m_uploader->flush(events, m_sessionId);
    if (!result.success) {
        recordAction("flush_upload", false, result.errorMessage, result.eventsFlushed);
        rebuildSnapshot();
        return false;
    }

    if (result.eventsFlushed > 0) {
        (void)m_dispatcher->clearQueuedEvents();
    }
    recordAction("flush_upload", true, "Queued analytics events flushed.", result.eventsFlushed);
    rebuildSnapshot();
    return true;
}

void AnalyticsPanel::clearLastAction() {
    m_lastAction = nlohmann::json::object();
    rebuildSnapshot();
}

nlohmann::json AnalyticsPanel::lastRenderSnapshot() const {
    return m_lastSnapshot;
}

void AnalyticsPanel::rebuildSnapshot() {
    nlohmann::json snapshot;
    snapshot["dispatcherBound"] = m_dispatcher != nullptr;
    snapshot["uploaderBound"] = m_uploader != nullptr;
    snapshot["endpointProfileBound"] = m_endpointProfile != nullptr;
    snapshot["endpointProfile"] = nullptr;
    snapshot["uploadMode"] = "disabled";
    snapshot["localExportPath"] = "";
    snapshot["privacyControllerBound"] = m_privacyController != nullptr;
    snapshot["sessionId"] = m_sessionId;
    snapshot["lastAction"] = m_lastAction.is_null() ? nlohmann::json::object() : m_lastAction;
    snapshot["statusMessages"] = nlohmann::json::array();

    if (m_dispatcher == nullptr) {
        snapshot["optIn"] = false;
        snapshot["privacyStatus"] =
            m_privacyController ? consentStateToString(m_privacyController->getConsentState()) : "unbound";
        snapshot["analyticsPermitted"] = false;
        snapshot["queuedEventCount"] = 0;
        snapshot["sessionEventCount"] = 0;
        snapshot["allowedCategories"] = nlohmann::json::array();
        snapshot["validationIssueCount"] = 0;
        snapshot["validationIssues"] = nlohmann::json::array();
        snapshot["errorCount"] = 0;
        snapshot["warningCount"] = 0;
        snapshot["recentEvents"] = nlohmann::json::array();
        snapshot["retentionMaxAgeTicks"] =
            m_privacyController ? m_privacyController->getRetentionPolicy().maxAgeTicks : 0;
        snapshot["retentionMaxEventCount"] =
            m_privacyController ? m_privacyController->getRetentionPolicy().maxEventCount : 0;
        snapshot["uploadStatus"] = "disabled";
        snapshot["disabledUploadMessage"] = "No analytics dispatcher is bound.";
        snapshot["statusMessages"].push_back("No analytics dispatcher is bound; analytics actions are disabled.");
        if (m_privacyController == nullptr) {
            snapshot["statusMessages"].push_back("No analytics privacy controller is bound.");
        }
        snapshot["actions"] = {
            {"setOptIn", false},
            {"applyEndpointProfile", false},
            {"clearQueue", false},
            {"flushUpload", false},
        };
        snapshot["actionDetails"] = {
            {"setOptIn", {{"enabled", false}, {"disabledReason", "No analytics dispatcher is bound."}}},
            {"applyEndpointProfile", {{"enabled", false}, {"disabledReason", "No analytics uploader is bound."}}},
            {"clearQueue", {{"enabled", false}, {"disabledReason", "No analytics dispatcher is bound."}}},
            {"flushUpload", {{"enabled", false}, {"disabledReason", "No analytics dispatcher is bound."}}},
        };
        m_lastSnapshot = std::move(snapshot);
        return;
    }

    snapshot["optIn"] = m_dispatcher->isOptIn();
    snapshot["privacyStatus"] =
        m_privacyController ? consentStateToString(m_privacyController->getConsentState()) : "unbound";
    snapshot["analyticsPermitted"] =
        m_privacyController ? m_privacyController->isAnalyticsPermitted() : m_dispatcher->isOptIn();
    snapshot["requiresConsentPrompt"] =
        m_privacyController && m_privacyController->getConsentState() == urpg::analytics::ConsentState::Unknown;
    if (m_privacyController == nullptr) {
        snapshot["statusMessages"].push_back(
            "No analytics privacy controller is bound; dispatcher opt-in is used as fallback.");
    } else if (m_privacyController->getConsentState() == urpg::analytics::ConsentState::Unknown) {
        snapshot["statusMessages"].push_back(
            "Analytics consent has not been recorded; uploads remain disabled until opt-in.");
    }
    snapshot["queuedEventCount"] = m_dispatcher->getQueuedEventCount();
    snapshot["sessionEventCount"] = m_dispatcher->getSessionEventCount();
    snapshot["allowedCategories"] = m_dispatcher->getAllowedCategories();
    snapshot["retentionMaxAgeTicks"] = m_privacyController ? m_privacyController->getRetentionPolicy().maxAgeTicks : 0;
    snapshot["retentionMaxEventCount"] =
        m_privacyController ? m_privacyController->getRetentionPolicy().maxEventCount : 0;

    if (m_endpointProfile) {
        snapshot["endpointProfile"] = m_endpointProfile->toJson();
        snapshot["endpointProfileStatus"] = urpg::release::providerProfileStatusToJson(
            urpg::analytics::analyticsEndpointProfileStatus(*m_endpointProfile));
        const auto profileDiagnostics = urpg::analytics::validateAnalyticsEndpointProfile(*m_endpointProfile);
        nlohmann::json diagnostics = nlohmann::json::array();
        for (const auto& diagnostic : profileDiagnostics) {
            diagnostics.push_back({
                {"code", diagnostic.code},
                {"message", diagnostic.message},
                {"target", diagnostic.target},
            });
        }
        snapshot["endpointProfileDiagnostics"] = diagnostics;
        snapshot["endpointProfilePrivacyApproved"] = m_endpointProfile->privacyReview.approved;
    } else {
        snapshot["endpointProfileStatus"] = nullptr;
        snapshot["endpointProfileDiagnostics"] = nlohmann::json::array();
        snapshot["endpointProfilePrivacyApproved"] = false;
        snapshot["statusMessages"].push_back("No analytics endpoint profile is bound.");
    }

    nlohmann::json allEvents = m_dispatcher->getBufferSnapshot();
    nlohmann::json recentEvents = nlohmann::json::array();
    nlohmann::json validationIssues = nlohmann::json::array();
    std::size_t errorCount = 0;
    std::size_t warningCount = 0;

    const size_t sampleSize = 10;
    size_t start = 0;
    if (allEvents.size() > sampleSize) {
        start = allEvents.size() - sampleSize;
    }

    for (size_t i = start; i < allEvents.size(); ++i) {
        recentEvents.push_back(allEvents[i]);
    }

    for (const auto& issue : m_dispatcher->getValidationIssues()) {
        if (issue.severity == urpg::analytics::AnalyticsValidationSeverity::Error) {
            ++errorCount;
        } else {
            ++warningCount;
        }

        validationIssues.push_back({
            {"event_name", issue.eventName},
            {"category_value", issue.categoryValue},
            {"parameter_key", issue.parameterKey},
            {"category", issueCategoryToString(issue.category)},
            {"severity", issueSeverityToString(issue.severity)},
            {"message", issue.message},
        });
    }

    snapshot["recentEvents"] = std::move(recentEvents);
    snapshot["validationIssueCount"] = validationIssues.size();
    snapshot["validationIssues"] = std::move(validationIssues);
    snapshot["errorCount"] = errorCount;
    snapshot["warningCount"] = warningCount;

    if (m_uploader && m_uploader->localJsonlExportPath().has_value()) {
        snapshot["uploadMode"] = "local_jsonl";
        snapshot["localExportPath"] = m_uploader->localJsonlExportPath()->generic_string();
    } else if (m_uploader && m_uploader->httpEndpoint().has_value()) {
        snapshot["uploadMode"] = m_uploader->uploadMode();
        snapshot["uploadEndpoint"] = m_uploader->httpEndpoint()->url;
    }

    std::string disabledMessage;
    if (!m_uploader) {
        disabledMessage = "No analytics uploader is bound.";
        snapshot["statusMessages"].push_back("No analytics uploader is bound; uploads are disabled.");
    } else if (!m_uploader->hasUploadHandler()) {
        disabledMessage = "Upload disabled: no upload handler configured.";
        snapshot["statusMessages"].push_back("Analytics uploader has no upload handler configured.");
    } else if (m_privacyController && !m_privacyController->isAnalyticsPermitted()) {
        disabledMessage = "Upload disabled: analytics consent is not granted.";
    } else if (!m_dispatcher->isOptIn()) {
        disabledMessage = "Upload disabled: analytics opt-in is off.";
    } else if (m_dispatcher->getQueuedEventCount() == 0) {
        disabledMessage = "No queued analytics events to upload.";
    }

    snapshot["uploadStatus"] = disabledMessage.empty() ? "ready" : "disabled";
    snapshot["disabledUploadMessage"] = disabledMessage;
    const bool canApplyEndpointProfile = m_uploader != nullptr && m_endpointProfile != nullptr;
    const bool canClearQueue = m_dispatcher->getQueuedEventCount() > 0;
    const auto endpointProfileDisabledReason = [this]() -> std::string {
        if (m_uploader == nullptr) {
            return "No analytics uploader is bound.";
        }
        if (m_endpointProfile == nullptr) {
            return "No analytics endpoint profile is bound.";
        }
        return "";
    };
    snapshot["actions"] = {
        {"setOptIn", true},
        {"applyEndpointProfile", canApplyEndpointProfile},
        {"clearQueue", canClearQueue},
        {"flushUpload", disabledMessage.empty()},
    };
    snapshot["actionDetails"] = {
        {"setOptIn", {{"enabled", true}, {"disabledReason", ""}}},
        {"applyEndpointProfile", {{"enabled", canApplyEndpointProfile}, {"disabledReason", endpointProfileDisabledReason()}}},
        {"clearQueue",
         {{"enabled", canClearQueue},
          {"disabledReason", canClearQueue ? "" : "No queued analytics events to clear."}}},
        {"flushUpload", {{"enabled", disabledMessage.empty()}, {"disabledReason", disabledMessage}}},
    };
    m_lastSnapshot = std::move(snapshot);
}

void AnalyticsPanel::recordAction(std::string action, bool success, std::string message, size_t affectedEvents) {
    m_lastAction = {
        {"action", std::move(action)},
        {"success", success},
        {"message", std::move(message)},
        {"affectedEvents", affectedEvents},
    };
}

std::string AnalyticsPanel::consentStateToString(urpg::analytics::ConsentState state) {
    switch (state) {
    case urpg::analytics::ConsentState::Granted:
        return "granted";
    case urpg::analytics::ConsentState::Denied:
        return "denied";
    case urpg::analytics::ConsentState::Unknown:
        return "unknown";
    }
    return "unknown";
}

} // namespace urpg::editor
