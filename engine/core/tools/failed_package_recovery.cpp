#include "engine/core/tools/failed_package_recovery.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace urpg::tools {
namespace {

bool isSafeId(const std::string& value) {
    const std::filesystem::path path(value);
    return !value.empty() && !path.is_absolute() && path.filename().string() == value && value != "." && value != "..";
}

bool isUnder(const std::filesystem::path& candidate, const std::filesystem::path& root) {
    const auto normalized_candidate = std::filesystem::weakly_canonical(candidate);
    const auto normalized_root = std::filesystem::weakly_canonical(root);
    auto candidate_it = normalized_candidate.begin();
    for (auto root_it = normalized_root.begin(); root_it != normalized_root.end(); ++root_it, ++candidate_it) {
        if (candidate_it == normalized_candidate.end() || *candidate_it != *root_it) return false;
    }
    return true;
}

} // namespace

FailedPackageRecoveryResult FailedPackageRecovery::quarantinePartialOutput(
    const std::filesystem::path& project_root, const std::filesystem::path& partial_output,
    const std::string& package_id) {
    FailedPackageRecoveryResult result;
    const auto staging_root = project_root / ".urpg" / "package-staging";
    if (!std::filesystem::is_directory(project_root) || !std::filesystem::is_directory(partial_output) ||
        !isSafeId(package_id) || !isUnder(partial_output, staging_root) || partial_output == staging_root) {
        result.code = "failed_package_recovery_scope_rejected";
        result.message = "Partial package cleanup is restricted to a named child of the project staging root.";
        return result;
    }
    std::ifstream marker_input(partial_output / ".urpg-package-staging.json", std::ios::binary);
    const auto marker = marker_input ? nlohmann::json::parse(marker_input, nullptr, false) : nlohmann::json{};
    marker_input.close();
    if (!marker.is_object() || marker.value("schema", "") != "urpg.package_staging.v1" ||
        marker.value("packageId", "") != package_id) {
        result.code = "failed_package_recovery_marker_rejected";
        result.message = "Partial output has no matching package-staging ownership marker.";
        return result;
    }
    std::error_code error;
    const auto recovery_root = project_root / ".urpg" / "recovery" / "failed-packages";
    std::filesystem::create_directories(recovery_root, error);
    auto destination = recovery_root / package_id;
    for (uint32_t suffix = 1; std::filesystem::exists(destination) && suffix < 10000; ++suffix) {
        destination = recovery_root / (package_id + "-" + std::to_string(suffix));
    }
    std::filesystem::rename(partial_output, destination, error);
    if (error) {
        result.code = "failed_package_recovery_move_failed";
        result.message = error.message();
        return result;
    }
    result.receipt_path = destination / "recovery_receipt.json";
    std::ofstream receipt(result.receipt_path, std::ios::binary | std::ios::trunc);
    receipt << nlohmann::json{{"schema", "urpg.failed_package_recovery.v1"},
                              {"packageId", package_id}, {"sourcesPreserved", true},
                              {"partialOutputRemoved", true},
                              {"quarantinedOutput", destination.generic_string()}}.dump(2) << '\n';
    if (!receipt) {
        result.code = "failed_package_recovery_receipt_failed";
        result.message = "Partial output was quarantined but its recovery receipt could not be written.";
        result.quarantined_output = std::move(destination);
        return result;
    }
    result.success = true;
    result.code = "failed_package_output_quarantined";
    result.message = "Partial package output was removed from staging, preserved for diagnosis, and sources were unchanged.";
    result.quarantined_output = std::move(destination);
    return result;
}

} // namespace urpg::tools
