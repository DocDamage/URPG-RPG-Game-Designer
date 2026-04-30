#include "engine/core/assets/asset_library.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <sstream>

namespace urpg::assets {

namespace {

std::optional<size_t> readCount(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end()) {
        return std::nullopt;
    }
    if (it->is_number_unsigned()) {
        return it->get<size_t>();
    }
    if (it->is_number_integer()) {
        const auto count = it->get<long long>();
        if (count >= 0) {
            return static_cast<size_t>(count);
        }
    }
    return std::nullopt;
}

std::optional<std::string> readString(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end() || !it->is_string()) {
        return std::nullopt;
    }
    return it->get<std::string>();
}

std::vector<std::string> readStringArray(const nlohmann::json& value, const char* key) {
    std::vector<std::string> out;
    const auto it = value.find(key);
    if (it == value.end() || !it->is_array()) {
        return out;
    }
    for (const auto& item : *it) {
        if (item.is_string()) {
            out.push_back(item.get<std::string>());
        }
    }
    return out;
}

std::vector<float> readFloatArray(const nlohmann::json& value, const char* key) {
    std::vector<float> out;
    const auto it = value.find(key);
    if (it == value.end() || !it->is_array()) {
        return out;
    }
    for (const auto& item : *it) {
        if (item.is_number()) {
            out.push_back(item.get<float>());
        }
    }
    return out;
}

int32_t readInt32(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end() || !it->is_number_integer()) {
        return 0;
    }
    const auto number = it->get<long long>();
    if (number <= 0) {
        return 0;
    }
    return static_cast<int32_t>(std::min<long long>(number, INT32_MAX));
}

uint64_t readUint64(const nlohmann::json& value, const char* key) {
    const auto it = value.find(key);
    if (it == value.end()) {
        return 0;
    }
    if (it->is_number_unsigned()) {
        return it->get<uint64_t>();
    }
    if (it->is_number_integer()) {
        const auto number = it->get<long long>();
        return number > 0 ? static_cast<uint64_t>(number) : 0;
    }
    return 0;
}

std::map<std::string, size_t> readCountMap(const nlohmann::json& value, const char* key) {
    std::map<std::string, size_t> out;
    const auto it = value.find(key);
    if (it == value.end() || !it->is_object()) {
        return out;
    }
    for (const auto& [name, count] : it->items()) {
        if (count.is_number_unsigned()) {
            out[name] = count.get<size_t>();
        } else if (count.is_number_integer()) {
            const auto signed_count = count.get<long long>();
            if (signed_count >= 0) {
                out[name] = static_cast<size_t>(signed_count);
            }
        }
    }
    return out;
}

void addCountMap(std::map<std::string, size_t>& target, const std::map<std::string, size_t>& source) {
    for (const auto& [key, count] : source) {
        target[key] += count;
    }
}

bool statusLess(const AssetRecord& lhs, const AssetRecord& rhs) {
    return lhs.path < rhs.path;
}

bool isSequenceAsset(const AssetRecord& asset) {
    return asset.media_kind == "image_sequence_collection" || asset.media_kind == "image_sequence";
}

std::string lowerPath(std::string value) {
    std::replace(value.begin(), value.end(), '\\', '/');
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

std::vector<std::string> parseCsvLine(std::string_view line) {
    std::vector<std::string> fields;
    std::string current;
    bool in_quotes = false;
    for (size_t i = 0; i < line.size(); ++i) {
        const char ch = line[i];
        if (ch == '"') {
            if (in_quotes && i + 1 < line.size() && line[i + 1] == '"') {
                current.push_back('"');
                ++i;
            } else {
                in_quotes = !in_quotes;
            }
            continue;
        }
        if (ch == ',' && !in_quotes) {
            fields.push_back(current);
            current.clear();
            continue;
        }
        current.push_back(ch);
    }
    fields.push_back(current);
    return fields;
}

bool truthy(std::string_view value) {
    return value == "yes" || value == "true" || value == "1";
}

bool hasStatus(const AssetRecord& record, AssetStatus status) {
    return record.statuses.contains(status);
}

bool isRuntimeReady(const AssetRecord& record) {
    return !record.normalized_path.empty() &&
           !hasStatus(record, AssetStatus::MissingFile) &&
           !hasStatus(record, AssetStatus::UnsupportedFormat) &&
           !hasStatus(record, AssetStatus::MissingLicense) &&
           !hasStatus(record, AssetStatus::Duplicate) &&
           !hasStatus(record, AssetStatus::Archived);
}

bool isPreviewable(const AssetRecord& record) {
    return !record.preview_path.empty() &&
           (record.preview_kind == "image" || record.preview_kind == "audio" || record.preview_kind == "video");
}

} // namespace

const char* toString(AssetStatus status) {
    switch (status) {
    case AssetStatus::Usable:
        return "usable";
    case AssetStatus::Risky:
        return "risky";
    case AssetStatus::Duplicate:
        return "duplicate";
    case AssetStatus::Oversized:
        return "oversized";
    case AssetStatus::MissingLicense:
        return "missing_license";
    case AssetStatus::MissingFile:
        return "missing_file";
    case AssetStatus::UnsupportedFormat:
        return "unsupported_format";
    case AssetStatus::CaseCollision:
        return "case_collision";
    case AssetStatus::Promoted:
        return "promoted";
    case AssetStatus::Archived:
        return "archived";
    }
    return "risky";
}

namespace {

bool canPromoteAsset(const AssetRecord& record, std::string& code, std::string& message) {
    if (record.path.empty()) {
        code = "asset_missing";
        message = "Asset record does not exist.";
        return false;
    }
    if (record.normalized_path.empty()) {
        code = "asset_not_normalized";
        message = "Asset must have a normalized path before promotion.";
        return false;
    }
    if (record.statuses.contains(AssetStatus::MissingFile)) {
        code = "asset_missing_file";
        message = "Missing files cannot be promoted.";
        return false;
    }
    if (record.statuses.contains(AssetStatus::UnsupportedFormat)) {
        code = "asset_unsupported_format";
        message = "Unsupported formats cannot be promoted.";
        return false;
    }
    if (record.statuses.contains(AssetStatus::MissingLicense)) {
        code = "asset_missing_license";
        message = "Assets without license evidence cannot be promoted.";
        return false;
    }
    if (record.statuses.contains(AssetStatus::Duplicate)) {
        code = "asset_duplicate";
        message = "Duplicate assets must be resolved before promotion.";
        return false;
    }
    if (record.statuses.contains(AssetStatus::Archived)) {
        code = "asset_archived";
        message = "Archived assets must be restored before promotion.";
        return false;
    }
    return true;
}

} // namespace

nlohmann::json AssetLibraryActionResult::toJson() const {
    return {{"action", action}, {"path", path}, {"success", success}, {"code", code}, {"message", message}};
}

std::string exportProvenancePacket(const AssetRecord& record) {
    nlohmann::json packet = {
        {"path", record.path},
        {"size_bytes", record.size_bytes},
        {"sha256", record.sha256},
        {"provenance", toJson(record.provenance)},
    };
    return packet.dump();
}

void AssetLibrary::clear() {
    snapshot_ = {};
    referenced_assets_.clear();
}

AssetRecord& AssetLibrary::ensureAsset(std::string path) {
    for (auto& asset : snapshot_.assets) {
        if (asset.path == path) {
            return asset;
        }
    }
    AssetRecord record{};
    record.path = std::move(path);
    record.statuses.insert(AssetStatus::Usable);
    snapshot_.assets.push_back(std::move(record));
    return snapshot_.assets.back();
}

void AssetLibrary::ingestHygieneSummary(const nlohmann::json& summary) {
    if (!summary.is_object()) {
        return;
    }
    snapshot_.file_count = readCount(summary, "file_count").value_or(snapshot_.file_count);
    snapshot_.duplicate_group_count = readCount(summary, "duplicate_groups").value_or(snapshot_.duplicate_group_count);
    snapshot_.oversize_count = readCount(summary, "oversize_count").value_or(snapshot_.oversize_count);
}

void AssetLibrary::ingestIntakeReport(const nlohmann::json& report) {
    if (!report.is_object()) {
        return;
    }
    const auto sources = report.find("sources");
    if (sources == report.end() || !sources->is_array()) {
        return;
    }

    for (const auto& source : *sources) {
        if (!source.is_object()) {
            continue;
        }
        const auto source_id = readString(source, "source_id").value_or("");
        const auto repo_name = readString(source, "repo_name").value_or("");
        const auto license = readString(source, "legal_disposition").value_or("");
        const bool export_eligible = readString(source, "promotion_status").value_or("") == "promoted" &&
                                     license.find("cc0") != std::string::npos;

        const auto assets = source.find("normalized_assets");
        if (assets == source.end() || !assets->is_array()) {
            continue;
        }
        for (const auto& path : *assets) {
            if (!path.is_string()) {
                continue;
            }
            auto& record = ensureAsset(path.get<std::string>());
            record.provenance.original_source = repo_name.empty() ? source_id : repo_name;
            record.provenance.license = license;
            record.provenance.normalized_path = record.path;
            record.provenance.export_eligible = export_eligible;
            if (!export_eligible) {
                record.statuses.insert(AssetStatus::Risky);
            }
            if (license.empty()) {
                record.statuses.insert(AssetStatus::MissingLicense);
                ++snapshot_.missing_license_count;
            }
        }
    }
    refreshDerivedCounts();
    sortSnapshot();
}

void AssetLibrary::ingestPromotionCatalog(const nlohmann::json& catalog) {
    if (!catalog.is_object()) {
        return;
    }
    snapshot_.promotion_status = readString(catalog, "promotion_status").value_or(snapshot_.promotion_status);
    snapshot_.export_eligible = catalog.value("export_eligible", snapshot_.export_eligible);

    const auto summary = catalog.find("summary");
    if (summary != catalog.end() && summary->is_object()) {
        snapshot_.catalog_asset_count +=
            readCount(*summary, "asset_count").value_or(readCount(*summary, "asset_record_count").value_or(0));
        snapshot_.canonical_asset_count +=
            readCount(*summary, "canonical_asset_count").value_or(readCount(*summary, "asset_record_count").value_or(0));
        snapshot_.duplicate_group_count +=
            readCount(*summary, "duplicate_group_count")
                .value_or(readCount(*summary, "potential_duplicate_group_count").value_or(0));
        snapshot_.duplicate_asset_count +=
            readCount(*summary, "duplicate_asset_count")
                .value_or(readCount(*summary, "potential_duplicate_asset_count").value_or(0));
        snapshot_.unsupported_count += readCount(*summary, "unsupported_count").value_or(0);
        addCountMap(snapshot_.category_counts, readCountMap(*summary, "category_counts"));
        addCountMap(snapshot_.kind_counts, readCountMap(*summary, "kind_counts"));
        snapshot_.promotion_status = readString(*summary, "promotion_status").value_or(snapshot_.promotion_status);
        snapshot_.export_eligible = summary->value("export_eligible", snapshot_.export_eligible);
    }

    const auto shards = catalog.find("shards");
    if (shards != catalog.end() && shards->is_array()) {
        snapshot_.catalog_shard_count += shards->size();
    }

    const auto assets = catalog.find("assets");
    if (assets == catalog.end() || !assets->is_array()) {
        return;
    }

    const auto source_id = readString(catalog, "source_id").value_or("");
    const auto source_root = readString(catalog, "source_root").value_or("");
    const bool catalog_export_eligible = catalog.value("export_eligible", false);

    for (const auto& asset : *assets) {
        if (!asset.is_object()) {
            continue;
        }
        const auto source_path = readString(asset, "source_path").value_or("");
        const auto normalized_path = readString(asset, "normalized_path").value_or(source_path);
        if (source_path.empty() && normalized_path.empty()) {
            continue;
        }

        auto& record = ensureAsset(source_path.empty() ? normalized_path : source_path);
        record.source_path = source_path;
        record.normalized_path = normalized_path;
        record.preview_path = readString(asset, "preview_path").value_or("");
        record.preview_kind = readString(asset, "preview_kind").value_or("");
        record.preview_width = readInt32(asset, "preview_width");
        record.preview_height = readInt32(asset, "preview_height");
        record.duration_ms = readInt32(asset, "duration_ms");
        record.waveform_peaks = readFloatArray(asset, "waveform_peaks");
        record.media_kind = readString(asset, "media_kind").value_or("");
        record.category = readString(asset, "category").value_or("");
        record.pack = readString(asset, "pack").value_or("");
        record.duplicate_of = readString(asset, "duplicate_of").value_or("");
        record.size_bytes = readUint64(asset, "size_bytes");
        record.frame_count = readCount(asset, "frame_count").value_or(0);
        record.sequence_count = readCount(asset, "sequence_count").value_or(0);
        record.sha256 = readString(asset, "sha256").value_or("");
        record.tags = readStringArray(asset, "tags");
        record.representative_sequences = asset.value("representative_sequences", nlohmann::json::array());
        record.provenance.original_source = record.pack.empty() ? source_id : record.pack;
        record.provenance.license = readString(asset, "license").value_or("");
        record.provenance.normalized_path = normalized_path;
        record.provenance.export_eligible = asset.value("export_eligible", catalog_export_eligible);

        if (record.provenance.license.empty()) {
            record.statuses.insert(AssetStatus::MissingLicense);
            ++snapshot_.missing_license_count;
        }
        if (!record.duplicate_of.empty() || readString(asset, "status").value_or("") == "duplicate") {
            record.statuses.insert(AssetStatus::Duplicate);
        }
        if (readString(asset, "status").value_or("") == "unsupported") {
            record.statuses.insert(AssetStatus::UnsupportedFormat);
        }
        if (source_path.rfind(source_root, 0) != 0 && !source_root.empty()) {
            record.statuses.insert(AssetStatus::Risky);
        }
    }

    refreshDerivedCounts();
    sortSnapshot();
}

void AssetLibrary::ingestPromotionManifest(const AssetPromotionManifest& manifest) {
    const std::string path = !manifest.sourcePath.empty() ? manifest.sourcePath : manifest.promotedPath;
    if (path.empty()) {
        return;
    }
    auto diagnostics = manifest.diagnostics;
    for (const auto& diagnostic : validateAssetPromotionManifest(manifest)) {
        if (std::find(diagnostics.begin(), diagnostics.end(), diagnostic) == diagnostics.end()) {
            diagnostics.push_back(diagnostic);
        }
    }

    auto& record = ensureAsset(path);
    record.asset_id = manifest.assetId;
    record.source_path = manifest.sourcePath;
    record.normalized_path = manifest.promotedPath.empty() ? record.normalized_path : manifest.promotedPath;
    record.promoted_path = manifest.promotedPath;
    record.license_id = manifest.licenseId;
    record.promotion_status = toString(manifest.status);
    record.include_in_runtime = manifest.package.includeInRuntime;
    record.required_for_release = manifest.package.requiredForRelease;
    record.preview_kind = manifest.preview.kind;
    record.preview_path = manifest.preview.thumbnailPath;
    record.preview_width = manifest.preview.width;
    record.preview_height = manifest.preview.height;
    record.provenance.normalized_path = manifest.promotedPath;
    record.provenance.license = manifest.licenseId;
    record.provenance.export_eligible = manifest.package.includeInRuntime && diagnostics.empty();
    record.promotion_diagnostics = diagnostics;

    if (manifest.status == AssetPromotionStatus::RuntimeReady && diagnostics.empty()) {
        record.statuses.insert(AssetStatus::Promoted);
        record.statuses.erase(AssetStatus::Risky);
        record.statuses.erase(AssetStatus::MissingFile);
        record.statuses.erase(AssetStatus::MissingLicense);
    }
    if (manifest.status == AssetPromotionStatus::Archived) {
        record.statuses.insert(AssetStatus::Archived);
        record.statuses.erase(AssetStatus::Promoted);
        record.include_in_runtime = false;
        record.required_for_release = false;
        record.provenance.export_eligible = false;
    }
    for (const auto& diagnostic : diagnostics) {
        if (diagnostic == "license_evidence_missing") {
            record.statuses.insert(AssetStatus::MissingLicense);
            record.include_in_runtime = false;
        } else if (diagnostic == "promoted_path_missing" || diagnostic == "preview_thumbnail_missing") {
            record.statuses.insert(AssetStatus::MissingFile);
            record.include_in_runtime = false;
        } else if (diagnostic == "archived_asset_packaged") {
            record.statuses.insert(AssetStatus::Archived);
            record.include_in_runtime = false;
            record.required_for_release = false;
        }
    }

    refreshDerivedCounts();
    sortSnapshot();
}

void AssetLibrary::ingestDuplicateCsv(std::string_view csv_text) {
    std::istringstream stream{std::string(csv_text)};
    std::string line;
    bool first_line = true;
    std::map<std::string, AssetDuplicateGroup> groups;

    while (std::getline(stream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        if (first_line) {
            first_line = false;
            continue;
        }

        const auto fields = parseCsvLine(line);
        if (fields.size() < 5) {
            continue;
        }
        AssetDuplicateEntry entry{};
        entry.sha256 = fields[0];
        try {
            entry.size_bytes = static_cast<uint64_t>(std::stoull(fields[1]));
        } catch (...) {
            entry.size_bytes = 0;
        }
        entry.path = fields[2];
        entry.recommended_keep = fields[3];
        entry.recommended_remove = truthy(fields[4]);

        auto& record = ensureAsset(entry.path);
        record.sha256 = entry.sha256;
        record.size_bytes = entry.size_bytes;
        record.statuses.insert(AssetStatus::Duplicate);

        auto& group = groups[entry.sha256];
        group.sha256 = entry.sha256;
        group.size_bytes = entry.size_bytes;
        group.entries.push_back(std::move(entry));
    }

    snapshot_.duplicate_groups.clear();
    for (auto& [_, group] : groups) {
        std::sort(group.entries.begin(), group.entries.end(), [](const auto& lhs, const auto& rhs) {
            return lhs.path < rhs.path;
        });
        snapshot_.duplicate_groups.push_back(std::move(group));
    }
    std::sort(snapshot_.duplicate_groups.begin(), snapshot_.duplicate_groups.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.sha256 < rhs.sha256;
    });
    snapshot_.duplicate_group_count = snapshot_.duplicate_groups.size();
    refreshDerivedCounts();
    sortSnapshot();
}

void AssetLibrary::addReferencedAsset(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    referenced_assets_.insert(std::move(path));
    snapshot_.referenced_asset_count = referenced_assets_.size();
}

void AssetLibrary::addUsageReference(std::string path, std::string owner_id) {
    std::replace(path.begin(), path.end(), '\\', '/');
    auto& record = ensureAsset(path);
    if (!owner_id.empty() && std::find(record.used_by.begin(), record.used_by.end(), owner_id) == record.used_by.end()) {
        record.used_by.push_back(std::move(owner_id));
        std::sort(record.used_by.begin(), record.used_by.end());
    }
    referenced_assets_.insert(record.path);
    refreshDerivedCounts();
}

AssetLibraryActionResult AssetLibrary::promoteAsset(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    auto found = findAsset(path);
    if (!found.has_value()) {
        return {"promote", path, false, "asset_not_found", "Asset was not found in the library."};
    }
    std::string code;
    std::string message;
    if (!canPromoteAsset(*found, code, message)) {
        return {"promote", path, false, code, message};
    }

    auto& record = ensureAsset(path);
    record.statuses.insert(AssetStatus::Promoted);
    record.statuses.erase(AssetStatus::Risky);
    record.provenance.export_eligible = true;
    refreshDerivedCounts();
    sortSnapshot();
    return {"promote", path, true, "asset_promoted", "Asset is promoted for runtime/export library use."};
}

AssetLibraryActionResult AssetLibrary::archiveAsset(std::string path, std::string reason) {
    std::replace(path.begin(), path.end(), '\\', '/');
    auto found = findAsset(path);
    if (!found.has_value()) {
        return {"archive", path, false, "asset_not_found", "Asset was not found in the library."};
    }
    auto& record = ensureAsset(path);
    record.statuses.insert(AssetStatus::Archived);
    record.statuses.erase(AssetStatus::Promoted);
    record.provenance.export_eligible = false;
    refreshDerivedCounts();
    sortSnapshot();
    if (reason.empty()) {
        reason = "Asset archived from editor library workflow.";
    }
    return {"archive", path, true, "asset_archived", reason};
}

void AssetLibrary::markMissingFile(std::string path) {
    ensureAsset(std::move(path)).statuses.insert(AssetStatus::MissingFile);
    refreshDerivedCounts();
}

void AssetLibrary::markUnsupportedFormat(std::string path) {
    ensureAsset(std::move(path)).statuses.insert(AssetStatus::UnsupportedFormat);
    refreshDerivedCounts();
}

void AssetLibrary::detectCaseCollisions() {
    std::map<std::string, std::vector<std::string>> by_lower;
    for (const auto& asset : snapshot_.assets) {
        by_lower[lowerPath(asset.path)].push_back(asset.path);
    }

    snapshot_.case_collision_count = 0;
    for (const auto& [_, paths] : by_lower) {
        if (paths.size() < 2) {
            continue;
        }
        for (const auto& path : paths) {
            ensureAsset(path).statuses.insert(AssetStatus::CaseCollision);
            ++snapshot_.case_collision_count;
        }
    }
    refreshDerivedCounts();
    sortSnapshot();
}

std::optional<AssetRecord> AssetLibrary::findAsset(std::string_view path) const {
    for (const auto& asset : snapshot_.assets) {
        if (asset.path == path) {
            return asset;
        }
    }
    return std::nullopt;
}

std::vector<AssetRecord> AssetLibrary::filterAssets(const AssetLibraryFilter& filter) const {
    std::vector<AssetRecord> result;
    for (const auto& asset : snapshot_.assets) {
        if (!filter.media_kind.empty() && asset.media_kind != filter.media_kind) {
            continue;
        }
        if (!filter.category.empty() && asset.category != filter.category) {
            continue;
        }
        if (!filter.required_tag.empty() &&
            std::find(asset.tags.begin(), asset.tags.end(), filter.required_tag) == asset.tags.end()) {
            continue;
        }
        if (filter.required_status.has_value() && !asset.statuses.contains(*filter.required_status)) {
            continue;
        }
        if (filter.referenced_only && !referenced_assets_.contains(asset.path)) {
            continue;
        }
        if (filter.runtime_ready_only && !isRuntimeReady(asset)) {
            continue;
        }
        if (filter.previewable_only && !isPreviewable(asset)) {
            continue;
        }
        result.push_back(asset);
    }
    return result;
}

void AssetLibrary::refreshDerivedCounts() {
    snapshot_.referenced_asset_count = referenced_assets_.size();
    snapshot_.runtime_ready_count = 0;
    snapshot_.previewable_count = 0;
    snapshot_.sequence_asset_count = 0;
    snapshot_.sequence_frame_count = 0;
    snapshot_.sequence_clip_count = 0;
    snapshot_.promoted_count = 0;
    snapshot_.archived_count = 0;
    for (const auto& asset : snapshot_.assets) {
        if (isRuntimeReady(asset)) {
            ++snapshot_.runtime_ready_count;
        }
        if (isPreviewable(asset)) {
            ++snapshot_.previewable_count;
        }
        if (isSequenceAsset(asset)) {
            ++snapshot_.sequence_asset_count;
            snapshot_.sequence_frame_count += asset.frame_count;
            snapshot_.sequence_clip_count += asset.sequence_count;
        }
        if (asset.statuses.contains(AssetStatus::Promoted)) {
            ++snapshot_.promoted_count;
        }
        if (asset.statuses.contains(AssetStatus::Archived)) {
            ++snapshot_.archived_count;
        }
    }
}

void AssetLibrary::sortSnapshot() {
    std::sort(snapshot_.assets.begin(), snapshot_.assets.end(), statusLess);
}

} // namespace urpg::assets
