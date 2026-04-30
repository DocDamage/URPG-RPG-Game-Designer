// Unit tests for CompatReportPanel - Editor Infrastructure
// Phase 2 - Compat Layer
//
// Tests verify the CompatReportPanel UI behavior including:
// - Navigation support
// - Filtering and sorting
// - Data projections for row rendering
// - Per-plugin status tracking
// - Warning/error event handling

#include "editor/compat/compat_report_panel.h"
#include "runtimes/compat_js/plugin_manager.h"
#include <algorithm>
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string_view>
#include <thread>
#include <utility>

using namespace urpg::editor;
using namespace urpg::compat;

namespace {

std::filesystem::path uniqueTempFixturePath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(stem) + "_" + std::to_string(ticks) + ".json");
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

PluginInfo makePluginInfo(std::string name) {
    PluginInfo info;
    info.name = std::move(name);
    info.version = "1.0";
    return info;
}

} // namespace

TEST_CASE("CompatCallRecord - Default initialization", "[compat][panel]") {
    CompatCallRecord record;

    REQUIRE(record.callCount == 0);
    REQUIRE(record.totalDurationUs == 0);
    REQUIRE(record.lastCallTimestamp == 0);
    REQUIRE(record.hasWarning == false);
    REQUIRE(record.hasError == false);
}

TEST_CASE("PluginCompatSummary - Score calculation", "[compat][panel]") {
    PluginCompatSummary summary;

    SECTION("Empty summary has 100% score") {
        summary.calculateScore();
        REQUIRE(summary.getCompatibilityScore() == 100);
    }

    SECTION("All FULL gives 100% score") {
        summary.fullCount = 10;
        summary.calculateScore();
        REQUIRE(summary.getCompatibilityScore() == 100);
    }

    SECTION("All UNSUPPORTED gives 0% score") {
        summary.unsupportedCount = 10;
        summary.calculateScore();
        REQUIRE(summary.getCompatibilityScore() == 0);
    }

    SECTION("Mixed status calculates weighted score") {
        summary.fullCount = 5;    // 5 * 100 = 500
        summary.partialCount = 3; // 3 * 70  = 210
        summary.stubCount = 2;    // 2 * 30  = 60
        summary.unsupportedCount = 0;
        // Total = 10, Weighted = 770, Score = 77
        summary.calculateScore();
        REQUIRE(summary.getCompatibilityScore() == 77);
    }

    SECTION("Equal distribution gives expected score") {
        summary.fullCount = 1;
        summary.partialCount = 1;
        summary.stubCount = 1;
        summary.unsupportedCount = 1;
        // (100 + 70 + 30 + 0) / 4 = 50
        summary.calculateScore();
        REQUIRE(summary.getCompatibilityScore() == 50);
    }
}

TEST_CASE("CompatEvent - Severity conversion", "[compat][panel]") {
    CompatEvent event;

    SECTION("String to severity conversion") {
        REQUIRE(CompatEvent::severityFromString("INFO") == CompatEvent::Severity::INFO);
        REQUIRE(CompatEvent::severityFromString("WARNING") == CompatEvent::Severity::WARNING);
        REQUIRE(CompatEvent::severityFromString("ERROR") == CompatEvent::Severity::ERROR);
        REQUIRE(CompatEvent::severityFromString("CRITICAL") == CompatEvent::Severity::CRITICAL);
        REQUIRE(CompatEvent::severityFromString("UNKNOWN") == CompatEvent::Severity::INFO);
    }

    SECTION("Severity to string conversion") {
        event.severity = CompatEvent::Severity::INFO;
        REQUIRE(event.severityToString() == "INFO");

        event.severity = CompatEvent::Severity::WARNING;
        REQUIRE(event.severityToString() == "WARNING");

        event.severity = CompatEvent::Severity::ERROR;
        REQUIRE(event.severityToString() == "ERROR");

        event.severity = CompatEvent::Severity::CRITICAL;
        REQUIRE(event.severityToString() == "CRITICAL");
    }
}

TEST_CASE("CompatReportModel - Basic operations", "[compat][panel]") {
    CompatReportModel model;

    SECTION("New model is empty") {
        auto summaries = model.getAllPluginSummaries();
        REQUIRE(summaries.empty());
    }

    SECTION("Record single call") {
        model.recordCall("testPlugin", "Window_Base", "initialize", CompatStatus::FULL, 100);

        auto calls = model.getPluginCalls("testPlugin");
        REQUIRE(calls.size() == 1);
        REQUIRE(calls[0].className == "Window_Base");
        REQUIRE(calls[0].methodName == "initialize");
        REQUIRE(calls[0].callCount == 1);
        REQUIRE(calls[0].status == CompatStatus::FULL);
    }

    SECTION("Multiple calls to same method aggregate") {
        model.recordCall("plugin1", "Window_Selectable", "select", CompatStatus::FULL, 50);
        model.recordCall("plugin1", "Window_Selectable", "select", CompatStatus::FULL, 75);
        model.recordCall("plugin1", "Window_Selectable", "select", CompatStatus::FULL, 25);

        auto calls = model.getPluginCalls("plugin1");
        REQUIRE(calls.size() == 1);
        REQUIRE(calls[0].callCount == 3);
        REQUIRE(calls[0].totalDurationUs == 150);
    }

    SECTION("Different methods create separate records") {
        model.recordCall("plugin1", "Window_Base", "update", CompatStatus::FULL);
        model.recordCall("plugin1", "Window_Base", "draw", CompatStatus::PARTIAL);

        auto calls = model.getPluginCalls("plugin1");
        REQUIRE(calls.size() == 2);
    }

    SECTION("Different plugins tracked separately") {
        model.recordCall("plugin1", "Window_Base", "update", CompatStatus::FULL);
        model.recordCall("plugin2", "Window_Base", "update", CompatStatus::STUB);

        auto calls1 = model.getPluginCalls("plugin1");
        auto calls2 = model.getPluginCalls("plugin2");

        REQUIRE(calls1.size() == 1);
        REQUIRE(calls1[0].status == CompatStatus::FULL);
        REQUIRE(calls2.size() == 1);
        REQUIRE(calls2[0].status == CompatStatus::STUB);
    }

    SECTION("Status transitions update warning/error flags on aggregated records") {
        model.recordCall("plugin1", "Window_Base", "update", CompatStatus::FULL);
        model.recordCall("plugin1", "Window_Base", "update", CompatStatus::UNSUPPORTED);

        auto calls = model.getPluginCalls("plugin1");
        REQUIRE(calls.size() == 1);
        REQUIRE(calls[0].callCount == 2);
        REQUIRE(calls[0].status == CompatStatus::UNSUPPORTED);
        REQUIRE(calls[0].hasWarning == false);
        REQUIRE(calls[0].hasError == true);

        auto summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.unsupportedCount == 1);
        REQUIRE(summary.errorCount == 1);
        REQUIRE(summary.warningCount == 0);

        model.recordCall("plugin1", "Window_Base", "update", CompatStatus::FULL);
        calls = model.getPluginCalls("plugin1");
        REQUIRE(calls.size() == 1);
        REQUIRE(calls[0].status == CompatStatus::FULL);
        REQUIRE(calls[0].hasWarning == false);
        REQUIRE(calls[0].hasError == false);

        summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.fullCount == 1);
        REQUIRE(summary.unsupportedCount == 0);
        REQUIRE(summary.errorCount == 0);
    }
}

TEST_CASE("CompatReportModel - Summary generation", "[compat][panel]") {
    CompatReportModel model;

    SECTION("Summary counts status correctly") {
        model.recordCall("plugin1", "Class1", "method1", CompatStatus::FULL);
        model.recordCall("plugin1", "Class1", "method2", CompatStatus::FULL);
        model.recordCall("plugin1", "Class2", "method3", CompatStatus::PARTIAL);
        model.recordCall("plugin1", "Class2", "method4", CompatStatus::STUB);
        model.recordCall("plugin1", "Class3", "method5", CompatStatus::UNSUPPORTED);

        auto summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.fullCount == 2);
        REQUIRE(summary.partialCount == 1);
        REQUIRE(summary.stubCount == 1);
        REQUIRE(summary.unsupportedCount == 1);
    }

    SECTION("Warning and error counts") {
        model.recordCall("plugin1", "Class1", "method1", CompatStatus::PARTIAL);
        model.recordCall("plugin1", "Class1", "method2", CompatStatus::STUB);
        model.recordCall("plugin1", "Class2", "method3", CompatStatus::UNSUPPORTED);

        auto summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.warningCount == 2); // PARTIAL + STUB
        REQUIRE(summary.errorCount == 1);   // UNSUPPORTED
    }
}

TEST_CASE("CompatReportModel - History and timestamps", "[compat][panel]") {
    CompatReportModel model;

    SECTION("Record call stamps first seen and last updated") {
        model.recordCall("plugin1", "Class", "method1", CompatStatus::FULL);

        auto summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.firstSeenTimestamp > 0);
        REQUIRE(summary.lastUpdatedTimestamp == summary.firstSeenTimestamp);
        REQUIRE(summary.scoreHistory.empty());

        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        model.recordCall("plugin1", "Class", "method2", CompatStatus::PARTIAL);

        summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.lastUpdatedTimestamp >= summary.firstSeenTimestamp);
    }

    SECTION("Session end records bounded score history") {
        for (int i = 0; i < 20; ++i) {
            model.startSession();
            model.recordCall("plugin1", "Class", "method1",
                             (i % 2 == 0) ? CompatStatus::FULL : CompatStatus::UNSUPPORTED);
            model.endSession();
        }

        const auto summary = model.getPluginSummary("plugin1");
        REQUIRE(summary.scoreHistory.size() == 16);
        REQUIRE(summary.scoreHistory.back() == summary.compatibilityScore);
    }
}

TEST_CASE("CompatReportModel - Event logging", "[compat][panel]") {
    CompatReportModel model;

    SECTION("Record and retrieve events") {
        CompatEvent event;
        event.pluginId = "plugin1";
        event.className = "Window_Base";
        event.methodName = "initialize";
        event.severity = CompatEvent::Severity::WARNING;
        event.message = "Partial implementation detected";

        model.recordEvent(event);

        auto events = model.getPluginEvents("plugin1");
        REQUIRE(events.size() == 1);
        REQUIRE(events[0].message == "Partial implementation detected");
    }

    SECTION("Get recent events with limit") {
        for (int i = 0; i < 20; i++) {
            CompatEvent event;
            event.pluginId = "plugin" + std::to_string(i);
            event.severity = CompatEvent::Severity::INFO;
            model.recordEvent(event);
        }

        auto events = model.getRecentEvents(5);
        REQUIRE(events.size() == 5);
    }

    SECTION("Get events by severity") {
        CompatEvent infoEvent, warningEvent, errorEvent;
        infoEvent.severity = CompatEvent::Severity::INFO;
        warningEvent.severity = CompatEvent::Severity::WARNING;
        errorEvent.severity = CompatEvent::Severity::ERROR;

        model.recordEvent(infoEvent);
        model.recordEvent(warningEvent);
        model.recordEvent(errorEvent);

        auto warnings = model.getEventsBySeverity(CompatEvent::Severity::WARNING);
        REQUIRE(warnings.size() == 2); // WARNING + ERROR
    }
}

TEST_CASE("CompatReportModel - Plugin failure diagnostics ingestion", "[compat][panel]") {
    CompatReportModel model;

    const std::string diagnosticsJsonl =
        R"({"seq":1,"subsystem":"plugin_manager","event":"compat_failure","plugin":"BrokenEntryFixture","command":"brokenCommand","operation":"load_plugin_js_entry","message":"Fixture JS command entry cannot be empty","severity":"WARN"})"
        "\n"
        R"({"seq":2,"subsystem":"plugin_manager","event":"compat_failure","plugin":"BrokenEvalFixture","command":"brokenEval","operation":"load_plugin_js_eval","message":"fixture eval failure","severity":"CRASH_PREVENTED"})"
        "\n"
        R"({"seq":3,"subsystem":"other_subsystem","event":"ignored","plugin":"Nope","command":"","operation":"noop","message":"ignore me"})"
        "\n"
        "{not json}";

    model.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    auto events = model.getRecentEvents(10);
    REQUIRE(events.size() == 2);

    bool foundEntryFailure = false;
    bool foundEvalFailure = false;
    for (const auto& event : events) {
        if (event.pluginId == "BrokenEntryFixture") {
            foundEntryFailure = true;
            REQUIRE(event.methodName == "load_plugin_js_entry");
            REQUIRE(event.severity == CompatEvent::Severity::WARNING);
            REQUIRE(event.navigationTarget == "plugin://BrokenEntryFixture#brokenCommand");
        }
        if (event.pluginId == "BrokenEvalFixture") {
            foundEvalFailure = true;
            REQUIRE(event.methodName == "load_plugin_js_eval");
            REQUIRE(event.message == "fixture eval failure");
            REQUIRE(event.severity == CompatEvent::Severity::CRITICAL);
            REQUIRE(event.navigationTarget == "plugin://BrokenEvalFixture#brokenEval");
        }
    }
    REQUIRE(foundEntryFailure);
    REQUIRE(foundEvalFailure);

    const auto entrySummary = model.getPluginSummary("BrokenEntryFixture");
    REQUIRE(entrySummary.partialCount == 1);
    REQUIRE(entrySummary.warningCount == 1);
    REQUIRE(entrySummary.unsupportedCount == 0);
    REQUIRE(entrySummary.errorCount == 0);

    const auto evalSummary = model.getPluginSummary("BrokenEvalFixture");
    REQUIRE(evalSummary.unsupportedCount == 1);
    REQUIRE(evalSummary.errorCount == 1);

    const auto exported = model.exportAsJson();
    REQUIRE(exported.find("BrokenEntryFixture") != std::string::npos);
    REQUIRE(exported.find("BrokenEvalFixture") != std::string::npos);
}

TEST_CASE("CompatReportModel - Plugin failure severity mapping covers all tags", "[compat][panel]") {
    CompatReportModel model;

    const std::string diagnosticsJsonl =
        R"({"seq":11,"subsystem":"plugin_manager","event":"compat_failure","plugin":"SeverityFixture","command":"warnCommand","operation":"warn_op","message":"warn message","severity":"WARN"})"
        "\n"
        R"({"seq":12,"subsystem":"plugin_manager","event":"compat_failure","plugin":"SeverityFixture","command":"softCommand","operation":"soft_fail_op","message":"soft fail message","severity":"SOFT_FAIL"})"
        "\n"
        R"({"seq":13,"subsystem":"plugin_manager","event":"compat_failure","plugin":"SeverityFixture","command":"hardCommand","operation":"hard_fail_op","message":"hard fail message","severity":"HARD_FAIL"})"
        "\n"
        R"({"seq":14,"subsystem":"plugin_manager","event":"compat_failure","plugin":"SeverityFixture","command":"crashCommand","operation":"crash_prevented_op","message":"crash prevented message","severity":"CRASH_PREVENTED"})";

    model.ingestPluginFailureDiagnosticsJsonl(diagnosticsJsonl);

    const auto events = model.getPluginEvents("SeverityFixture");
    REQUIRE(events.size() == 4);

    const auto requireEvent = [&](std::string_view methodName) -> const CompatEvent& {
        const auto it = std::find_if(events.begin(), events.end(),
                                     [&](const CompatEvent& event) { return event.methodName == methodName; });
        REQUIRE(it != events.end());
        return *it;
    };

    REQUIRE(requireEvent("warn_op").severity == CompatEvent::Severity::WARNING);
    REQUIRE(requireEvent("soft_fail_op").severity == CompatEvent::Severity::ERROR);
    REQUIRE(requireEvent("hard_fail_op").severity == CompatEvent::Severity::ERROR);
    REQUIRE(requireEvent("crash_prevented_op").severity == CompatEvent::Severity::CRITICAL);

    const auto summary = model.getPluginSummary("SeverityFixture");
    REQUIRE(summary.partialCount == 1);
    REQUIRE(summary.unsupportedCount == 3);
    REQUIRE(summary.warningCount == 1);
    REQUIRE(summary.errorCount == 3);
}

TEST_CASE("CompatReportModel - Session management", "[compat][panel]") {
    CompatReportModel model;

    SECTION("Start and end session") {
        model.startSession();
        model.recordCall("plugin1", "Class1", "method1", CompatStatus::FULL);
        model.endSession();

        auto summaries = model.getAllPluginSummaries();
        REQUIRE(summaries.size() == 1);
    }

    SECTION("Clear history") {
        model.recordCall("plugin1", "Class1", "method1", CompatStatus::FULL);

        model.clearHistory();

        auto summaries = model.getAllPluginSummaries();
        REQUIRE(summaries.empty());
    }
}

TEST_CASE("CompatReportModel - Navigation", "[compat][panel]") {
    CompatReportModel model;
    std::string lastTarget;

    model.setNavigationHandler([&lastTarget](const std::string& target) { lastTarget = target; });

    SECTION("Navigate to target") {
        model.navigateTo("event://123");
        REQUIRE(lastTarget == "event://123");
    }

    SECTION("Empty handler does nothing") {
        CompatReportModel modelNoHandler;
        modelNoHandler.navigateTo("test"); // Should not crash
    }
}

TEST_CASE("CompatReportModel - Export", "[compat][panel]") {
    CompatReportModel model;

    model.recordCall("plugin1", "Window_Base", "initialize", CompatStatus::FULL);
    model.recordCall("plugin1", "Window_Selectable", "select", CompatStatus::PARTIAL);

    SECTION("Export as JSON") {
        std::string json = model.exportAsJson();
        REQUIRE(json.find("\"pluginId\": \"plugin1\"") != std::string::npos);
        REQUIRE(json.find("compatScore") != std::string::npos);
    }

    SECTION("Export as CSV") {
        std::string csv = model.exportAsCsv();
        REQUIRE(csv.find("Plugin ID") != std::string::npos);
        REQUIRE(csv.find("plugin1") != std::string::npos);
    }
}

TEST_CASE("CompatReportModel - Project compatibility score", "[compat][panel]") {
    CompatReportModel model;

    SECTION("Empty project has 100% score") {
        REQUIRE(model.getProjectCompatibilityScore() == 100);
    }

    SECTION("Project score aggregates plugins") {
        model.recordCall("plugin1", "Class1", "method1", CompatStatus::FULL);
        model.recordCall("plugin1", "Class1", "method2", CompatStatus::FULL);
        model.recordCall("plugin2", "Class1", "method1", CompatStatus::UNSUPPORTED);

        int32_t score = model.getProjectCompatibilityScore();
        REQUIRE(score >= 0);
        REQUIRE(score <= 100);
    }
}

TEST_CASE("CompatReportView - Sorting", "[compat][panel]") {
    CompatReportModel model;

    // Create plugins with different scores
    model.recordCall("pluginA", "Class", "method", CompatStatus::FULL);
    model.recordCall("pluginB", "Class", "method", CompatStatus::UNSUPPORTED);
    model.recordCall("pluginC", "Class", "method", CompatStatus::PARTIAL);

    CompatReportView view(model);

    SECTION("Sort by compatibility score ascending") {
        view.setSortBy(CompatReportView::SortBy::COMPATIBILITY_SCORE, true);
        auto summaries = view.getVisibleSummaries();

        REQUIRE(summaries.size() >= 2);
        // First should have lower score than last
        REQUIRE(summaries.front().getCompatibilityScore() <= summaries.back().getCompatibilityScore());
    }

    SECTION("Sort by compatibility score descending") {
        view.setSortBy(CompatReportView::SortBy::COMPATIBILITY_SCORE, false);
        auto summaries = view.getVisibleSummaries();

        REQUIRE(summaries.size() >= 2);
        // First should have higher score than last
        REQUIRE(summaries.front().getCompatibilityScore() >= summaries.back().getCompatibilityScore());
    }

    SECTION("Sort by call count includes unsupported calls") {
        CompatReportModel callCountModel;
        callCountModel.recordCall("pluginA", "Class", "fullMethod", CompatStatus::FULL);
        callCountModel.recordCall("pluginB", "Class", "unsupportedMethod1", CompatStatus::UNSUPPORTED);
        callCountModel.recordCall("pluginB", "Class", "unsupportedMethod2", CompatStatus::UNSUPPORTED);

        CompatReportView callCountView(callCountModel);
        callCountView.setSortBy(CompatReportView::SortBy::CALL_COUNT, false);
        auto summaries = callCountView.getVisibleSummaries();

        REQUIRE(summaries.size() == 2);
        REQUIRE(summaries[0].pluginId == "pluginB");
        REQUIRE(summaries[0].totalCalls == 2);
        REQUIRE(summaries[1].pluginId == "pluginA");
        REQUIRE(summaries[1].totalCalls == 1);
    }

    SECTION("Sort by last updated prefers newer plugin activity") {
        CompatReportModel updatedModel;
        updatedModel.recordCall("pluginA", "Class", "method", CompatStatus::FULL);
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        updatedModel.recordCall("pluginB", "Class", "method", CompatStatus::FULL);

        CompatReportView updatedView(updatedModel);
        updatedView.setSortBy(CompatReportView::SortBy::LAST_UPDATED, false);

        const auto summaries = updatedView.getVisibleSummaries();
        REQUIRE(summaries.size() == 2);
        REQUIRE(summaries[0].pluginId == "pluginB");
        REQUIRE(summaries[1].pluginId == "pluginA");

        const auto rows = updatedView.getSummaryRows();
        REQUIRE(rows.size() == 2);
        REQUIRE(rows[0].lastUpdated != "Never");
        REQUIRE(rows[1].lastUpdated != "Never");
    }
}

TEST_CASE("CompatReportView - Filtering", "[compat][panel]") {
    CompatReportModel model;

    model.recordCall("pluginA", "Class", "method", CompatStatus::FULL);
    model.recordCall("pluginB", "Class", "method", CompatStatus::UNSUPPORTED);

    CompatReportView view(model);
    CompatReportView::Filter filter;

    SECTION("Filter by plugin ID") {
        filter.pluginId = "pluginA";
        view.setFilter(filter);

        auto summaries = view.getVisibleSummaries();
        REQUIRE(summaries.size() == 1);
        REQUIRE(summaries[0].pluginId == "pluginA");
    }

    SECTION("Hide fully compatible plugins") {
        filter.hideFullCompat = true;
        view.setFilter(filter);

        auto summaries = view.getVisibleSummaries();
        for (const auto& s : summaries) {
            REQUIRE(s.getCompatibilityScore() < 100);
        }
    }

    SECTION("Show only with issues") {
        filter.showOnlyWithIssues = true;
        view.setFilter(filter);

        auto summaries = view.getVisibleSummaries();
        for (const auto& s : summaries) {
            REQUIRE(s.getCompatibilityScore() < 100);
        }
    }
}

TEST_CASE("CompatReportView - Plugin selection", "[compat][panel]") {
    CompatReportModel model;
    CompatReportView view(model);

    SECTION("Select plugin") {
        view.selectPlugin("testPlugin");
        REQUIRE(view.getSelectedPlugin() == "testPlugin");
        REQUIRE(view.isDetailView() == true);
    }

    SECTION("Clear selection") {
        view.selectPlugin("testPlugin");
        view.clearSelection();
        REQUIRE(view.getSelectedPlugin().empty());
        REQUIRE(view.isDetailView() == false);
    }
}

TEST_CASE("CompatReportView - Visibility", "[compat][panel]") {
    CompatReportModel model;
    CompatReportView view(model);

    SECTION("Default visibility") {
        REQUIRE(view.isVisible() == false);
    }

    SECTION("Set visible") {
        view.setVisible(true);
        REQUIRE(view.isVisible() == true);

        view.setVisible(false);
        REQUIRE(view.isVisible() == false);
    }
}

TEST_CASE("CompatReportView - Event filtering", "[compat][panel]") {
    CompatReportModel model;

    // Add events with different severities
    CompatEvent infoEvent, warningEvent, errorEvent;
    infoEvent.severity = CompatEvent::Severity::INFO;
    warningEvent.severity = CompatEvent::Severity::WARNING;
    errorEvent.severity = CompatEvent::Severity::ERROR;

    model.recordEvent(infoEvent);
    model.recordEvent(warningEvent);
    model.recordEvent(errorEvent);

    CompatReportView view(model);
    CompatReportView::Filter filter;

    SECTION("Filter by minimum severity") {
        filter.minSeverity = CompatEvent::Severity::WARNING;
        view.setFilter(filter);

        auto events = view.getVisibleEvents();
        REQUIRE(events.size() == 2); // WARNING + ERROR
    }
}

TEST_CASE("CompatReportView - Export", "[compat][panel]") {
    CompatReportModel model;
    model.recordCall("plugin1", "Class", "method", CompatStatus::FULL);

    CompatReportView view(model);
    const auto uniqueSuffix = std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    const auto jsonPath = std::filesystem::temp_directory_path() / ("urpg_compat_report_" + uniqueSuffix + ".json");
    const auto csvPath = std::filesystem::temp_directory_path() / ("urpg_compat_report_" + uniqueSuffix + ".csv");

    SECTION("Export to JSON file") {
        view.exportReport("json", jsonPath.string());
        REQUIRE(std::filesystem::exists(jsonPath));
        std::error_code ec;
        std::filesystem::remove(jsonPath, ec);
    }

    SECTION("Export to CSV file") {
        view.exportReport("csv", csvPath.string());
        REQUIRE(std::filesystem::exists(csvPath));
        std::error_code ec;
        std::filesystem::remove(csvPath, ec);
    }
}

TEST_CASE("CompatReportPanel - Refresh ingests PluginManager diagnostics artifacts", "[compat][panel][integration]") {
    auto& pluginManager = urpg::compat::PluginManager::instance();
    pluginManager.unloadAllPlugins();
    pluginManager.clearFailureDiagnostics();

    CompatReportPanel panel;

    pluginManager.executeCommand("MissingPlugin", "missingCommand", {});
    REQUIRE_FALSE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    panel.refresh();

    auto& model = panel.getModel();
    auto missingPluginEvents = model.getPluginEvents("MissingPlugin");
    REQUIRE(missingPluginEvents.size() == 1);
    REQUIRE(missingPluginEvents[0].className == "PluginManager");
    REQUIRE(missingPluginEvents[0].methodName == "execute_command");
    REQUIRE(missingPluginEvents[0].severity == CompatEvent::Severity::WARNING);
    REQUIRE(missingPluginEvents[0].navigationTarget == "plugin://MissingPlugin#missingCommand");

    const auto missingPluginSummary = model.getPluginSummary("MissingPlugin");
    REQUIRE(missingPluginSummary.partialCount == 1);
    REQUIRE(missingPluginSummary.warningCount == 1);
    REQUIRE(missingPluginSummary.unsupportedCount == 0);
    REQUIRE(missingPluginSummary.errorCount == 0);

    REQUIRE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    pluginManager.executeCommandByName("invalidCommandName", {});
    REQUIRE_FALSE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    panel.update();

    auto genericEvents = model.getPluginEvents("");
    REQUIRE(genericEvents.size() == 1);
    REQUIRE(genericEvents[0].methodName == "execute_command_by_name_parse");
    REQUIRE(genericEvents[0].navigationTarget.empty());

    REQUIRE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    pluginManager.clearFailureDiagnostics();
    pluginManager.unloadAllPlugins();
}

TEST_CASE("CompatReportPanel - curated save-data lifecycle keeps execution and failure buckets separate",
          "[compat][panel][integration]") {
    auto& pluginManager = urpg::compat::PluginManager::instance();
    pluginManager.unloadAllPlugins();
    pluginManager.clearExecutionDiagnostics();
    pluginManager.clearFailureDiagnostics();

    constexpr const char* kPlugin = "CGMZ_MenuCommandWindow";
    REQUIRE(pluginManager.registerPlugin(makePluginInfo(kPlugin)));

    const std::vector<std::string> validCommands = {
        "addSaveCommand",
        "addLoadCommand",
        "addContinueCommand",
        "addOptionsCommand",
    };
    for (const auto& command : validCommands) {
        REQUIRE(pluginManager.registerCommand(kPlugin, command, [](const std::vector<urpg::Value>&) -> urpg::Value {
            return urpg::Value();
        }));
        pluginManager.executeCommand(kPlugin, command, {});
    }

    pluginManager.executeCommand(kPlugin, "missingSaveCommand", {});
    pluginManager.executeCommand(kPlugin, "missingLoadCommand", {});

    REQUIRE_FALSE(pluginManager.exportExecutionDiagnosticsJsonl().empty());
    REQUIRE_FALSE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    CompatReportPanel panel;
    panel.refresh();

    const auto& model = panel.getModel();
    const auto warningEvents = model.getEventsBySeverity(CompatEvent::Severity::WARNING);
    REQUIRE(warningEvents.size() == 2);
    for (const auto& event : warningEvents) {
        REQUIRE(event.pluginId == kPlugin);
        REQUIRE(event.severity == CompatEvent::Severity::WARNING);
        REQUIRE(event.methodName == "execute_command");
    }

    const auto allEvents = model.getRecentEvents(10);
    REQUIRE(allEvents.size() == 6);

    const auto summary = model.getPluginSummary(kPlugin);
    REQUIRE(summary.fullCount == validCommands.size());
    REQUIRE(summary.partialCount == 1);
    REQUIRE(summary.warningCount == 1);
    REQUIRE(summary.unsupportedCount == 0);
    REQUIRE(summary.errorCount == 0);

    const auto calls = model.getPluginCalls(kPlugin);
    const auto missingCall = std::find_if(calls.begin(), calls.end(), [](const CompatCallRecord& call) {
        return call.className == "PluginManager" && call.methodName == "execute_command";
    });
    REQUIRE(missingCall != calls.end());
    REQUIRE(missingCall->callCount == 2);
    REQUIRE(missingCall->status == CompatStatus::PARTIAL);

    REQUIRE(pluginManager.exportExecutionDiagnosticsJsonl().empty());
    REQUIRE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    pluginManager.unloadAllPlugins();
}

TEST_CASE("CompatReportPanel - Export contains runtime QuickJS failure diagnostics", "[compat][panel][integration]") {
    auto& pluginManager = urpg::compat::PluginManager::instance();
    pluginManager.unloadAllPlugins();
    pluginManager.clearFailureDiagnostics();

    const auto runtimeFailureFixture = uniqueTempFixturePath("urpg_runtime_report_fixture");
    writeTextFile(runtimeFailureFixture,
                  R"({
  "name": "RuntimeReportFixture",
  "commands": [
    {
      "name": "brokenRuntime",
      "entry": "brokenRuntimeEntry",
      "js": "// @urpg-fail-call brokenRuntimeEntry report runtime failure"
    }
  ]
})");

    REQUIRE(pluginManager.loadPlugin(runtimeFailureFixture.string()));
    const auto result = pluginManager.executeCommand("RuntimeReportFixture", "brokenRuntime", {});
    REQUIRE(std::holds_alternative<std::monostate>(result.v));
    REQUIRE_FALSE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    CompatReportPanel panel;
    panel.refresh();

    const std::string exported = panel.getModel().exportAsJson();
    REQUIRE(exported.find("RuntimeReportFixture") != std::string::npos);
    REQUIRE(exported.find("execute_command_quickjs_call") != std::string::npos);
    REQUIRE(exported.find("Host function error: report runtime failure") != std::string::npos);

    REQUIRE(pluginManager.exportFailureDiagnosticsJsonl().empty());

    pluginManager.clearFailureDiagnostics();
    pluginManager.unloadAllPlugins();
    std::error_code ec;
    std::filesystem::remove(runtimeFailureFixture, ec);
}

TEST_CASE("CompatReportPanel - Integration", "[compat][panel][integration]") {
    CompatReportModel model;
    CompatReportView view(model);

    SECTION("Full workflow") {
        // Start session
        model.startSession();

        // Record various calls
        model.recordCall("YEP_Core", "Window_Base", "initialize", CompatStatus::FULL, 150);
        model.recordCall("YEP_Core", "Window_Base", "update", CompatStatus::FULL, 50);
        model.recordCall("YEP_Core", "Window_Selectable", "select", CompatStatus::PARTIAL, 100);
        model.recordCall("YEP_Battle", "Sprite_Actor", "update", CompatStatus::STUB, 200);
        model.recordCall("YEP_Battle", "Sprite_Enemy", "update", CompatStatus::UNSUPPORTED, 0);

        // Record events
        CompatEvent event;
        event.pluginId = "YEP_Battle";
        event.className = "Sprite_Enemy";
        event.methodName = "update";
        event.severity = CompatEvent::Severity::ERROR;
        event.message = "Method not implemented";
        model.recordEvent(event);

        // End session
        model.endSession();

        // Verify summaries
        auto summaries = view.getVisibleSummaries();
        REQUIRE(summaries.size() == 2);

        // Verify project score
        int32_t score = model.getProjectCompatibilityScore();
        REQUIRE(score >= 0);
        REQUIRE(score <= 100);

        // Export
        std::string json = model.exportAsJson();
        REQUIRE(!json.empty());
    }
}
