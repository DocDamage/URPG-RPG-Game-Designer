#include "engine/core/map/grid_part_prefab_lifecycle.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Smart prefab update preserves overrides applies atomically and detaches", "[grid_part][prefab][lifecycle]") {
    using namespace urpg::map;
    GridPartCatalog catalog;
    GridPartDefinition wall; wall.part_id="wall"; wall.footprint={1,1,false,true}; wall.default_properties={{"material","wood"}};
    GridPartDefinition door; door.part_id="door"; door.footprint={1,1,false,true};
    REQUIRE(catalog.addDefinition(wall)); REQUIRE(catalog.addDefinition(door));
    GridPartDocument document("map.prefab", 10, 10);
    PlacedPartInstance existing; existing.instance_id="map.prefab:smart_prefab:house:wall:2:2"; existing.part_id="wall";
    existing.grid_x=2; existing.grid_y=2;
    existing.properties={{"smart_prefab.id","house"},{"smart_prefab.version","1"},{"smart_prefab.operation_id","wall"},
                         {"smart_prefab.anchor","2,2"},{"smart_prefab.group_id","map.prefab:smart_prefab:house:2:2"},
                         {"material","stone"},{"smart_prefab.override.material","stone"}};
    REQUIRE(document.placePart(existing));
    GridPartSmartPrefab next; next.prefab_id="house"; next.version="2"; next.dependencies={"asset:house.png"};
    next.operations={{"wall","wall",1,0,0,{{"material","brick"}}},{"door","door",0,1,0,{}}};
    const auto preview = previewGridPartPrefabUpdate(document,catalog,next);
    REQUIRE(preview.valid); REQUIRE(preview.affected_instance_count == 2); REQUIRE(preview.preserved_override_count == 1);
    REQUIRE(preview.package_closure == std::vector<std::string>{"asset:house.png","part:door","part:wall"});
    GridPartCommandHistory history;
    REQUIRE(applyGridPartPrefabUpdate(document,history,preview));
    REQUIRE(document.parts().size() == 2);
    REQUIRE(document.findPart(existing.instance_id)->grid_x == 3);
    REQUIRE(document.findPart(existing.instance_id)->properties.at("material") == "stone");
    REQUIRE(document.findPart(existing.instance_id)->properties.at("smart_prefab.version") == "2");
    REQUIRE(history.undo(document)); REQUIRE(document.parts().size() == 1);
    REQUIRE(history.redo(document)); REQUIRE(document.parts().size() == 2);
    REQUIRE(detachGridPartPrefabGroup(document,history,"map.prefab:smart_prefab:house:2:2"));
    for (const auto& instance : document.parts()) REQUIRE_FALSE(instance.properties.contains("smart_prefab.id"));
    REQUIRE(history.undo(document));
    REQUIRE(document.parts()[0].properties.contains("smart_prefab.id"));
}
