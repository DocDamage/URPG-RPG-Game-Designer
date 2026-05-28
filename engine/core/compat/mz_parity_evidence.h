#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::compat {

struct MzReferenceCaptureEvidence {
    std::string capture_id;
    std::string project_id;
    std::string scene_id;
    std::string backend;
    std::string frame_hash;
    int32_t width = 0;
    int32_t height = 0;
    std::string legal_use = "repo_owned";
    bool release_authoritative = false;

    nlohmann::json toJson() const;
};

struct MzBackendObservationEvidence {
    std::string observation_id;
    std::string project_id;
    std::string scene_id;
    std::string backend;
    std::string frame_hash;
    int32_t width = 0;
    int32_t height = 0;

    nlohmann::json toJson() const;
};

struct MzParityComparisonRow {
    std::string project_id;
    std::string scene_id;
    std::string reference_backend;
    std::string observed_backend;
    std::string reference_hash;
    std::string observed_hash;
    std::string status = "missing_observation";
    bool dimensions_match = false;

    nlohmann::json toJson() const;
};

struct MzParityEvidenceReport {
    std::vector<MzReferenceCaptureEvidence> references;
    std::vector<MzBackendObservationEvidence> backend_observations;
    std::vector<MzParityComparisonRow> comparisons;
    int32_t reference_count = 0;
    int32_t backend_count = 0;
    int32_t failed_comparison_count = 0;
    bool release_authoritative = false;

    nlohmann::json toJson() const;
};

MzParityEvidenceReport BuildMzParityEvidenceReport(
    std::vector<MzReferenceCaptureEvidence> references,
    std::vector<MzBackendObservationEvidence> backend_observations);

} // namespace urpg::compat
