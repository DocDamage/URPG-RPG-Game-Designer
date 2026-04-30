#include "engine/core/map/grid_part_serializer.h"

#include <catch2/catch_test_macros.hpp>

#include <nlohmann/json.hpp>

#include <string>
#include <utility>

using namespace urpg::map;

namespace {

PlacedPartInstance makePart(std::string instanceId, std::string partId, int32_t x, int32_t y) {
    PlacedPartInstance part;
    part.instance_id = std::move(instanceId);
    part.part_id = std::move(partId);
    part.category = GridPartCategory::Prop;
    part.layer = GridPartLayer::Object;
    part.grid_x = x;
    part.grid_y = y;
    part.grid_z = 1;
    part.width = 2;
    part.height = 1;
    part.rot_y = 90.0f;
    part.scale = 1.25f;
    part.locked = true;
    part.hidden = false;
    part.properties["zeta"] = "last";
    part.properties["alpha"] = "first";
    return part;
}

} // namespace

TEST_CASE("Grid part serializer writes deterministic authoring JSON", "[grid_part][serializer]") {
    GridPartDocument document("map001", 8, 6, 4);
    REQUIRE(document.placePart(makePart("map001:prop.b:2:2", "prop.b", 2, 2)));
    REQUIRE(document.placePart(makePart("map001:prop.a:1:1", "prop.a", 1, 1)));

    const auto json = GridPartDocumentToJson(document);

    REQUIRE(json.at("schemaVersion") == 1);
    REQUIRE(json.at("mapId") == "map001");
    REQUIRE(json.at("width") == 8);
    REQUIRE(json.at("height") == 6);
    REQUIRE(json.at("chunkSize") == 4);
    REQUIRE(json.at("parts").size() == 2);
    REQUIRE(json.at("parts")[0].at("instanceId") == "map001:prop.a:1:1");
    REQUIRE(json.at("parts")[1].at("instanceId") == "map001:prop.b:2:2");
    REQUIRE(json.at("parts")[0].at("properties").begin().key() == "alpha");
}

TEST_CASE("Grid part serializer round-trips a document", "[grid_part][serializer]") {
    GridPartDocument document("map001", 8, 6, 4);
    REQUIRE(document.placePart(makePart("map001:prop.a:1:1", "prop.a", 1, 1)));

    const auto parsed = GridPartDocumentFromJson(GridPartDocumentToJson(document));

    REQUIRE(parsed.has_value());
    REQUIRE(parsed->mapId() == "map001");
    REQUIRE(parsed->width() == 8);
    REQUIRE(parsed->height() == 6);
    REQUIRE(parsed->chunkSize() == 4);
    REQUIRE(parsed->parts().size() == 1);
    const auto* part = parsed->findPart("map001:prop.a:1:1");
    REQUIRE(part != nullptr);
    REQUIRE(part->part_id == "prop.a");
    REQUIRE(part->grid_x == 1);
    REQUIRE(part->grid_y == 1);
    REQUIRE(part->grid_z == 1);
    REQUIRE(part->width == 2);
    REQUIRE(part->height == 1);
    REQUIRE(part->rot_y == 90.0f);
    REQUIRE(part->scale == 1.25f);
    REQUIRE(part->locked);
    REQUIRE(part->properties.at("alpha") == "first");
    REQUIRE(parsed->dirtyChunks().empty());
}

TEST_CASE("Grid part serializer rejects invalid persisted payloads", "[grid_part][serializer]") {
    nlohmann::json json = {
        {"schemaVersion", 1},
        {"mapId", "map001"},
        {"width", 4},
        {"height", 4},
        {"parts", nlohmann::json::array({
                      {{"instanceId", "map001:prop.crate:9:9"},
                       {"partId", "prop.crate"},
                       {"category", "Prop"},
                       {"layer", "Object"},
                       {"x", 9},
                       {"y", 9},
                       {"width", 1},
                       {"height", 1}},
                  })},
    };

    REQUIRE_FALSE(GridPartDocumentFromJson(json).has_value());

    json["schemaVersion"] = 99;
    REQUIRE_FALSE(GridPartDocumentFromJson(json).has_value());

    json["schemaVersion"] = 1;
    json["parts"][0]["category"] = "DefinitelyNotACategory";
    json["parts"][0]["x"] = 1;
    json["parts"][0]["y"] = 1;
    REQUIRE_FALSE(GridPartDocumentFromJson(json).has_value());
}
