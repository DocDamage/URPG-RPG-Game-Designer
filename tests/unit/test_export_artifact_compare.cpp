#include "engine/core/export/export_artifact_compare.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("export artifact compare reports changed schemas assets missing files and signatures", "[export][artifact_compare][ffs07]") {
    const nlohmann::json baseline = {
        {"bundleSignature", "sig-a"},
        {"entries", {
            {{"path", "data/schemas/save.schema.json"}, {"integrityTag", "schema-a"}, {"rawSize", 10}},
            {{"path", "assets/hero.png"}, {"integrityTag", "hero-a"}, {"rawSize", 20}},
            {{"path", "assets/missing.png"}, {"integrityTag", "missing-a"}, {"rawSize", 30}},
        }},
    };
    const nlohmann::json candidate = {
        {"bundleSignature", "sig-b"},
        {"entries", {
            {{"path", "data/schemas/save.schema.json"}, {"integrityTag", "schema-b"}, {"rawSize", 10}},
            {{"path", "assets/hero.png"}, {"integrityTag", "hero-b"}, {"rawSize", 20}},
        }},
    };

    const auto result = urpg::exporting::CompareExportArtifacts(baseline, candidate);

    REQUIRE(result.changed_schemas == std::vector<std::string>{"data/schemas/save.schema.json"});
    REQUIRE(result.changed_assets == std::vector<std::string>{"assets/hero.png"});
    REQUIRE(result.missing_files == std::vector<std::string>{"assets/missing.png"});
    REQUIRE(result.signature_changed);
    REQUIRE(result.manifest_changed);
}
