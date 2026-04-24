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
