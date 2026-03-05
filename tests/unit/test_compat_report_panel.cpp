// Unit tests for CompatReportPanel - Editor Infrastructure
// Phase 2 - Compat Layer
//
// Tests verify the CompatReportPanel UI behavior including:
// - Navigation support
// - Filtering and sorting
// - Data projections for row rendering
// - Per-plugin status tracking
// - Warning/error event handling

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "editor/compat/compat_report_panel.h"
#include <chrono>
#include <thread>

using namespace urpg::editor;
using namespace urpg::compat;

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
        summary.fullCount = 5;      // 5 * 100 = 500
        summary.partialCount = 3;   // 3 * 70  = 210
        summary.stubCount = 2;      // 2 * 30  = 60
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
        REQUIRE(summary.warningCount == 2);  // PARTIAL + STUB
        REQUIRE(summary.errorCount == 1);    // UNSUPPORTED
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
        REQUIRE(warnings.size() == 2);  // WARNING + ERROR
    }
}

TEST_CASE("CompatReportModel - Plugin failure diagnostics ingestion", "[compat][panel]") {
    CompatReportModel model;

    const std::string diagnosticsJsonl =
        R"({"seq":1,"subsystem":"plugin_manager","event":"compat_failure","plugin":"BrokenEntryFixture","command":"brokenCommand","operation":"load_plugin_js_entry","message":"Fixture JS command entry cannot be empty"})"
        "\n"
        R"({"seq":2,"subsystem":"plugin_manager","event":"compat_failure","plugin":"BrokenEvalFixture","command":"brokenEval","operation":"load_plugin_js_eval","message":"fixture eval failure"})"
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
            REQUIRE(event.severity == CompatEvent::Severity::ERROR);
            REQUIRE(event.navigationTarget == "plugin://BrokenEntryFixture#brokenCommand");
        }
        if (event.pluginId == "BrokenEvalFixture") {
            foundEvalFailure = true;
            REQUIRE(event.methodName == "load_plugin_js_eval");
            REQUIRE(event.message == "fixture eval failure");
            REQUIRE(event.navigationTarget == "plugin://BrokenEvalFixture#brokenEval");
        }
    }
    REQUIRE(foundEntryFailure);
    REQUIRE(foundEvalFailure);

    const auto entrySummary = model.getPluginSummary("BrokenEntryFixture");
    REQUIRE(entrySummary.unsupportedCount == 1);
    REQUIRE(entrySummary.errorCount == 1);

    const auto evalSummary = model.getPluginSummary("BrokenEvalFixture");
    REQUIRE(evalSummary.unsupportedCount == 1);
    REQUIRE(evalSummary.errorCount == 1);

    const auto exported = model.exportAsJson();
    REQUIRE(exported.find("BrokenEntryFixture") != std::string::npos);
    REQUIRE(exported.find("BrokenEvalFixture") != std::string::npos);
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
    
    model.setNavigationHandler([&lastTarget](const std::string& target) {
        lastTarget = target;
    });
    
    SECTION("Navigate to target") {
        model.navigateTo("event://123");
        REQUIRE(lastTarget == "event://123");
    }
    
    SECTION("Empty handler does nothing") {
        CompatReportModel modelNoHandler;
        modelNoHandler.navigateTo("test");  // Should not crash
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
        REQUIRE(events.size() == 2);  // WARNING + ERROR
    }
}

TEST_CASE("CompatReportView - Export", "[compat][panel]") {
    CompatReportModel model;
    model.recordCall("plugin1", "Class", "method", CompatStatus::FULL);
    
    CompatReportView view(model);
    
    SECTION("Export to JSON file") {
        view.exportReport("json", "test_export.json");
        // File should be created (basic smoke test)
    }
    
    SECTION("Export to CSV file") {
        view.exportReport("csv", "test_export.csv");
        // File should be created (basic smoke test)
    }
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
