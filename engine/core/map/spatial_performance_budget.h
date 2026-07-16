#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace urpg::map {
struct SpatialPerformanceBudget {
    uint64_t max_cells=262144; size_t max_layers=128; size_t max_events=10000;
    uint64_t max_undo_bytes=256ull*1024ull*1024ull; uint64_t max_snapshot_ms=250;
};
struct SpatialPerformanceInput {
    uint64_t cells=0; size_t layers=0; size_t events=0; uint64_t undo_bytes=0; uint64_t snapshot_ms=0;
};
struct SpatialPerformanceDiagnostic { std::string severity, code, message; uint64_t measured=0, budget=0; };
struct SpatialPerformanceReport {
    bool within_budget=true; bool render_event_labels=true; bool live_navigation_preview=true;
    size_t snapshot_frequency_divisor=1; std::vector<SpatialPerformanceDiagnostic> diagnostics;
};
SpatialPerformanceReport evaluateSpatialPerformance(const SpatialPerformanceInput& input,
                                                    const SpatialPerformanceBudget& budget = {});
} // namespace urpg::map
