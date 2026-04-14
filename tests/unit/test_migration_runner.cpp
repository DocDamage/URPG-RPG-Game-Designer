#include "engine/core/migrate/migration_runner.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

TEST_CASE("Migration runner applies rename and set operations", "[migrate]") {
    nlohmann::json doc = {
        {"_urpg_format_version", "1.0"},
        {"player", {{"atk", 10}}}
    };

    nlohmann::json spec = {
        {"from", "1.0"},
        {"to", "1.1"},
        {"ops", nlohmann::json::array({
            {
                {"op", "rename"},
                {"fromPath", "/player/atk"},
                {"toPath", "/player/attack"}
            },
            {
                {"op", "set"},
                {"path", "/_engine_version_min"},
                {"value", "0.9.0"}
            }
        })}
    };

    const auto error = urpg::MigrationRunner::Apply(spec, doc);
    REQUIRE_FALSE(error.has_value());
    REQUIRE(doc["_urpg_format_version"] == "1.1");
    REQUIRE(doc["player"]["attack"] == 10);
    REQUIRE_FALSE(doc["player"].contains("atk"));
    REQUIRE(doc["_engine_version_min"] == "0.9.0");
}

TEST_CASE("Migration runner reports version mismatch", "[migrate]") {
    nlohmann::json doc = {
        {"_urpg_format_version", "2.0"}
    };

    nlohmann::json spec = {
        {"from", "1.0"},
        {"to", "1.1"},
        {"ops", nlohmann::json::array()}
    };

    const auto error = urpg::MigrationRunner::Apply(spec, doc);
    REQUIRE(error.has_value());
    REQUIRE(error->code == urpg::MigrationErrorCode::VersionMismatch);
}

TEST_CASE("Migration runner upgrades imported save metadata into URPG runtime shape", "[migrate][save]") {
    nlohmann::json doc = {
        {"_urpg_format_version", "mz_compat_1"},
        {"meta", {
            {"slotId", 5},
            {"mapName", "Ruined Archive"},
            {"playtimeSeconds", 1234},
            {"saveVersion", "mz-import"}
        }},
        {"pluginHeader", {
            {"uiTab", "party"},
            {"thumbnailHash", "thumb123"}
        }}
    };

    nlohmann::json spec = {
        {"from", "mz_compat_1"},
        {"to", "1.0"},
        {"ops", nlohmann::json::array({
            {{"op", "rename"}, {"fromPath", "/meta/slotId"}, {"toPath", "/meta/_slot_id"}},
            {{"op", "rename"}, {"fromPath", "/meta/mapName"}, {"toPath", "/meta/_map_display_name"}},
            {{"op", "rename"}, {"fromPath", "/meta/playtimeSeconds"}, {"toPath", "/meta/_playtime_seconds"}},
            {{"op", "rename"}, {"fromPath", "/meta/saveVersion"}, {"toPath", "/meta/_save_version"}},
            {{"op", "rename"}, {"fromPath", "/pluginHeader/thumbnailHash"}, {"toPath", "/meta/_thumbnail_hash"}},
            {{"op", "rename"}, {"fromPath", "/pluginHeader/uiTab"}, {"toPath", "/meta/_ui_tab"}}
        })}
    };

    const auto error = urpg::MigrationRunner::Apply(spec, doc);
    REQUIRE_FALSE(error.has_value());
    REQUIRE(doc["_urpg_format_version"] == "1.0");
    REQUIRE(doc["meta"]["_slot_id"] == 5);
    REQUIRE(doc["meta"]["_map_display_name"] == "Ruined Archive");
    REQUIRE(doc["meta"]["_playtime_seconds"] == 1234);
    REQUIRE(doc["meta"]["_save_version"] == "mz-import");
    REQUIRE(doc["meta"]["_thumbnail_hash"] == "thumb123");
    REQUIRE(doc["meta"]["_ui_tab"] == "party");
    REQUIRE_FALSE(doc["meta"].contains("slotId"));
    REQUIRE_FALSE(doc["pluginHeader"].contains("uiTab"));
}
