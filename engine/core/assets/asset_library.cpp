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

bool statusLess(const AssetRecord& lhs, const AssetRecord& rhs) {
    return lhs.path < rhs.path;
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
    }
    return "risky";
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
    sortSnapshot();
}

void AssetLibrary::ingestPromotionCatalog(const nlohmann::json& catalog) {
    if (!catalog.is_object()) {
        return;
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
        record.media_kind = readString(asset, "media_kind").value_or("");
        record.category = readString(asset, "category").value_or("");
        record.pack = readString(asset, "pack").value_or("");
        record.duplicate_of = readString(asset, "duplicate_of").value_or("");
        record.size_bytes = readUint64(asset, "size_bytes");
        record.sha256 = readString(asset, "sha256").value_or("");
        record.tags = readStringArray(asset, "tags");
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
    sortSnapshot();
}

void AssetLibrary::addReferencedAsset(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    referenced_assets_.insert(std::move(path));
}

void AssetLibrary::markMissingFile(std::string path) {
    ensureAsset(std::move(path)).statuses.insert(AssetStatus::MissingFile);
}

void AssetLibrary::markUnsupportedFormat(std::string path) {
    ensureAsset(std::move(path)).statuses.insert(AssetStatus::UnsupportedFormat);
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

void AssetLibrary::sortSnapshot() {
    std::sort(snapshot_.assets.begin(), snapshot_.assets.end(), statusLess);
}

} // namespace urpg::assets
