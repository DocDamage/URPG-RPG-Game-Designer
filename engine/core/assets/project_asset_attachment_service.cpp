#include "engine/core/assets/project_asset_attachment_service.h"
#include "engine/core/security/sha256.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <sstream>
#include <stb_image.h>
#include <stb_image_write.h>
#include <string_view>
#include <vector>

namespace urpg::assets {

namespace {

std::atomic_uint64_t attachmentStagingSequence{0};
constexpr int kRuntimeTilesetCellSize = 48;

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

bool pathInside(const std::filesystem::path& root, const std::filesystem::path& child) {
    const auto rootNormalized = std::filesystem::weakly_canonical(root);
    const auto childNormalized = std::filesystem::weakly_canonical(child);
    return childNormalized == rootNormalized ||
           std::mismatch(rootNormalized.begin(), rootNormalized.end(), childNormalized.begin()).first ==
               rootNormalized.end();
}

ProjectAssetAttachmentResult blocked(std::string code, std::string message, std::vector<std::string> diagnostics = {}) {
    ProjectAssetAttachmentResult result;
    result.success = false;
    result.code = std::move(code);
    result.message = std::move(message);
    result.diagnostics = std::move(diagnostics);
    return result;
}

std::string hashFile(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    const std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(input), {});
    return security::Sha256::toHex(security::Sha256::compute(bytes));
}

std::string attachmentRevision(const AssetPromotionManifest& manifest,
                               const std::filesystem::path& payload,
                               const std::filesystem::path& attachmentManifest) {
    nlohmann::json identity = {{"schema", "urpg.project_asset_attachment_revision.v1"},
                               {"asset_id", manifest.assetId},
                               {"reviewed_source", manifest.sourcePath},
                               {"promoted_payload", manifest.promotedPath},
                               {"promoted_payload_sha256", hashFile(manifest.promotedPath)},
                               {"destination_payload_sha256", std::filesystem::is_regular_file(payload) ? hashFile(payload) : ""},
                               {"destination_manifest_sha256",
                                std::filesystem::is_regular_file(attachmentManifest) ? hashFile(attachmentManifest) : ""}};
    const auto serialized = identity.dump();
    return security::Sha256::toHex(security::Sha256::compute({serialized.begin(), serialized.end()}));
}

bool isSafeOperationId(const std::string& value) {
    return !value.empty() && std::all_of(value.begin(), value.end(), [](const unsigned char character) {
        return std::isalnum(character) || character == '_' || character == '-' || character == '.';
    });
}

std::filesystem::path operationReceiptPath(const std::filesystem::path& projectRoot, const std::string& operationId) {
    return projectRoot / ".urpg" / "asset-attachment-operations" / (operationId + ".json");
}

std::string requestFingerprint(const ProjectAssetAttachmentRequest& request, const std::string& sourceRevision) {
    const nlohmann::json value = {{"schema", "urpg.project_asset_attachment_request.v1"},
                                  {"asset", serializeAssetPromotionManifest(request.manifest)},
                                  {"project_root", request.projectRoot.generic_string()},
                                  {"policy", static_cast<int>(request.conflictPolicy)},
                                  {"source_revision", sourceRevision}};
    const auto serialized = value.dump();
    return security::Sha256::toHex(security::Sha256::compute({serialized.begin(), serialized.end()}));
}

std::filesystem::path siblingWorkingPath(const std::filesystem::path& target, const std::string_view purpose) {
    const auto sequence = attachmentStagingSequence.fetch_add(1, std::memory_order_relaxed);
    const auto timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
                               std::chrono::steady_clock::now().time_since_epoch())
                               .count();
    // Keep transactional sibling names independent of the destination's name.
    // Derived manifests and reference markers already contain long stable IDs;
    // prefixing those names again can exceed the legacy Windows path limit and
    // make an otherwise valid atomic stage fail before revision admission.
    return target.parent_path() / (".urpg-" + std::string(purpose) + "-" + std::to_string(timestamp) + "-" +
                                   std::to_string(sequence));
}

void removeIfPresent(const std::filesystem::path& path) {
    std::error_code error;
    std::filesystem::remove_all(path, error);
}

bool writeJsonFile(const std::filesystem::path& path, const nlohmann::json& value) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << value.dump(2) << '\n';
    return static_cast<bool>(output);
}

std::filesystem::path attachmentJournalRoot(const std::filesystem::path& projectRoot) {
    return projectRoot / ".urpg" / "asset-attachment-transactions";
}

bool isSafeRelativePath(const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute()) return false;
    return std::none_of(path.begin(), path.end(), [](const auto& part) { return part == ".."; });
}

std::string hashText(const std::string& value);

struct DerivedAttachmentCandidate {
    bool valid = false;
    std::string code;
    std::string message;
    std::vector<std::string> diagnostics;
    AssetPromotionManifest manifest;
};

bool isSha256Hex(const std::string& value) {
    return value.size() == 64 &&
           std::all_of(value.begin(), value.end(), [](const unsigned char character) { return std::isxdigit(character); });
}

struct DerivedTilesetCandidate {
    DerivedTilesetCandidate() = default;
    DerivedTilesetCandidate(bool isValid, std::string diagnosticCode, std::string diagnosticMessage,
                            std::vector<std::string> candidateDiagnostics = {})
        : valid(isValid), code(std::move(diagnosticCode)), message(std::move(diagnosticMessage)),
          diagnostics(std::move(candidateDiagnostics)) {}

    bool valid = false;
    std::string code;
    std::string message;
    std::vector<std::string> diagnostics;
    std::string sourceAssetId;
    std::string tilesetId;
    std::string sourceRevision;
    std::string derivedRevision;
    std::string reviewRevision;
    int columns = 0;
    int rows = 0;
    int tileWidth = 0;
    int tileHeight = 0;
    std::filesystem::path tileDirectory;
    std::filesystem::path manifestPath;
    std::vector<std::filesystem::path> tilePaths;
};

std::string tilesetFilename(const size_t index) {
    std::ostringstream output;
    output << std::setw(6) << std::setfill('0') << index << ".png";
    return output.str();
}

DerivedTilesetCandidate derivedTilesetCandidate(const AssetPromotionManifest& source,
                                                const std::filesystem::path& derivedManifestPath) {
    auto sourceDiagnostics = validateAssetPromotionManifest(source);
    sourceDiagnostics.insert(sourceDiagnostics.end(), source.diagnostics.begin(), source.diagnostics.end());
    if (!sourceDiagnostics.empty() || source.status != AssetPromotionStatus::RuntimeReady || !source.package.includeInRuntime) {
        return {false, "asset_derived_tileset_source_invalid",
                "Only a valid runtime-ready promoted asset can supply a derived tileset.", sourceDiagnostics};
    }
    if (!std::filesystem::is_regular_file(source.promotedPath)) {
        return {false, "asset_derived_tileset_source_payload_missing", "The promoted source payload is missing."};
    }
    if (!std::filesystem::is_regular_file(derivedManifestPath)) {
        return {false, "asset_derived_tileset_manifest_missing", "The derived tileset manifest is missing."};
    }
    try {
        std::ifstream input(derivedManifestPath, std::ios::binary);
        const auto revision = nlohmann::json::parse(input, nullptr, false);
        if (revision.is_discarded() || revision.value("schema", "") != "urpg.asset_transform_revision.v1" ||
            revision.value("operation", "") != "tileset_slice") {
            return {false, "asset_derived_tileset_manifest_invalid", "The derived revision is not a valid tileset slice."};
        }
        const auto derivedRevision = revision.value("derived_revision", "");
        const auto sourceRevision = hashFile(source.promotedPath);
        if (!isSha256Hex(derivedRevision) || revision.value("source_asset_id", "") != source.assetId ||
            revision.value("source_revision", "") != sourceRevision || !revision.contains("grid") ||
            !revision["grid"].is_object() || !revision.contains("output_paths") || !revision["output_paths"].is_array()) {
            return {false, "asset_derived_tileset_provenance_invalid",
                    "The derived tileset no longer matches its reviewed promoted source or manifest."};
        }
        const auto& grid = revision["grid"];
        const int columns = grid.value("columns", 0);
        const int rows = grid.value("rows", 0);
        const int tileWidth = grid.value("tile_width", 0);
        const int tileHeight = grid.value("tile_height", 0);
        const int margin = grid.value("margin", -1);
        const int spacing = grid.value("spacing", -1);
        const int tileCount = grid.value("tile_count", 0);
        const nlohmann::json identity = {
            {"schema", "urpg.asset_transform_revision.v1"},
            {"operation", "tileset_slice"},
            {"operation_id", revision.value("operation_id", "")},
            {"source_asset_id", source.assetId},
            {"source_revision", sourceRevision},
            {"tile_width", tileWidth},
            {"tile_height", tileHeight},
            {"margin", margin},
            {"spacing", spacing},
        };
        if (revision.value("operation_id", "").empty() || columns < 1 || rows < 1 || tileWidth < 1 ||
            tileHeight < 1 || margin < 0 || spacing < 0 || tileCount != columns * rows ||
            revision["output_paths"].size() != static_cast<size_t>(tileCount) || derivedRevision != hashText(identity.dump())) {
            return {false, "asset_derived_tileset_grid_invalid", "The derived tileset grid is incomplete or invalid."};
        }
        std::error_code error;
        const auto manifestDirectory = std::filesystem::weakly_canonical(derivedManifestPath.parent_path(), error);
        const auto expectedTileDirectory = manifestDirectory / (derivedRevision + ".tiles");
        const auto tileDirectory = std::filesystem::weakly_canonical(expectedTileDirectory, error);
        if (error || !std::filesystem::is_directory(tileDirectory)) {
            return {false, "asset_derived_tileset_output_missing", "The derived tileset output directory is missing."};
        }
        std::vector<std::filesystem::path> tilePaths;
        nlohmann::json tileHashes = nlohmann::json::array();
        tilePaths.reserve(static_cast<size_t>(tileCount));
        for (size_t index = 0; index < static_cast<size_t>(tileCount); ++index) {
            if (!revision["output_paths"][index].is_string()) {
                return {false, "asset_derived_tileset_output_invalid", "A derived tileset output path is invalid."};
            }
            const auto tilePath = std::filesystem::weakly_canonical(
                std::filesystem::path(revision["output_paths"][index].get<std::string>()), error);
            if (error || !std::filesystem::is_regular_file(tilePath) || tilePath.parent_path() != tileDirectory ||
                tilePath.filename() != tilesetFilename(index)) {
                return {false, "asset_derived_tileset_output_invalid",
                        "The derived tileset output paths do not match the deterministic tile bundle."};
            }
            tileHashes.push_back(hashFile(tilePath));
            tilePaths.push_back(tilePath);
        }
        const nlohmann::json reviewIdentity = {
            {"schema", "urpg.project_derived_tileset_assignment_review.v1"},
            {"source_asset_id", source.assetId},
            {"source_revision", sourceRevision},
            {"derived_revision", derivedRevision},
            {"grid", grid},
            {"tile_sha256", tileHashes},
        };
        DerivedTilesetCandidate candidate;
        candidate.valid = true;
        candidate.sourceAssetId = source.assetId;
        candidate.tilesetId = source.assetId + ".tileset." + derivedRevision.substr(0, 16);
        candidate.sourceRevision = sourceRevision;
        candidate.derivedRevision = derivedRevision;
        candidate.reviewRevision = hashText(reviewIdentity.dump());
        candidate.columns = columns;
        candidate.rows = rows;
        candidate.tileWidth = tileWidth;
        candidate.tileHeight = tileHeight;
        candidate.tileDirectory = tileDirectory;
        candidate.manifestPath = std::filesystem::weakly_canonical(derivedManifestPath, error);
        candidate.tilePaths = std::move(tilePaths);
        if (error) {
            return {false, "asset_derived_tileset_manifest_path_invalid",
                    "The derived tileset manifest path could not be resolved."};
        }
        return candidate;
    } catch (const nlohmann::json::exception&) {
        return {false, "asset_derived_tileset_manifest_invalid", "The derived tileset manifest is malformed."};
    }
}

DerivedAttachmentCandidate derivedAttachmentCandidate(const AssetPromotionManifest& source,
                                                      const std::filesystem::path& derivedManifestPath) {
    auto sourceDiagnostics = validateAssetPromotionManifest(source);
    sourceDiagnostics.insert(sourceDiagnostics.end(), source.diagnostics.begin(), source.diagnostics.end());
    if (!sourceDiagnostics.empty() ||
        source.status != AssetPromotionStatus::RuntimeReady || !source.package.includeInRuntime) {
        return {false, "asset_derived_attachment_source_invalid",
                "Only a valid runtime-ready promoted asset can supply a derived attachment.", sourceDiagnostics, {}};
    }
    if (!std::filesystem::is_regular_file(source.promotedPath)) {
        return {false, "asset_derived_attachment_source_payload_missing", "The promoted source payload is missing.", {}, {}};
    }
    if (!std::filesystem::is_regular_file(derivedManifestPath)) {
        return {false, "asset_derived_revision_manifest_missing", "The derived revision manifest is missing.", {}, {}};
    }

    std::ifstream input(derivedManifestPath, std::ios::binary);
    const auto revision = nlohmann::json::parse(input, nullptr, false);
    if (revision.is_discarded() || revision.value("schema", "") != "urpg.asset_transform_revision.v1") {
        return {false, "asset_derived_revision_manifest_invalid", "The derived revision manifest is invalid.", {}, {}};
    }
    const auto operation = revision.value("operation", "");
    if (operation != "image_crop_scale" && operation != "image_palette" && operation != "image_palette_extract" &&
        operation != "audio_trim_fade_gain_pcm16") {
        return {false, "asset_derived_revision_not_attachable",
                "This revision has no single media output that can be attached through the project asset owner.", {}, {}};
    }
    const auto derivedRevision = revision.value("derived_revision", "");
    const auto outputPath = std::filesystem::path(revision.value("output_path", ""));
    if (derivedRevision.size() != 64 ||
        !std::all_of(derivedRevision.begin(), derivedRevision.end(), [](unsigned char character) { return std::isxdigit(character); }) ||
        revision.value("source_asset_id", "") != source.assetId || revision.value("source_revision", "") != hashFile(source.promotedPath) ||
        outputPath.empty() || !std::filesystem::is_regular_file(outputPath)) {
        return {false, "asset_derived_revision_provenance_invalid",
                "The derived revision no longer matches its reviewed promoted source or output.", {}, {}};
    }

    std::error_code error;
    const auto manifestDirectory = std::filesystem::weakly_canonical(derivedManifestPath.parent_path(), error);
    const auto canonicalOutput = std::filesystem::weakly_canonical(outputPath, error);
    const auto expectedFilename = derivedRevision + (operation == "audio_trim_fade_gain_pcm16" ? ".wav" : ".png");
    if (error || canonicalOutput.parent_path() != manifestDirectory || canonicalOutput.filename() != expectedFilename) {
        return {false, "asset_derived_revision_output_invalid",
                "The derived output must be the deterministic single file beside its revision manifest.", {}, {}};
    }

    AssetPromotionManifest attached = source;
    attached.assetId = source.assetId + ".revision." + derivedRevision.substr(0, 16);
    attached.sourcePath = source.promotedPath;
    attached.sourceSha256 = hashFile(outputPath);
    attached.promotedPath = canonicalOutput.generic_string();
    attached.preview.thumbnailPath = attached.promotedPath;
    attached.authoredMetadata["derived_revision"] = {
        {"schema", "urpg.project_asset_derived_revision.v1"},
        {"operation", operation},
        {"source_asset_id", source.assetId},
        {"source_revision", revision.value("source_revision", "")},
        {"derived_revision", derivedRevision},
        {"manifest_path", std::filesystem::weakly_canonical(derivedManifestPath).generic_string()},
        {"output_sha256", attached.sourceSha256},
    };
    return {true, {}, {}, {}, std::move(attached)};
}

std::string hashText(const std::string& value) {
    return security::Sha256::toHex(security::Sha256::compute({value.begin(), value.end()}));
}

std::filesystem::path tilesetAssignmentReceiptPath(const std::filesystem::path& projectRoot,
                                                   const std::string& operationId) {
    return projectRoot / ".urpg" / "tileset-assignment-operations" / (operationId + ".json");
}

std::string tilesetAssignmentFingerprint(const ProjectDerivedTilesetAssignmentRequest& request,
                                         const std::string& reviewRevision) {
    const nlohmann::json value = {
        {"schema", "urpg.project_derived_tileset_assignment_request.v1"},
        {"asset", serializeAssetPromotionManifest(request.source)},
        {"derived_manifest_path", request.derivedManifestPath.generic_string()},
        {"project_root", request.projectRoot.generic_string()},
        {"policy", static_cast<int>(request.conflictPolicy)},
        {"review_revision", reviewRevision},
        {"expected_source_revision", request.expectedSourceRevision},
    };
    return hashText(value.dump());
}

nlohmann::json projectTilesetManifest(const DerivedTilesetCandidate& candidate, const std::string& tilesetId,
                                      const std::filesystem::path& projectRoot, const std::string& atlasSha256) {
    nlohmann::json tilePaths = nlohmann::json::array();
    for (size_t index = 0; index < candidate.tilePaths.size(); ++index) {
        tilePaths.push_back({{"index", index},
                             {"path", (std::filesystem::path("content") / "tilesets" / sanitizeSegment(tilesetId) /
                                       "tiles" / tilesetFilename(index))
                                          .generic_string()},
                             {"sha256", hashFile(candidate.tilePaths[index])}});
    }
    return {
        {"schema", "urpg.project_derived_tileset_assignment.v1"},
        {"tileset_id", tilesetId},
        {"source_asset_id", candidate.sourceAssetId},
        {"source_revision", candidate.sourceRevision},
        {"review_revision", candidate.reviewRevision},
        {"derived_revision", candidate.derivedRevision},
        {"derived_manifest_path", candidate.manifestPath.generic_string()},
        {"grid", {{"columns", candidate.columns}, {"rows", candidate.rows}, {"tile_width", candidate.tileWidth},
                  {"tile_height", candidate.tileHeight}, {"tile_count", candidate.tilePaths.size()}}},
        {"tile_paths", std::move(tilePaths)},
        {"atlas", {{"path", (std::filesystem::path("content") / "tilesets" / sanitizeSegment(tilesetId) / "atlas.png")
                                 .generic_string()},
                   {"sha256", atlasSha256},
                   {"width", candidate.columns * kRuntimeTilesetCellSize},
                   {"height", candidate.rows * kRuntimeTilesetCellSize},
                   {"cell_width", kRuntimeTilesetCellSize},
                   {"cell_height", kRuntimeTilesetCellSize}}},
        {"project_root", projectRoot.generic_string()},
    };
}

bool packTilesetAtlas(const DerivedTilesetCandidate& candidate, const std::filesystem::path& atlasPath,
                      std::string& failureMessage) {
    if (candidate.columns <= 0 || candidate.rows <= 0 || candidate.columns > 16384 / kRuntimeTilesetCellSize ||
        candidate.rows > 16384 / kRuntimeTilesetCellSize) {
        failureMessage = "The tileset atlas dimensions are invalid or exceed the native packing limit.";
        return false;
    }
    const auto atlasWidth = candidate.columns * kRuntimeTilesetCellSize;
    const auto atlasHeight = candidate.rows * kRuntimeTilesetCellSize;
    const auto pixelCount = static_cast<size_t>(atlasWidth) * static_cast<size_t>(atlasHeight);
    if (pixelCount > (std::numeric_limits<size_t>::max() / 4U) || pixelCount > (1U << 28U)) {
        failureMessage = "The tileset atlas exceeds the native packing memory limit.";
        return false;
    }
    std::vector<std::uint8_t> atlas(pixelCount * 4U, 0U);
    // Assignment writes a canonical top-origin PNG. AssetLoader performs its
    // own explicit OpenGL upload flip when this atlas is consumed later.
    stbi_set_flip_vertically_on_load(0);
    for (size_t index = 0; index < candidate.tilePaths.size(); ++index) {
        int width = 0;
        int height = 0;
        int channels = 0;
        const auto decoded = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>(
            stbi_load(candidate.tilePaths[index].string().c_str(), &width, &height, &channels, STBI_rgb_alpha),
            stbi_image_free);
        if (decoded == nullptr || width != candidate.tileWidth || height != candidate.tileHeight) {
            failureMessage = "A reviewed tileset PNG could not be decoded with its declared grid dimensions.";
            return false;
        }
        const auto tileX = static_cast<int>(index % static_cast<size_t>(candidate.columns));
        const auto tileY = static_cast<int>(index / static_cast<size_t>(candidate.columns));
        for (int y = 0; y < kRuntimeTilesetCellSize; ++y) {
            const auto sourceY = (y * candidate.tileHeight) / kRuntimeTilesetCellSize;
            for (int x = 0; x < kRuntimeTilesetCellSize; ++x) {
                const auto sourceX = (x * candidate.tileWidth) / kRuntimeTilesetCellSize;
                const auto destination =
                    (static_cast<size_t>(tileY * kRuntimeTilesetCellSize + y) * atlasWidth +
                     static_cast<size_t>(tileX * kRuntimeTilesetCellSize + x)) *
                    4U;
                const auto source =
                    (static_cast<size_t>(sourceY) * static_cast<size_t>(candidate.tileWidth) + sourceX) * 4U;
                std::copy_n(decoded.get() + source, 4U, atlas.begin() + destination);
            }
        }
    }
    if (stbi_write_png(atlasPath.string().c_str(), atlasWidth, atlasHeight, 4, atlas.data(), atlasWidth * 4) == 0) {
        failureMessage = "The packed tileset atlas could not be written.";
        return false;
    }
    return true;
}

std::filesystem::path derivedAttachmentReferencePath(const std::filesystem::path& derivedManifestPath,
                                                     const std::filesystem::path& projectRoot) {
    std::error_code error;
    const auto normalizedProject = std::filesystem::exists(projectRoot)
                                       ? std::filesystem::weakly_canonical(projectRoot, error)
                                       : std::filesystem::absolute(projectRoot, error).lexically_normal();
    if (error) return {};
    // The full derived revision and project hashes are retained inside the
    // marker and validated on read. Use bounded path keys here so the marker
    // itself remains creatable on Windows installations without long-path
    // support.
    auto projectKey = hashText(normalizedProject.generic_string());
    if (projectKey.size() > 32U) projectKey.resize(32U);
    return derivedManifestPath.parent_path() / (derivedManifestPath.stem().string() + ".attachment-refs") /
           (projectKey + ".json");
}

struct DerivedAttachmentReferencePreparation {
    bool success = false;
    bool created = false;
    std::filesystem::path markerPath;
    std::string code;
    std::string message;
};

DerivedAttachmentReferencePreparation prepareDerivedAttachmentReference(
    const AssetPromotionManifest& attached, const std::filesystem::path& derivedManifestPath,
    const std::filesystem::path& projectRoot, const std::string& operationId) {
    const auto markerPath = derivedAttachmentReferencePath(derivedManifestPath, projectRoot);
    if (markerPath.empty()) {
        return {false, false, {}, "asset_derived_attachment_reference_path_invalid",
                "The derived attachment reference path could not be resolved."};
    }
    std::error_code error;
    std::filesystem::create_directories(markerPath.parent_path(), error);
    if (error) {
        return {false, false, markerPath, "asset_derived_attachment_reference_directory_failed", error.message()};
    }
    if (std::filesystem::is_regular_file(markerPath)) {
        std::ifstream input(markerPath, std::ios::binary);
        const auto existing = nlohmann::json::parse(input, nullptr, false);
        if (existing.is_discarded() ||
            existing.value("schema", "") != "urpg.asset_transform_attachment_reference.v1" ||
            existing.value("derived_asset_id", "") != attached.assetId) {
            return {false, false, markerPath, "asset_derived_attachment_reference_invalid",
                    "The existing derived attachment reference is invalid."};
        }
        if (existing.value("state", "") == "attached") return {true, false, markerPath, {}, {}};
        if (existing.value("state", "") == "prepared" && existing.value("operation_id", "") == operationId) {
            return {true, false, markerPath, {}, {}};
        }
        return {false, false, markerPath, "asset_derived_attachment_recovery_required",
                "A prior derived attachment did not finish. Recover or inspect it before attaching this revision."};
    }
    const nlohmann::json marker = {
        {"schema", "urpg.asset_transform_attachment_reference.v1"},
        {"state", "prepared"},
        {"operation_id", operationId},
        {"derived_asset_id", attached.assetId},
        {"derived_manifest_path", std::filesystem::weakly_canonical(derivedManifestPath).generic_string()},
        {"project_root", std::filesystem::absolute(projectRoot, error).lexically_normal().generic_string()},
    };
    if (error) {
        return {false, false, markerPath, "asset_derived_attachment_reference_path_invalid", error.message()};
    }
    auto temporaryKey = hashText(operationId + markerPath.generic_string());
    if (temporaryKey.size() > 12U) temporaryKey.resize(12U);
    const auto staged = markerPath.parent_path() / (".ref-" + temporaryKey + ".tmp");
    if (!writeJsonFile(staged, marker)) {
        removeIfPresent(staged);
        return {false, false, markerPath, "asset_derived_attachment_reference_write_failed",
                "The derived attachment reference could not be staged."};
    }
    std::filesystem::rename(staged, markerPath, error);
    if (error) {
        removeIfPresent(staged);
        return {false, false, markerPath, "asset_derived_attachment_reference_publish_failed", error.message()};
    }
    return {true, true, markerPath, {}, {}};
}

bool finalizeDerivedAttachmentReference(const std::filesystem::path& markerPath,
                                        const std::filesystem::path& projectManifestPath) {
    std::ifstream input(markerPath, std::ios::binary);
    auto marker = nlohmann::json::parse(input, nullptr, false);
    if (marker.is_discarded() || marker.value("schema", "") != "urpg.asset_transform_attachment_reference.v1") {
        return false;
    }
    marker["state"] = "attached";
    marker["project_manifest_path"] = projectManifestPath.generic_string();
    return writeJsonFile(markerPath, marker);
}

std::optional<std::filesystem::path> canonicalExistingPath(const std::filesystem::path& path) {
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(path, error);
    if (error || !std::filesystem::is_regular_file(canonical)) {
        return std::nullopt;
    }
    return canonical;
}

bool matchesDerivedManifestPath(const nlohmann::json& value, const std::filesystem::path& expected) {
    if (!value.is_string()) {
        return false;
    }
    const auto candidate = canonicalExistingPath(value.get<std::string>());
    return candidate.has_value() && *candidate == expected;
}

std::vector<std::filesystem::path> findDurableDerivedReferenceManifests(
    const std::filesystem::path& projectRoot, const std::filesystem::path& derivedManifestPath,
    const std::string& derivedAssetId) {
    std::vector<std::filesystem::path> matches;
    const auto considerAssetManifest = [&](const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        const auto manifest = nlohmann::json::parse(input, nullptr, false);
        if (manifest.is_discarded() || manifest.value("assetId", "") != derivedAssetId) {
            return;
        }
        const auto metadata = manifest.value("authoredMetadata", nlohmann::json::object());
        const auto revision = metadata.value("derived_revision", nlohmann::json::object());
        if (revision.value("schema", "") == "urpg.project_asset_derived_revision.v1" &&
            matchesDerivedManifestPath(revision.value("manifest_path", nlohmann::json{}), derivedManifestPath)) {
            matches.push_back(path);
        }
    };
    const auto considerTilesetManifest = [&](const std::filesystem::path& path) {
        std::ifstream input(path, std::ios::binary);
        const auto manifest = nlohmann::json::parse(input, nullptr, false);
        if (manifest.is_discarded() || manifest.value("schema", "") != "urpg.project_derived_tileset_assignment.v1" ||
            manifest.value("tileset_id", "") != derivedAssetId) {
            return;
        }
        if (matchesDerivedManifestPath(manifest.value("derived_manifest_path", nlohmann::json{}), derivedManifestPath)) {
            matches.push_back(path);
        }
    };
    std::error_code error;
    const auto assetManifests = projectRoot / "content" / "assets" / "manifests";
    if (std::filesystem::is_directory(assetManifests, error) && !error) {
        for (const auto& entry : std::filesystem::directory_iterator(assetManifests, error)) {
            if (error) return {};
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                considerAssetManifest(entry.path());
            }
        }
    }
    error.clear();
    const auto tilesetManifests = projectRoot / "content" / "tilesets";
    if (std::filesystem::is_directory(tilesetManifests, error) && !error) {
        for (const auto& entry : std::filesystem::directory_iterator(tilesetManifests, error)) {
            if (error) return {};
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                considerTilesetManifest(entry.path());
            }
        }
    }
    return matches;
}

std::filesystem::path journalPathFor(const std::filesystem::path& projectRoot) {
    const auto sequence = attachmentStagingSequence.fetch_add(1, std::memory_order_relaxed);
    return attachmentJournalRoot(projectRoot) / ("attachment-" + std::to_string(sequence) + ".json");
}

void recoverAttachmentTransactions(const std::filesystem::path& projectRoot) {
    const auto journalRoot = attachmentJournalRoot(projectRoot);
    std::error_code error;
    if (!std::filesystem::is_directory(journalRoot, error) || error) return;
    for (const auto& entry : std::filesystem::directory_iterator(journalRoot, error)) {
        if (error || !entry.is_regular_file()) continue;
        std::ifstream input(entry.path(), std::ios::binary);
        const auto journal = nlohmann::json::parse(input, nullptr, false);
        input.close();
        const auto readPath = [&](const char* key) {
            const auto relative = std::filesystem::path(journal.value(key, ""));
            return isSafeRelativePath(relative) ? projectRoot / relative : std::filesystem::path{};
        };
        if (journal.is_discarded() || journal.value("schema", "") != "urpg.asset_attachment_transaction.v1") continue;
        const auto payload = readPath("payload_path");
        const auto manifest = readPath("manifest_path");
        const auto stagedPayload = readPath("staged_payload_path");
        const auto stagedManifest = readPath("staged_manifest_path");
        const auto payloadBackup = readPath("payload_backup_path");
        const auto manifestBackup = readPath("manifest_backup_path");
        if (payload.empty() || manifest.empty() || stagedPayload.empty() || stagedManifest.empty() ||
            payloadBackup.empty() || manifestBackup.empty()) {
            continue;
        }
        const bool hadPayload = journal.value("had_payload", false);
        const bool hadManifest = journal.value("had_manifest", false);
        const auto state = journal.value("state", "prepared");
        if (std::filesystem::exists(payloadBackup)) {
            removeIfPresent(payload);
            std::filesystem::rename(payloadBackup, payload, error);
        } else if (!hadPayload && state == "payload_published") {
            removeIfPresent(payload);
        }
        if (std::filesystem::exists(manifestBackup)) {
            removeIfPresent(manifest);
            std::filesystem::rename(manifestBackup, manifest, error);
        } else if (!hadManifest && state == "payload_published") {
            removeIfPresent(manifest);
        }
        removeIfPresent(stagedPayload);
        removeIfPresent(stagedManifest);
        if (std::filesystem::exists(payload) == hadPayload && std::filesystem::exists(manifest) == hadManifest) {
            removeIfPresent(payloadBackup);
            removeIfPresent(manifestBackup);
            removeIfPresent(entry.path());
        }
    }
}

} // namespace

ProjectAssetAttachmentPlan ProjectAssetAttachmentService::planPromotedAssetAttachment(
    const AssetPromotionManifest& manifest,
    const std::filesystem::path& projectRoot,
    const ProjectAssetAttachmentConflictPolicy conflictPolicy) const {
    ProjectAssetAttachmentPlan plan;
    plan.assetId = manifest.assetId;
    auto diagnostics = validateAssetPromotionManifest(manifest);
    diagnostics.insert(diagnostics.end(), manifest.diagnostics.begin(), manifest.diagnostics.end());
    if (!diagnostics.empty()) {
        plan.diagnostics = std::move(diagnostics);
        return plan;
    }
    if (manifest.status != AssetPromotionStatus::RuntimeReady || !manifest.package.includeInRuntime) {
        plan.diagnostics.push_back("asset_not_runtime_ready");
        return plan;
    }
    if (manifest.promotedPath.empty() || !std::filesystem::is_regular_file(manifest.promotedPath)) {
        plan.diagnostics.push_back("promoted_payload_missing");
        return plan;
    }
    if (projectRoot.empty()) {
        plan.diagnostics.push_back("project_root_missing");
        return plan;
    }

    const auto assetSegment = sanitizeSegment(manifest.assetId);
    plan.payloadPath = projectRoot / "content" / "assets" / "imported" / assetSegment /
                       std::filesystem::path(manifest.promotedPath).filename();
    plan.manifestPath = projectRoot / "content" / "assets" / "manifests" / (assetSegment + ".json");
    const auto projectContent = projectRoot / "content";
    std::error_code error;
    // Planning is read-only, so a new project need not have created its
    // content directory yet. Resolve existing paths canonically and use an
    // absolute lexical containment check for not-yet-created destinations.
    const auto normalizedRoot = std::filesystem::exists(projectContent)
                                    ? std::filesystem::weakly_canonical(projectContent, error)
                                    : std::filesystem::absolute(projectContent, error).lexically_normal();
    if (error) {
        plan.diagnostics.push_back("project_attachment_path_unresolvable");
        return plan;
    }
    const auto normalizedPayload = std::filesystem::exists(plan.payloadPath.parent_path())
                                       ? std::filesystem::weakly_canonical(plan.payloadPath.parent_path(), error) /
                                             plan.payloadPath.filename()
                                       : std::filesystem::absolute(plan.payloadPath, error).lexically_normal();
    if (error || !pathInside(normalizedRoot, normalizedPayload)) {
        plan.diagnostics.push_back("project_attachment_path_escape");
        return plan;
    }
    plan.sourceRevision = attachmentRevision(manifest, plan.payloadPath, plan.manifestPath);
    // Conflict policy is enforced by apply. A plan must remain inspectable for
    // an existing attachment so its revision can detect an external edit
    // before a caller chooses Replace, Keep Both, or Relink Existing.
    (void)conflictPolicy;
    plan.valid = true;
    return plan;
}

ProjectAssetAttachmentPlan ProjectAssetAttachmentService::planDerivedRevisionAttachment(
    const AssetPromotionManifest& source, const std::filesystem::path& derivedManifestPath,
    const std::filesystem::path& projectRoot, const ProjectAssetAttachmentConflictPolicy conflictPolicy) const {
    const auto candidate = derivedAttachmentCandidate(source, derivedManifestPath);
    if (!candidate.valid) {
        ProjectAssetAttachmentPlan plan;
        plan.assetId = source.assetId;
        plan.diagnostics = candidate.diagnostics;
        plan.diagnostics.insert(plan.diagnostics.begin(), candidate.code);
        return plan;
    }
    return planPromotedAssetAttachment(candidate.manifest, projectRoot, conflictPolicy);
}

ProjectAssetAttachmentResult ProjectAssetAttachmentService::attachDerivedRevision(
    const ProjectDerivedAssetAttachmentRequest& request) const {
    if (request.operationId.empty()) {
        return blocked("asset_derived_attachment_operation_id_required",
                       "Derived revision attachment requires the stable operation ID from its reviewed plan.");
    }
    const auto candidate = derivedAttachmentCandidate(request.source, request.derivedManifestPath);
    if (!candidate.valid) {
        return blocked(candidate.code, candidate.message, candidate.diagnostics);
    }
    const bool hasOperationReceipt = std::filesystem::is_regular_file(
        operationReceiptPath(request.projectRoot, request.operationId));
    if (!hasOperationReceipt) {
        const auto plan = planPromotedAssetAttachment(candidate.manifest, request.projectRoot, request.conflictPolicy);
        if (!plan.valid) {
            return blocked(plan.diagnostics.empty() ? "asset_attachment_plan_invalid" : plan.diagnostics.front(),
                           "Derived asset attachment plan is no longer valid.", plan.diagnostics);
        }
        if (request.expectedSourceRevision.empty() || request.expectedSourceRevision != plan.sourceRevision) {
            return blocked("asset_attachment_source_revision_mismatch",
                           "The reviewed derived asset changed. Refresh the plan before attaching it.");
        }
    }
    const auto reference = prepareDerivedAttachmentReference(candidate.manifest, request.derivedManifestPath,
                                                              request.projectRoot, request.operationId);
    if (!reference.success) return blocked(reference.code, reference.message);
    ProjectAssetAttachmentRequest attachment;
    attachment.manifest = candidate.manifest;
    attachment.projectRoot = request.projectRoot;
    attachment.conflictPolicy = request.conflictPolicy;
    attachment.operationId = request.operationId;
    attachment.expectedSourceRevision = request.expectedSourceRevision;
    auto result = attachPromotedAsset(attachment);
    if (!result.success) {
        if (reference.created) removeIfPresent(reference.markerPath);
        return result;
    }
    if (!finalizeDerivedAttachmentReference(reference.markerPath, result.manifestPath)) {
        result.code = "project_asset_attached_reference_tracking_pending";
        result.message = "The project asset was attached, but its derived revision remains protected until reference tracking is recovered.";
        result.diagnostics.push_back("asset_derived_attachment_reference_finalize_failed");
    }
    return result;
}

ProjectAssetAttachmentResult ProjectAssetAttachmentService::recoverDerivedAttachmentReference(
    const std::filesystem::path& derivedManifestPath, const std::filesystem::path& projectRoot) const {
    const auto canonicalDerived = canonicalExistingPath(derivedManifestPath);
    if (!canonicalDerived.has_value() || projectRoot.empty()) {
        return blocked("asset_derived_attachment_recovery_request_invalid",
                       "Derived reference recovery requires an existing manifest and project root.");
    }
    const auto markerPath = derivedAttachmentReferencePath(*canonicalDerived, projectRoot);
    if (markerPath.empty() || !std::filesystem::is_regular_file(markerPath)) {
        return blocked("asset_derived_attachment_reference_missing",
                       "No derived attachment reference marker exists for this project.");
    }
    std::ifstream input(markerPath, std::ios::binary);
    const auto marker = nlohmann::json::parse(input, nullptr, false);
    if (marker.is_discarded() || marker.value("schema", "") != "urpg.asset_transform_attachment_reference.v1" ||
        !matchesDerivedManifestPath(marker.value("derived_manifest_path", nlohmann::json{}), *canonicalDerived) ||
        marker.value("derived_asset_id", "").empty()) {
        return blocked("asset_derived_attachment_reference_invalid",
                       "The derived attachment reference marker is invalid.");
    }
    std::error_code error;
    const auto normalizedProject = std::filesystem::exists(projectRoot)
                                       ? std::filesystem::weakly_canonical(projectRoot, error)
                                       : std::filesystem::absolute(projectRoot, error).lexically_normal();
    if (error || marker.value("project_root", "") != normalizedProject.generic_string()) {
        return blocked("asset_derived_attachment_reference_invalid",
                       "The derived attachment reference marker belongs to another project.");
    }
    const auto state = marker.value("state", "");
    if (state == "attached") {
        const auto projectManifest = canonicalExistingPath(marker.value("project_manifest_path", ""));
        if (!projectManifest.has_value() || !pathInside(normalizedProject, *projectManifest)) {
            return blocked("asset_derived_attachment_reference_invalid",
                           "The attached derived reference does not name a project-owned manifest.");
        }
        ProjectAssetAttachmentResult result;
        result.success = true;
        result.code = "asset_derived_attachment_reference_already_attached";
        result.message = "The derived revision is already protected by its project attachment reference.";
        result.manifestPath = *projectManifest;
        return result;
    }
    if (state != "prepared") {
        return blocked("asset_derived_attachment_reference_invalid",
                       "The derived attachment reference marker has an invalid state.");
    }
    const auto matches = findDurableDerivedReferenceManifests(normalizedProject, *canonicalDerived,
                                                                marker.value("derived_asset_id", ""));
    if (matches.empty()) {
        return blocked("asset_derived_attachment_recovery_required",
                       "No durable project attachment proves this prepared reference completed; it remains protected.");
    }
    if (matches.size() != 1U) {
        return blocked("asset_derived_attachment_recovery_ambiguous",
                       "Multiple project manifests match this prepared reference; it remains protected.");
    }
    if (!finalizeDerivedAttachmentReference(markerPath, matches.front())) {
        return blocked("asset_derived_attachment_reference_finalize_failed",
                       "The durable attachment was found, but the reference marker could not be finalized.");
    }
    ProjectAssetAttachmentResult result;
    result.success = true;
    result.code = "asset_derived_attachment_reference_recovered";
    result.message = "The prepared derived reference was finalized from its project-owned attachment.";
    result.manifestPath = matches.front();
    return result;
}

ProjectDerivedTilesetAssignmentPlan ProjectAssetAttachmentService::planDerivedTilesetAssignment(
    const AssetPromotionManifest& source, const std::filesystem::path& derivedManifestPath,
    const std::filesystem::path& projectRoot, const ProjectAssetAttachmentConflictPolicy conflictPolicy) const {
    (void)conflictPolicy;
    ProjectDerivedTilesetAssignmentPlan plan;
    const auto candidate = derivedTilesetCandidate(source, derivedManifestPath);
    if (!candidate.valid) {
        plan.diagnostics = candidate.diagnostics;
        plan.diagnostics.insert(plan.diagnostics.begin(), candidate.code);
        return plan;
    }
    if (projectRoot.empty()) {
        plan.diagnostics.push_back("project_root_missing");
        return plan;
    }
    const auto segment = sanitizeSegment(candidate.tilesetId);
    plan.valid = true;
    plan.tilesetId = candidate.tilesetId;
    plan.sourceRevision = candidate.reviewRevision;
    plan.derivedRevision = candidate.manifestPath.stem().string();
    plan.columns = candidate.columns;
    plan.rows = candidate.rows;
    plan.tileWidth = candidate.tileWidth;
    plan.tileHeight = candidate.tileHeight;
    plan.tileDirectory = projectRoot / "content" / "tilesets" / segment / "tiles";
    plan.manifestPath = projectRoot / "content" / "tilesets" / (segment + ".json");
    return plan;
}

ProjectAssetAttachmentResult ProjectAssetAttachmentService::assignDerivedTileset(
    const ProjectDerivedTilesetAssignmentRequest& request) const {
    if (!isSafeOperationId(request.operationId)) {
        return blocked("project_tileset_assignment_operation_id_invalid",
                       "Tileset assignment requires a stable safe operation ID from its reviewed plan.");
    }
    if (request.expectedSourceRevision.empty()) {
        return blocked("project_tileset_assignment_source_revision_required",
                       "Tileset assignment requires the review revision from its current plan.");
    }
    const auto candidate = derivedTilesetCandidate(request.source, request.derivedManifestPath);
    if (!candidate.valid) return blocked(candidate.code, candidate.message, candidate.diagnostics);
    const auto fingerprint = tilesetAssignmentFingerprint(request, candidate.reviewRevision);
    const auto receiptPath = tilesetAssignmentReceiptPath(request.projectRoot, request.operationId);
    if (std::filesystem::is_regular_file(receiptPath)) {
        std::ifstream input(receiptPath, std::ios::binary);
        const auto receipt = nlohmann::json::parse(input, nullptr, false);
        if (receipt.is_discarded() || receipt.value("schema", "") != "urpg.project_derived_tileset_assignment_receipt.v1" ||
            receipt.value("request_fingerprint", "") != fingerprint) {
            return blocked("project_tileset_assignment_operation_mismatch",
                           "The operation ID was already used for a different tileset assignment request.");
        }
        ProjectAssetAttachmentResult result;
        result.success = receipt.value("success", false);
        result.code = receipt.value("code", "project_tileset_assignment_receipt_invalid");
        result.message = "The completed tileset assignment was returned without reapplying it.";
        result.payloadPath = receipt.value("tile_directory", "");
        result.manifestPath = receipt.value("manifest_path", "");
        result.sourceRevision = receipt.value("review_revision", "");
        result.operationId = request.operationId;
        return result;
    }
    const auto plan = planDerivedTilesetAssignment(request.source, request.derivedManifestPath, request.projectRoot,
                                                    request.conflictPolicy);
    if (!plan.valid) {
        return blocked(plan.diagnostics.empty() ? "project_tileset_assignment_plan_invalid" : plan.diagnostics.front(),
                       "The tileset assignment plan is no longer valid.", plan.diagnostics);
    }
    if (request.expectedSourceRevision != plan.sourceRevision) {
        return blocked("project_tileset_assignment_source_revision_mismatch",
                       "The reviewed tileset bundle changed. Refresh the plan before assigning it.");
    }

    auto tilesetId = plan.tilesetId;
    auto tileDirectory = plan.tileDirectory;
    auto manifestPath = plan.manifestPath;
    const auto destinationDirectory = [&] { return tileDirectory.parent_path(); };
    const auto hasCollision = [&] {
        return std::filesystem::exists(destinationDirectory()) || std::filesystem::exists(manifestPath);
    };
    if (hasCollision() && request.conflictPolicy == ProjectAssetAttachmentConflictPolicy::Cancel) {
        return blocked("project_tileset_assignment_conflict_requires_resolution",
                       "A project tileset assignment with this stable ID already exists. Choose Replace, Keep Both, or Relink Existing.");
    }
    if (hasCollision() && request.conflictPolicy == ProjectAssetAttachmentConflictPolicy::RelinkExisting) {
        ProjectAssetAttachmentResult result;
        result.success = true;
        result.code = "project_tileset_relinked_existing";
        result.message = "Existing project tileset assignment was retained and relinked.";
        result.payloadPath = tileDirectory;
        result.manifestPath = manifestPath;
        result.sourceRevision = plan.sourceRevision;
        result.operationId = request.operationId;
        return result;
    }
    if (hasCollision() && request.conflictPolicy == ProjectAssetAttachmentConflictPolicy::KeepBoth) {
        const auto original = tilesetId;
        bool foundAvailableDestination = false;
        for (size_t suffix = 2; suffix < 10'000; ++suffix) {
            tilesetId = original + "-" + std::to_string(suffix);
            const auto segment = sanitizeSegment(tilesetId);
            tileDirectory = request.projectRoot / "content" / "tilesets" / segment / "tiles";
            manifestPath = request.projectRoot / "content" / "tilesets" / (segment + ".json");
            if (!std::filesystem::exists(destinationDirectory()) && !std::filesystem::exists(manifestPath)) {
                foundAvailableDestination = true;
                break;
            }
        }
        if (!foundAvailableDestination) {
            return blocked("project_tileset_assignment_keep_both_exhausted",
                           "No available project tileset ID could be allocated for Keep Both.");
        }
    }

    AssetPromotionManifest referenceAsset;
    referenceAsset.assetId = tilesetId;
    const auto reference = prepareDerivedAttachmentReference(referenceAsset, candidate.manifestPath, request.projectRoot,
                                                              request.operationId);
    if (!reference.success) return blocked(reference.code, reference.message);

    std::error_code error;
    std::filesystem::create_directories(destinationDirectory().parent_path(), error);
    if (error) {
        if (reference.created) removeIfPresent(reference.markerPath);
        return blocked("project_tileset_assignment_directory_create_failed", error.message());
    }
    const auto stagedDirectory = siblingWorkingPath(destinationDirectory(), "stage-tileset");
    const auto stagedTiles = stagedDirectory / "tiles";
    const auto stagedAtlas = stagedDirectory / "atlas.png";
    const auto stagedManifest = siblingWorkingPath(manifestPath, "stage-tileset-manifest");
    std::filesystem::create_directories(stagedTiles, error);
    if (error) {
        if (reference.created) removeIfPresent(reference.markerPath);
        return blocked("project_tileset_assignment_stage_create_failed", error.message());
    }
    for (size_t index = 0; index < candidate.tilePaths.size(); ++index) {
        std::filesystem::copy_file(candidate.tilePaths[index], stagedTiles / tilesetFilename(index),
                                   std::filesystem::copy_options::none, error);
        if (error) {
            removeIfPresent(stagedDirectory);
            if (reference.created) removeIfPresent(reference.markerPath);
            return blocked("project_tileset_assignment_stage_copy_failed", error.message());
        }
    }
    std::string atlasFailure;
    if (!packTilesetAtlas(candidate, stagedAtlas, atlasFailure)) {
        removeIfPresent(stagedDirectory);
        if (reference.created) removeIfPresent(reference.markerPath);
        return blocked("project_tileset_assignment_atlas_pack_failed", atlasFailure);
    }
    if (!writeJsonFile(stagedManifest,
                       projectTilesetManifest(candidate, tilesetId, request.projectRoot, hashFile(stagedAtlas)))) {
        removeIfPresent(stagedDirectory);
        removeIfPresent(stagedManifest);
        if (reference.created) removeIfPresent(reference.markerPath);
        return blocked("project_tileset_assignment_stage_write_failed", "The project tileset manifest could not be staged.");
    }

    const auto directoryBackup = siblingWorkingPath(destinationDirectory(), "backup-tileset");
    const auto manifestBackup = siblingWorkingPath(manifestPath, "backup-tileset-manifest");
    const bool hadDirectory = std::filesystem::exists(destinationDirectory());
    const bool hadManifest = std::filesystem::exists(manifestPath);
    if (hadDirectory) std::filesystem::rename(destinationDirectory(), directoryBackup, error);
    if (!error && hadManifest) std::filesystem::rename(manifestPath, manifestBackup, error);
    if (!error) std::filesystem::rename(stagedDirectory, destinationDirectory(), error);
    if (!error) std::filesystem::rename(stagedManifest, manifestPath, error);
    if (error) {
        removeIfPresent(destinationDirectory());
        removeIfPresent(manifestPath);
        if (hadDirectory && std::filesystem::exists(directoryBackup)) std::filesystem::rename(directoryBackup, destinationDirectory(), error);
        if (hadManifest && std::filesystem::exists(manifestBackup)) std::filesystem::rename(manifestBackup, manifestPath, error);
        removeIfPresent(stagedDirectory);
        removeIfPresent(stagedManifest);
        if (reference.created) removeIfPresent(reference.markerPath);
        return blocked("project_tileset_assignment_publish_failed", "The staged project tileset assignment could not be published.");
    }
    removeIfPresent(directoryBackup);
    removeIfPresent(manifestBackup);

    ProjectAssetAttachmentResult result;
    result.success = true;
    result.code = "project_derived_tileset_assigned";
    result.message = "The reviewed derived tileset bundle was assigned to project-owned content.";
    result.payloadPath = tileDirectory;
    result.manifestPath = manifestPath;
    result.sourceRevision = plan.sourceRevision;
    result.operationId = request.operationId;
    if (!finalizeDerivedAttachmentReference(reference.markerPath, manifestPath)) {
        result.code = "project_tileset_assigned_reference_tracking_pending";
        result.message = "The project tileset was assigned, but its derived revision remains protected until reference tracking is recovered.";
        result.diagnostics.push_back("asset_derived_attachment_reference_finalize_failed");
    }
    std::filesystem::create_directories(receiptPath.parent_path(), error);
    if (error || !writeJsonFile(receiptPath, {{"schema", "urpg.project_derived_tileset_assignment_receipt.v1"},
                                               {"request_fingerprint", fingerprint}, {"success", result.success},
                                               {"code", result.code}, {"tile_directory", result.payloadPath.generic_string()},
                                               {"manifest_path", result.manifestPath.generic_string()},
                                               {"review_revision", result.sourceRevision}})) {
        result.code = "project_tileset_assigned_receipt_pending";
        result.message = "The project tileset was assigned, but its operation receipt could not be written.";
    }
    return result;
}

ProjectAssetAttachmentResult ProjectAssetAttachmentService::attachPromotedAsset(
    const ProjectAssetAttachmentRequest& request) const {
    if (!request.operationId.empty() && !isSafeOperationId(request.operationId)) {
        return blocked("asset_attachment_operation_id_invalid", "Attachment operation IDs must be stable safe identifiers.");
    }
    if (!request.operationId.empty() && request.expectedSourceRevision.empty()) {
        return blocked("asset_attachment_source_revision_required",
                       "Checked asset attachment operations require the planned source revision.");
    }
    const auto fingerprint = requestFingerprint(request, request.expectedSourceRevision);
    const auto receiptPath = request.operationId.empty() ? std::filesystem::path{} :
                                                        operationReceiptPath(request.projectRoot, request.operationId);
    if (!receiptPath.empty() && std::filesystem::is_regular_file(receiptPath)) {
        std::ifstream input(receiptPath, std::ios::binary);
        const auto receipt = nlohmann::json::parse(input, nullptr, false);
        if (receipt.is_discarded() || receipt.value("schema", "") != "urpg.project_asset_attachment_receipt.v1") {
            return blocked("asset_attachment_operation_receipt_invalid", "The existing attachment operation receipt is invalid.");
        }
        if (receipt.value("request_fingerprint", "") != fingerprint) {
            return blocked("asset_attachment_operation_mismatch",
                           "The operation ID was already used for a different attachment request.");
        }
        ProjectAssetAttachmentResult result;
        result.success = receipt.value("success", false);
        result.code = receipt.value("code", "asset_attachment_operation_receipt_invalid");
        result.message = "The completed attachment operation was returned without reapplying it.";
        result.payloadPath = receipt.value("payload_path", "");
        result.manifestPath = receipt.value("manifest_path", "");
        result.sourceRevision = receipt.value("source_revision", "");
        result.operationId = request.operationId;
        return result;
    }
    const auto plan = planPromotedAssetAttachment(request.manifest, request.projectRoot, request.conflictPolicy);
    if (!plan.valid) {
        return blocked(plan.diagnostics.empty() ? "asset_attachment_plan_invalid" : plan.diagnostics.front(),
                       "Asset attachment plan is no longer valid.", plan.diagnostics);
    }
    if (!request.expectedSourceRevision.empty() && request.expectedSourceRevision != plan.sourceRevision) {
        return blocked("asset_attachment_source_revision_mismatch",
                       "The reviewed attachment source changed. Refresh the plan before applying it.");
    }
    auto result = attachPromotedAsset(request.manifest, request.projectRoot, request.conflictPolicy);
    result.sourceRevision = plan.sourceRevision;
    result.operationId = request.operationId;
    if (!result.success || receiptPath.empty()) {
        return result;
    }
    std::error_code error;
    std::filesystem::create_directories(receiptPath.parent_path(), error);
    if (error) {
        return blocked("asset_attachment_operation_receipt_write_failed", error.message());
    }
    const nlohmann::json receipt = {{"schema", "urpg.project_asset_attachment_receipt.v1"},
                                    {"request_fingerprint", fingerprint},
                                    {"success", result.success},
                                    {"code", result.code},
                                    {"payload_path", result.payloadPath.generic_string()},
                                    {"manifest_path", result.manifestPath.generic_string()},
                                    {"source_revision", result.sourceRevision}};
    std::ofstream output(receiptPath, std::ios::binary | std::ios::trunc);
    output << receipt.dump(2) << '\n';
    if (!output) {
        return blocked("asset_attachment_operation_receipt_write_failed", "The attachment succeeded but its operation receipt could not be written.");
    }
    return result;
}

ProjectAssetAttachmentResult ProjectAssetAttachmentService::attachPromotedAsset(
    const AssetPromotionManifest& manifest, const std::filesystem::path& projectRoot,
    const ProjectAssetAttachmentConflictPolicy conflictPolicy) const {
    auto diagnostics = validateAssetPromotionManifest(manifest);
    diagnostics.insert(diagnostics.end(), manifest.diagnostics.begin(), manifest.diagnostics.end());
    if (!diagnostics.empty()) {
        return blocked("asset_promotion_invalid", "Promoted asset manifest has unresolved diagnostics.", diagnostics);
    }
    if (manifest.status != AssetPromotionStatus::RuntimeReady || !manifest.package.includeInRuntime) {
        return blocked("asset_not_runtime_ready", "Only runtime-ready promoted assets can be attached to a project.");
    }
    if (manifest.promotedPath.empty()) {
        return blocked("promoted_payload_missing", "Promoted asset payload path is empty.");
    }
    if (projectRoot.empty()) {
        return blocked("project_root_missing", "A project root is required for attachment.");
    }

    const auto sourcePayload = std::filesystem::path(manifest.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePayload)) {
        return blocked("promoted_payload_missing", "Promoted asset payload file does not exist.");
    }

    recoverAttachmentTransactions(projectRoot);

    const auto projectContent = projectRoot / "content";
    const auto importedRoot = projectContent / "assets" / "imported";
    const auto manifestRoot = projectContent / "assets" / "manifests";
    auto assetSegment = sanitizeSegment(manifest.assetId);
    auto destinationPayload = importedRoot / assetSegment / sourcePayload.filename();
    auto destinationManifest = manifestRoot / (assetSegment + ".json");

    std::error_code error;
    std::filesystem::create_directories(destinationPayload.parent_path(), error);
    if (error) {
        return blocked("project_asset_directory_create_failed", error.message());
    }
    std::filesystem::create_directories(destinationManifest.parent_path(), error);
    if (error) {
        return blocked("project_manifest_directory_create_failed", error.message());
    }
    if (!pathInside(projectContent, destinationPayload) || !pathInside(projectContent, destinationManifest)) {
        return blocked("project_attachment_path_escape", "Project attachment destination escaped the project content root.");
    }

    const bool collision = std::filesystem::exists(destinationPayload) || std::filesystem::exists(destinationManifest);
    if (collision && conflictPolicy == ProjectAssetAttachmentConflictPolicy::Cancel) {
        return blocked("project_attachment_conflict_requires_resolution",
                       "An attachment with this stable asset ID already exists. Choose Replace, Keep Both, or Relink Existing.");
    }
    if (collision && conflictPolicy == ProjectAssetAttachmentConflictPolicy::RelinkExisting) {
        ProjectAssetAttachmentResult result;
        result.success = true;
        result.code = "project_asset_relinked_existing";
        result.message = "Existing project attachment was retained and relinked.";
        result.payloadPath = destinationPayload;
        result.manifestPath = destinationManifest;
        return result;
    }
    if (collision && conflictPolicy == ProjectAssetAttachmentConflictPolicy::KeepBoth) {
        const auto original = assetSegment;
        for (size_t suffix = 2; suffix < 10'000; ++suffix) {
            assetSegment = original + "-" + std::to_string(suffix);
            destinationPayload = importedRoot / assetSegment / sourcePayload.filename();
            destinationManifest = manifestRoot / (assetSegment + ".json");
            if (!std::filesystem::exists(destinationPayload) && !std::filesystem::exists(destinationManifest)) break;
        }
    }
    std::filesystem::create_directories(destinationPayload.parent_path(), error);
    if (error) {
        return blocked("project_asset_directory_create_failed", error.message());
    }

    auto projectManifest = manifest;
    projectManifest.sourcePath = manifest.promotedPath;
    projectManifest.promotedPath = destinationPayload.generic_string();
    projectManifest.preview.thumbnailPath =
        manifest.preview.kind == "image" || manifest.preview.kind == "audio" ? destinationPayload.generic_string()
                                                                              : manifest.preview.thumbnailPath;
    projectManifest.package.includeInRuntime = true;
    projectManifest.package.requiredForRelease = manifest.package.requiredForRelease;
    projectManifest.diagnostics.clear();

    const auto stagedPayload = siblingWorkingPath(destinationPayload, "stage-payload");
    const auto stagedManifest = siblingWorkingPath(destinationManifest, "stage-manifest");
    std::filesystem::copy_file(sourcePayload, stagedPayload, std::filesystem::copy_options::overwrite_existing, error);
    if (error) {
        return blocked("project_asset_stage_copy_failed", error.message());
    }
    std::ofstream out(stagedManifest, std::ios::binary | std::ios::trunc);
    if (!out) {
        removeIfPresent(stagedPayload);
        return blocked("project_manifest_stage_write_failed", "Project asset manifest staging file could not be opened.");
    }
    out << serializeAssetPromotionManifest(projectManifest).dump(2);
    out << '\n';
    if (!out) {
        out.close();
        removeIfPresent(stagedPayload);
        removeIfPresent(stagedManifest);
        return blocked("project_manifest_stage_write_failed", "Project asset manifest staging file could not be written.");
    }
    out.close();

    const auto payloadBackup = siblingWorkingPath(destinationPayload, "backup-payload");
    const auto manifestBackup = siblingWorkingPath(destinationManifest, "backup-manifest");
    const bool hadPayload = std::filesystem::exists(destinationPayload);
    const bool hadManifest = std::filesystem::exists(destinationManifest);
    const auto transactionPath = journalPathFor(projectRoot);
    std::filesystem::create_directories(transactionPath.parent_path(), error);
    if (error) {
        removeIfPresent(stagedPayload);
        removeIfPresent(stagedManifest);
        return blocked("project_attachment_journal_create_failed", error.message());
    }
    const auto relativePath = [&](const std::filesystem::path& path) {
        return path.lexically_relative(projectRoot).generic_string();
    };
    const nlohmann::json transaction = {
        {"schema", "urpg.asset_attachment_transaction.v1"},
        {"state", "prepared"},
        {"payload_path", relativePath(destinationPayload)},
        {"manifest_path", relativePath(destinationManifest)},
        {"staged_payload_path", relativePath(stagedPayload)},
        {"staged_manifest_path", relativePath(stagedManifest)},
        {"payload_backup_path", relativePath(payloadBackup)},
        {"manifest_backup_path", relativePath(manifestBackup)},
        {"had_payload", hadPayload},
        {"had_manifest", hadManifest},
    };
    auto mutableTransaction = transaction;
    if (!writeJsonFile(transactionPath, mutableTransaction)) {
        removeIfPresent(stagedPayload);
        removeIfPresent(stagedManifest);
        removeIfPresent(transactionPath);
        return blocked("project_attachment_journal_write_failed", "Attachment transaction journal could not be written.");
    }
    bool payloadBackedUp = false;
    bool manifestBackedUp = false;
    bool payloadPublished = false;
    bool manifestPublished = false;
    const auto rollback = [&] {
        if (manifestPublished) removeIfPresent(destinationManifest);
        if (payloadPublished) removeIfPresent(destinationPayload);
        if (manifestBackedUp) {
            std::error_code restoreError;
            std::filesystem::rename(manifestBackup, destinationManifest, restoreError);
        }
        if (payloadBackedUp) {
            std::error_code restoreError;
            std::filesystem::rename(payloadBackup, destinationPayload, restoreError);
        }
        removeIfPresent(stagedPayload);
        removeIfPresent(stagedManifest);
        removeIfPresent(transactionPath);
    };
    if (hadPayload) {
        std::filesystem::rename(destinationPayload, payloadBackup, error);
        if (error) {
            rollback();
            return blocked("project_attachment_backup_failed", error.message());
        }
        payloadBackedUp = true;
        mutableTransaction["state"] = "payload_backed_up";
        if (!writeJsonFile(transactionPath, mutableTransaction)) {
            rollback();
            return blocked("project_attachment_journal_write_failed", "Attachment transaction journal could not be updated.");
        }
    }
    if (hadManifest) {
        std::filesystem::rename(destinationManifest, manifestBackup, error);
        if (error) {
            rollback();
            return blocked("project_attachment_backup_failed", error.message());
        }
        manifestBackedUp = true;
        mutableTransaction["state"] = "backed_up";
        if (!writeJsonFile(transactionPath, mutableTransaction)) {
            rollback();
            return blocked("project_attachment_journal_write_failed", "Attachment transaction journal could not be updated.");
        }
    }
    std::filesystem::rename(stagedPayload, destinationPayload, error);
    if (error) {
        rollback();
        return blocked("project_attachment_publish_failed", error.message());
    }
    payloadPublished = true;
    mutableTransaction["state"] = "payload_published";
    if (!writeJsonFile(transactionPath, mutableTransaction)) {
        rollback();
        return blocked("project_attachment_journal_write_failed", "Attachment transaction journal could not be updated.");
    }
    std::filesystem::rename(stagedManifest, destinationManifest, error);
    if (error) {
        rollback();
        return blocked("project_attachment_publish_failed", error.message());
    }
    manifestPublished = true;
    if (payloadBackedUp) removeIfPresent(payloadBackup);
    if (manifestBackedUp) removeIfPresent(manifestBackup);
    removeIfPresent(transactionPath);

    ProjectAssetAttachmentResult result;
    result.success = true;
    result.code = "project_asset_attached";
    result.message = "Promoted asset was attached to the project.";
    result.payloadPath = destinationPayload;
    result.manifestPath = destinationManifest;
    return result;
}

} // namespace urpg::assets
