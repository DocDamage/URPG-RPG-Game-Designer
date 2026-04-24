#include "engine/core/assets/asset_cleanup_planner.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("AssetCleanupPlanner refuses to delete referenced duplicate files", "[assets][asset_library]") {
    urpg::assets::AssetLibrary library;
    library.ingestDuplicateCsv(
        "sha256,size_bytes,path_rel,recommended_keep,recommended_remove\n"
        "aaa,10,content/hero.png,imports/hero.png,yes\n"
        "aaa,10,imports/hero.png,imports/hero.png,no\n"
        "bbb,20,imports/unused-copy.png,imports/unused.png,yes\n"
        "bbb,20,imports/unused.png,imports/unused.png,no\n");
    library.addReferencedAsset("content/hero.png");

    urpg::assets::AssetCleanupPlanner planner;
    const auto plan = planner.buildDuplicateCleanupPlan(library);

    REQUIRE(plan.actions.size() == 2);
    REQUIRE(plan.allowed_count == 1);
    REQUIRE(plan.refused_count == 1);
    REQUIRE(plan.actions[0].candidate_path == "content/hero.png");
    REQUIRE_FALSE(plan.actions[0].allowed);
    REQUIRE(plan.actions[0].reason == "candidate_is_referenced");
    REQUIRE(plan.actions[1].candidate_path == "imports/unused-copy.png");
    REQUIRE(plan.actions[1].allowed);
}
