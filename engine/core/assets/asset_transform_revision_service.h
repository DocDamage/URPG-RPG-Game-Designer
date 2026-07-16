#pragma once

#include "engine/core/assets/asset_promotion_manifest.h"

#include <filesystem>
#include <string>
#include <vector>

namespace urpg::assets {

// First PFU-03 transform operation. It derives deterministic sprite-atlas
// metadata only; it never mutates the reviewed promoted media payload.
struct AssetAtlasMetadataPlan {
    std::string operationId;
    AssetPromotionManifest source;
    std::filesystem::path derivedRoot;
    int32_t atlasWidth = 0;
    int32_t atlasHeight = 0;
    int32_t frameWidth = 0;
    int32_t frameHeight = 0;
};

struct AssetTransformRevisionResult {
    bool success = false;
    std::string code;
    std::string message;
    std::string sourceRevision;
    std::string derivedRevision;
    std::filesystem::path manifestPath;
    std::vector<std::string> diagnostics;
    std::filesystem::path outputPath;
};

// Derived revisions are removable only while their adjacent attachment
// reference ledger has no prepared or attached project entry.
struct AssetTransformRevisionRemovalRequest {
    std::filesystem::path derivedRoot;
    std::string assetId;
    std::string derivedRevision;
};

// Recovery is deliberately narrower than a resumable transform job: it only
// removes known, unpublished same-directory staging names for one asset.
// Final revisions, attachment ledgers, and arbitrary temporary files remain
// outside this owner.
struct AssetTransformStagingRecoveryRequest {
    std::filesystem::path derivedRoot;
    std::string assetId;
};

struct AssetImageCropScalePlan {
    std::string operationId;
    AssetPromotionManifest source;
    std::filesystem::path derivedRoot;
    int32_t cropX = 0;
    int32_t cropY = 0;
    int32_t cropWidth = 0;
    int32_t cropHeight = 0;
    int32_t outputWidth = 0;
    int32_t outputHeight = 0;
};

// Maps each decoded RGBA source pixel to the nearest explicitly authored
// palette entry. The palette is ordered and part of the revision identity.
struct AssetImagePalettePlan {
    std::string operationId;
    AssetPromotionManifest source;
    std::filesystem::path derivedRoot;
    std::vector<uint32_t> colorsRgba;
    bool dither = false;
};

// Extracts an ordered exact-RGBA palette directly from a reviewed promoted
// image, then applies the same deterministic nearest-colour reduction as the
// explicit palette operation. Selection is frequency-descending with RGBA
// ascending tie breaking, and both the requested size and selected entries are
// part of the immutable derived revision identity.
struct AssetImagePaletteExtractPlan {
    std::string operationId;
    AssetPromotionManifest source;
    std::filesystem::path derivedRoot;
    int32_t maxColors = 0;
    bool dither = false;
};

struct AssetTilesetSlicePlan {
    std::string operationId;
    AssetPromotionManifest source;
    std::filesystem::path derivedRoot;
    int32_t tileWidth = 0;
    int32_t tileHeight = 0;
    int32_t margin = 0;
    int32_t spacing = 0;
};

// Initial F24 revision lane. The native implementation intentionally accepts
// only PCM16 WAV payloads; other codecs must first take the existing governed
// conversion/review path.
struct AssetAudioTrimFadeGainPlan {
    std::string operationId;
    AssetPromotionManifest source;
    std::filesystem::path derivedRoot;
    uint64_t startFrame = 0;
    uint64_t endFrame = 0; // Exclusive; must be greater than startFrame.
    uint64_t fadeInFrames = 0;
    uint64_t fadeOutFrames = 0;
    // Gain in thousandths of one dB, bounded to [-96 dB, +24 dB].
    int32_t gainMilliDb = 0;
    // Disabled when negative. Loop points are relative to the derived output.
    int64_t loopStartFrame = -1;
    int64_t loopEndFrame = -1;
};

// Read-only source information used to author exact frame-based audio
// transforms. This deliberately shares the PCM16 admission rule with the
// transform operation, rather than estimating frame positions from catalog
// duration metadata.
struct AssetAudioSourceInspectionResult {
    bool success = false;
    std::string code;
    std::string message;
    std::string sourceRevision;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint64_t frameCount = 0;
    uint64_t durationMs = 0;
    std::vector<float> waveformPeaks;
};

class AssetTransformRevisionService {
  public:
    AssetTransformRevisionResult createAtlasMetadataRevision(const AssetAtlasMetadataPlan& plan) const;
    AssetTransformRevisionResult createImageCropScaleRevision(const AssetImageCropScalePlan& plan) const;
    AssetTransformRevisionResult createImagePaletteRevision(const AssetImagePalettePlan& plan) const;
    AssetTransformRevisionResult createImagePaletteExtractRevision(const AssetImagePaletteExtractPlan& plan) const;
    AssetTransformRevisionResult createTilesetSliceRevision(const AssetTilesetSlicePlan& plan) const;
    AssetAudioSourceInspectionResult inspectAudioTrimFadeGainSource(const AssetPromotionManifest& source) const;
    AssetTransformRevisionResult createAudioTrimFadeGainRevision(const AssetAudioTrimFadeGainPlan& plan) const;
    AssetTransformRevisionResult recoverStagedRevisions(const AssetTransformStagingRecoveryRequest& request) const;
    AssetTransformRevisionResult removeDerivedRevision(const AssetTransformRevisionRemovalRequest& request) const;
};

} // namespace urpg::assets
