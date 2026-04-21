#include "engine/core/localization/completeness_checker.h"
#include "engine/core/localization/locale_catalog.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Missing keys are detected when target lacks master keys", "[localization][completeness]") {
    urpg::localization::LocaleCatalog master;
    master.loadFromJson(nlohmann::json::parse(R"({
        "locale": "en",
        "keys": { "A": "1", "B": "2", "C": "3" }
    })"));

    urpg::localization::LocaleCatalog target;
    target.loadFromJson(nlohmann::json::parse(R"({
        "locale": "ja",
        "keys": { "A": "1", "C": "3" }
    })"));

    urpg::localization::CompletenessChecker checker;
    checker.setMasterCatalog(master);

    const auto missing = checker.checkAgainst(target);
    REQUIRE(missing.size() == 1);
    REQUIRE(missing[0] == "B");
}

TEST_CASE("Extra keys are detected when target has keys master lacks", "[localization][completeness]") {
    urpg::localization::LocaleCatalog master;
    master.loadFromJson(nlohmann::json::parse(R"({
        "locale": "en",
        "keys": { "A": "1", "B": "2" }
    })"));

    urpg::localization::LocaleCatalog target;
    target.loadFromJson(nlohmann::json::parse(R"({
        "locale": "ja",
        "keys": { "A": "1", "B": "2", "C": "3", "D": "4" }
    })"));

    urpg::localization::CompletenessChecker checker;
    checker.setMasterCatalog(master);

    std::vector<std::string> missing;
    std::vector<std::string> extra;
    checker.checkAgainst(target, missing, extra);

    REQUIRE(missing.empty());
    REQUIRE(extra.size() == 2);
    REQUIRE(extra[0] == "C");
    REQUIRE(extra[1] == "D");
}

TEST_CASE("Coverage percent is 100 when identical, 0 when empty, 50 when half", "[localization][completeness]") {
    urpg::localization::LocaleCatalog master;
    master.loadFromJson(nlohmann::json::parse(R"({
        "locale": "en",
        "keys": { "A": "1", "B": "2" }
    })"));

    urpg::localization::CompletenessChecker checker;
    checker.setMasterCatalog(master);

    urpg::localization::LocaleCatalog identical;
    identical.loadFromJson(nlohmann::json::parse(R"({
        "locale": "fr",
        "keys": { "A": "1", "B": "2" }
    })"));
    REQUIRE(checker.getCoveragePercent(identical) == 100.0f);

    urpg::localization::LocaleCatalog empty;
    empty.loadFromJson(nlohmann::json::parse(R"({
        "locale": "de",
        "keys": {}
    })"));
    REQUIRE(checker.getCoveragePercent(empty) == 0.0f);

    urpg::localization::LocaleCatalog half;
    half.loadFromJson(nlohmann::json::parse(R"({
        "locale": "es",
        "keys": { "A": "1" }
    })"));
    REQUIRE(checker.getCoveragePercent(half) == 50.0f);
}
