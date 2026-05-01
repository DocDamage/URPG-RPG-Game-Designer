#pragma once

#include "engine/core/assets/asset_promotion_manifest.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace urpg::assets {

enum class AssetImportSourceKind {
    Folder,
    File,
    Zip,
    ExternalArchive,
    UnsupportedArchive,
};

enum class AssetImportStatus {
    Pending,
    Quarantined,
    ReviewReady,
    Failed,
};

struct AssetImportDiagnostic {
    std::string code;
    std::string message;
    std::string path;
};

struct AssetImportRecord {
    AssetImportRecord() = default;
    AssetImportRecord(std::string assetId_, std::string relativePath_, std::string normalizedPath_,
                      std::string extension_, std::string mediaKind_, std::string category_, std::string pack_,
                      std::string sha256_, uint64_t sizeBytes_, int32_t width_, int32_t height_,
                      int32_t durationMs_, bool duplicate_, std::string duplicateOf_, bool sourceOnly_,
                      bool toolingOnly_, bool runtimeReady_, bool licenseRequired_,
                      std::vector<std::string> diagnostics_,
                      std::string conversionTargetPath_ = {},
                      std::vector<std::string> conversionCommand_ = {},
                      bool conversionRequired_ = false,
                      std::string sequenceId_ = {},
                      int32_t sequenceFrameIndex_ = -1,
                      int32_t sequenceFrameCount_ = 0,
                      bool previewAvailable_ = false,
                      std::string previewKind_ = "none",
                      std::string noPreviewDiagnostic_ = {})
        : assetId(std::move(assetId_)),
          relativePath(std::move(relativePath_)),
          normalizedPath(std::move(normalizedPath_)),
          extension(std::move(extension_)),
          mediaKind(std::move(mediaKind_)),
          category(std::move(category_)),
          pack(std::move(pack_)),
          sha256(std::move(sha256_)),
          sizeBytes(sizeBytes_),
          width(width_),
          height(height_),
          durationMs(durationMs_),
          duplicate(duplicate_),
          duplicateOf(std::move(duplicateOf_)),
          sourceOnly(sourceOnly_),
          toolingOnly(toolingOnly_),
          runtimeReady(runtimeReady_),
          licenseRequired(licenseRequired_),
          diagnostics(std::move(diagnostics_)),
          conversionTargetPath(std::move(conversionTargetPath_)),
          conversionCommand(std::move(conversionCommand_)),
          conversionRequired(conversionRequired_),
          sequenceId(std::move(sequenceId_)),
          sequenceFrameIndex(sequenceFrameIndex_),
          sequenceFrameCount(sequenceFrameCount_),
          previewAvailable(previewAvailable_),
          previewKind(std::move(previewKind_)),
          noPreviewDiagnostic(std::move(noPreviewDiagnostic_)) {}

    std::string assetId;
    std::string relativePath;
    std::string normalizedPath;
    std::string extension;
    std::string mediaKind;
    std::string category;
    std::string pack;
    std::string sha256;
    uint64_t sizeBytes = 0;
    int32_t width = 0;
    int32_t height = 0;
    int32_t durationMs = 0;
    bool duplicate = false;
    std::string duplicateOf;
    bool sourceOnly = false;
    bool toolingOnly = false;
    bool runtimeReady = false;
    bool licenseRequired = true;
    std::vector<std::string> diagnostics;
    std::string conversionTargetPath;
    std::vector<std::string> conversionCommand;
    bool conversionRequired = false;
    std::string sequenceId;
    int32_t sequenceFrameIndex = -1;
    int32_t sequenceFrameCount = 0;
    bool previewAvailable = false;
    std::string previewKind = "none";
    std::string noPreviewDiagnostic;
};

struct AssetImportSummary {
    size_t filesScanned = 0;
    size_t readyCount = 0;
    size_t needsConversionCount = 0;
    size_t duplicateCount = 0;
    size_t missingLicenseCount = 0;
    size_t unsupportedCount = 0;
    size_t sourceOnlyCount = 0;
    size_t errorCount = 0;
};

struct AssetImportSession {
    std::string schemaVersion = "1.0.0";
    std::string sessionId;
    AssetImportSourceKind sourceKind = AssetImportSourceKind::Folder;
    std::string sourcePath;
    std::string managedSourceRoot;
    std::string createdAt;
    AssetImportStatus status = AssetImportStatus::Pending;
    AssetImportSummary summary;
    std::vector<AssetImportRecord> records;
    std::vector<AssetImportDiagnostic> diagnostics;
};

const char* toString(AssetImportSourceKind kind);
AssetImportSourceKind assetImportSourceKindFromString(const std::string& value);
const char* toString(AssetImportStatus status);
AssetImportStatus assetImportStatusFromString(const std::string& value);

nlohmann::json serializeAssetImportSession(const AssetImportSession& session);
AssetImportSession deserializeAssetImportSession(const nlohmann::json& value);
AssetImportSummary summarizeAssetImportSession(const AssetImportSession& session);
AssetPromotionManifest planAssetPromotionManifest(const AssetImportSession& session, const AssetImportRecord& record,
                                                  std::string licenseId, std::string promotedRoot,
                                                  bool includeInRuntime);
nlohmann::json buildAssetImportSessionRows(const std::vector<AssetImportSession>& sessions);
nlohmann::json buildAssetImportReviewRows(const std::vector<AssetImportSession>& sessions);

} // namespace urpg::assets
