#include "engine/core/map/spatial_performance_budget.h"
#include <algorithm>
namespace urpg::map {
SpatialPerformanceReport evaluateSpatialPerformance(const SpatialPerformanceInput& input,const SpatialPerformanceBudget& budget){
    SpatialPerformanceReport report; const auto add=[&](std::string code,std::string message,uint64_t measured,uint64_t limit){report.within_budget=false;report.diagnostics.push_back({"warning",std::move(code),std::move(message),measured,limit});};
    if(input.cells>budget.max_cells){add("spatial_cell_budget_exceeded","Map cell count exceeds the interactive budget.",input.cells,budget.max_cells);report.live_navigation_preview=false;}
    if(input.layers>budget.max_layers){add("spatial_layer_budget_exceeded","Layer count exceeds the interactive budget.",input.layers,budget.max_layers);report.snapshot_frequency_divisor=4;}
    if(input.events>budget.max_events){add("spatial_event_budget_exceeded","Event count exceeds the interactive budget.",input.events,budget.max_events);report.render_event_labels=false;}
    if(input.undo_bytes>budget.max_undo_bytes){add("spatial_undo_budget_exceeded","Undo memory exceeds the configured budget.",input.undo_bytes,budget.max_undo_bytes);report.snapshot_frequency_divisor=8;}
    if(input.snapshot_ms>budget.max_snapshot_ms){add("spatial_snapshot_budget_exceeded","Spatial snapshot latency exceeds the interactive budget.",input.snapshot_ms,budget.max_snapshot_ms);report.live_navigation_preview=false;report.snapshot_frequency_divisor=std::max<size_t>(report.snapshot_frequency_divisor,4);}
    return report;
}
} // namespace urpg::map
