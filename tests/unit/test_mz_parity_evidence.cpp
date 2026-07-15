#include "engine/core/compat/mz_parity_evidence.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("MZ parity evidence aggregates reference captures and backend observations",
          "[compat][mz_parity_evidence]") {
    using namespace urpg::compat;

    std::vector<MzReferenceCaptureEvidence> references = {
        {
            "minimal_jrpg_title_capture",
            "minimal_jrpg_project",
            "title_screen",
            "mz_reference_headless",
            "7c6e2a5b9f1d4c30",
            816,
            624,
            "repo_owned",
            false,
        },
        {
            "two_map_event_capture",
            "two_map_event_project",
            "town_to_interior_transfer",
            "mz_reference_headless",
            "94e8b2c71a5f03dd",
            816,
            624,
            "repo_owned",
            false,
        },
    };
    std::vector<MzBackendObservationEvidence> observations = {
        {
            "minimal_title_urpg_headless",
            "minimal_jrpg_project",
            "title_screen",
            "urpg_headless",
            "7c6e2a5b9f1d4c30",
            816,
            624,
        },
        {
            "two_map_urpg_opengl",
            "two_map_event_project",
            "town_to_interior_transfer",
            "urpg_opengl",
            "1111222233334444",
            816,
            624,
        },
    };

    const auto report = BuildMzParityEvidenceReport(std::move(references), std::move(observations));
    const auto json = report.toJson();

    REQUIRE(report.reference_count == 2);
    REQUIRE(report.backend_count == 2);
    REQUIRE(report.failed_comparison_count == 1);
    REQUIRE(report.release_authoritative == false);
    REQUIRE(json["release_authoritative"] == false);
    REQUIRE(json["comparisons"].size() == 2);
    REQUIRE(json["comparisons"][0]["status"] == "matched");
    REQUIRE(json["comparisons"][1]["status"] == "delta");
    REQUIRE(json["comparisons"][1]["observed_backend"] == "urpg_opengl");
}
