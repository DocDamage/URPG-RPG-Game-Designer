#include "engine/core/assets/asset_background_job.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::assets {
namespace {

std::optional<AssetJobStage> parseStage(const std::string& value) {
    if (value == "scan") return AssetJobStage::Scan;
    if (value == "import") return AssetJobStage::Import;
    if (value == "hash") return AssetJobStage::Hash;
    if (value == "thumbnail") return AssetJobStage::Thumbnail;
    if (value == "transform") return AssetJobStage::Transform;
    if (value == "audio_analysis") return AssetJobStage::AudioAnalysis;
    if (value == "package") return AssetJobStage::Package;
    return std::nullopt;
}

std::optional<AssetJobState> parseState(const std::string& value) {
    if (value == "pending") return AssetJobState::Pending;
    if (value == "running") return AssetJobState::Running;
    if (value == "paused") return AssetJobState::Paused;
    if (value == "cancelled") return AssetJobState::Cancelled;
    if (value == "failed") return AssetJobState::Failed;
    if (value == "completed") return AssetJobState::Completed;
    return std::nullopt;
}

} // namespace

const char* assetJobStageName(const AssetJobStage stage) {
    switch (stage) {
    case AssetJobStage::Scan: return "scan";
    case AssetJobStage::Import: return "import";
    case AssetJobStage::Hash: return "hash";
    case AssetJobStage::Thumbnail: return "thumbnail";
    case AssetJobStage::Transform: return "transform";
    case AssetJobStage::AudioAnalysis: return "audio_analysis";
    case AssetJobStage::Package: return "package";
    }
    return "scan";
}

const char* assetJobStateName(const AssetJobState state) {
    switch (state) {
    case AssetJobState::Pending: return "pending";
    case AssetJobState::Running: return "running";
    case AssetJobState::Paused: return "paused";
    case AssetJobState::Cancelled: return "cancelled";
    case AssetJobState::Failed: return "failed";
    case AssetJobState::Completed: return "completed";
    }
    return "pending";
}

AssetBackgroundJob::AssetBackgroundJob(std::string job_id, std::string kind, std::vector<AssetJobStage> stages) {
    snapshot_.job_id = std::move(job_id);
    snapshot_.kind = std::move(kind);
    snapshot_.stages = std::move(stages);
}

void AssetBackgroundJob::appendLog(std::string message) {
    const auto stage = snapshot_.stages.empty() ? AssetJobStage::Scan
                                                : snapshot_.stages[std::min(snapshot_.stage_index, snapshot_.stages.size() - 1)];
    snapshot_.logs.push_back({next_log_sequence_++, stage, std::move(message)});
}

void AssetBackgroundJob::updateOverallProgress() {
    if (snapshot_.stages.empty()) {
        snapshot_.overall_progress = 0.0;
        return;
    }
    snapshot_.overall_progress = std::clamp(
        (static_cast<double>(snapshot_.stage_index) + snapshot_.stage_progress) /
            static_cast<double>(snapshot_.stages.size()),
        0.0, 1.0);
}

bool AssetBackgroundJob::start() {
    if (snapshot_.job_id.empty() || snapshot_.kind.empty() || snapshot_.stages.empty() ||
        snapshot_.state != AssetJobState::Pending) return false;
    snapshot_.state = AssetJobState::Running;
    snapshot_.attempt = 1;
    appendLog("Job started.");
    return true;
}

bool AssetBackgroundJob::reportProgress(const double progress, std::string message) {
    if (snapshot_.state != AssetJobState::Running || snapshot_.cancel_requested || progress < snapshot_.stage_progress ||
        progress < 0.0 || progress > 1.0) return false;
    snapshot_.stage_progress = progress;
    updateOverallProgress();
    if (!message.empty()) appendLog(std::move(message));
    return true;
}

bool AssetBackgroundJob::completeStage(std::string message) {
    if (snapshot_.state != AssetJobState::Running || snapshot_.cancel_requested || snapshot_.stages.empty()) return false;
    snapshot_.stage_progress = 1.0;
    updateOverallProgress();
    appendLog(message.empty() ? "Stage completed." : std::move(message));
    if (snapshot_.stage_index + 1 == snapshot_.stages.size()) {
        snapshot_.state = AssetJobState::Completed;
        snapshot_.overall_progress = 1.0;
        return true;
    }
    ++snapshot_.stage_index;
    snapshot_.stage_progress = 0.0;
    updateOverallProgress();
    return true;
}

bool AssetBackgroundJob::requestCancel(std::string message) {
    if (snapshot_.state != AssetJobState::Running) return false;
    snapshot_.cancel_requested = true;
    snapshot_.state = AssetJobState::Cancelled;
    appendLog(std::move(message));
    return true;
}

bool AssetBackgroundJob::resume() {
    if (snapshot_.state != AssetJobState::Cancelled && snapshot_.state != AssetJobState::Paused) return false;
    snapshot_.state = AssetJobState::Running;
    snapshot_.cancel_requested = false;
    snapshot_.recovered_interruption = false;
    ++snapshot_.attempt;
    appendLog("Job resumed from its durable checkpoint.");
    return true;
}

bool AssetBackgroundJob::fail(std::string failure) {
    if (snapshot_.state != AssetJobState::Running || failure.empty()) return false;
    snapshot_.state = AssetJobState::Failed;
    snapshot_.failure = std::move(failure);
    appendLog("Job failed: " + snapshot_.failure);
    return true;
}

bool AssetBackgroundJob::retry() {
    if (snapshot_.state != AssetJobState::Failed) return false;
    snapshot_.state = AssetJobState::Running;
    snapshot_.failure.clear();
    ++snapshot_.attempt;
    appendLog("Failed stage scheduled for retry.");
    return true;
}

bool AssetBackgroundJob::saveCheckpoint(const std::filesystem::path& path, std::string* error) const {
    nlohmann::json stages = nlohmann::json::array();
    for (const auto stage : snapshot_.stages) stages.push_back(assetJobStageName(stage));
    nlohmann::json logs = nlohmann::json::array();
    for (const auto& log : snapshot_.logs) logs.push_back({{"sequence", log.sequence}, {"stage", assetJobStageName(log.stage)}, {"message", log.message}});
    const nlohmann::json payload = {
        {"schema", "urpg.asset_background_job.v1"}, {"job_id", snapshot_.job_id}, {"kind", snapshot_.kind},
        {"state", assetJobStateName(snapshot_.state)}, {"stages", stages}, {"stage_index", snapshot_.stage_index},
        {"stage_progress", snapshot_.stage_progress}, {"overall_progress", snapshot_.overall_progress},
        {"attempt", snapshot_.attempt}, {"cancel_requested", snapshot_.cancel_requested},
        {"recovered_interruption", snapshot_.recovered_interruption}, {"failure", snapshot_.failure}, {"logs", logs},
    };
    std::error_code filesystemError;
    std::filesystem::create_directories(path.parent_path(), filesystemError);
    if (filesystemError) { if (error) *error = filesystemError.message(); return false; }
    const auto temporary = path.parent_path() / ("." + path.filename().string() + ".tmp");
    { std::ofstream output(temporary, std::ios::binary | std::ios::trunc); output << payload.dump(2) << '\n';
      if (!output) { if (error) *error = "Unable to write job checkpoint."; return false; } }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        if (error) *error = "Unable to atomically publish job checkpoint.";
        std::filesystem::remove(temporary, filesystemError); return false;
    }
#else
    std::filesystem::rename(temporary, path, filesystemError);
    if (filesystemError) { if (error) *error = filesystemError.message(); std::filesystem::remove(temporary, filesystemError); return false; }
#endif
    return true;
}

std::optional<AssetBackgroundJob> AssetBackgroundJob::loadCheckpoint(const std::filesystem::path& path, std::string* error) {
    std::ifstream input(path, std::ios::binary);
    const auto payload = nlohmann::json::parse(input, nullptr, false);
    if (!payload.is_object() || payload.value("schema", "") != "urpg.asset_background_job.v1") {
        if (error) *error = "Asset job checkpoint is missing or malformed.";
        return std::nullopt;
    }
    std::vector<AssetJobStage> stages;
    for (const auto& value : payload.value("stages", nlohmann::json::array())) {
        if (!value.is_string()) return std::nullopt;
        const auto stage = parseStage(value.get<std::string>()); if (!stage) return std::nullopt; stages.push_back(*stage);
    }
    const auto state = parseState(payload.value("state", ""));
    if (!state || stages.empty()) { if (error) *error = "Asset job checkpoint contains an unknown state or stage."; return std::nullopt; }
    AssetBackgroundJob job(payload.value("job_id", ""), payload.value("kind", ""), std::move(stages));
    job.snapshot_.state = *state;
    job.snapshot_.stage_index = payload.value("stage_index", 0U);
    if (job.snapshot_.stage_index >= job.snapshot_.stages.size()) return std::nullopt;
    job.snapshot_.stage_progress = payload.value("stage_progress", 0.0);
    job.snapshot_.overall_progress = payload.value("overall_progress", 0.0);
    job.snapshot_.attempt = payload.value("attempt", 0U);
    job.snapshot_.cancel_requested = payload.value("cancel_requested", false);
    job.snapshot_.failure = payload.value("failure", "");
    for (const auto& row : payload.value("logs", nlohmann::json::array())) {
        const auto stage = parseStage(row.value("stage", "")); if (!stage) continue;
        const auto sequence = row.value("sequence", 0ULL);
        job.snapshot_.logs.push_back({sequence, *stage, row.value("message", "")});
        job.next_log_sequence_ = std::max(job.next_log_sequence_, sequence + 1);
    }
    if (job.snapshot_.state == AssetJobState::Running) {
        job.snapshot_.state = AssetJobState::Paused;
        job.snapshot_.recovered_interruption = true;
        job.appendLog("Interrupted running job recovered in a paused state.");
    }
    return job;
}

} // namespace urpg::assets
