#include "engine/core/assets/global_asset_library_store.h"

#include <algorithm>
#include <fstream>
#include <map>

namespace urpg::assets {

namespace {

std::string sanitizeSegment(std::string value) {
    for (auto& ch : value) {
        const bool keep = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
                          ch == '-' || ch == '_' || ch == '.';
        if (!keep) {
            ch = '-';
        }
    }
    return value.empty() ? "asset" : value;
}

GlobalAssetLibraryStoreResult failed(std::string code, std::string message, std::filesystem::path path = {}) {
    GlobalAssetLibraryStoreResult result;
    result.success = false;
    result.code = std::move(code);
    result.message = std::move(message);
    result.path = std::move(path);
    return result;
}

GlobalAssetLibraryStoreResult writeJsonFile(const std::filesystem::path& path, const nlohmann::json& value,
                                            std::string successCode) {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    if (error) {
        return failed("global_library_directory_create_failed", error.message(), path);
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) {
        return failed("global_library_manifest_open_failed", "Global asset library manifest could not be opened.", path);
    }
    out << value.dump(2);
    out << '\n';
    if (!out) {
        return failed("global_library_manifest_write_failed", "Global asset library manifest could not be written.", path);
    }

    GlobalAssetLibraryStoreResult result;
    result.success = true;
    result.code = std::move(successCode);
    result.message = "Global asset library manifest was written.";
    result.path = path;
    return result;
}

std::vector<std::filesystem::path> discoverImportSessionManifests(const std::filesystem::path& libraryRoot) {
    std::vector<std::filesystem::path> manifests;
    const auto catalogRoot = libraryRoot / "catalog" / "import_sessions";
    if (std::filesystem::is_directory(catalogRoot)) {
        for (const auto& entry : std::filesystem::directory_iterator(catalogRoot)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                manifests.push_back(entry.path());
            }
        }
    }
    const auto sourcesRoot = libraryRoot / "sources";
    if (std::filesystem::is_directory(sourcesRoot)) {
        for (const auto& entry : std::filesystem::directory_iterator(sourcesRoot)) {
            if (!entry.is_directory()) {
                continue;
            }
            const auto manifest = entry.path() / "source_manifest.json";
            if (std::filesystem::is_regular_file(manifest)) {
                manifests.push_back(manifest);
            }
        }
    }
    std::sort(manifests.begin(), manifests.end());
    return manifests;
}

std::vector<std::filesystem::path> discoverPromotedAssetManifests(const std::filesystem::path& libraryRoot) {
    std::vector<std::filesystem::path> manifests;
    const auto promotedRoot = libraryRoot / "promoted";
    if (!std::filesystem::is_directory(promotedRoot)) {
        return manifests;
    }
    for (const auto& entry : std::filesystem::directory_iterator(promotedRoot)) {
        if (!entry.is_directory()) {
            continue;
        }
        const auto manifest = entry.path() / "asset_promotion_manifest.json";
        if (std::filesystem::is_regular_file(manifest)) {
            manifests.push_back(manifest);
        }
    }
    std::sort(manifests.begin(), manifests.end());
    return manifests;
}

} // namespace

GlobalAssetLibraryStore::GlobalAssetLibraryStore(std::filesystem::path libraryRoot)
    : library_root_(std::move(libraryRoot)) {}

std::filesystem::path GlobalAssetLibraryStore::catalogDatabasePath() const {
    return library_root_.parent_path() / "asset-index" / "asset_catalog.db";
}

std::filesystem::path GlobalAssetLibraryStore::importSessionManifestPath(std::string_view sessionId) const {
    return library_root_ / "catalog" / "import_sessions" / (sanitizeSegment(std::string(sessionId)) + ".json");
}

std::filesystem::path GlobalAssetLibraryStore::sourceManifestPath(std::string_view sessionId) const {
    return library_root_ / "sources" / sanitizeSegment(std::string(sessionId)) / "source_manifest.json";
}

std::filesystem::path GlobalAssetLibraryStore::promotedAssetManifestPath(std::string_view assetId) const {
    return library_root_ / "promoted" / sanitizeSegment(std::string(assetId)) / "asset_promotion_manifest.json";
}

GlobalAssetLibraryStoreResult GlobalAssetLibraryStore::writeImportSession(const AssetImportSession& session) const {
    if (session.sessionId.empty()) {
        return failed("import_session_id_missing", "Import session id is required.");
    }
    const auto catalogResult =
        writeJsonFile(importSessionManifestPath(session.sessionId), serializeAssetImportSession(session),
                      "import_session_written");
    if (!catalogResult.success) {
        return catalogResult;
    }
    return writeJsonFile(sourceManifestPath(session.sessionId), serializeAssetImportSession(session),
                         "import_session_written");
}

GlobalAssetLibraryStoreResult GlobalAssetLibraryStore::writePromotedAssetManifest(
    const AssetPromotionManifest& manifest) const {
    if (manifest.assetId.empty()) {
        return failed("promoted_asset_id_missing", "Promoted asset id is required.");
    }
    return writeJsonFile(promotedAssetManifestPath(manifest.assetId), serializeAssetPromotionManifest(manifest),
                         "promoted_asset_manifest_written");
}

std::vector<AssetImportSession> GlobalAssetLibraryStore::loadImportSessions() const {
    std::map<std::string, AssetImportSession> sessionsById;
    for (const auto& manifestPath : discoverImportSessionManifests(library_root_)) {
        std::ifstream in(manifestPath, std::ios::binary);
        if (!in) {
            continue;
        }
        auto session = deserializeAssetImportSession(nlohmann::json::parse(in));
        if (!session.sessionId.empty()) {
            sessionsById[session.sessionId] = std::move(session);
        }
    }
    std::vector<AssetImportSession> sessions;
    sessions.reserve(sessionsById.size());
    for (auto& [_, session] : sessionsById) {
        sessions.push_back(std::move(session));
    }
    return sessions;
}

std::vector<AssetPromotionManifest> GlobalAssetLibraryStore::loadPromotedAssetManifests() const {
    std::vector<AssetPromotionManifest> manifests;
    for (const auto& manifestPath : discoverPromotedAssetManifests(library_root_)) {
        std::ifstream in(manifestPath, std::ios::binary);
        if (!in) {
            continue;
        }
        manifests.push_back(deserializeAssetPromotionManifest(nlohmann::json::parse(in)));
    }
    return manifests;
}

} // namespace urpg::assets
