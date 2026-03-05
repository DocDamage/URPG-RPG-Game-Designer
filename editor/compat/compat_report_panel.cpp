// Compat Report Panel - Editor Infrastructure Implementation
// Phase 2 - Compat Layer
//
// Provides real-time compatibility status reporting for MZ plugins.

#include "compat_report_panel.h"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>

namespace urpg {
namespace editor {

using namespace std::chrono;

// ============================================================================
// Helper Functions
// ============================================================================

static std::string formatTimestamp(uint64_t timestampMs) {
    auto secs = timestampMs / 1000;
    auto mins = secs / 60;
    auto hrs = mins / 60;
    auto days = hrs / 24;
    
    std::stringstream ss;
    ss << days << "d " << (hrs % 24) << "h " << (mins % 60) << "m " << (secs % 60) << "s";
    return ss.str();
}

static std::string escapeJson(const std::string& s) {
    std::string result;
    result.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c; break;
        }
    }
    return result;
}

static std::string escapeCsv(const std::string& s) {
    if (s.find(',') != std::string::npos || s.find('"') != std::string::npos) {
        std::string result = "\"";
        for (char c : s) {
            if (c == '"') result += "\"\"";
            else result += c;
        }
        result += "\"";
        return result;
    }
    return s;
}

// ============================================================================
// CompatEvent Implementation
// ============================================================================

std::string CompatEvent::severityToString() const {
    switch (severity) {
        case Severity::INFO: return "INFO";
        case Severity::WARNING: return "WARNING";
        case Severity::ERROR: return "ERROR";
        case Severity::CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

CompatEvent::Severity CompatEvent::severityFromString(const std::string& s) {
    if (s == "INFO") return Severity::INFO;
    if (s == "WARNING") return Severity::WARNING;
    if (s == "ERROR") return Severity::ERROR;
    if (s == "CRITICAL") return Severity::CRITICAL;
    return Severity::INFO;
}

// ============================================================================
// CompatReportModel Implementation
// ============================================================================

CompatReportModel::CompatReportModel() = default;

CompatReportModel::~CompatReportModel() = default;

void CompatReportModel::recordCall(const std::string& pluginId, const std::string& className,
                                        const std::string& methodName, CompatStatus status,
                                        uint64_t durationUs) {
    auto& records = pluginCalls_[pluginId];
    
    auto it = std::find_if(records.begin(), records.end(),
        [&](const CompatCallRecord& r) {
            return r.className == className && r.methodName == methodName;
        });
    
    if (it != records.end()) {
        it->callCount++;
        it->totalDurationUs += durationUs;
        it->lastCallTimestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        it->status = status;
    } else {
        CompatCallRecord record;
        record.pluginId = pluginId;
        record.className = className;
        record.methodName = methodName;
        record.status = status;
        record.callCount = 1;
        record.totalDurationUs = durationUs;
        record.lastCallTimestamp = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
        record.hasWarning = (status == CompatStatus::PARTIAL || status == CompatStatus::STUB);
        record.hasError = (status == CompatStatus::UNSUPPORTED);
        records.push_back(record);
    }
    
    summariesDirty_ = true;
}

void CompatReportModel::recordEvent(const CompatEvent& event) {
    eventLog_.push_back(event);
    
    std::sort(eventLog_.begin(), eventLog_.end(),
        [](const CompatEvent& a, const CompatEvent& b) {
            return a.timestamp > b.timestamp;
        });
}

void CompatReportModel::ingestPluginFailureDiagnosticsJsonl(std::string_view diagnostics_jsonl) {
    if (diagnostics_jsonl.empty()) {
        return;
    }

    std::istringstream stream{std::string(diagnostics_jsonl)};
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        nlohmann::json row = nlohmann::json::parse(line, nullptr, false);
        if (row.is_discarded() || !row.is_object()) {
            continue;
        }

        if (row.value("subsystem", "") != "plugin_manager" ||
            row.value("event", "") != "compat_failure") {
            continue;
        }

        CompatEvent event;
        event.timestamp = row.value(
            "seq",
            static_cast<uint64_t>(
                duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()
            )
        );
        event.pluginId = row.value("plugin", "");
        event.className = "PluginManager";
        event.methodName = row.value("operation", "");
        event.severity = CompatEvent::Severity::ERROR;
        event.message = row.value("message", "");
        event.sourceFile = row.value("subsystem", "");

        const std::string command = row.value("command", "");
        if (!event.pluginId.empty()) {
            event.navigationTarget = "plugin://" + event.pluginId;
            if (!command.empty()) {
                event.navigationTarget += "#" + command;
            }
        }

        recordEvent(event);

        if (!event.pluginId.empty()) {
            const std::string methodName =
                event.methodName.empty() ? "unknown_failure" : event.methodName;
            recordCall(
                event.pluginId,
                "PluginManager",
                methodName,
                CompatStatus::UNSUPPORTED
            );
        }
    }
}

void CompatReportModel::recalculateSummaries() const {
    if (!summariesDirty_) return;
    
    for (const auto& pair : pluginCalls_) {
        const std::string& pluginId = pair.first;
        const std::vector<CompatCallRecord>& calls = pair.second;
        
        PluginCompatSummary summary;
        summary.pluginId = pluginId;
        
        for (const auto& call : calls) {
            switch (call.status) {
                case CompatStatus::FULL: summary.fullCount++; break;
                case CompatStatus::PARTIAL: summary.partialCount++; break;
                case CompatStatus::STUB: summary.stubCount++; break;
                case CompatStatus::UNSUPPORTED: summary.unsupportedCount++; break;
            }
            summary.totalCalls += call.callCount;
            summary.totalDurationUs += call.totalDurationUs;
            if (call.hasWarning) summary.warningCount++;
            if (call.hasError) summary.errorCount++;
        }
        
        summary.calculateScore();
        pluginSummaries_[pluginId] = summary;
    }
    
    summariesDirty_ = false;
}

std::vector<PluginCompatSummary> CompatReportModel::getAllPluginSummaries() const {
    recalculateSummaries();
    
    std::vector<PluginCompatSummary> result;
    result.reserve(pluginCalls_.size());
    
    for (const auto& pair : pluginCalls_) {
        result.push_back(getPluginSummary(pair.first));
    }
    
    return result;
}

PluginCompatSummary CompatReportModel::getPluginSummary(const std::string& pluginId) const {
    recalculateSummaries();
    
    auto it = pluginSummaries_.find(pluginId);
    if (it != pluginSummaries_.end()) {
        return it->second;
    }
    
    PluginCompatSummary empty;
    empty.pluginId = pluginId;
    return empty;
}

std::vector<CompatCallRecord> CompatReportModel::getPluginCalls(const std::string& pluginId) const {
    auto it = pluginCalls_.find(pluginId);
    if (it == pluginCalls_.end()) {
        return {};
    }
    return it->second;
}

std::vector<CompatEvent> CompatReportModel::getPluginEvents(const std::string& pluginId) const {
    std::vector<CompatEvent> result;
    std::copy_if(eventLog_.begin(), eventLog_.end(), std::back_inserter(result),
        [&pluginId](const CompatEvent& e) {
            return e.pluginId == pluginId;
        });
    return result;
}

std::vector<CompatEvent> CompatReportModel::getRecentEvents(uint32_t limit) const {
    std::vector<CompatEvent> result;
    uint32_t count = 0;
    for (const auto& event : eventLog_) {
        result.push_back(event);
        if (++count >= limit) break;
    }
    return result;
}

std::vector<CompatEvent> CompatReportModel::getEventsBySeverity(CompatEvent::Severity minSeverity) const {
    std::vector<CompatEvent> result;
    
    int minOrder = static_cast<int>(minSeverity);
    
    for (const auto& event : eventLog_) {
        int eventOrder = static_cast<int>(event.severity);
        if (eventOrder >= minOrder) {
            result.push_back(event);
        }
    }
    
    return result;
}

void CompatReportModel::setNavigationHandler(NavigationHandler handler) {
    navigationHandler_ = std::move(handler);
}

void CompatReportModel::navigateTo(const std::string& target) {
    if (navigationHandler_) {
        navigationHandler_(target);
    }
}

void CompatReportModel::startSession() {
    sessionActive_ = true;
    sessionStartTime_ = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}

void CompatReportModel::endSession() {
    sessionActive_ = false;
}

void CompatReportModel::clearHistory() {
    pluginCalls_.clear();
    pluginSummaries_.clear();
    eventLog_.clear();
    summariesDirty_ = true;
}

std::string CompatReportModel::exportAsJson() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"exportTime\": \"" << formatTimestamp(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()) << "\",\n";
    ss << "  \"sessionStart\": \"" << formatTimestamp(sessionStartTime_) << "\",\n";
    ss << "  \"plugins\": [\n";
    
    auto summaries = getAllPluginSummaries();
    bool first = true;
    for (const auto& summary : summaries) {
        if (!first) ss << ",\n";
        first = false;
        
        ss << "    {\n";
        ss << "      \"pluginId\": \"" << escapeJson(summary.pluginId) << "\",\n";
        ss << "      \"pluginName\": \"" << escapeJson(summary.pluginName) << "\",\n";
        ss << "      \"version\": \"" << escapeJson(summary.version) << "\",\n";
        ss << "      \"compatScore\": " << summary.compatibilityScore << ",\n";
        ss << "      \"fullCount\": " << summary.fullCount << ",\n";
        ss << "      \"partialCount\": " << summary.partialCount << ",\n";
        ss << "      \"stubCount\": " << summary.stubCount << ",\n";
        ss << "      \"unsupportedCount\": " << summary.unsupportedCount << ",\n";
        ss << "      \"warningCount\": " << summary.warningCount << ",\n";
        ss << "      \"errorCount\": " << summary.errorCount << "\n";
        ss << "    }";
    }
    
    ss << "\n  ],\n";
    ss << "  \"events\": [\n";
    
    first = true;
    for (const auto& event : eventLog_) {
        if (!first) ss << ",\n";
        first = false;
        
        ss << "    {\n";
        ss << "      \"timestamp\": \"" << formatTimestamp(event.timestamp) << "\",\n";
        ss << "      \"pluginId\": \"" << escapeJson(event.pluginId) << "\",\n";
        ss << "      \"severity\": \"" << event.severityToString() << "\",\n";
        ss << "      \"className\": \"" << escapeJson(event.className) << "\",\n";
        ss << "      \"methodName\": \"" << escapeJson(event.methodName) << "\",\n";
        ss << "      \"message\": \"" << escapeJson(event.message) << "\"\n";
        ss << "    }";
    }
    
    ss << "\n  ]\n";
    ss << "}\n";
    
    return ss.str();
}

std::string CompatReportModel::exportAsCsv() const {
    std::stringstream ss;
    
    ss << "Plugin ID,Plugin Name,Version,Compat Score,Full Count,Partial Count,Stub Count,Unsupported Count,Warning Count,Error Count\n";
    
    auto summaries = getAllPluginSummaries();
    for (const auto& summary : summaries) {
        ss << escapeCsv(summary.pluginId) << ","
           << escapeCsv(summary.pluginName) << ","
           << escapeCsv(summary.version) << ","
           << summary.compatibilityScore << ","
           << summary.fullCount << ","
           << summary.partialCount << ","
           << summary.stubCount << ","
           << summary.unsupportedCount << ","
           << summary.warningCount << ","
           << summary.errorCount << "\n";
    }
    
    return ss.str();
}

int32_t CompatReportModel::getProjectCompatibilityScore() const {
    auto summaries = getAllPluginSummaries();
    if (summaries.empty()) return 100;
    
    int32_t totalScore = 0;
    for (const auto& s : summaries) {
        totalScore += s.compatibilityScore;
    }
    return totalScore / static_cast<int32_t>(summaries.size());
}

// ============================================================================
// CompatReportView Implementation
// ============================================================================

CompatReportView::CompatReportView(CompatReportModel& model)
    : model_(model)
{
}

void CompatReportView::setSortBy(SortBy sortBy, bool ascending) {
    sortBy_ = sortBy;
    ascending_ = ascending;
}

void CompatReportView::setFilter(const Filter& filter) {
    filter_ = filter;
}

std::vector<PluginCompatSummary> CompatReportView::getVisibleSummaries() const {
    auto summaries = model_.getAllPluginSummaries();
    
    std::vector<PluginCompatSummary> filtered;
    for (const auto& s : summaries) {
        if (!filter_.pluginId.empty() && s.pluginId != filter_.pluginId) continue;
        if (filter_.hideFullCompat && s.compatibilityScore == 100) continue;
        if (filter_.showOnlyWithIssues && s.compatibilityScore == 100) continue;
        filtered.push_back(s);
    }
    
    std::sort(filtered.begin(), filtered.end(),
        [this](const PluginCompatSummary& a, const PluginCompatSummary& b) {
            int cmp = 0;
            switch (sortBy_) {
                case SortBy::PLUGIN_NAME:
                    cmp = a.pluginName.compare(b.pluginName);
                    break;
                case SortBy::COMPATIBILITY_SCORE:
                    cmp = a.compatibilityScore - b.compatibilityScore;
                    break;
                case SortBy::WARNING_COUNT:
                    cmp = static_cast<int32_t>(a.warningCount) - static_cast<int32_t>(b.warningCount);
                    break;
                case SortBy::CALL_COUNT: {
                    int64_t aTotal = a.fullCount + a.partialCount + a.stubCount;
                    int64_t bTotal = b.fullCount + b.partialCount + b.stubCount;
                    cmp = static_cast<int>(aTotal - bTotal);
                    break;
                }
                case SortBy::LAST_UPDATED:
                    cmp = 0;
                    break;
            }
            return ascending_ ? cmp < 0 : cmp > 0;
        });
    
    return filtered;
}

std::vector<CompatEvent> CompatReportView::getVisibleEvents() const {
    auto events = model_.getRecentEvents(1000);
    
    std::vector<CompatEvent> filtered;
    for (const auto& e : events) {
        if (!filter_.pluginId.empty() && e.pluginId != filter_.pluginId) continue;
        
        int minOrder = static_cast<int>(filter_.minSeverity);
        int eventOrder = static_cast<int>(e.severity);
        if (eventOrder < minOrder) continue;
        
        filtered.push_back(e);
    }
    
    return filtered;
}

void CompatReportView::selectPlugin(const std::string& pluginId) {
    selectedPlugin_ = pluginId;
    detailView_ = true;
    if (onPluginSelected_) {
        onPluginSelected_(pluginId);
    }
}

void CompatReportView::clearSelection() {
    selectedPlugin_.clear();
    detailView_ = false;
}

void CompatReportView::setVisible(bool visible) {
    visible_ = visible;
}

bool CompatReportView::isVisible() const {
    return visible_;
}

void CompatReportView::setDetailView(bool detailView) {
    detailView_ = detailView;
}

bool CompatReportView::isDetailView() const {
    return detailView_;
}

void CompatReportView::refresh() {
}

void CompatReportView::exportReport(const std::string& format, const std::string& path) {
    std::ofstream file(path);
    if (!file.is_open()) return;
    
    if (format == "json") {
        file << model_.exportAsJson();
    } else if (format == "csv") {
        file << model_.exportAsCsv();
    }
}

void CompatReportView::setOnPluginSelected(PluginSelectedHandler handler) {
    onPluginSelected_ = std::move(handler);
}

void CompatReportView::setOnNavigate(NavigateHandler handler) {
    onNavigate_ = std::move(handler);
}

std::vector<CompatReportView::SummaryRow> CompatReportView::getSummaryRows() const {
    auto summaries = getVisibleSummaries();
    std::vector<SummaryRow> rows;
    rows.reserve(summaries.size());
    
    for (const auto& s : summaries) {
        SummaryRow row;
        row.pluginId = s.pluginId;
        row.displayName = s.pluginName.empty() ? s.pluginId : s.pluginName;
        row.score = s.compatibilityScore;
        row.scoreColor = getScoreColor(row.score);
        row.fullCount = s.fullCount;
        row.partialCount = s.partialCount;
        row.stubCount = s.stubCount;
        row.unsupportedCount = s.unsupportedCount;
        row.warningCount = s.warningCount;
        row.errorCount = s.errorCount;
        row.lastUpdated = "N/A";
        rows.push_back(row);
    }
    
    return rows;
}

std::vector<CompatReportView::EventRow> CompatReportView::getEventRows() const {
    auto events = getVisibleEvents();
    std::vector<EventRow> rows;
    rows.reserve(events.size());
    
    for (const auto& e : events) {
        EventRow row;
        row.timestamp = e.timestamp;
        row.timeStr = "N/A";
        row.pluginId = e.pluginId;
        row.severityIcon = getSeverityIcon(e.severity);
        row.severityColor = getSeverityColor(e.severity);
        row.message = e.message;
        row.location = e.sourceFile.empty() ? "" : e.sourceFile + ":" + std::to_string(e.sourceLine);
        row.hasNavigation = !e.navigationTarget.empty();
        rows.push_back(row);
    }
    
    return rows;
}

std::string CompatReportView::getScoreColor(int32_t score) {
    if (score >= 90) return "#4CAF50";
    if (score >= 70) return "#8BC34A";
    if (score >= 50) return "#FFC107";
    if (score >= 30) return "#FF9800";
    return "#F44336";
}

std::string CompatReportView::getSeverityColor(CompatEvent::Severity severity) {
    switch (severity) {
        case CompatEvent::Severity::INFO: return "#2196F3";
        case CompatEvent::Severity::WARNING: return "#FF9800";
        case CompatEvent::Severity::ERROR: return "#F44336";
        case CompatEvent::Severity::CRITICAL: return "#9C27B0";
        default: return "#9E9E9E";
    }
}

std::string CompatReportView::getSeverityIcon(CompatEvent::Severity severity) {
    switch (severity) {
        case CompatEvent::Severity::INFO: return "info";
        case CompatEvent::Severity::WARNING: return "warning";
        case CompatEvent::Severity::ERROR: return "error";
        case CompatEvent::Severity::CRITICAL: return "critical";
        default: return "unknown";
    }
}

// ============================================================================
// CompatReportPanel Implementation
// ============================================================================

CompatReportPanel::CompatReportPanel()
    : view_(model_)
{
}

CompatReportPanel::~CompatReportPanel() = default;

void CompatReportPanel::render() {
    if (!visible_) return;
    
    if (detailView_) {
        renderDetailView();
    } else {
        renderSummaryView();
    }
}

void CompatReportPanel::update() {
}

void CompatReportPanel::renderSummaryView() {
    auto summaries = view_.getVisibleSummaries();
    
    for (const auto& summary : summaries) {
        renderPluginHeader(summary);
    }
}

void CompatReportPanel::renderDetailView() {
    if (selectedPlugin_.empty()) {
        renderSummaryView();
        return;
    }
    
    auto calls = model_.getPluginCalls(selectedPlugin_);
    
    renderCallTable(calls);
    
    renderEventTimeline();
}

void CompatReportPanel::renderEventTimeline() {
    auto events = model_.getRecentEvents(50);
    
    for (const auto& event : events) {
        (void)event;
    }
}

void CompatReportPanel::renderPluginHeader(const PluginCompatSummary& summary) {
    (void)summary;
}

void CompatReportPanel::renderCallTable(const std::vector<CompatCallRecord>& calls) {
    for (const auto& call : calls) {
        (void)call;
    }
}

void CompatReportPanel::setOnPluginSelected(PluginSelectedHandler handler) {
    onPluginSelected_ = std::move(handler);
}

void CompatReportPanel::setOnNavigate(NavigateHandler handler) {
    onNavigate_ = std::move(handler);
}

void CompatReportPanel::selectPlugin(const std::string& pluginId) {
    selectedPlugin_ = pluginId;
    detailView_ = true;
    if (onPluginSelected_) {
        onPluginSelected_(pluginId);
    }
}

void CompatReportPanel::clearSelection() {
    selectedPlugin_.clear();
    detailView_ = false;
}

void CompatReportPanel::refresh() {
}

void CompatReportPanel::exportReport(const std::string& format, const std::string& path) {
    view_.exportReport(format, path);
}

} // namespace editor
} // namespace urpg
