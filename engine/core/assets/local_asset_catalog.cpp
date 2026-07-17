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

LocalAssetCatalogQuery normalizeQuery(LocalAssetCatalogQuery query) {
    query.text = normalized(std::move(query.text));
    query.mediaKind = normalized(std::move(query.mediaKind));
    query.extension = normalized(std::move(query.extension));
    query.pack = normalized(std::move(query.pack));
    query.category = normalized(std::move(query.category));
    return query;
}

bool matches(const LocalAssetCatalogRecord& record, const LocalAssetCatalogQuery& query) {
    if (!query.text.empty()) {
        bool found = contains(record.normalizedFilename, query.text) ||
                     contains(record.normalizedVirtualPath, query.text) ||
                     contains(record.normalizedExtension, query.text) ||
                     contains(record.normalizedPack, query.text) ||
                     contains(record.normalizedCategory, query.text);
        found = found || std::any_of(record.normalizedTags.begin(), record.normalizedTags.end(), [&](const auto& tag) {
            return contains(tag, query.text);
        });
        if (!found) {
            return false;
        }
    }
    if (!query.mediaKind.empty() && normalized(record.mediaKind) != query.mediaKind) {
        return false;
    }
    if (!query.extension.empty() && record.normalizedExtension != query.extension) {
        return false;
    }
    if (!query.pack.empty() && !contains(record.normalizedPack, query.pack)) {
        return false;
    }
    if (!query.category.empty() && !contains(record.normalizedCategory, query.category)) {
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
    if (!loaded_) {
        LocalAssetCatalogPage page;
        page.diagnostics.push_back("catalog_not_loaded");
        return page;
    }
    auto job = beginQuery(query);
    while (!job.advance(8192)) {
    }
    return job.result();
}

LocalAssetCatalogQueryJob LocalAssetCatalog::beginQuery(const LocalAssetCatalogQuery& query) const {
    if (!loaded_) {
        return {};
    }
    return LocalAssetCatalogQueryJob(catalogDirectory_, metadata_, query);
}

LocalAssetCatalogQueryJob::LocalAssetCatalogQueryJob(std::filesystem::path catalogDirectory,
                                                     LocalAssetCatalogMetadata metadata,
                                                     LocalAssetCatalogQuery query)
    : catalogDirectory_(std::move(catalogDirectory)), metadata_(std::move(metadata)), query_(std::move(query)),
      normalizedQuery_(normalizeQuery(query_)) {
    normalizedQuery_.pageSize = std::clamp(normalizedQuery_.pageSize, size_t{1}, kMaximumPageSize);
    for (const auto& shard : metadata_.shards) {
        expectedRecords_ += shard.recordCount;
    }
    if (metadata_.shards.empty()) {
        complete_ = true;
    }
}

bool LocalAssetCatalogQueryJob::advance(const size_t maximumRecords) {
    if (complete_ || cancelled_) {
        return true;
    }
    if (maximumRecords == 0) {
        return false;
    }

    size_t consumed = 0;
    while (consumed < maximumRecords && shardIndex_ < metadata_.shards.size()) {
        const auto& shard = metadata_.shards[shardIndex_];
        if (!input_.is_open()) {
            input_.open(catalogDirectory_ / shard.path);
            shardLineNumber_ = 0;
            if (!input_) {
                addDiagnostic(page_.diagnostics, "catalog_shard_unreadable: " + shard.path + remediation());
                processedRecords_ += shard.recordCount;
                input_.clear();
                ++shardIndex_;
                continue;
            }
        }

        std::string line;
        if (!std::getline(input_, line)) {
            input_.close();
            ++shardIndex_;
            continue;
        }

        ++consumed;
        ++processedRecords_;
        ++shardLineNumber_;
        const auto value = nlohmann::json::parse(line, nullptr, false);
        if (value.is_discarded() || !value.is_object()) {
            addDiagnostic(page_.diagnostics, "catalog_record_invalid: " + shard.path + ":" +
                                                  std::to_string(shardLineNumber_));
            continue;
        }
        const auto record = parseRecord(value);
        if (!matches(record, normalizedQuery_)) {
            continue;
        }
        ++page_.totalMatches;
        if (page_.totalMatches > normalizedQuery_.offset &&
            page_.records.size() < normalizedQuery_.pageSize) {
            page_.records.push_back(record);
        }
    }

    if (shardIndex_ >= metadata_.shards.size()) {
        complete_ = true;
        page_.hasMore = page_.totalMatches > normalizedQuery_.offset + page_.records.size();
    }
    return complete_;
}

void LocalAssetCatalogQueryJob::cancel() {
    cancelled_ = true;
    complete_ = true;
    if (input_.is_open()) {
        input_.close();
    }
}

LocalAssetCatalogQueryProgress LocalAssetCatalogQueryJob::progress() const {
    return {processedRecords_, expectedRecords_, complete_, cancelled_};
}

} // namespace urpg::assets
