#pragma once

// Compat Report Panel - Editor Infrastructure
// Phase 2 - Compat Layer
//
// Provides real-time compatibility status reporting for MZ plugins.
// Per Section 4 - Compat Report panel: per-plugin status, warnings trending over time,
// one-click jump to offending line.
//
// This panel integrates with the compatibility layer to surface:
// - API status (FULL/PARTIAL/STUB/UNSUPPORTED)
// - Call counts and performance metrics
// - Deviations and warnings
// - Plugin-specific compatibility scores

#include "engine/core/sync/source_authority.h"
#include "runtimes/compat_js/quickjs_runtime.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace editor {

// Forward declarations
class CompatReportModel;
class CompatReportView;

// CompatStatus from quickjs_runtime.h is used directly
using compat::CompatStatus;

// Individual API call record for detailed reporting
struct CompatCallRecord {
    std::string pluginId;
    std::string className;
    std::string methodName;
    CompatStatus status;
    std::string deviationNote;
    uint32_t callCount = 0;
    uint64_t totalDurationUs = 0;  // Microseconds
    uint64_t lastCallTimestamp = 0;
    bool hasWarning = false;
    bool hasError = false;
};

// Plugin-level compatibility summary
struct PluginCompatSummary {
    std::string pluginId;
    std::string pluginName;
    std::string version;
    
    // Status counts
    uint32_t fullCount = 0;
    uint32_t partialCount = 0;
    uint32_t stubCount = 0;
    uint32_t unsupportedCount = 0;
    
    // Overall score (0-100)
    int32_t compatibilityScore = 0;
    
    // Performance metrics
    uint32_t totalCalls = 0;
    uint64_t totalDurationUs = 0;
    
    // Issues
    uint32_t warningCount = 0;
    uint32_t errorCount = 0;
    
    // Trend data (last N sessions)
    std::vector<int32_t> scoreHistory;
    
    // Timestamps
    uint64_t firstSeenTimestamp = 0;
    uint64_t lastUpdatedTimestamp = 0;
    
    // Calculate score from status counts
    void calculateScore() {
        uint32_t total = fullCount + partialCount + stubCount + unsupportedCount;
        if (total == 0) {
            compatibilityScore = 100;
            return;
        }
        // Weighted scoring: FULL=100, PARTIAL=70, STUB=30, UNSUPPORTED=0
        int32_t weightedSum = (fullCount * 100) + (partialCount * 70) + (stubCount * 30);
        compatibilityScore = weightedSum / static_cast<int32_t>(total);
    }
    
    // Get compatibility score (calculates if needed)
    int32_t getCompatibilityScore() const {
        const_cast<PluginCompatSummary*>(this)->calculateScore();
        return compatibilityScore;
    }
};

// Warning/error event for timeline display
struct CompatEvent {
    enum class Severity : uint8_t {
        INFO = 0,
        WARNING = 1,
        ERROR = 2,
        CRITICAL = 3
    };
    
    uint64_t timestamp = 0;
    std::string pluginId;
    std::string className;
    std::string methodName;
    Severity severity = Severity::INFO;
    std::string message;
    std::string sourceFile;
    int32_t sourceLine = -1;
    
    // For navigation
    std::string navigationTarget;  // Event/block ID or file:line
    
    // Helper methods
    std::string severityToString() const;
    static Severity severityFromString(const std::string& s);
};

// CompatReportModel - Data model for compatibility reporting
//
// Collects data from the compatibility layer and maintains
// historical records for trending analysis.
//
class CompatReportModel {
public:
    CompatReportModel();
    ~CompatReportModel();
    
    // Record API usage
    void recordCall(const std::string& pluginId, const std::string& className,
                    const std::string& methodName, CompatStatus status,
                    uint64_t durationUs = 0);
    
    // Record events (warnings/errors)
    void recordEvent(const CompatEvent& event);

    // Ingest PluginManager structured failure diagnostics (JSONL).
    void ingestPluginFailureDiagnosticsJsonl(std::string_view diagnostics_jsonl);
    
    // Get summaries
    std::vector<PluginCompatSummary> getAllPluginSummaries() const;
    PluginCompatSummary getPluginSummary(const std::string& pluginId) const;
    
    // Get detailed records
    std::vector<CompatCallRecord> getPluginCalls(const std::string& pluginId) const;
    std::vector<CompatEvent> getPluginEvents(const std::string& pluginId) const;
    std::vector<CompatEvent> getRecentEvents(uint32_t limit = 100) const;
    
    // Get events by severity
    std::vector<CompatEvent> getEventsBySeverity(CompatEvent::Severity minSeverity) const;
    
    // Navigation support
    using NavigationHandler = std::function<void(const std::string& target)>;
    void setNavigationHandler(NavigationHandler handler);
    void navigateTo(const std::string& target);
    
    // Session management
    void startSession();
    void endSession();
    void clearHistory();
    
    // Export for auditing
    std::string exportAsJson() const;
    std::string exportAsCsv() const;
    
    // Get overall project compatibility score
    int32_t getProjectCompatibilityScore() const;
    
private:
    // Per-plugin call records
    std::unordered_map<std::string, std::vector<CompatCallRecord>> pluginCalls_;
    
    // Per-plugin summaries (cached)
    mutable std::unordered_map<std::string, PluginCompatSummary> pluginSummaries_;
    mutable bool summariesDirty_ = true;
    
    // Event log (all events across all plugins)
    std::vector<CompatEvent> eventLog_;
    
    // Navigation callback
    NavigationHandler navigationHandler_;
    
    // Session tracking
    uint64_t sessionStartTime_ = 0;
    bool sessionActive_ = false;
    
    // Recalculate summaries from call records
    void recalculateSummaries() const;
};

// CompatReportView - View/ViewModel for UI rendering
//
// Provides sorted/filtered projections of the model data
// for rendering in the editor panel.
//
class CompatReportView {
public:
    // Sort options
    enum class SortBy : uint8_t {
        PLUGIN_NAME = 0,
        COMPATIBILITY_SCORE = 1,
        WARNING_COUNT = 2,
        CALL_COUNT = 3,
        LAST_UPDATED = 4
    };
    
    // Filter options
    struct Filter {
        std::string pluginId;  // Empty = all plugins
        CompatEvent::Severity minSeverity = CompatEvent::Severity::INFO;
        bool hideFullCompat = false;  // Hide plugins with 100% score
        bool showOnlyWithIssues = false;
    };
    
    explicit CompatReportView(CompatReportModel& model);
    ~CompatReportView() = default;
    
    // Sorting
    void setSortBy(SortBy sortBy, bool ascending = true);
    SortBy getSortBy() const { return sortBy_; }
    bool isAscending() const { return ascending_; }
    
    // Filtering
    void setFilter(const Filter& filter);
    const Filter& getFilter() const { return filter_; }
    
    // Get filtered/sorted data
    std::vector<PluginCompatSummary> getVisibleSummaries() const;
    std::vector<CompatEvent> getVisibleEvents() const;
    
    // Row projection for table rendering
    struct SummaryRow {
        std::string pluginId;
        std::string displayName;
        int32_t score;
        std::string scoreColor;  // CSS color for score
        uint32_t fullCount;
        uint32_t partialCount;
        uint32_t stubCount;
        uint32_t unsupportedCount;
        uint32_t warningCount;
        uint32_t errorCount;
        std::string lastUpdated;  // Human-readable
    };
    
    std::vector<SummaryRow> getSummaryRows() const;
    
    // Event row for timeline/event list
    struct EventRow {
        uint64_t timestamp;
        std::string timeStr;  // Human-readable
        std::string pluginId;
        std::string severityIcon;
        std::string severityColor;
        std::string message;
        std::string location;  // file:line or event/block
        bool hasNavigation;
    };
    
    std::vector<EventRow> getEventRows() const;
    
    // Score color coding
    static std::string getScoreColor(int32_t score);
    static std::string getSeverityColor(CompatEvent::Severity severity);
    static std::string getSeverityIcon(CompatEvent::Severity severity);
    
    // Event callbacks
    using PluginSelectedHandler = std::function<void(const std::string& pluginId)>;
    using NavigateHandler = std::function<void(const std::string& target)>;
    
    void setOnPluginSelected(PluginSelectedHandler handler);
    void setOnNavigate(NavigateHandler handler);
    
    // Selection
    void selectPlugin(const std::string& pluginId);
    void clearSelection();
    std::string getSelectedPlugin() const { return selectedPlugin_; }
    
    // Visibility
    void setVisible(bool visible);
    bool isVisible() const;
    
    // Detail view mode
    void setDetailView(bool detailView);
    bool isDetailView() const;
    
    // Export
    void exportReport(const std::string& format, const std::string& path);
    
    // Refresh view
    void refresh();
    
private:
    CompatReportModel& model_;
    SortBy sortBy_ = SortBy::COMPATIBILITY_SCORE;
    bool ascending_ = false;  // Highest scores first by default
    Filter filter_;
    
    // Selection state
    std::string selectedPlugin_;
    bool detailView_ = false;
    bool visible_ = false;
    
    // Callbacks
    PluginSelectedHandler onPluginSelected_;
    NavigateHandler onNavigate_;
};

// CompatReportPanel - Main panel controller
//
// Integrates model and view, handles UI events and updates.
//
class CompatReportPanel {
public:
    using PluginSelectedHandler = std::function<void(const std::string& pluginId)>;
    using NavigateHandler = std::function<void(const std::string& target)>;
    
    CompatReportPanel();
    ~CompatReportPanel();
    
    // Model access (for integration with compat layer)
    CompatReportModel& getModel() { return model_; }
    
    // UI rendering
    void render();
    void update();
    
    // Event handlers
    void setOnPluginSelected(PluginSelectedHandler handler);
    void setOnNavigate(NavigateHandler handler);
    
    // Selection state
    void selectPlugin(const std::string& pluginId);
    void clearSelection();
    std::string getSelectedPlugin() const { return selectedPlugin_; }
    
    // Panel state
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    
    void setDetailView(bool detailView) { detailView_ = detailView; }
    bool isDetailView() const { return detailView_; }
    
    // Refresh data from compat layer
    void refresh();
    
    // Export functionality
    void exportReport(const std::string& format, const std::string& path);
    
private:
    CompatReportModel model_;
    CompatReportView view_;
    
    bool visible_ = true;
    bool detailView_ = false;  // false = summary, true = detail
    std::string selectedPlugin_;
    
    PluginSelectedHandler onPluginSelected_;
    NavigateHandler onNavigate_;
    
    // Render helpers
    void renderSummaryView();
    void renderDetailView();
    void renderEventTimeline();
    void renderPluginHeader(const PluginCompatSummary& summary);
    void renderCallTable(const std::vector<CompatCallRecord>& calls);
};

} // namespace editor
} // namespace urpg
