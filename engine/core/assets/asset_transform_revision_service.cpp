#include "engine/core/assets/asset_transform_revision_service.h"

#include "engine/core/security/sha256.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <set>
#include <sstream>
#include <string_view>
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

namespace urpg::assets {
namespace {

std::string sha256File(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    const std::vector<std::uint8_t> bytes(std::istreambuf_iterator<char>(input), {});
    return security::Sha256::toHex(security::Sha256::compute(bytes));
}

std::string sha256Text(const std::string& value) {
    return security::Sha256::toHex(security::Sha256::compute({value.begin(), value.end()}));
}

AssetTransformRevisionResult blocked(std::string code, std::string message, std::vector<std::string> diagnostics = {}) {
    return {false, std::move(code), std::move(message), {}, {}, {}, std::move(diagnostics), {}};
}

bool isSafePathSegment(const std::string& value) {
    const auto path = std::filesystem::path(value);
    return !value.empty() && value != "." && value != ".." && !path.has_parent_path() && !path.has_root_path();
}

bool isSha256Hex(const std::string& value) {
    return value.size() == 64 && std::all_of(value.begin(), value.end(), [](unsigned char character) {
        return std::isxdigit(character) != 0;
    });
}

bool hasStagedRevisionSuffix(const std::string_view filename, const std::string_view suffix) {
    if (!filename.ends_with(suffix)) return false;
    return isSha256Hex(std::string(filename.substr(0, filename.size() - suffix.size())));
}

bool isStagedRevisionFileName(const std::string_view filename) {
    return hasStagedRevisionSuffix(filename, ".png.tmp") || hasStagedRevisionSuffix(filename, ".wav.tmp") ||
           hasStagedRevisionSuffix(filename, ".json.tmp");
}

bool isStagedTilesetDirectoryName(const std::string_view filename) {
    return hasStagedRevisionSuffix(filename, ".tiles.tmp");
}

bool isEligibleSource(const AssetPromotionManifest& source, AssetTransformRevisionResult* result) {
    const auto diagnostics = validateAssetPromotionManifest(source);
    if (!diagnostics.empty()) {
        *result = blocked("asset_transform_source_invalid", "The promoted source manifest is not eligible for transformation.",
                          diagnostics);
        return false;
    }
    if (source.status != AssetPromotionStatus::RuntimeReady || !source.package.includeInRuntime) {
        *result = blocked("asset_transform_source_not_runtime_ready", "Only runtime-ready promoted assets can create revisions.");
        return false;
    }
    if (!isSafePathSegment(source.assetId)) {
        *result = blocked("asset_transform_asset_id_invalid", "The source asset ID is not safe for a derived revision path.");
        return false;
    }
    return true;
}

uint16_t readLe16(const std::vector<uint8_t>& bytes, const size_t offset) {
    return static_cast<uint16_t>(bytes[offset]) | (static_cast<uint16_t>(bytes[offset + 1]) << 8U);
}

uint32_t readLe32(const std::vector<uint8_t>& bytes, const size_t offset) {
    return static_cast<uint32_t>(bytes[offset]) | (static_cast<uint32_t>(bytes[offset + 1]) << 8U) |
           (static_cast<uint32_t>(bytes[offset + 2]) << 16U) | (static_cast<uint32_t>(bytes[offset + 3]) << 24U);
}

void appendLe16(std::vector<uint8_t>* bytes, const uint16_t value) {
    bytes->push_back(static_cast<uint8_t>(value & 0xFFU));
    bytes->push_back(static_cast<uint8_t>((value >> 8U) & 0xFFU));
}

void appendLe32(std::vector<uint8_t>* bytes, const uint32_t value) {
    for (uint32_t shift = 0; shift < 32; shift += 8) bytes->push_back(static_cast<uint8_t>((value >> shift) & 0xFFU));
}

struct Pcm16Wav {
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    std::vector<int16_t> samples;
};

bool readPcm16Wav(const std::filesystem::path& path, Pcm16Wav* wav, std::string* error) {
    std::ifstream input(path, std::ios::binary);
    const std::vector<uint8_t> bytes(std::istreambuf_iterator<char>(input), {});
    if (bytes.size() < 44 || std::string_view(reinterpret_cast<const char*>(bytes.data()), 4) != "RIFF" ||
        std::string_view(reinterpret_cast<const char*>(bytes.data() + 8), 4) != "WAVE") {
        *error = "The promoted audio payload is not a RIFF/WAVE file.";
        return false;
    }
    uint16_t format = 0;
    uint16_t channels = 0;
    uint32_t sampleRate = 0;
    uint16_t bitsPerSample = 0;
    const uint8_t* audio = nullptr;
    size_t audioSize = 0;
    for (size_t offset = 12; offset + 8 <= bytes.size();) {
        const auto chunkSize = static_cast<size_t>(readLe32(bytes, offset + 4));
        const auto payload = offset + 8;
        if (payload > bytes.size() || chunkSize > bytes.size() - payload) break;
        const std::string_view id(reinterpret_cast<const char*>(bytes.data() + offset), 4);
        if (id == "fmt " && chunkSize >= 16) {
            format = readLe16(bytes, payload);
            channels = readLe16(bytes, payload + 2);
            sampleRate = readLe32(bytes, payload + 4);
            bitsPerSample = readLe16(bytes, payload + 14);
        } else if (id == "data") {
            audio = bytes.data() + payload;
            audioSize = chunkSize;
        }
        offset = payload + chunkSize + (chunkSize % 2U);
    }
    if (format != 1 || channels == 0 || sampleRate == 0 || bitsPerSample != 16 || audio == nullptr ||
        audioSize == 0 || audioSize % (static_cast<size_t>(channels) * 2U) != 0) {
        *error = "Only non-empty PCM16 WAV payloads with valid frame alignment are supported.";
        return false;
    }
    wav->channels = channels;
    wav->sampleRate = sampleRate;
    wav->samples.resize(audioSize / 2U);
    for (size_t index = 0; index < wav->samples.size(); ++index) {
        const auto value = static_cast<uint16_t>(audio[index * 2U]) | (static_cast<uint16_t>(audio[index * 2U + 1]) << 8U);
        wav->samples[index] = static_cast<int16_t>(value);
    }
    return true;
}

bool writePcm16Wav(const std::filesystem::path& path, const Pcm16Wav& wav) {
    if (wav.samples.size() > (std::numeric_limits<uint32_t>::max() / 2U)) return false;
    const auto dataSize = static_cast<uint32_t>(wav.samples.size() * 2U);
    const auto byteRate = wav.sampleRate * static_cast<uint32_t>(wav.channels) * 2U;
    std::vector<uint8_t> bytes;
    bytes.reserve(static_cast<size_t>(dataSize) + 44U);
    bytes.insert(bytes.end(), {'R', 'I', 'F', 'F'});
    appendLe32(&bytes, 36U + dataSize);
    bytes.insert(bytes.end(), {'W', 'A', 'V', 'E', 'f', 'm', 't', ' '});
    appendLe32(&bytes, 16U);
    appendLe16(&bytes, 1U);
    appendLe16(&bytes, wav.channels);
    appendLe32(&bytes, wav.sampleRate);
    appendLe32(&bytes, byteRate);
    appendLe16(&bytes, static_cast<uint16_t>(wav.channels * 2U));
    appendLe16(&bytes, 16U);
    bytes.insert(bytes.end(), {'d', 'a', 't', 'a'});
    appendLe32(&bytes, dataSize);
    for (const auto sample : wav.samples) appendLe16(&bytes, static_cast<uint16_t>(sample));
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    return output.good();
}

} // namespace

AssetTransformRevisionResult AssetTransformRevisionService::createAtlasMetadataRevision(
    const AssetAtlasMetadataPlan& plan) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(plan.source, &eligibility)) return eligibility;
    if (plan.operationId.empty() || plan.derivedRoot.empty() || plan.atlasWidth <= 0 || plan.atlasHeight <= 0 ||
        plan.frameWidth <= 0 || plan.frameHeight <= 0 || plan.atlasWidth % plan.frameWidth != 0 ||
        plan.atlasHeight % plan.frameHeight != 0) {
        return blocked("asset_transform_atlas_plan_invalid",
                       "Atlas dimensions and frame dimensions must be positive, divisible values.");
    }
    const auto sourcePath = std::filesystem::path(plan.source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) {
        return blocked("asset_transform_source_payload_missing", "The promoted source payload is missing.");
    }

    const auto sourceRevision = sha256File(sourcePath);
    const nlohmann::json identity = {{"schema", "urpg.asset_transform_revision.v1"},
                                     {"operation", "atlas_metadata"},
                                     {"operation_id", plan.operationId},
                                     {"source_asset_id", plan.source.assetId},
                                     {"source_revision", sourceRevision},
                                     {"atlas_width", plan.atlasWidth},
                                     {"atlas_height", plan.atlasHeight},
                                     {"frame_width", plan.frameWidth},
                                     {"frame_height", plan.frameHeight}};
    const auto derivedRevision = sha256Text(identity.dump());
    const auto outputDirectory = plan.derivedRoot / plan.source.assetId / "revisions";
    const auto manifestPath = outputDirectory / (derivedRevision + ".json");
    const nlohmann::json manifest = {
        {"schema", "urpg.asset_transform_revision.v1"},
        {"operation", "atlas_metadata"},
        {"operation_id", plan.operationId},
        {"source_asset_id", plan.source.assetId},
        {"source_promoted_path", plan.source.promotedPath},
        {"source_revision", sourceRevision},
        {"derived_revision", derivedRevision},
        {"atlas", {{"width", plan.atlasWidth}, {"height", plan.atlasHeight},
                    {"frame_width", plan.frameWidth}, {"frame_height", plan.frameHeight},
                    {"frame_count", (plan.atlasWidth / plan.frameWidth) * (plan.atlasHeight / plan.frameHeight)}}},
    };
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) return blocked("asset_transform_directory_create_failed", error.message());
    if (std::filesystem::is_regular_file(manifestPath)) {
        std::ifstream existing(manifestPath, std::ios::binary);
        const auto existingJson = nlohmann::json::parse(existing, nullptr, false);
        if (existingJson == manifest) {
            return {true, "asset_transform_revision_reused", "The identical derived revision already exists.",
                    sourceRevision, derivedRevision, manifestPath, {}, {}};
        }
        return blocked("asset_transform_revision_collision", "A different derived manifest occupies this revision ID.");
    }
    const auto stagingPath = manifestPath.string() + ".tmp";
    std::ofstream output(stagingPath, std::ios::binary | std::ios::trunc);
    output << manifest.dump(2) << '\n';
    output.close();
    if (!output) return blocked("asset_transform_manifest_write_failed", "The derived revision manifest could not be written.");
    std::filesystem::rename(stagingPath, manifestPath, error);
    if (error) {
        std::filesystem::remove(stagingPath, error);
        return blocked("asset_transform_manifest_publish_failed", error.message());
    }
    return {true, "asset_transform_revision_created", "A deterministic atlas metadata revision was created.",
            sourceRevision, derivedRevision, manifestPath, {}, {}};
}

AssetTransformRevisionResult AssetTransformRevisionService::createImageCropScaleRevision(
    const AssetImageCropScalePlan& plan) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(plan.source, &eligibility)) return eligibility;
    if (plan.operationId.empty() || plan.derivedRoot.empty() || plan.cropX < 0 || plan.cropY < 0 ||
        plan.cropWidth <= 0 || plan.cropHeight <= 0 || plan.outputWidth <= 0 || plan.outputHeight <= 0) {
        return blocked("asset_transform_crop_plan_invalid", "Crop and output dimensions must be positive and in bounds.");
    }
    const auto sourcePath = std::filesystem::path(plan.source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) {
        return blocked("asset_transform_source_payload_missing", "The promoted source payload is missing.");
    }
    int sourceWidth = 0;
    int sourceHeight = 0;
    int channels = 0;
    stbi_uc* decoded = stbi_load(sourcePath.string().c_str(), &sourceWidth, &sourceHeight, &channels, STBI_rgb_alpha);
    if (decoded == nullptr) {
        return blocked("asset_transform_source_decode_failed", stbi_failure_reason());
    }
    const auto decodedPixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>(decoded, stbi_image_free);
    if (plan.cropX + plan.cropWidth > sourceWidth || plan.cropY + plan.cropHeight > sourceHeight) {
        return blocked("asset_transform_crop_out_of_bounds", "The crop rectangle exceeds the promoted source image.");
    }
    const auto sourceRevision = sha256File(sourcePath);
    const nlohmann::json identity = {{"schema", "urpg.asset_transform_revision.v1"},
                                     {"operation", "image_crop_scale"}, {"operation_id", plan.operationId},
                                     {"source_asset_id", plan.source.assetId}, {"source_revision", sourceRevision},
                                     {"crop", {{"x", plan.cropX}, {"y", plan.cropY}, {"width", plan.cropWidth}, {"height", plan.cropHeight}}},
                                     {"output", {{"width", plan.outputWidth}, {"height", plan.outputHeight}}}};
    const auto derivedRevision = sha256Text(identity.dump());
    const auto outputDirectory = plan.derivedRoot / plan.source.assetId / "revisions";
    const auto outputPath = outputDirectory / (derivedRevision + ".png");
    const auto manifestPath = outputDirectory / (derivedRevision + ".json");
    const nlohmann::json manifest = {{"schema", "urpg.asset_transform_revision.v1"}, {"operation", "image_crop_scale"},
                                     {"operation_id", plan.operationId}, {"source_asset_id", plan.source.assetId},
                                     {"source_promoted_path", plan.source.promotedPath}, {"source_revision", sourceRevision},
                                     {"derived_revision", derivedRevision}, {"output_path", outputPath.generic_string()},
                                     {"crop", identity["crop"]}, {"output", identity["output"]}, {"resample", "nearest"}};
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) return blocked("asset_transform_directory_create_failed", error.message());
    if (std::filesystem::is_regular_file(manifestPath)) {
        std::ifstream existing(manifestPath, std::ios::binary);
        const auto existingJson = nlohmann::json::parse(existing, nullptr, false);
        if (existingJson == manifest && std::filesystem::is_regular_file(outputPath)) {
            return {true, "asset_transform_revision_reused", "The identical derived revision already exists.",
                    sourceRevision, derivedRevision, manifestPath, {}, outputPath};
        }
        return blocked("asset_transform_revision_collision", "A different or incomplete revision occupies this ID.");
    }
    std::vector<stbi_uc> outputPixels(static_cast<size_t>(plan.outputWidth) * plan.outputHeight * 4);
    for (int outputY = 0; outputY < plan.outputHeight; ++outputY) {
        const int sourceY = plan.cropY + (outputY * plan.cropHeight) / plan.outputHeight;
        for (int outputX = 0; outputX < plan.outputWidth; ++outputX) {
            const int sourceX = plan.cropX + (outputX * plan.cropWidth) / plan.outputWidth;
            const auto sourceOffset = static_cast<size_t>(sourceY * sourceWidth + sourceX) * 4;
            const auto outputOffset = static_cast<size_t>(outputY * plan.outputWidth + outputX) * 4;
            std::copy_n(decodedPixels.get() + sourceOffset, 4, outputPixels.data() + outputOffset);
        }
    }
    const auto stagedOutput = outputPath.string() + ".tmp";
    if (stbi_write_png(stagedOutput.c_str(), plan.outputWidth, plan.outputHeight, 4, outputPixels.data(), plan.outputWidth * 4) == 0) {
        return blocked("asset_transform_output_write_failed", "The derived PNG could not be written.");
    }
    std::filesystem::rename(stagedOutput, outputPath, error);
    if (error) { std::filesystem::remove(stagedOutput, error); return blocked("asset_transform_output_publish_failed", error.message()); }
    const auto stagedManifest = manifestPath.string() + ".tmp";
    std::ofstream output(stagedManifest, std::ios::binary | std::ios::trunc);
    output << manifest.dump(2) << '\n';
    output.close();
    if (!output) { std::filesystem::remove(outputPath, error); return blocked("asset_transform_manifest_write_failed", "The derived revision manifest could not be written."); }
    std::filesystem::rename(stagedManifest, manifestPath, error);
    if (error) { std::filesystem::remove(stagedManifest, error); std::filesystem::remove(outputPath, error); return blocked("asset_transform_manifest_publish_failed", error.message()); }
    return {true, "asset_transform_revision_created", "A deterministic image crop and scale revision was created.",
            sourceRevision, derivedRevision, manifestPath, {}, outputPath};
}

AssetTransformRevisionResult AssetTransformRevisionService::createImagePaletteRevision(
    const AssetImagePalettePlan& plan) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(plan.source, &eligibility)) return eligibility;
    if (plan.operationId.empty() || plan.derivedRoot.empty() || plan.colorsRgba.size() < 2U ||
        plan.colorsRgba.size() > 256U) {
        return blocked("asset_transform_palette_plan_invalid", "Palette revisions require two to 256 explicit colors.");
    }
    std::set<uint32_t> uniqueColors(plan.colorsRgba.begin(), plan.colorsRgba.end());
    if (uniqueColors.size() != plan.colorsRgba.size()) {
        return blocked("asset_transform_palette_plan_invalid", "Palette colors must be unique and ordered.");
    }
    const auto sourcePath = std::filesystem::path(plan.source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) {
        return blocked("asset_transform_source_payload_missing", "The promoted source payload is missing.");
    }
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* decoded = stbi_load(sourcePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (decoded == nullptr) return blocked("asset_transform_source_decode_failed", stbi_failure_reason());
    const auto decodedPixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>(decoded, stbi_image_free);
    const auto sourceRevision = sha256File(sourcePath);
    const nlohmann::json identity = {{"schema", "urpg.asset_transform_revision.v1"}, {"operation", "image_palette"},
                                     {"operation_id", plan.operationId}, {"source_asset_id", plan.source.assetId},
                                     {"source_revision", sourceRevision}, {"palette_rgba", plan.colorsRgba},
                                     {"dither", plan.dither ? "floyd_steinberg_rgba_fixed16" : "none"}};
    const auto derivedRevision = sha256Text(identity.dump());
    const auto outputDirectory = plan.derivedRoot / plan.source.assetId / "revisions";
    const auto outputPath = outputDirectory / (derivedRevision + ".png");
    const auto manifestPath = outputDirectory / (derivedRevision + ".json");
    const nlohmann::json manifest = {{"schema", "urpg.asset_transform_revision.v1"}, {"operation", "image_palette"},
                                     {"operation_id", plan.operationId}, {"source_asset_id", plan.source.assetId},
                                     {"source_promoted_path", plan.source.promotedPath}, {"source_revision", sourceRevision},
                                     {"derived_revision", derivedRevision}, {"output_path", outputPath.generic_string()},
                                     {"width", width}, {"height", height}, {"palette_rgba", plan.colorsRgba},
                                     {"mapping", "nearest_rgba_squared"},
                                     {"dither", plan.dither ? "floyd_steinberg_rgba_fixed16" : "none"}};
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) return blocked("asset_transform_directory_create_failed", error.message());
    if (std::filesystem::is_regular_file(manifestPath)) {
        std::ifstream existing(manifestPath, std::ios::binary);
        if (nlohmann::json::parse(existing, nullptr, false) == manifest && std::filesystem::is_regular_file(outputPath)) {
            return {true, "asset_transform_revision_reused", "The identical palette revision already exists.",
                    sourceRevision, derivedRevision, manifestPath, {}, outputPath};
        }
        return blocked("asset_transform_revision_collision", "A different or incomplete revision occupies this ID.");
    }
    const auto pixelCount = static_cast<size_t>(width) * height;
    std::vector<stbi_uc> outputPixels(pixelCount * 4U);
    std::vector<int32_t> ditherErrors(plan.dither ? pixelCount * 4U : 0U, 0);
    const auto addDitherError = [&](const int x, const int y, const size_t channel, const int32_t error,
                                    const int weight) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            ditherErrors[(static_cast<size_t>(y) * width + x) * 4U + channel] += (error * weight) / 16;
        }
    };
    for (int y = 0; y < height; ++y) for (int x = 0; x < width; ++x) {
        const auto pixel = static_cast<size_t>(y) * width + x;
        const auto* source = decodedPixels.get() + (pixel * 4U);
        int32_t working[4] = {};
        for (size_t channel = 0; channel < 4U; ++channel) {
            const auto error = plan.dither ? ditherErrors[pixel * 4U + channel] : 0;
            working[channel] = std::clamp(static_cast<int32_t>(source[channel]) * 16 + error, 0, 255 * 16);
        }
        uint32_t selected = plan.colorsRgba.front();
        uint64_t bestDistance = std::numeric_limits<uint64_t>::max();
        for (const auto color : plan.colorsRgba) {
            uint64_t distance = 0;
            for (size_t channel = 0; channel < 4U; ++channel) {
                const auto paletteValue = static_cast<int32_t>((color >> ((3U - channel) * 8U)) & 0xFFU) * 16;
                const auto delta = working[channel] - paletteValue;
                distance += static_cast<uint64_t>(delta * delta);
            }
            if (distance < bestDistance) { bestDistance = distance; selected = color; }
        }
        for (size_t channel = 0; channel < 4U; ++channel) {
            const auto paletteValue = static_cast<int32_t>((selected >> ((3U - channel) * 8U)) & 0xFFU) * 16;
            const auto residual = working[channel] - paletteValue;
            outputPixels[pixel * 4U + channel] = static_cast<stbi_uc>(paletteValue / 16);
            if (plan.dither) {
                addDitherError(x + 1, y, channel, residual, 7);
                addDitherError(x - 1, y + 1, channel, residual, 3);
                addDitherError(x, y + 1, channel, residual, 5);
                addDitherError(x + 1, y + 1, channel, residual, 1);
            }
        }
    }
    const auto stagedOutput = outputPath.string() + ".tmp";
    if (stbi_write_png(stagedOutput.c_str(), width, height, 4, outputPixels.data(), width * 4) == 0) {
        return blocked("asset_transform_output_write_failed", "The derived palette PNG could not be written.");
    }
    std::filesystem::rename(stagedOutput, outputPath, error);
    if (error) { std::filesystem::remove(stagedOutput, error); return blocked("asset_transform_output_publish_failed", error.message()); }
    const auto stagedManifest = manifestPath.string() + ".tmp";
    std::ofstream manifestOutput(stagedManifest, std::ios::binary | std::ios::trunc);
    manifestOutput << manifest.dump(2) << '\n';
    manifestOutput.close();
    if (!manifestOutput) { std::filesystem::remove(outputPath, error); return blocked("asset_transform_manifest_write_failed", "The derived manifest could not be written."); }
    std::filesystem::rename(stagedManifest, manifestPath, error);
    if (error) { std::filesystem::remove(stagedManifest, error); std::filesystem::remove(outputPath, error); return blocked("asset_transform_manifest_publish_failed", error.message()); }
    return {true, "asset_transform_revision_created", "A deterministic palette revision was created.",
            sourceRevision, derivedRevision, manifestPath, {}, outputPath};
}

AssetTransformRevisionResult AssetTransformRevisionService::createImagePaletteExtractRevision(
    const AssetImagePaletteExtractPlan& plan) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(plan.source, &eligibility)) return eligibility;
    if (plan.operationId.empty() || plan.derivedRoot.empty() || plan.maxColors < 2 || plan.maxColors > 256) {
        return blocked("asset_transform_palette_extract_plan_invalid",
                       "Automatic palette extraction requires a requested size from two to 256 colors.");
    }
    const auto sourcePath = std::filesystem::path(plan.source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) {
        return blocked("asset_transform_source_payload_missing", "The promoted source payload is missing.");
    }
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* decoded = stbi_load(sourcePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (decoded == nullptr) return blocked("asset_transform_source_decode_failed", stbi_failure_reason());
    const auto decodedPixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>(decoded, stbi_image_free);

    std::map<uint32_t, size_t> frequencies;
    const auto pixelCount = static_cast<size_t>(width) * height;
    for (size_t pixel = 0; pixel < pixelCount; ++pixel) {
        const auto* source = decodedPixels.get() + (pixel * 4U);
        const auto color = (static_cast<uint32_t>(source[0]) << 24U) |
                           (static_cast<uint32_t>(source[1]) << 16U) |
                           (static_cast<uint32_t>(source[2]) << 8U) | static_cast<uint32_t>(source[3]);
        ++frequencies[color];
    }
    if (frequencies.size() < 2U) {
        return blocked("asset_transform_palette_extract_insufficient_colors",
                       "Automatic palette extraction needs an image with at least two exact RGBA colors.");
    }
    std::vector<std::pair<uint32_t, size_t>> ranked(frequencies.begin(), frequencies.end());
    std::sort(ranked.begin(), ranked.end(), [](const auto& left, const auto& right) {
        return left.second != right.second ? left.second > right.second : left.first < right.first;
    });
    const auto selectedCount = std::min(static_cast<size_t>(plan.maxColors), ranked.size());
    std::vector<uint32_t> colors;
    colors.reserve(selectedCount);
    for (size_t index = 0; index < selectedCount; ++index) colors.push_back(ranked[index].first);

    const auto sourceRevision = sha256File(sourcePath);
    const nlohmann::json identity = {
        {"schema", "urpg.asset_transform_revision.v1"},
        {"operation", "image_palette_extract"},
        {"operation_id", plan.operationId},
        {"source_asset_id", plan.source.assetId},
        {"source_revision", sourceRevision},
        {"selection", "exact_rgba_frequency_desc_then_rgba_asc"},
        {"requested_max_colors", plan.maxColors},
        {"palette_rgba", colors},
        {"dither", plan.dither ? "floyd_steinberg_rgba_fixed16" : "none"},
    };
    const auto derivedRevision = sha256Text(identity.dump());
    const auto outputDirectory = plan.derivedRoot / plan.source.assetId / "revisions";
    const auto outputPath = outputDirectory / (derivedRevision + ".png");
    const auto manifestPath = outputDirectory / (derivedRevision + ".json");
    const nlohmann::json manifest = {
        {"schema", "urpg.asset_transform_revision.v1"},
        {"operation", "image_palette_extract"},
        {"operation_id", plan.operationId},
        {"source_asset_id", plan.source.assetId},
        {"source_promoted_path", plan.source.promotedPath},
        {"source_revision", sourceRevision},
        {"derived_revision", derivedRevision},
        {"output_path", outputPath.generic_string()},
        {"width", width},
        {"height", height},
        {"requested_max_colors", plan.maxColors},
        {"palette_rgba", colors},
        {"selection", "exact_rgba_frequency_desc_then_rgba_asc"},
        {"mapping", "nearest_rgba_squared"},
        {"dither", plan.dither ? "floyd_steinberg_rgba_fixed16" : "none"},
    };
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) return blocked("asset_transform_directory_create_failed", error.message());
    if (std::filesystem::is_regular_file(manifestPath)) {
        std::ifstream existing(manifestPath, std::ios::binary);
        if (nlohmann::json::parse(existing, nullptr, false) == manifest && std::filesystem::is_regular_file(outputPath)) {
            return {true, "asset_transform_revision_reused", "The identical extracted palette revision already exists.",
                    sourceRevision, derivedRevision, manifestPath, {}, outputPath};
        }
        return blocked("asset_transform_revision_collision", "A different or incomplete revision occupies this ID.");
    }
    std::vector<stbi_uc> outputPixels(pixelCount * 4U);
    std::vector<int32_t> ditherErrors(plan.dither ? pixelCount * 4U : 0U, 0);
    const auto addDitherError = [&](const int x, const int y, const size_t channel, const int32_t error,
                                    const int weight) {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            ditherErrors[(static_cast<size_t>(y) * width + x) * 4U + channel] += (error * weight) / 16;
        }
    };
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto pixel = static_cast<size_t>(y) * width + x;
            const auto* source = decodedPixels.get() + (pixel * 4U);
            int32_t working[4] = {};
            for (size_t channel = 0; channel < 4U; ++channel) {
                const auto error = plan.dither ? ditherErrors[pixel * 4U + channel] : 0;
                working[channel] = std::clamp(static_cast<int32_t>(source[channel]) * 16 + error, 0, 255 * 16);
            }
            uint32_t selected = colors.front();
            uint64_t bestDistance = std::numeric_limits<uint64_t>::max();
            for (const auto color : colors) {
                uint64_t distance = 0;
                for (size_t channel = 0; channel < 4U; ++channel) {
                    const auto paletteValue = static_cast<int32_t>((color >> ((3U - channel) * 8U)) & 0xFFU) * 16;
                    const auto delta = working[channel] - paletteValue;
                    distance += static_cast<uint64_t>(delta * delta);
                }
                if (distance < bestDistance) {
                    bestDistance = distance;
                    selected = color;
                }
            }
            for (size_t channel = 0; channel < 4U; ++channel) {
                const auto paletteValue = static_cast<int32_t>((selected >> ((3U - channel) * 8U)) & 0xFFU) * 16;
                const auto residual = working[channel] - paletteValue;
                outputPixels[pixel * 4U + channel] = static_cast<stbi_uc>(paletteValue / 16);
                if (plan.dither) {
                    addDitherError(x + 1, y, channel, residual, 7);
                    addDitherError(x - 1, y + 1, channel, residual, 3);
                    addDitherError(x, y + 1, channel, residual, 5);
                    addDitherError(x + 1, y + 1, channel, residual, 1);
                }
            }
        }
    }
    const auto stagedOutput = outputPath.string() + ".tmp";
    if (stbi_write_png(stagedOutput.c_str(), width, height, 4, outputPixels.data(), width * 4) == 0) {
        return blocked("asset_transform_output_write_failed", "The derived palette PNG could not be written.");
    }
    std::filesystem::rename(stagedOutput, outputPath, error);
    if (error) {
        std::filesystem::remove(stagedOutput, error);
        return blocked("asset_transform_output_publish_failed", error.message());
    }
    const auto stagedManifest = manifestPath.string() + ".tmp";
    std::ofstream manifestOutput(stagedManifest, std::ios::binary | std::ios::trunc);
    manifestOutput << manifest.dump(2) << '\n';
    manifestOutput.close();
    if (!manifestOutput) {
        std::filesystem::remove(outputPath, error);
        return blocked("asset_transform_manifest_write_failed", "The derived manifest could not be written.");
    }
    std::filesystem::rename(stagedManifest, manifestPath, error);
    if (error) {
        std::filesystem::remove(stagedManifest, error);
        std::filesystem::remove(outputPath, error);
        return blocked("asset_transform_manifest_publish_failed", error.message());
    }
    return {true, "asset_transform_revision_created", "A deterministic extracted palette revision was created.",
            sourceRevision, derivedRevision, manifestPath, {}, outputPath};
}

AssetTransformRevisionResult AssetTransformRevisionService::createTilesetSliceRevision(
    const AssetTilesetSlicePlan& plan) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(plan.source, &eligibility)) return eligibility;
    if (plan.operationId.empty() || plan.derivedRoot.empty() || plan.tileWidth <= 0 || plan.tileHeight <= 0 ||
        plan.margin < 0 || plan.spacing < 0) {
        return blocked("asset_transform_tileset_plan_invalid", "Tileset dimensions must be positive and margin/spacing non-negative.");
    }
    const auto sourcePath = std::filesystem::path(plan.source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) return blocked("asset_transform_source_payload_missing", "The promoted source payload is missing.");
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* decoded = stbi_load(sourcePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (decoded == nullptr) return blocked("asset_transform_source_decode_failed", stbi_failure_reason());
    const auto decodedPixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>(decoded, stbi_image_free);
    const auto usableWidth = width - (plan.margin * 2);
    const auto usableHeight = height - (plan.margin * 2);
    if (usableWidth < plan.tileWidth || usableHeight < plan.tileHeight ||
        (usableWidth + plan.spacing) % (plan.tileWidth + plan.spacing) != 0 ||
        (usableHeight + plan.spacing) % (plan.tileHeight + plan.spacing) != 0) {
        return blocked("asset_transform_tileset_grid_invalid", "Tileset dimensions do not form an exact margin/spacing grid.");
    }
    const auto columns = (usableWidth + plan.spacing) / (plan.tileWidth + plan.spacing);
    const auto rows = (usableHeight + plan.spacing) / (plan.tileHeight + plan.spacing);
    const auto sourceRevision = sha256File(sourcePath);
    const nlohmann::json identity = {{"schema", "urpg.asset_transform_revision.v1"}, {"operation", "tileset_slice"},
                                     {"operation_id", plan.operationId}, {"source_asset_id", plan.source.assetId},
                                     {"source_revision", sourceRevision}, {"tile_width", plan.tileWidth},
                                     {"tile_height", plan.tileHeight}, {"margin", plan.margin}, {"spacing", plan.spacing}};
    const auto derivedRevision = sha256Text(identity.dump());
    const auto outputDirectory = plan.derivedRoot / plan.source.assetId / "revisions";
    const auto tileDirectory = outputDirectory / (derivedRevision + ".tiles");
    const auto manifestPath = outputDirectory / (derivedRevision + ".json");
    nlohmann::json outputs = nlohmann::json::array();
    for (int row = 0; row < rows; ++row) for (int column = 0; column < columns; ++column) {
        const auto index = row * columns + column;
        std::ostringstream filename;
        filename << std::setw(6) << std::setfill('0') << index << ".png";
        outputs.push_back((tileDirectory / filename.str()).generic_string());
    }
    const nlohmann::json manifest = {{"schema", "urpg.asset_transform_revision.v1"}, {"operation", "tileset_slice"},
                                     {"operation_id", plan.operationId}, {"source_asset_id", plan.source.assetId},
                                     {"source_promoted_path", plan.source.promotedPath}, {"source_revision", sourceRevision},
                                     {"derived_revision", derivedRevision}, {"output_paths", outputs},
                                     {"grid", {{"tile_width", plan.tileWidth}, {"tile_height", plan.tileHeight},
                                                {"margin", plan.margin}, {"spacing", plan.spacing},
                                                {"columns", columns}, {"rows", rows}, {"tile_count", columns * rows}}}};
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) return blocked("asset_transform_directory_create_failed", error.message());
    if (std::filesystem::is_regular_file(manifestPath)) {
        std::ifstream existing(manifestPath, std::ios::binary);
        if (nlohmann::json::parse(existing, nullptr, false) == manifest && std::filesystem::is_directory(tileDirectory)) {
            return {true, "asset_transform_revision_reused", "The identical tileset slice revision already exists.",
                    sourceRevision, derivedRevision, manifestPath, {}, tileDirectory};
        }
        return blocked("asset_transform_revision_collision", "A different or incomplete revision occupies this ID.");
    }
    const auto stagedDirectory = tileDirectory.string() + ".tmp";
    std::filesystem::create_directories(stagedDirectory, error);
    if (error) return blocked("asset_transform_output_write_failed", error.message());
    for (int row = 0; row < rows; ++row) for (int column = 0; column < columns; ++column) {
        std::vector<stbi_uc> tile(static_cast<size_t>(plan.tileWidth) * plan.tileHeight * 4U);
        for (int y = 0; y < plan.tileHeight; ++y) for (int x = 0; x < plan.tileWidth; ++x) {
            const auto sourceX = plan.margin + column * (plan.tileWidth + plan.spacing) + x;
            const auto sourceY = plan.margin + row * (plan.tileHeight + plan.spacing) + y;
            std::copy_n(decodedPixels.get() + static_cast<size_t>(sourceY * width + sourceX) * 4U, 4,
                        tile.data() + static_cast<size_t>(y * plan.tileWidth + x) * 4U);
        }
        const auto index = row * columns + column;
        std::ostringstream filename;
        filename << std::setw(6) << std::setfill('0') << index << ".png";
        if (stbi_write_png((std::filesystem::path(stagedDirectory) / filename.str()).string().c_str(), plan.tileWidth,
                           plan.tileHeight, 4, tile.data(), plan.tileWidth * 4) == 0) {
            std::filesystem::remove_all(stagedDirectory, error);
            return blocked("asset_transform_output_write_failed", "A derived tile PNG could not be written.");
        }
    }
    std::filesystem::rename(stagedDirectory, tileDirectory, error);
    if (error) { std::filesystem::remove_all(stagedDirectory, error); return blocked("asset_transform_output_publish_failed", error.message()); }
    const auto stagedManifest = manifestPath.string() + ".tmp";
    std::ofstream manifestOutput(stagedManifest, std::ios::binary | std::ios::trunc);
    manifestOutput << manifest.dump(2) << '\n';
    manifestOutput.close();
    if (!manifestOutput) { std::filesystem::remove_all(tileDirectory, error); return blocked("asset_transform_manifest_write_failed", "The derived manifest could not be written."); }
    std::filesystem::rename(stagedManifest, manifestPath, error);
    if (error) { std::filesystem::remove(stagedManifest, error); std::filesystem::remove_all(tileDirectory, error); return blocked("asset_transform_manifest_publish_failed", error.message()); }
    return {true, "asset_transform_revision_created", "A deterministic tileset slice revision was created.",
            sourceRevision, derivedRevision, manifestPath, {}, tileDirectory};
}

AssetAudioSourceInspectionResult AssetTransformRevisionService::inspectAudioTrimFadeGainSource(
    const AssetPromotionManifest& source) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(source, &eligibility)) {
        return {false, eligibility.code, eligibility.message};
    }
    const auto sourcePath = std::filesystem::path(source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) {
        return {false, "asset_transform_source_payload_missing", "The promoted source payload is missing."};
    }
    Pcm16Wav wav;
    std::string wavError;
    if (!readPcm16Wav(sourcePath, &wav, &wavError)) {
        return {false, "asset_transform_audio_decode_unsupported", wavError};
    }
    const auto frameCount = static_cast<uint64_t>(wav.samples.size() / wav.channels);
    constexpr size_t waveformBucketCount = 128;
    std::vector<float> waveform(waveformBucketCount, 0.0F);
    for (uint64_t frame = 0; frame < frameCount; ++frame) {
        const auto bucket = std::min<size_t>(
            waveform.size() - 1U, static_cast<size_t>((frame * waveform.size()) / frameCount));
        for (uint16_t channel = 0; channel < wav.channels; ++channel) {
            const auto sample = wav.samples[static_cast<size_t>(frame * wav.channels + channel)];
            const auto amplitude = std::abs(static_cast<int32_t>(sample));
            waveform[bucket] = std::max(waveform[bucket], static_cast<float>(amplitude) / 32768.0F);
        }
    }
    return {true, "asset_audio_source_inspection_ready", "PCM16 WAV source inspection is ready.", sha256File(sourcePath),
            wav.channels, wav.sampleRate, frameCount, (frameCount * 1000U) / wav.sampleRate, std::move(waveform)};
}

AssetTransformRevisionResult AssetTransformRevisionService::createAudioTrimFadeGainRevision(
    const AssetAudioTrimFadeGainPlan& plan) const {
    AssetTransformRevisionResult eligibility;
    if (!isEligibleSource(plan.source, &eligibility)) return eligibility;
    if (plan.operationId.empty() || plan.derivedRoot.empty() || plan.endFrame <= plan.startFrame ||
        plan.gainMilliDb < -96000 || plan.gainMilliDb > 24000 ||
        ((plan.loopStartFrame < 0) != (plan.loopEndFrame < 0))) {
        return blocked("asset_transform_audio_plan_invalid", "Audio trim, gain, and loop parameters are invalid.");
    }
    const auto sourcePath = std::filesystem::path(plan.source.promotedPath);
    if (!std::filesystem::is_regular_file(sourcePath)) {
        return blocked("asset_transform_source_payload_missing", "The promoted source payload is missing.");
    }
    Pcm16Wav source;
    std::string wavError;
    if (!readPcm16Wav(sourcePath, &source, &wavError)) {
        return blocked("asset_transform_audio_decode_unsupported", wavError);
    }
    const auto sourceFrames = static_cast<uint64_t>(source.samples.size() / source.channels);
    const auto outputFrames = plan.endFrame - plan.startFrame;
    if (plan.endFrame > sourceFrames || plan.fadeInFrames > outputFrames || plan.fadeOutFrames > outputFrames ||
        (plan.loopStartFrame >= 0 && (plan.loopStartFrame >= plan.loopEndFrame ||
                                      static_cast<uint64_t>(plan.loopEndFrame) > outputFrames))) {
        return blocked("asset_transform_audio_range_invalid", "Audio trim, fade, or loop points exceed the source/output range.");
    }
    const auto sourceRevision = sha256File(sourcePath);
    const nlohmann::json identity = {{"schema", "urpg.asset_transform_revision.v1"},
                                     {"operation", "audio_trim_fade_gain_pcm16"}, {"operation_id", plan.operationId},
                                     {"source_asset_id", plan.source.assetId}, {"source_revision", sourceRevision},
                                     {"start_frame", plan.startFrame}, {"end_frame", plan.endFrame},
                                     {"fade_in_frames", plan.fadeInFrames}, {"fade_out_frames", plan.fadeOutFrames},
                                     {"gain_milli_db", plan.gainMilliDb}, {"loop_start_frame", plan.loopStartFrame},
                                     {"loop_end_frame", plan.loopEndFrame},
                                     {"audio_quality_contract", "loop_seam_sample_delta_v1"}};
    const auto derivedRevision = sha256Text(identity.dump());
    const auto outputDirectory = plan.derivedRoot / plan.source.assetId / "revisions";
    const auto outputPath = outputDirectory / (derivedRevision + ".wav");
    const auto manifestPath = outputDirectory / (derivedRevision + ".json");
    const auto gain = std::pow(10.0, static_cast<double>(plan.gainMilliDb) / 20000.0);
    Pcm16Wav output{source.channels, source.sampleRate, {}};
    output.samples.reserve(static_cast<size_t>(outputFrames) * source.channels);
    double sumSquares = 0.0;
    int32_t peak = 0;
    std::vector<float> waveform(64, 0.0F);
    for (uint64_t frame = 0; frame < outputFrames; ++frame) {
        double factor = gain;
        if (plan.fadeInFrames != 0 && frame < plan.fadeInFrames) factor *= static_cast<double>(frame) / plan.fadeInFrames;
        if (plan.fadeOutFrames != 0 && frame >= outputFrames - plan.fadeOutFrames) {
            factor *= static_cast<double>(outputFrames - frame - 1U) / plan.fadeOutFrames;
        }
        const auto bucket = std::min<size_t>(waveform.size() - 1U, static_cast<size_t>((frame * waveform.size()) / outputFrames));
        for (uint16_t channel = 0; channel < source.channels; ++channel) {
            const auto sourceSample = source.samples[static_cast<size_t>((plan.startFrame + frame) * source.channels + channel)];
            const auto scaled = static_cast<int32_t>(std::lround(static_cast<double>(sourceSample) * factor));
            const auto clamped = std::clamp(scaled, -32768, 32767);
            output.samples.push_back(static_cast<int16_t>(clamped));
            const auto amplitude = std::abs(clamped);
            peak = std::max(peak, amplitude);
            sumSquares += static_cast<double>(clamped) * clamped;
            waveform[bucket] = std::max(waveform[bucket], static_cast<float>(amplitude) / 32768.0F);
        }
    }
    const auto rms = std::sqrt(sumSquares / static_cast<double>(output.samples.size()));
    const auto peakDbfs = peak == 0 ? -std::numeric_limits<double>::infinity() : 20.0 * std::log10(static_cast<double>(peak) / 32768.0);
    const auto rmsDbfs = rms == 0.0 ? -std::numeric_limits<double>::infinity() : 20.0 * std::log10(rms / 32768.0);
    nlohmann::json loopSeam = {{"enabled", false},
                               {"metrics", "pcm16_adjacent_boundary_delta_not_listening_test"}};
    if (plan.loopStartFrame >= 0) {
        int32_t maxDelta = 0;
        double sumDeltaSquares = 0.0;
        for (uint16_t channel = 0; channel < output.channels; ++channel) {
            const auto loopEndSample = static_cast<int32_t>(
                output.samples[static_cast<size_t>((plan.loopEndFrame - 1) * output.channels + channel)]);
            const auto loopStartSample = static_cast<int32_t>(
                output.samples[static_cast<size_t>(plan.loopStartFrame * output.channels + channel)]);
            const auto delta = std::abs(loopEndSample - loopStartSample);
            maxDelta = std::max(maxDelta, delta);
            sumDeltaSquares += static_cast<double>(delta) * delta;
        }
        loopSeam = {{"enabled", true},
                    {"start_frame", plan.loopStartFrame},
                    {"end_frame", plan.loopEndFrame},
                    {"max_normalized_delta", static_cast<double>(maxDelta) / 65535.0},
                    {"rms_normalized_delta", std::sqrt(sumDeltaSquares / output.channels) / 65535.0},
                    {"metrics", "pcm16_adjacent_boundary_delta_not_listening_test"}};
    }
    const nlohmann::json manifest = {{"schema", "urpg.asset_transform_revision.v1"},
                                     {"operation", "audio_trim_fade_gain_pcm16"}, {"operation_id", plan.operationId},
                                     {"audio_quality_contract", "loop_seam_sample_delta_v1"},
                                     {"source_asset_id", plan.source.assetId}, {"source_promoted_path", plan.source.promotedPath},
                                     {"source_revision", sourceRevision}, {"derived_revision", derivedRevision},
                                     {"output_path", outputPath.generic_string()}, {"codec", "pcm_s16le_wav"},
                                     {"sample_rate", source.sampleRate}, {"channels", source.channels},
                                     {"frame_count", outputFrames}, {"duration_ms", (outputFrames * 1000U) / source.sampleRate},
                                     {"trim", {{"start_frame", plan.startFrame}, {"end_frame", plan.endFrame}}},
                                     {"fade", {{"in_frames", plan.fadeInFrames}, {"out_frames", plan.fadeOutFrames}}},
                                     {"gain_milli_db", plan.gainMilliDb}, {"loop", {{"start_frame", plan.loopStartFrame}, {"end_frame", plan.loopEndFrame}}},
                                     {"quality", {{"peak_dbfs", peakDbfs}, {"rms_dbfs", rmsDbfs},
                                                  {"metrics", "sample_peak_and_rms_not_lufs"}, {"loop_seam", loopSeam}}},
                                     {"waveform_peaks", waveform}};
    std::error_code error;
    std::filesystem::create_directories(outputDirectory, error);
    if (error) return blocked("asset_transform_directory_create_failed", error.message());
    if (std::filesystem::is_regular_file(manifestPath)) {
        std::ifstream existing(manifestPath, std::ios::binary);
        if (nlohmann::json::parse(existing, nullptr, false) == manifest && std::filesystem::is_regular_file(outputPath)) {
            return {true, "asset_transform_revision_reused", "The identical derived audio revision already exists.",
                    sourceRevision, derivedRevision, manifestPath, {}, outputPath};
        }
        return blocked("asset_transform_revision_collision", "A different or incomplete revision occupies this ID.");
    }
    const auto stagedOutput = outputPath.string() + ".tmp";
    if (!writePcm16Wav(stagedOutput, output)) return blocked("asset_transform_output_write_failed", "The derived WAV could not be written.");
    std::filesystem::rename(stagedOutput, outputPath, error);
    if (error) { std::filesystem::remove(stagedOutput, error); return blocked("asset_transform_output_publish_failed", error.message()); }
    const auto stagedManifest = manifestPath.string() + ".tmp";
    std::ofstream manifestOutput(stagedManifest, std::ios::binary | std::ios::trunc);
    manifestOutput << manifest.dump(2) << '\n';
    manifestOutput.close();
    if (!manifestOutput) { std::filesystem::remove(outputPath, error); return blocked("asset_transform_manifest_write_failed", "The derived manifest could not be written."); }
    std::filesystem::rename(stagedManifest, manifestPath, error);
    if (error) { std::filesystem::remove(stagedManifest, error); std::filesystem::remove(outputPath, error); return blocked("asset_transform_manifest_publish_failed", error.message()); }
    return {true, "asset_transform_revision_created", "A deterministic PCM16 WAV trim, fade, and gain revision was created.",
            sourceRevision, derivedRevision, manifestPath, {}, outputPath};
}

AssetTransformRevisionResult AssetTransformRevisionService::recoverStagedRevisions(
    const AssetTransformStagingRecoveryRequest& request) const {
    if (request.derivedRoot.empty() || !isSafePathSegment(request.assetId)) {
        return blocked("asset_transform_staging_recovery_request_invalid",
                       "Staged revision recovery requires a safe asset ID and derived root.");
    }

    const auto revisionsRoot = request.derivedRoot / request.assetId / "revisions";
    std::error_code error;
    if (!std::filesystem::exists(revisionsRoot, error)) {
        if (error) {
            return blocked("asset_transform_staging_recovery_inspection_failed", error.message());
        }
        return {true, "asset_transform_staging_recovery_clean", "No derived revision staging artifacts were found.",
                {}, {}, {}, {}, {}};
    }
    const auto rootStatus = std::filesystem::symlink_status(revisionsRoot, error);
    if (error || std::filesystem::is_symlink(rootStatus) || !std::filesystem::is_directory(rootStatus)) {
        return blocked("asset_transform_staging_recovery_root_invalid",
                       error ? error.message() : "The asset derived revision root is not a non-symlink directory.");
    }

    std::vector<std::filesystem::path> stagedFiles;
    std::vector<std::filesystem::path> stagedDirectories;
    for (std::filesystem::directory_iterator iterator(revisionsRoot, error), end; !error && iterator != end;
         iterator.increment(error)) {
        const auto filename = iterator->path().filename().string();
        const bool stagedFile = isStagedRevisionFileName(filename);
        const bool stagedDirectory = isStagedTilesetDirectoryName(filename);
        if (!stagedFile && !stagedDirectory) continue;

        const auto status = iterator->symlink_status(error);
        if (error || std::filesystem::is_symlink(status) ||
            (stagedFile && !std::filesystem::is_regular_file(status)) ||
            (stagedDirectory && !std::filesystem::is_directory(status))) {
            return blocked("asset_transform_staging_recovery_artifact_invalid",
                           error ? error.message() : "A staged revision artifact has an unexpected filesystem type.");
        }
        if (stagedFile) {
            stagedFiles.push_back(iterator->path());
        } else {
            stagedDirectories.push_back(iterator->path());
        }
    }
    if (error) return blocked("asset_transform_staging_recovery_inspection_failed", error.message());

    std::vector<std::string> diagnostics;
    for (const auto& stagedFile : stagedFiles) {
        if (!std::filesystem::remove(stagedFile, error) || error) {
            return blocked("asset_transform_staging_recovery_remove_failed",
                           error ? error.message() : "A staged revision file could not be removed.", std::move(diagnostics));
        }
        diagnostics.push_back("removed_staged_revision_file:" + stagedFile.filename().string());
    }
    for (const auto& stagedDirectory : stagedDirectories) {
        const auto removed = std::filesystem::remove_all(stagedDirectory, error);
        if (error || removed == 0) {
            return blocked("asset_transform_staging_recovery_remove_failed",
                           error ? error.message() : "A staged tileset directory could not be removed.", std::move(diagnostics));
        }
        diagnostics.push_back("removed_staged_tileset_directory:" + stagedDirectory.filename().string());
    }
    if (diagnostics.empty()) {
        return {true, "asset_transform_staging_recovery_clean", "No derived revision staging artifacts were found.",
                {}, {}, {}, {}, {}};
    }
    return {true, "asset_transform_staging_recovery_complete",
            "Removed deterministic unpublished derived revision staging artifacts.", {}, {}, {}, std::move(diagnostics), {}};
}

AssetTransformRevisionResult AssetTransformRevisionService::removeDerivedRevision(
    const AssetTransformRevisionRemovalRequest& request) const {
    if (request.derivedRoot.empty() || !isSafePathSegment(request.assetId) || !isSha256Hex(request.derivedRevision)) {
        return blocked("asset_transform_removal_request_invalid",
                       "Derived revision removal requires a safe asset ID, root, and SHA-256 revision ID.");
    }
    const auto manifestPath = request.derivedRoot / request.assetId / "revisions" / (request.derivedRevision + ".json");
    if (!std::filesystem::is_regular_file(manifestPath)) {
        return blocked("asset_transform_revision_missing", "The requested derived revision manifest does not exist.");
    }
    std::ifstream input(manifestPath, std::ios::binary);
    const auto manifest = nlohmann::json::parse(input, nullptr, false);
    input.close();
    if (manifest.is_discarded() || manifest.value("schema", "") != "urpg.asset_transform_revision.v1" ||
        manifest.value("source_asset_id", "") != request.assetId ||
        manifest.value("derived_revision", "") != request.derivedRevision) {
        return blocked("asset_transform_removal_manifest_invalid",
                       "The requested path is not the expected derived revision manifest.");
    }
    const auto attachmentReferenceRoot =
        manifestPath.parent_path() / (request.derivedRevision + ".attachment-refs");
    if (std::filesystem::is_directory(attachmentReferenceRoot)) {
        std::error_code referenceError;
        for (const auto& entry : std::filesystem::directory_iterator(attachmentReferenceRoot, referenceError)) {
            if (referenceError || !entry.is_regular_file()) {
                return blocked("asset_transform_attachment_reference_invalid",
                               "The derived revision has an invalid attachment reference ledger.");
            }
            std::ifstream referenceInput(entry.path(), std::ios::binary);
            const auto reference = nlohmann::json::parse(referenceInput, nullptr, false);
            if (reference.is_discarded() ||
                reference.value("schema", "") != "urpg.asset_transform_attachment_reference.v1") {
                return blocked("asset_transform_attachment_reference_invalid",
                               "The derived revision has an invalid attachment reference ledger.");
            }
            const auto state = reference.value("state", "");
            if (state == "attached") {
                return blocked("asset_transform_revision_attached",
                               "The derived revision is attached to a project and cannot be removed.");
            }
            if (state == "prepared") {
                return blocked("asset_transform_attachment_recovery_required",
                               "A derived revision attachment is incomplete and must be recovered before removal.");
            }
            return blocked("asset_transform_attachment_reference_invalid",
                           "The derived revision has an invalid attachment reference state.");
        }
        if (referenceError) {
            return blocked("asset_transform_attachment_reference_invalid",
                           "The derived revision attachment references could not be inspected.");
        }
    }
    const auto sourceRevision = manifest.value("source_revision", "");
    std::error_code error;
    if (const auto outputs = manifest.find("output_paths"); outputs != manifest.end()) {
        if (!outputs->is_array() || outputs->empty()) {
            return blocked("asset_transform_removal_output_invalid", "The derived manifest has invalid tileset outputs.");
        }
        const auto expectedDirectory = manifestPath.parent_path() / (request.derivedRevision + ".tiles");
        for (const auto& outputValue : *outputs) {
            if (!outputValue.is_string()) return blocked("asset_transform_removal_output_invalid", "The derived manifest has invalid tileset outputs.");
            const auto outputPath = std::filesystem::path(outputValue.get<std::string>()).lexically_normal();
            if (outputPath.parent_path().lexically_normal() != expectedDirectory.lexically_normal() ||
                outputPath.extension() != ".png" || !std::filesystem::is_regular_file(outputPath) ||
                !std::filesystem::remove(outputPath, error) || error) {
                return blocked("asset_transform_revision_output_remove_failed",
                               error ? error.message() : "A derived tileset output could not be removed.");
            }
        }
        if (!std::filesystem::remove(expectedDirectory, error) || error) {
            return blocked("asset_transform_revision_output_remove_failed",
                           error ? error.message() : "The derived tileset output directory was not empty.");
        }
    }
    if (const auto outputPathValue = manifest.value("output_path", std::string{}); !outputPathValue.empty()) {
        const auto outputPath = std::filesystem::path(outputPathValue).lexically_normal();
        const auto expectedParent = manifestPath.parent_path().lexically_normal();
        const auto expectedStem = request.derivedRevision;
        const auto outputExtension = outputPath.extension().string();
        if (outputPath.parent_path().lexically_normal() != expectedParent || outputPath.stem() != expectedStem ||
            (outputExtension != ".png" && outputExtension != ".wav")) {
            return blocked("asset_transform_removal_output_invalid",
                           "The derived manifest names an output outside its deterministic revision path.");
        }
        if (std::filesystem::exists(outputPath) && (!std::filesystem::is_regular_file(outputPath) ||
                                                    !std::filesystem::remove(outputPath, error) || error)) {
            return blocked("asset_transform_revision_output_remove_failed",
                           error ? error.message() : "The derived revision output could not be removed.");
        }
    }
    if (!std::filesystem::remove(manifestPath, error) || error) {
        return blocked("asset_transform_revision_remove_failed",
                       error ? error.message() : "The derived revision manifest could not be removed.");
    }
    return {true, "asset_transform_revision_removed", "The un-attached derived revision manifest was removed.",
            sourceRevision, request.derivedRevision, manifestPath, {}, {}};
}

} // namespace urpg::assets
