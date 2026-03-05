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
