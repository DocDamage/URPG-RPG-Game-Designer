#pragma once

#include <filesystem>
#include <cstdint>
#include <string>
#include <vector>

namespace urpg::assets {

enum class RelinkConfidence { High, Medium, Low };

struct RelinkCandidate {
    std::filesystem::path path;
    RelinkConfidence confidence = RelinkConfidence::Low;
};

struct MissingAsset {
    std::string asset_id;
    std::filesystem::path manifest_path;
    std::filesystem::path recorded_promoted_path;
    std::string source_sha256;
    uint64_t recorded_size_bytes = 0;
    std::vector<std::filesystem::path> affected_reference_paths;
    std::vector<RelinkCandidate> candidates;
};

class AssetRelinkService {
public:
    std::vector<MissingAsset> scanMissingAssets(const std::filesystem::path& project_root) const;
    bool applyRelink(const std::filesystem::path& project_root,
                     const std::string& asset_id,
                     const std::filesystem::path& new_payload_path);
    bool undoLastRelink(const std::filesystem::path& project_root, const std::string& asset_id);

    static std::string calculateSha256(const std::filesystem::path& path);
};

} // namespace urpg::assets
