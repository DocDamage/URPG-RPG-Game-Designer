#include "engine/core/assets/local_asset_catalog.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <fstream>
#include <limits>
#include <system_error>

namespace urpg::assets {
namespace {

constexpr size_t kMaximumPageSize = 200;
constexpr size_t kMaximumQueryDiagnostics = 25;

std::string normalized(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::string remediation() {
    return std::string(" Regenerate with: ") + kLocalAssetCatalogRegenerateCommand;
}

void addDiagnostic(std::vector<std::string>& diagnostics, std::string message) {
    if (diagnostics.size() < kMaximumQueryDiagnostics) {
        diagnostics.push_back(std::move(message));
    }
}

LocalAssetCatalogRecord parseRecord(const nlohmann::json& value) {
    LocalAssetCatalogRecord record;
    record.assetId = value.value("asset_id", "");
    record.virtualPath = value.value("virtual_path", "");
    record.sourceRoot = value.value("source_root", "");
    record.filename = value.value("filename", "");
    record.extension = value.value("extension", "");
    record.mediaKind = value.value("media_kind", "");
    record.archiveKind = value.value("archive_kind", "");
    record.sizeBytes = value.value("size_bytes", uint64_t{0});
    record.modifiedTimeNs = value.value("mtime_ns", int64_t{0});
    record.sha256 = value.value("sha256", "");
    record.pack = value.value("pack", "");
    record.category = value.value("category", "");
    if (const auto tags = value.find("tags"); tags != value.end() && tags->is_array()) {
        for (const auto& tag : *tags) {
            if (tag.is_string()) {
                record.tags.push_back(tag.get<std::string>());
            }
        }
    }
    record.normalizedFilename = value.value("normalized_filename", normalized(record.filename));
    record.normalizedVirtualPath = value.value("normalized_virtual_path", normalized(record.virtualPath));
    record.normalizedExtension = value.value("normalized_extension", normalized(record.extension));
    record.normalizedPack = value.value("normalized_pack", normalized(record.pack));
    record.normalizedCategory = value.value("normalized_category", normalized(record.category));
    if (const auto tags = value.find("normalized_tags"); tags != value.end() && tags->is_array()) {
        for (const auto& tag : *tags) {
            if (tag.is_string()) {
                record.normalizedTags.push_back(tag.get<std::string>());
            }
        }
    }
    if (record.normalizedTags.empty()) {
        for (const auto& tag : record.tags) {
            record.normalizedTags.push_back(normalized(tag));
        }
    }
    return record;
}

bool contains(const std::string& haystack, const std::string& needle) {
    return needle.empty() || haystack.find(needle) != std::string::npos;
}

bool matches(const LocalAssetCatalogRecord& record, const LocalAssetCatalogQuery& query) {
    const auto text = normalized(query.text);
    if (!text.empty()) {
        bool found = contains(record.normalizedFilename, text) || contains(record.normalizedVirtualPath, text) ||
                     contains(record.normalizedExtension, text) || contains(record.normalizedPack, text) ||
                     contains(record.normalizedCategory, text);
        found = found || std::any_of(record.normalizedTags.begin(), record.normalizedTags.end(), [&](const auto& tag) {
            return contains(tag, text);
        });
        if (!found) {
            return false;
        }
    }
    if (!query.mediaKind.empty() && normalized(record.mediaKind) != normalized(query.mediaKind)) {
        return false;
    }
    if (!query.extension.empty() && record.normalizedExtension != normalized(query.extension)) {
        return false;
    }
    if (!query.pack.empty() && !contains(record.normalizedPack, normalized(query.pack))) {
        return false;
    }
    if (!query.category.empty() && !contains(record.normalizedCategory, normalized(query.category))) {
        return false;
    }
    return !query.archiveOnly || !record.archiveKind.empty();
}

} // namespace

void LocalAssetCatalog::clear() {
    catalogDirectory_.clear();
    metadata_ = {};
    loaded_ = false;
}

LocalAssetCatalogLoadResult LocalAssetCatalog::load(const std::filesystem::path& catalogDirectory) {
    LocalAssetCatalogLoadResult result;
    const auto manifestPath = catalogDirectory / "catalog_meta.json";
    std::ifstream input(manifestPath);
    if (!input) {
        result.diagnostics.push_back("catalog_meta_missing:" + remediation());
        return result;
    }

    const auto value = nlohmann::json::parse(input, nullptr, false);
    if (value.is_discarded() || !value.is_object()) {
        result.diagnostics.push_back("catalog_meta_invalid:" + remediation());
        return result;
    }
    if (value.value("schema_version", "") != kLocalAssetCatalogSchema) {
        result.diagnostics.push_back("catalog_schema_incompatible: expected " + std::string(kLocalAssetCatalogSchema) +
                                     ", found " + value.value("schema_version", "missing") + "." + remediation());
        return result;
    }
    const auto shards = value.find("shards");
    if (shards == value.end() || !shards->is_array()) {
        result.diagnostics.push_back("catalog_shards_missing:" + remediation());
        return result;
    }

    LocalAssetCatalogMetadata candidate;
    candidate.schemaVersion = value.value("schema_version", "");
    candidate.generatedAt = value.value("generated_at", "");
    candidate.scanComplete = value.value("scan_complete", false);
    const auto counts = value.value("counts", nlohmann::json::object());
    candidate.assetCount = counts.value("asset_count", size_t{0});
    candidate.hashPendingCount = counts.value("hash_pending_count", size_t{0});
    candidate.archiveCount = counts.value("archive_count", size_t{0});
    for (const auto& shard : *shards) {
        if (!shard.is_object()) {
            result.diagnostics.push_back("catalog_shard_invalid:" + remediation());
            return result;
        }
        const auto path = shard.value("path", "");
        if (path.empty() || std::filesystem::path(path).has_parent_path()) {
            result.diagnostics.push_back("catalog_shard_path_invalid:" + remediation());
            return result;
        }
        candidate.shards.push_back({path, shard.value("record_count", size_t{0})});
    }
    if (const auto roots = value.find("roots"); roots != value.end() && roots->is_array()) {
        for (const auto& root : *roots) {
            if (root.is_object()) {
                candidate.roots.push_back({root.value("id", ""), root.value("state", "unknown"),
                                           root.value("asset_count", size_t{0}),
                                           root.value("hash_pending_count", size_t{0})});
            }
        }
    }

    catalogDirectory_ = catalogDirectory;
    metadata_ = std::move(candidate);
    loaded_ = true;
    result.success = true;
    return result;
}

LocalAssetCatalogPage LocalAssetCatalog::query(const LocalAssetCatalogQuery& query) const {
    LocalAssetCatalogPage page;
    if (!loaded_) {
        page.diagnostics.push_back("catalog_not_loaded");
        return page;
    }
    const size_t pageSize = std::clamp(query.pageSize, size_t{1}, kMaximumPageSize);
    for (const auto& shard : metadata_.shards) {
        std::ifstream input(catalogDirectory_ / shard.path);
        if (!input) {
            addDiagnostic(page.diagnostics, "catalog_shard_unreadable: " + shard.path + remediation());
            continue;
        }
        std::string line;
        size_t lineNumber = 0;
        while (std::getline(input, line)) {
            ++lineNumber;
            const auto value = nlohmann::json::parse(line, nullptr, false);
            if (value.is_discarded() || !value.is_object()) {
                addDiagnostic(page.diagnostics, "catalog_record_invalid: " + shard.path + ":" +
                                                  std::to_string(lineNumber));
                continue;
            }
            const auto record = parseRecord(value);
            if (!matches(record, query)) {
                continue;
            }
            ++page.totalMatches;
            if (page.totalMatches > query.offset && page.records.size() < pageSize) {
                page.records.push_back(record);
            }
        }
    }
    page.hasMore = page.totalMatches > query.offset + page.records.size();
    return page;
}

} // namespace urpg::assets
