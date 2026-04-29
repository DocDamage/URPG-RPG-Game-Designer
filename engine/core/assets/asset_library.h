#pragma once

#include "engine/core/assets/asset_provenance.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace urpg::assets {

enum class AssetStatus : uint16_t {
    Usable = 1 << 0,
    Risky = 1 << 1,
    Duplicate = 1 << 2,
    Oversized = 1 << 3,
    MissingLicense = 1 << 4,
    MissingFile = 1 << 5,
    UnsupportedFormat = 1 << 6,
    CaseCollision = 1 << 7,
    Promoted = 1 << 8,
    Archived = 1 << 9,
};

struct AssetRecord {
    std::string path;
    std::string source_path;
    std::string normalized_path;
    std::string preview_path;
    std::string preview_kind;
    std::string media_kind;
    std::string category;
    std::string pack;
    std::string duplicate_of;
    uint64_t size_bytes = 0;
    std::string sha256;
    std::vector<std::string> tags;
    std::vector<std::string> used_by;
    std::set<AssetStatus> statuses;
    AssetProvenance provenance;
};

struct AssetLibraryFilter {
    std::string media_kind;
    std::string category;
    std::string required_tag;
    std::optional<AssetStatus> required_status;
    bool referenced_only = false;
    bool runtime_ready_only = false;
    bool previewable_only = false;
};

struct AssetDuplicateEntry {
    std::string sha256;
    uint64_t size_bytes = 0;
    std::string path;
    std::string recommended_keep;
    bool recommended_remove = false;
};

struct AssetDuplicateGroup {
    std::string sha256;
    uint64_t size_bytes = 0;
    std::vector<AssetDuplicateEntry> entries;
};

struct AssetLibraryActionResult {
    std::string action;
    std::string path;
    bool success = false;
    std::string code;
    std::string message;
    nlohmann::json toJson() const;
};

struct AssetLibrarySnapshot {
    size_t file_count = 0;
    size_t catalog_asset_count = 0;
    size_t canonical_asset_count = 0;
    size_t duplicate_group_count = 0;
    size_t duplicate_asset_count = 0;
    size_t oversize_count = 0;
    size_t unsupported_count = 0;
    size_t missing_license_count = 0;
    size_t case_collision_count = 0;
    size_t catalog_shard_count = 0;
    size_t referenced_asset_count = 0;
    size_t runtime_ready_count = 0;
    size_t previewable_count = 0;
    size_t promoted_count = 0;
    size_t archived_count = 0;
    bool export_eligible = false;
    std::string promotion_status;
    std::map<std::string, size_t> category_counts;
    std::map<std::string, size_t> kind_counts;
    std::vector<AssetRecord> assets;
    std::vector<AssetDuplicateGroup> duplicate_groups;
};

class AssetLibrary {
public:
    void clear();
    void ingestHygieneSummary(const nlohmann::json& summary);
    void ingestIntakeReport(const nlohmann::json& report);
    void ingestPromotionCatalog(const nlohmann::json& catalog);
    void ingestDuplicateCsv(std::string_view csv_text);
    void addReferencedAsset(std::string path);
    void addUsageReference(std::string path, std::string owner_id);
    AssetLibraryActionResult promoteAsset(std::string path);
    AssetLibraryActionResult archiveAsset(std::string path, std::string reason);
    void markMissingFile(std::string path);
    void markUnsupportedFormat(std::string path);
    void detectCaseCollisions();

    const AssetLibrarySnapshot& snapshot() const { return snapshot_; }
    const std::set<std::string>& referencedAssets() const { return referenced_assets_; }
    std::optional<AssetRecord> findAsset(std::string_view path) const;
    std::vector<AssetRecord> filterAssets(const AssetLibraryFilter& filter) const;

private:
    AssetRecord& ensureAsset(std::string path);
    void refreshDerivedCounts();
    void sortSnapshot();

    AssetLibrarySnapshot snapshot_{};
    std::set<std::string> referenced_assets_;
};

const char* toString(AssetStatus status);
std::string exportProvenancePacket(const AssetRecord& record);

} // namespace urpg::assets
