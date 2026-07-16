#include "editor/spatial/spatial_authoring_workspace.h"
#include "engine/core/map/spatial_performance_budget.h"
#include "engine/core/presentation/presentation_schema.h"
#include "engine/core/scene/map_scene.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_message.hpp>
#include <chrono>

TEST_CASE("Large spatial document records budgets and degrades expensive helpers gracefully", "[spatial][perf][stress]") {
    using Workspace=urpg::editor::SpatialAuthoringWorkspace; using Clock=std::chrono::steady_clock;
    urpg::presentation::SpatialMapOverlay overlay; overlay.mapId="stress";overlay.elevation.width=64;overlay.elevation.height=64;overlay.elevation.levels.assign(4096,0);
    urpg::scene::MapScene map("stress",64,64);Workspace workspace;workspace.SetTargets(&map,&overlay);
    REQUIRE(workspace.AddPerspectiveLayer("ground","Ground","tile")); REQUIRE(workspace.AddPerspectiveLayer("events","Events","event"));
    for(int index=0;index<46;++index) REQUIRE(workspace.AddPerspectiveLayer("detail_"+std::to_string(index),"Detail","tile"));
    workspace.SetPerspectiveTilePaletteOptions({{"grass","Grass","town","grass","","","terrain","grass.png"}});
    std::vector<Workspace::Perspective2DNativeTileEdit> tiles;tiles.reserve(4096);
    for(int y=0;y<64;++y)for(int x=0;x<64;++x)tiles.push_back({"ground","town","grass",x,y});
    REQUIRE(workspace.applyNativeTileEdits(workspace.perspectiveDocumentRevision(),tiles).success);
    std::vector<Workspace::Perspective2DNativeEventMessageEdit> events;events.reserve(512);
    for(int index=0;index<512;++index)events.push_back({"event_"+std::to_string(index),"events","Event","Stress",index%64,index/64});
    REQUIRE(workspace.applyNativeEventMessageEdits(workspace.perspectiveDocumentRevision(),events).success);
    const auto started=Clock::now();const auto saved=workspace.PreparePerspectiveMapDraftSave();const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-started).count();
    REQUIRE(saved.success);REQUIRE(saved.layer_count==48);REQUIRE(saved.painted_tile_count==4096);REQUIRE(saved.event_count==512);REQUIRE(elapsed<15000);
    const auto report=urpg::map::evaluateSpatialPerformance({4096,48,512,static_cast<uint64_t>(saved.serialized_document_json.size())*64,static_cast<uint64_t>(elapsed)});
    UNSCOPED_INFO("snapshot_ms="<<elapsed<<" serialized_bytes="<<saved.serialized_document_json.size());
    if(!report.within_budget) REQUIRE(report.snapshot_frequency_divisor>1);
    const auto overloaded=urpg::map::evaluateSpatialPerformance({300000,200,12000,300ull*1024ull*1024ull,1000});
    REQUIRE_FALSE(overloaded.within_budget);REQUIRE_FALSE(overloaded.render_event_labels);REQUIRE_FALSE(overloaded.live_navigation_preview);REQUIRE(overloaded.snapshot_frequency_divisor==8);REQUIRE(overloaded.diagnostics.size()==5);
}
