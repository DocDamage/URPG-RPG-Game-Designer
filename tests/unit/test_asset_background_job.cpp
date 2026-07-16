#include "engine/core/assets/asset_background_job.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

using namespace urpg::assets;

TEST_CASE("Asset background jobs cancel and restart at every durable stage", "[assets][background_job]") {
    const std::vector stages{AssetJobStage::Scan, AssetJobStage::Import, AssetJobStage::Hash,
                             AssetJobStage::Thumbnail, AssetJobStage::Transform,
                             AssetJobStage::AudioAnalysis, AssetJobStage::Package};
    for (std::size_t target = 0; target < stages.size(); ++target) {
        AssetBackgroundJob job("job-" + std::to_string(target), "asset_pipeline", stages);
        REQUIRE(job.start());
        for (std::size_t stage = 0; stage < target; ++stage) REQUIRE(job.completeStage());
        REQUIRE(job.reportProgress(0.5, "Halfway through stage."));
        const auto progress = job.snapshot().overall_progress;
        REQUIRE(job.requestCancel());
        REQUIRE(job.snapshot().state == AssetJobState::Cancelled);
        REQUIRE_FALSE(job.reportProgress(0.75));
        REQUIRE(job.resume());
        REQUIRE(job.snapshot().attempt == 2);
        REQUIRE(job.snapshot().overall_progress == progress);
        REQUIRE(job.reportProgress(0.75));
        REQUIRE(job.completeStage());
    }
}

TEST_CASE("Asset background job checkpoints recover interruption and retry failures", "[assets][background_job][recovery]") {
    const auto root = std::filesystem::temp_directory_path() /
        ("urpg_asset_job_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto checkpoint = root / "job.json";
    AssetBackgroundJob job("job-durable", "thumbnail_pipeline", {AssetJobStage::Hash, AssetJobStage::Thumbnail});
    REQUIRE(job.start());
    REQUIRE(job.reportProgress(0.4, "Hashing chunk 4."));
    REQUIRE(job.saveCheckpoint(checkpoint));

    auto recovered = AssetBackgroundJob::loadCheckpoint(checkpoint);
    REQUIRE(recovered.has_value());
    REQUIRE(recovered->snapshot().state == AssetJobState::Paused);
    REQUIRE(recovered->snapshot().recovered_interruption);
    REQUIRE(recovered->snapshot().stage_progress == 0.4);
    REQUIRE(recovered->resume());
    REQUIRE(recovered->completeStage("Hash complete."));
    REQUIRE(recovered->fail("Decoder unavailable."));
    REQUIRE(recovered->snapshot().state == AssetJobState::Failed);
    REQUIRE(recovered->retry());
    REQUIRE(recovered->snapshot().attempt == 3);
    REQUIRE(recovered->completeStage("Thumbnail complete."));
    REQUIRE(recovered->snapshot().state == AssetJobState::Completed);
    REQUIRE(recovered->snapshot().overall_progress == 1.0);
    REQUIRE(recovered->saveCheckpoint(checkpoint));
    std::filesystem::remove_all(root);
}
