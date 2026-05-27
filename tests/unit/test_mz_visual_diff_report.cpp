#include "engine/core/compat/mz_visual_diff_report.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MZ visual diff report records scene comparison pass and fail rows",
          "[compat][mz_visual_diff]") {
    urpg::compat::MzVisualDiffReport report;
    report.addComparison({"menu_status", "ref_hash", "urpg_hash", 0.25});
    report.addComparison({"battle_hud", "ref_battle", "urpg_battle", 3.75});

    REQUIRE(report.scene_count == 2);
    REQUIRE(report.passed_scene_count == 1);
    REQUIRE(report.failed_scene_count == 1);
    REQUIRE(report.rows[0].scene_id == "battle_hud");
    REQUIRE(report.rows[0].status == urpg::compat::MzVisualDiffStatus::Failed);
    REQUIRE(report.rows[1].status == urpg::compat::MzVisualDiffStatus::Passed);
    REQUIRE(report.toJson()["release_authoritative"] == false);
    REQUIRE(report.toJson()["rows"][0]["status"] == "failed");
    REQUIRE(report.toJson()["rows"][1]["pixel_delta_percent"] == 0.25);
}

TEST_CASE("MZ visual diff report stays non-authoritative without captured references",
          "[compat][mz_visual_diff]") {
    const auto report = urpg::compat::BuildEmptyMzVisualDiffReport();

    REQUIRE(report.release_authoritative == false);
    REQUIRE(report.scene_count == 0);
    REQUIRE(report.toJson()["rows"].empty());
}
