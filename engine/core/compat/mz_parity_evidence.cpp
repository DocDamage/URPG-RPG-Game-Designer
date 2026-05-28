#include "engine/core/compat/mz_parity_evidence.h"

#include <algorithm>
#include <tuple>

namespace urpg::compat {

nlohmann::json MzReferenceCaptureEvidence::toJson() const {
    return {
        {"capture_id", capture_id},
        {"project_id", project_id},
        {"scene_id", scene_id},
        {"backend", backend},
        {"frame_hash", frame_hash},
        {"dimensions", {{"width", width}, {"height", height}}},
        {"legal_use", legal_use},
        {"release_authoritative", release_authoritative},
    };
}

nlohmann::json MzBackendObservationEvidence::toJson() const {
    return {
        {"observation_id", observation_id},
        {"project_id", project_id},
        {"scene_id", scene_id},
        {"backend", backend},
        {"frame_hash", frame_hash},
        {"dimensions", {{"width", width}, {"height", height}}},
    };
}

nlohmann::json MzParityComparisonRow::toJson() const {
    return {
        {"project_id", project_id},
        {"scene_id", scene_id},
        {"reference_backend", reference_backend},
        {"observed_backend", observed_backend},
        {"reference_hash", reference_hash},
        {"observed_hash", observed_hash},
        {"status", status},
        {"dimensions_match", dimensions_match},
    };
}

nlohmann::json MzParityEvidenceReport::toJson() const {
    nlohmann::json referenceJson = nlohmann::json::array();
    for (const auto& reference : references) {
        referenceJson.push_back(reference.toJson());
    }

    nlohmann::json backendJson = nlohmann::json::array();
    for (const auto& observation : backend_observations) {
        backendJson.push_back(observation.toJson());
    }

    nlohmann::json comparisonJson = nlohmann::json::array();
    for (const auto& comparison : comparisons) {
        comparisonJson.push_back(comparison.toJson());
    }

    return {
        {"reference_count", reference_count},
        {"backend_count", backend_count},
        {"failed_comparison_count", failed_comparison_count},
        {"release_authoritative", release_authoritative},
        {"references", std::move(referenceJson)},
        {"backend_observations", std::move(backendJson)},
        {"comparisons", std::move(comparisonJson)},
    };
}

MzParityEvidenceReport BuildMzParityEvidenceReport(
    std::vector<MzReferenceCaptureEvidence> references,
    std::vector<MzBackendObservationEvidence> backend_observations) {
    std::sort(references.begin(), references.end(), [](const auto& a, const auto& b) {
        return std::tie(a.project_id, a.scene_id, a.capture_id) < std::tie(b.project_id, b.scene_id, b.capture_id);
    });
    std::sort(backend_observations.begin(), backend_observations.end(), [](const auto& a, const auto& b) {
        return std::tie(a.project_id, a.scene_id, a.backend, a.observation_id) <
               std::tie(b.project_id, b.scene_id, b.backend, b.observation_id);
    });

    MzParityEvidenceReport report;
    report.references = std::move(references);
    report.backend_observations = std::move(backend_observations);
    report.reference_count = static_cast<int32_t>(report.references.size());
    report.backend_count = static_cast<int32_t>(report.backend_observations.size());
    report.release_authoritative = false;

    for (const auto& reference : report.references) {
        bool matched = false;
        for (const auto& observation : report.backend_observations) {
            if (observation.project_id != reference.project_id || observation.scene_id != reference.scene_id) {
                continue;
            }
            matched = true;
            MzParityComparisonRow row;
            row.project_id = reference.project_id;
            row.scene_id = reference.scene_id;
            row.reference_backend = reference.backend;
            row.observed_backend = observation.backend;
            row.reference_hash = reference.frame_hash;
            row.observed_hash = observation.frame_hash;
            row.dimensions_match = reference.width == observation.width && reference.height == observation.height;
            row.status = row.dimensions_match && reference.frame_hash == observation.frame_hash ? "matched" : "delta";
            report.comparisons.push_back(std::move(row));
        }
        if (!matched) {
            MzParityComparisonRow row;
            row.project_id = reference.project_id;
            row.scene_id = reference.scene_id;
            row.reference_backend = reference.backend;
            row.reference_hash = reference.frame_hash;
            report.comparisons.push_back(std::move(row));
        }
    }

    std::sort(report.comparisons.begin(), report.comparisons.end(), [](const auto& a, const auto& b) {
        return std::tie(a.project_id, a.scene_id, a.observed_backend) <
               std::tie(b.project_id, b.scene_id, b.observed_backend);
    });

    report.failed_comparison_count = 0;
    for (const auto& comparison : report.comparisons) {
        if (comparison.status != "matched") {
            ++report.failed_comparison_count;
        }
    }

    return report;
}

} // namespace urpg::compat
