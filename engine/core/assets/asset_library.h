#pragma once

#include "engine/core/assets/asset_promotion_manifest.h"
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
    std::string asset_id;
    std::string path;
    std::string source_path;
    std::string normalized_path;
    std::string preview_path;
    std::string preview_kind;
    int32_t preview_width = 0;
    int32_t preview_height = 0;
    int32_t duration_ms = 0;
    std::vector<float> waveform_peaks;
    std::string media_kind;
    std::string category;
    std::string game_use_category;
    std::string pack;
    std::string source_bundle_id;
    std::string package_destination;
    std::string distribution;
    std::string duplicate_of;
    uint64_t size_bytes = 0;
    size_t frame_count = 0;
    size_t sequence_count = 0;
    std::string sha256;
    std::vector<std::string> tags;
    std::vector<std::string> game_use_tags;
    nlohmann::json representative_sequences = nlohmann::json::array();
    std::vector<std::string> used_by;
    std::set<AssetStatus> statuses;
    AssetProvenance provenance;
    std::string promotion_status;
    std::string promoted_path;
    std::string license_id;
    bool include_in_runtime = false;
    bool required_for_release = false;
    bool release_eligible = false;
    std::vector<std::string> promotion_diagnostics;
    nlohmann::json authored_metadata = nlohmann::json::object();
};

struct AssetLibraryFilter {
    std::string media_kind;
    std::string category;
    std::string game_use_category;
    std::string required_tag;
    std::string required_game_use_tag;
    std::string source_bundle_id;
    std::optional<AssetStatus> required_status;
    bool referenced_only = false;
    bool runtime_ready_only = false;
    bool previewable_only = false;
    bool project_attached_only = false;
    bool attachable_only = false;
    bool release_eligible_only = false;
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
    size_t sequence_asset_count = 0;
    size_t sequence_frame_count = 0;
    size_t sequence_clip_count = 0;
    size_t promoted_count = 0;
    size_t archived_count = 0;
    bool export_eligible = false;
    std::string promotion_status;
    std::map<std::string, size_t> category_counts;
    std::map<std::string, size_t> game_use_category_counts;
    std::map<std::string, size_t> game_use_tag_counts;
    std::map<std::string, size_t> kind_counts;
    std::map<std::string, size_t> source_bundle_counts;
    std::vector<AssetRecord> assets;
    std::vector<AssetDuplicateGroup> duplicate_groups;
};

class AssetLibrary;

struct AssetPromotionCatalogIngestProgress {
    std::string stage = "header";
    size_t total_items = 1;
    size_t processed_items = 0;
    size_t last_slice_items = 0;
    size_t existing_records_total = 0;
    size_t catalog_records_total = 0;
    size_t materialized_records_total = 0;
    uint64_t starting_index_revision = 0;
    uint64_t published_index_revision = 0;
    bool complete = false;
    bool failed = false;
    std::string code = "asset_promotion_catalog_ingest_pending";
};

class AssetPromotionCatalogIngestJob {
  public:
    static constexpr size_t kDefaultMaximumItemsPerSlice = 64;

    AssetPromotionCatalogIngestJob(const AssetPromotionCatalogIngestJob&) = delete;
    AssetPromotionCatalogIngestJob& operator=(const AssetPromotionCatalogIngestJob&) = delete;
    AssetPromotionCatalogIngestJob(AssetPromotionCatalogIngestJob&&) noexcept = default;
    AssetPromotionCatalogIngestJob& operator=(AssetPromotionCatalogIngestJob&&) noexcept = default;

    bool advance(size_t maximum_items = kDefaultMaximumItemsPerSlice);
    bool complete() const { return progress_.complete; }
    bool failed() const { return progress_.failed; }
    const AssetPromotionCatalogIngestProgress& progress() const { return progress_; }

  private:
    friend class AssetLibrary;
    enum class Stage { Header, CopyExisting, ApplyCatalog, Materialize, Complete, Failed };

    AssetPromotionCatalogIngestJob(AssetLibrary* library, nlohmann::json catalog);
    void applyHeader();
    void copyExistingRecord();
    void applyCatalogRecord();
    void beginMaterialization();
    void materializeRecord();
    void publish();
    void fail(std::string code);

    AssetLibrary* library_ = nullptr;
    nlohmann::json catalog_;
    Stage stage_ = Stage::Header;
    size_t existing_cursor_ = 0;
    size_t catalog_cursor_ = 0;
    std::map<std::string, AssetRecord> staged_records_;
    std::vector<AssetRecord> staged_assets_;
    std::map<std::string, std::vector<size_t>> staged_filter_index_;
    AssetLibrarySnapshot staged_derived_counts_{};
    std::string promotion_status_;
    bool export_eligible_ = false;
    size_t catalog_asset_count_delta_ = 0;
    size_t canonical_asset_count_delta_ = 0;
    size_t duplicate_group_count_delta_ = 0;
    size_t duplicate_asset_count_delta_ = 0;
    size_t unsupported_count_delta_ = 0;
    size_t catalog_shard_count_delta_ = 0;
    size_t missing_license_count_delta_ = 0;
    std::map<std::string, size_t> category_count_deltas_;
    std::map<std::string, size_t> kind_count_deltas_;
    AssetPromotionCatalogIngestProgress progress_{};
};

class AssetLibrary {
  public:
    void clear();
    void ingestHygieneSummary(const nlohmann::json& summary);
    void ingestIntakeReport(const nlohmann::json& report);
    void ingestPromotionCatalog(const nlohmann::json& catalog);
    AssetPromotionCatalogIngestJob beginPromotionCatalogIngest(nlohmann::json catalog);
    void ingestAssetBundleManifest(const nlohmann::json& manifest);
    void ingestPromotionManifest(const AssetPromotionManifest& manifest);
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
    uint64_t filterIndexRevision() const { return filter_index_revision_; }
    size_t lastFilterCandidateCount() const { return last_filter_candidate_count_; }

  private:
    friend class AssetPromotionCatalogIngestJob;
    AssetRecord& ensureAsset(std::string path);
    void refreshDerivedCounts();
    void sortSnapshot();
    void rebuildFilterIndex();

    AssetLibrarySnapshot snapshot_{};
    std::set<std::string> referenced_assets_;
    std::map<std::string, std::vector<size_t>> filter_index_;
    uint64_t filter_index_revision_ = 0;
    mutable size_t last_filter_candidate_count_ = 0;
};

const char* toString(AssetStatus status);
std::string exportProvenancePacket(const AssetRecord& record);

} // namespace urpg::assets
