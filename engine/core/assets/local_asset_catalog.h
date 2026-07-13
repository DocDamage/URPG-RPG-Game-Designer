#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

inline constexpr const char* kLocalAssetCatalogSchema = "urpg.asset_catalog.v1";
inline constexpr const char* kLocalAssetCatalogRegenerateCommand =
    "python tools/assets/catalog_interchange.py --db .urpg/asset-index/asset_catalog.db";

struct LocalAssetCatalogShard {
    std::string path;
    size_t recordCount = 0;
};

struct LocalAssetCatalogRoot {
    std::string id;
    std::string state;
    size_t assetCount = 0;
    size_t hashPendingCount = 0;
};

struct LocalAssetCatalogMetadata {
    std::string schemaVersion;
    std::string generatedAt;
    bool scanComplete = false;
    size_t assetCount = 0;
    size_t hashPendingCount = 0;
    size_t archiveCount = 0;
    std::vector<LocalAssetCatalogRoot> roots;
    std::vector<LocalAssetCatalogShard> shards;
};

struct LocalAssetCatalogRecord {
    std::string assetId;
    std::string virtualPath;
    std::string sourceRoot;
    std::string filename;
    std::string extension;
    std::string mediaKind;
    std::string archiveKind;
    uint64_t sizeBytes = 0;
    int64_t modifiedTimeNs = 0;
    std::string sha256;
    std::string pack;
    std::string category;
    std::vector<std::string> tags;
    std::string normalizedFilename;
    std::string normalizedVirtualPath;
    std::string normalizedExtension;
    std::string normalizedPack;
    std::string normalizedCategory;
    std::vector<std::string> normalizedTags;
};

struct LocalAssetCatalogQuery {
    std::string text;
    std::string mediaKind;
    std::string extension;
    std::string pack;
    std::string category;
    bool archiveOnly = false;
    size_t offset = 0;
    size_t pageSize = 50;
};

struct LocalAssetCatalogPage {
    std::vector<LocalAssetCatalogRecord> records;
    size_t totalMatches = 0;
    bool hasMore = false;
    std::vector<std::string> diagnostics;
};

struct LocalAssetCatalogLoadResult {
    bool success = false;
    std::vector<std::string> diagnostics;
};

// Streams metadata shards on demand. Loading a catalog only reads catalog_meta.json;
// query() materializes no more than a single requested page of records.
class LocalAssetCatalog {
  public:
    LocalAssetCatalogLoadResult load(const std::filesystem::path& catalogDirectory);
    void clear();

    bool isLoaded() const { return loaded_; }
    const LocalAssetCatalogMetadata& metadata() const { return metadata_; }
    LocalAssetCatalogPage query(const LocalAssetCatalogQuery& query) const;

  private:
    std::filesystem::path catalogDirectory_;
    LocalAssetCatalogMetadata metadata_;
    bool loaded_ = false;
};

} // namespace urpg::assets
