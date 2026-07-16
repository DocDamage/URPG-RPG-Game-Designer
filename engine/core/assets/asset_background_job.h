#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <vector>

namespace urpg::assets {

enum class AssetJobStage { Scan, Import, Hash, Thumbnail, Transform, AudioAnalysis, Package };
enum class AssetJobState { Pending, Running, Paused, Cancelled, Failed, Completed };

struct AssetJobLogEntry {
    unsigned long long sequence = 0;
    AssetJobStage stage = AssetJobStage::Scan;
    std::string message;
};

struct AssetBackgroundJobSnapshot {
    std::string job_id;
    std::string kind;
    AssetJobState state = AssetJobState::Pending;
    std::vector<AssetJobStage> stages;
    std::size_t stage_index = 0;
    double stage_progress = 0.0;
    double overall_progress = 0.0;
    unsigned int attempt = 0;
    bool cancel_requested = false;
    bool recovered_interruption = false;
    std::string failure;
    std::vector<AssetJobLogEntry> logs;
};

class AssetBackgroundJob {
public:
    AssetBackgroundJob() = default;
    AssetBackgroundJob(std::string job_id, std::string kind, std::vector<AssetJobStage> stages);

    bool start();
    bool reportProgress(double progress, std::string message = {});
    bool completeStage(std::string message = {});
    bool requestCancel(std::string message = "Cancellation requested.");
    bool resume();
    bool fail(std::string failure);
    bool retry();

    bool saveCheckpoint(const std::filesystem::path& path, std::string* error = nullptr) const;
    static std::optional<AssetBackgroundJob> loadCheckpoint(const std::filesystem::path& path,
                                                            std::string* error = nullptr);
    const AssetBackgroundJobSnapshot& snapshot() const { return snapshot_; }

private:
    void appendLog(std::string message);
    void updateOverallProgress();

    AssetBackgroundJobSnapshot snapshot_;
    unsigned long long next_log_sequence_ = 1;
};

const char* assetJobStageName(AssetJobStage stage);
const char* assetJobStateName(AssetJobState state);

} // namespace urpg::assets
