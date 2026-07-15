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

} // namespace urpg::assets
