#include <catch2/catch_test_macros.hpp>
#include "editor/perf/perf_diagnostics_panel.h"
#include "editor/perf/perf_diagnostics_model.h"
#include "engine/core/perf/perf_profiler.h"

using namespace urpg::editor;
using namespace urpg::perf;

TEST_CASE("PerfDiagnosticsPanel: snapshot is empty when no model is bound", "[perf][editor][diagnostics]") {
    PerfDiagnosticsPanel panel;
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().empty());
}

TEST_CASE("PerfDiagnosticsPanel: snapshot reflects profiler data when model is bound and profiler has frames",
          "[perf][editor][diagnostics]") {
    PerfProfiler profiler;
    PerfDiagnosticsModel model;
    PerfDiagnosticsPanel panel;

    model.attachProfiler(&profiler);
    panel.bindModel(&model);

    profiler.beginFrame();
    profiler.endFrame();

    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.contains("frame_summary"));
    REQUIRE(snapshot.contains("sections"));
    REQUIRE(snapshot["frame_summary"].contains("lastFrameUs"));
}

TEST_CASE("PerfDiagnosticsPanel: model detach returns empty snapshot", "[perf][editor][diagnostics]") {
    PerfProfiler profiler;
    PerfDiagnosticsModel model;
    PerfDiagnosticsPanel panel;

    model.attachProfiler(&profiler);
    panel.bindModel(&model);
    panel.render();

    REQUIRE_FALSE(panel.lastRenderSnapshot().empty());

    model.detachProfiler();
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().empty());
}
