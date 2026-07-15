#include "engine/core/assets/asset_relink_service.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/security/sha256.h"
#include "engine/core/save/save_journal.h"

#include <cctype>
#include <chrono>
#include <fstream>
#include <algorithm>
#include <system_error>

namespace urpg::assets {

namespace {

bool pathInside(const std::filesystem::path& root, const std::filesystem::path& candidate) {
    std::error_code ec;
    const auto normalizedRoot = std::filesystem::weakly_canonical(root, ec);
    if (ec) return false;
    const auto normalizedCandidate = std::filesystem::weakly_canonical(candidate, ec);
    if (ec) return false;
    return normalizedCandidate == normalizedRoot ||
        std::mismatch(normalizedRoot.begin(), normalizedRoot.end(), normalizedCandidate.begin()).first ==
            normalizedRoot.end();
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

uint64_t recordedSize(const AssetPromotionManifest& manifest) {
    for (const auto* key : {"source_size_bytes", "size_bytes", "sizeBytes"}) {
        if (const auto found = manifest.authoredMetadata.find(key);
            found != manifest.authoredMetadata.end() && found->is_number_unsigned()) {
            return found->get<uint64_t>();
        }
    }
    return 0;
}

std::string safeFileSegment(std::string value) {
    for (auto& ch : value) {
        const auto byte = static_cast<unsigned char>(ch);
        if (!std::isalnum(byte) && ch != '-' && ch != '_') ch = '-';
    }
    return value.empty() ? "asset" : value;
}

} // namespace

std::string AssetRelinkService::calculateSha256(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return "";
    }
    std::vector<std::uint8_t> buffer((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return urpg::security::Sha256::toHex(urpg::security::Sha256::compute(buffer));
}

std::vector<MissingAsset> AssetRelinkService::scanMissingAssets(const std::filesystem::path& project_root) const {
    std::vector<MissingAsset> result;
    if (project_root.empty() || !std::filesystem::exists(project_root)) {
        return result;
    }

    const auto manifest_dir = project_root / "content" / "assets" / "manifests";
    if (!std::filesystem::exists(manifest_dir)) {
        return result;
    }

    // Collect all manifests and identify missing ones
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(manifest_dir, ec)) {
        if (entry.is_regular_file() && entry.path().extension() == ".json") {
            std::ifstream in(entry.path(), std::ios::binary);
            if (!in) {
                continue;
            }
            nlohmann::json val = nlohmann::json::parse(in, nullptr, false);
            if (val.is_discarded() || !val.is_object()) {
                continue;
            }

            auto manifest = deserializeAssetPromotionManifest(val);
            // Verify if payload exists. Under project attachments, promotedPath holds the destination payload path.
            // If it is absolute, verify directly; if relative, check relative to project root.
            std::filesystem::path p_path = manifest.promotedPath;
            if (p_path.is_relative()) {
                p_path = project_root / p_path;
            }

            if (manifest.promotedPath.empty() || !std::filesystem::exists(p_path)) {
                MissingAsset missing;
                missing.asset_id = manifest.assetId;
                missing.manifest_path = entry.path();
                missing.recorded_promoted_path = manifest.promotedPath;
                missing.source_sha256 = manifest.sourceSha256;
                missing.recorded_size_bytes = recordedSize(manifest);
                missing.affected_reference_paths.push_back(entry.path());
                result.push_back(std::move(missing));
            }
        }
    }

    if (result.empty()) {
        return result;
    }

    // Collect all candidate files in content/assets/imported/
    const auto imported_dir = project_root / "content" / "assets" / "imported";
    std::vector<std::filesystem::path> candidate_files;
    if (std::filesystem::exists(imported_dir)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(imported_dir, ec)) {
            if (entry.is_regular_file()) {
                candidate_files.push_back(entry.path());
            }
        }
    }

    // For each missing asset, find matching candidates
    for (auto& missing : result) {
        std::filesystem::path old_filename = missing.recorded_promoted_path.filename();

        for (const auto& candidate : candidate_files) {
            RelinkCandidate c;
            c.path = candidate;

            // Match by Hash (High Confidence)
            if (!missing.source_sha256.empty()) {
                std::string c_hash = calculateSha256(candidate);
                if (c_hash == missing.source_sha256) {
                    c.confidence = RelinkConfidence::High;
                    missing.candidates.push_back(std::move(c));
                    continue;
                }
            }

            const bool nameMatches = lowercase(candidate.filename().string()) == lowercase(old_filename.string());
            const auto candidateSize = std::filesystem::file_size(candidate, ec);
            if (ec) {
                ec.clear();
                continue;
            }

            // Size plus name is an explicitly lower-confidence suggestion than a content hash.
            if (missing.recorded_size_bytes > 0 && candidateSize == missing.recorded_size_bytes && nameMatches) {
                c.confidence = RelinkConfidence::Medium;
                missing.candidates.push_back(std::move(c));
                continue;
            }

            if (nameMatches) {
                c.confidence = RelinkConfidence::Low;
                missing.candidates.push_back(std::move(c));
            }
        }

        // Sort candidates so High confidence is first, then Medium, then Low
        std::sort(missing.candidates.begin(), missing.candidates.end(), [](const auto& a, const auto& b) {
            return static_cast<int>(a.confidence) < static_cast<int>(b.confidence);
        });
    }

    return result;
}

bool AssetRelinkService::applyRelink(const std::filesystem::path& project_root,
                                     const std::string& asset_id,
                                     const std::filesystem::path& new_payload_path) {
    if (project_root.empty() || asset_id.empty() || new_payload_path.empty()) {
        return false;
    }

    const auto importedRoot = project_root / "content" / "assets" / "imported";
    if (!std::filesystem::is_regular_file(new_payload_path) || !pathInside(importedRoot, new_payload_path)) {
        return false;
    }

    const auto manifestRoot = project_root / "content" / "assets" / "manifests";
    std::filesystem::path manifest_path;
    AssetPromotionManifest manifest;
    nlohmann::json originalManifest;
    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(manifestRoot, ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != ".json") continue;
        std::ifstream in(entry.path(), std::ios::binary);
        const nlohmann::json value = nlohmann::json::parse(in, nullptr, false);
        if (!value.is_object()) continue;
        auto candidateManifest = deserializeAssetPromotionManifest(value);
        if (candidateManifest.assetId == asset_id) {
            manifest_path = entry.path();
            manifest = std::move(candidateManifest);
            originalManifest = value;
            break;
        }
    }
    if (manifest_path.empty()) {
        return false;
    }

    // Normalize new path relative to project root
    std::filesystem::path relative_payload = std::filesystem::relative(new_payload_path, project_root, ec);
    if (ec || relative_payload.empty() || relative_payload.is_absolute()) {
        return false;
    }
    manifest.promotedPath = relative_payload.generic_string();

    const auto& before = originalManifest;
    auto after = originalManifest;
    after["promotedPath"] = manifest.promotedPath;
    const auto historyRoot = project_root / ".urpg" / "asset-relink-history";
    const auto historyPath = historyRoot /
        (safeFileSegment(asset_id) + "-" +
         std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".json");
    const auto relativeManifestPath = std::filesystem::relative(manifest_path, project_root, ec);
    if (ec || relativeManifestPath.empty()) return false;
    const nlohmann::json history = {
        {"schema_version", "urpg.asset_relink_history.v1"},
        {"asset_id", asset_id},
        {"manifest_path", relativeManifestPath.generic_string()},
        {"before", before},
        {"after", after},
    };

    std::string writeError;
    if (!urpg::SaveJournal::WriteAtomically(historyPath, history.dump(2) + "\n", &writeError)) {
        return false;
    }
    if (urpg::SaveJournal::WriteAtomically(manifest_path, after.dump(2) + "\n", &writeError)) {
        return true;
    }
    std::filesystem::remove(historyPath, ec);
    return false;
}

bool AssetRelinkService::undoLastRelink(const std::filesystem::path& project_root, const std::string& asset_id) {
    if (project_root.empty() || asset_id.empty()) return false;
    const auto historyRoot = project_root / ".urpg" / "asset-relink-history";
    std::error_code ec;
    std::filesystem::path newest;
    std::filesystem::file_time_type newestTime{};
    for (const auto& entry : std::filesystem::directory_iterator(historyRoot, ec)) {
        if (!entry.is_regular_file(ec) || entry.path().extension() != ".json") continue;
        std::ifstream input(entry.path(), std::ios::binary);
        const auto history = nlohmann::json::parse(input, nullptr, false);
        if (!history.is_object() || history.value("schema_version", "") != "urpg.asset_relink_history.v1" ||
            history.value("asset_id", "") != asset_id) {
            continue;
        }
        const auto writeTime = entry.last_write_time(ec);
        if (!ec && (newest.empty() || writeTime > newestTime)) {
            newest = entry.path();
            newestTime = writeTime;
        }
        ec.clear();
    }
    if (newest.empty()) return false;

    std::ifstream input(newest, std::ios::binary);
    const auto history = nlohmann::json::parse(input, nullptr, false);
    if (!history.is_object() || !history.contains("before") || !history["before"].is_object()) return false;
    const auto relativeManifest = std::filesystem::path(history.value("manifest_path", ""));
    if (relativeManifest.empty() || relativeManifest.is_absolute()) return false;
    const auto manifestPath = (project_root / relativeManifest).lexically_normal();
    const auto manifestRoot = project_root / "content" / "assets" / "manifests";
    if (!pathInside(manifestRoot, manifestPath)) return false;

    std::string writeError;
    if (!urpg::SaveJournal::WriteAtomically(manifestPath, history["before"].dump(2) + "\n", &writeError)) {
        return false;
    }
    std::filesystem::remove(newest, ec);
    return true;
}

} // namespace urpg::assets
