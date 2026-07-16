#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace urpg::assets {

struct ArchiveCatalogLimits {
    size_t maxEntryCount = 10'000;
    uint64_t maxExpandedBytes = 512ull * 1024ull * 1024ull;
    uint64_t maxEntryExpandedBytes = 128ull * 1024ull * 1024ull;
    uint64_t maxCompressionRatio = 1'000;
};

struct ArchiveCatalogEntry {
    std::string path;
    uint64_t compressedBytes = 0;
    uint64_t expandedBytes = 0;
    bool directory = false;
};

struct ArchiveCatalogResult {
    bool success = false;
    std::string code;
    std::string message;
    std::vector<ArchiveCatalogEntry> entries;
    std::vector<std::string> diagnostics;
};

struct ArchiveCatalogProcessResult {
    int exitCode = 0;
    std::string stdoutText;
    std::string stderrText;
};

using ArchiveCatalogCommandExecutor = std::function<ArchiveCatalogProcessResult(const std::vector<std::string>&)>;

class ArchiveCatalog {
  public:
    ArchiveCatalogResult list(const std::filesystem::path& archivePath,
                              const ArchiveCatalogLimits& limits = {},
                              const std::vector<std::string>& externalExtractorCommand = {},
                              ArchiveCatalogCommandExecutor executor = {}) const;
};

enum class ExternalAssetSourcePolicy { Reject, AllowNonPortable, RequireProjectCopy };

struct AssetSourceCustodyEntry {
    std::string asset_id;
    std::filesystem::path source_path;
    bool required_for_package = false;
};

struct AssetSourceCustodyDiagnostic {
    std::string severity;
    std::string code;
    std::string asset_id;
    std::string source_path;
    std::string message;
};

struct PortableAssetSourceAudit {
    bool portable = true;
    bool package_allowed = true;
    size_t inspected_count = 0;
    size_t external_count = 0;
    size_t missing_count = 0;
    std::vector<AssetSourceCustodyDiagnostic> diagnostics;
};

PortableAssetSourceAudit auditPortableAssetSources(const std::filesystem::path& projectRoot,
                                                    const std::vector<AssetSourceCustodyEntry>& sources,
                                                    ExternalAssetSourcePolicy policy);

} // namespace urpg::assets
