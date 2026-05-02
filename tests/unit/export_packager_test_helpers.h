#pragma once

#include "engine/core/tools/export_packager.h"

#include <cstdint>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::tests::export_packager {

extern const char kBundleMagic[];
extern const char kAssetDiscoveryManifestPath[];
extern const char kDevBootstrapMetadataPath[];

struct ProcessResult {
    int exitCode = -1;
    std::string stdoutText;
    std::string stderrText;
};

void WriteFile(const std::filesystem::path& path, const std::string& content = "");
void WritePromotedSourceManifest(const std::filesystem::path& root, const std::string& sourceId,
                                 const std::string& legalDisposition = "cc0_candidate_recorded_for_export");
void WriteAssetLicenseManifest(const std::filesystem::path& root, const std::vector<std::string>& paths);

std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path);
std::string ReadFileText(const std::filesystem::path& path);
nlohmann::json ReadBundleManifest(const std::filesystem::path& path);
std::vector<uint8_t> ReadBundleEntryBytes(const std::filesystem::path& path, const nlohmann::json& entry);
std::vector<uint8_t> DecodeBundleEntryBytes(const std::filesystem::path& path, urpg::tools::ExportTarget target,
                                            const nlohmann::json& entry);
std::string ComputeBundleIntegrityTag(const nlohmann::json& entry, const std::vector<uint8_t>& storedBytes,
                                      urpg::tools::ExportTarget target);
std::string ComputeBundleSignature(const std::filesystem::path& path, const nlohmann::json& manifest,
                                   urpg::tools::ExportTarget target);

#if defined(_WIN32) && defined(URPG_EXPORT_SMOKE_APP_PATH)
std::filesystem::path GetExportSmokeAppPath();
#endif

#ifdef URPG_PACK_CLI_PATH
std::filesystem::path GetPackCliPath();
ProcessResult RunPackCli(const std::vector<std::string>& args);
#endif

#ifdef _WIN32
bool LaunchProcessAndCaptureOutput(const std::filesystem::path& executable, const std::filesystem::path& markerPath,
                                   const std::filesystem::path& logPath);
#endif

} // namespace urpg::tests::export_packager
