#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {

std::filesystem::path repoRoot() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

nlohmann::json loadJson(const std::filesystem::path& path) {
    std::ifstream input(path);
    REQUIRE(input.is_open());
    auto json = nlohmann::json::parse(input, nullptr, false);
    REQUIRE_FALSE(json.is_discarded());
    return json;
}

void requireRequiredField(const nlohmann::json& schema, const std::string& field) {
    REQUIRE(schema.at("required").is_array());
    REQUIRE(std::find(schema.at("required").begin(), schema.at("required").end(), field) != schema.at("required").end());
    REQUIRE(schema.at("properties").contains(field));
}

} // namespace

TEST_CASE("MZ project corpus schema and minimal descriptor expose legal compatibility evidence contract",
          "[compat][mz_project_corpus]") {
    const auto root = repoRoot();
    REQUIRE_FALSE(root.empty());

    const auto schema = loadJson(root / "content" / "compat" / "mz_project_corpus.schema.json");
    REQUIRE(schema.at("$id") == "https://urpg.dev/schemas/mz_project_corpus.schema.json");
    REQUIRE(schema.at("title") == "URPG RPG Maker MZ Project Corpus Descriptor");

    for (const std::string field : {
             "schemaVersion",
             "projectId",
             "sourceLicense",
             "legalUse",
             "fixtureScope",
             "maps",
             "events",
             "plugins",
             "saves",
             "assets",
             "expectedCoverage",
         }) {
        requireRequiredField(schema, field);
    }

    const auto legalUseEnum = schema.at("properties").at("legalUse").at("enum");
    REQUIRE(std::find(legalUseEnum.begin(), legalUseEnum.end(), "repo_owned") != legalUseEnum.end());
    REQUIRE(std::find(legalUseEnum.begin(), legalUseEnum.end(), "permissive_sample") != legalUseEnum.end());
    REQUIRE(std::find(legalUseEnum.begin(), legalUseEnum.end(), "owner_provided_private") != legalUseEnum.end());

    const auto fixture =
        loadJson(root / "imports" / "fixtures" / "compat" / "mz_projects" / "minimal_jrpg_project.json");
    REQUIRE(fixture.at("schemaVersion") == "1.0.0");
    REQUIRE(fixture.at("projectId") == "minimal_jrpg_project");
    REQUIRE(fixture.at("sourceLicense") == "repo_owned");
    REQUIRE(fixture.at("legalUse") == "repo_owned");
    REQUIRE(fixture.at("fixtureScope").at("copyrightedRpgMakerPayloadsIncluded") == false);
    REQUIRE(fixture.at("maps").at("count") == 1);
    REQUIRE(fixture.at("events").at("supportedCommandCount") == 10);
    REQUIRE(fixture.at("plugins").at("fixtureManifestDirectory") == "tests/compat/fixtures/plugins");
    REQUIRE(fixture.at("expectedCoverage").at("runtimeParityClaim") == false);
}

TEST_CASE("MZ reference capture descriptors expose legal non-authoritative parity evidence",
          "[compat][mz_project_corpus]") {
    const auto root = repoRoot();
    REQUIRE_FALSE(root.empty());

    const auto schema = loadJson(root / "content" / "compat" / "mz_reference_captures.schema.json");
    REQUIRE(schema.at("$id") == "https://urpg.dev/schemas/mz_reference_captures.schema.json");
    REQUIRE(schema.at("title") == "URPG RPG Maker MZ Reference Capture Descriptor");

    for (const std::string field : {
             "schemaVersion",
             "captureId",
             "projectId",
             "sceneId",
             "backend",
             "frameHash",
             "dimensions",
             "legalUse",
             "source",
             "releaseAuthoritative",
         }) {
        requireRequiredField(schema, field);
    }

    const auto twoMapProject =
        loadJson(root / "imports" / "fixtures" / "compat" / "mz_projects" / "two_map_event_project.json");
    REQUIRE(twoMapProject.at("schemaVersion") == "1.0.0");
    REQUIRE(twoMapProject.at("projectId") == "two_map_event_project");
    REQUIRE(twoMapProject.at("fixtureScope").at("copyrightedRpgMakerPayloadsIncluded") == false);
    REQUIRE(twoMapProject.at("maps").at("count") == 2);
    REQUIRE(twoMapProject.at("events").at("supportedCommandCount") >= 16);
    REQUIRE(twoMapProject.at("expectedCoverage").at("runtimeParityClaim") == false);
    REQUIRE(twoMapProject.at("expectedCoverage").at("visualParityClaim") == false);

    for (const auto& name : {"minimal_jrpg_title_capture.json", "two_map_event_capture.json"}) {
        const auto capture = loadJson(root / "imports" / "fixtures" / "compat" / "mz_references" / name);
        REQUIRE(capture.at("schemaVersion") == "1.0.0");
        REQUIRE(capture.at("backend") == "mz_reference_headless");
        REQUIRE(capture.at("legalUse") == "repo_owned");
        REQUIRE(capture.at("releaseAuthoritative") == false);
        REQUIRE(capture.at("source").at("copyrightedRpgMakerPayloadsIncluded") == false);
        REQUIRE(capture.at("dimensions").at("width") == 816);
        REQUIRE(capture.at("dimensions").at("height") == 624);
    }
}
