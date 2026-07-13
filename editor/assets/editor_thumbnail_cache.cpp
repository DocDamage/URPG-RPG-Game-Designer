#include "editor/assets/editor_thumbnail_cache.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <fstream>
#include <iterator>
#include <stb_image.h>
#include <system_error>
#include <utility>

namespace urpg::editor {
namespace {

std::string normalizedExtension(const std::filesystem::path& path) {
    auto extension = path.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char value) { return static_cast<char>(std::tolower(value)); });
    return extension;
}

bool supportsThumbnail(const std::filesystem::path& path) {
    const auto extension = normalizedExtension(path);
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" ||
           extension == ".gif";
}

EditorThumbnailDecodedImage resizeToPreview(const EditorThumbnailRequest& request, const unsigned char* pixels,
                                            int sourceWidth, int sourceHeight) {
    EditorThumbnailDecodedImage result;
    if (sourceWidth <= 0 || sourceHeight <= 0 || pixels == nullptr) {
        result.diagnosticCode = "thumbnail_decode_invalid_dimensions";
        return result;
    }
    const uint32_t width = std::max(1u, std::min(request.requestedWidth, static_cast<uint32_t>(sourceWidth)));
    const uint32_t height = std::max(1u, std::min(request.requestedHeight, static_cast<uint32_t>(sourceHeight)));
    result.width = width;
    result.height = height;
    result.rgba.resize(static_cast<size_t>(width) * height * 4u);
    for (uint32_t y = 0; y < height; ++y) {
        const auto sourceY = static_cast<uint32_t>((static_cast<uint64_t>(y) * sourceHeight) / height);
        for (uint32_t x = 0; x < width; ++x) {
            const auto sourceX = static_cast<uint32_t>((static_cast<uint64_t>(x) * sourceWidth) / width);
            const auto sourceOffset = (static_cast<size_t>(sourceY) * sourceWidth + sourceX) * 4u;
            const auto destinationOffset = (static_cast<size_t>(y) * width + x) * 4u;
            std::copy_n(pixels + sourceOffset, 4u, result.rgba.data() + destinationOffset);
        }
    }
    result.success = true;
    result.diagnosticCode = "thumbnail_ready";
    return result;
}

EditorThumbnailDecodedImage decodeGifFrame(const EditorThumbnailRequest& request) {
    std::ifstream input(request.sourcePath, std::ios::binary);
    const std::vector<stbi_uc> bytes{std::istreambuf_iterator<char>(input), {}};
    if (bytes.empty()) return {false, "thumbnail_decode_failed", 0, 0, 1, 0, {}};
    int* delays = nullptr;
    int width = 0, height = 0, frames = 0, channels = 0;
    stbi_uc* pixels = stbi_load_gif_from_memory(bytes.data(), static_cast<int>(bytes.size()), &delays, &width, &height,
                                                &frames, &channels, STBI_rgb_alpha);
    if (pixels == nullptr || frames <= 0) {
        if (delays) stbi_image_free(delays);
        return {false, "thumbnail_decode_failed", 0, 0, 1, 0, {}};
    }
    const auto frame = std::min(request.gifFrameIndex, static_cast<uint32_t>(frames - 1));
    const auto frameBytes = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
    auto result = resizeToPreview(request, pixels + frameBytes * frame, width, height);
    result.frameCount = static_cast<uint32_t>(frames);
    result.frameDurationMs = delays != nullptr && delays[frame] > 0 ? static_cast<uint32_t>(delays[frame]) : 100;
    stbi_image_free(pixels);
    if (delays) stbi_image_free(delays);
    return result;
}

} // namespace

EditorThumbnailCache::EditorThumbnailCache() : EditorThumbnailCache(Configuration{}, {}, {}) {}

EditorThumbnailCache::EditorThumbnailCache(Configuration configuration, Decoder decoder, TextureUploader uploader)
    : configuration_(configuration), decoder_(std::move(decoder)), uploader_(std::move(uploader)) {
    if (!decoder_) {
        decoder_ = decodeImage;
    }
    if (!uploader_) {
        uploader_ = [](const EditorThumbnailDecodedImage& image) {
            auto texture = std::make_shared<urpg::Texture>();
            if (!texture->loadFromMemory(image.rgba, static_cast<int>(image.width), static_cast<int>(image.height))) {
                return std::shared_ptr<urpg::Texture>{};
            }
            return texture;
        };
    }
}

EditorThumbnailCache::~EditorThumbnailCache() { shutdown(); }

std::string EditorThumbnailCache::cacheKey(const EditorThumbnailRequest& request) {
    std::error_code error;
    const auto canonical = std::filesystem::weakly_canonical(request.sourcePath, error);
    const auto stablePath = error ? request.sourcePath.lexically_normal() : canonical;
    return stablePath.generic_string() + "|" + std::to_string(request.sizeBytes) + "|" +
           std::to_string(request.modifiedTimeNs) + "|" + std::to_string(request.requestedWidth) + "x" +
           std::to_string(request.requestedHeight) + "|gif=" + std::to_string(request.gifFrameIndex);
}

EditorThumbnailDecodedImage EditorThumbnailCache::decodeImage(const EditorThumbnailRequest& request) {
    if (request.hashPending) {
        return {false, "thumbnail_hash_pending", 0, 0, 1, 0, {}};
    }
    if (!supportsThumbnail(request.sourcePath)) {
        return {false, "thumbnail_unsupported_format", 0, 0, 1, 0, {}};
    }
    std::error_code error;
    if (!std::filesystem::is_regular_file(request.sourcePath, error) || error) {
        return {false, "thumbnail_source_missing", 0, 0, 1, 0, {}};
    }
    if (normalizedExtension(request.sourcePath) == ".gif") return decodeGifFrame(request);
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels = stbi_load(request.sourcePath.string().c_str(), &width, &height, &channels, STBI_rgb_alpha);
    if (pixels == nullptr) {
        return {false, "thumbnail_decode_failed", 0, 0, 1, 0, {}};
    }
    const auto result = resizeToPreview(request, pixels, width, height);
    stbi_image_free(pixels);
    return result;
}

void EditorThumbnailCache::touch(Entry& entry) {
    lru_.splice(lru_.begin(), lru_, entry.lruIt);
    entry.lruIt = lru_.begin();
}

void EditorThumbnailCache::beginDecode(const EditorThumbnailRequest& request, const std::string& key) {
    lru_.push_front(key);
    Entry entry;
    entry.snapshot = {key, EditorThumbnailState::Queued, "thumbnail_queued", 0, 0, 0, 1, 0};
    entry.lruIt = lru_.begin();
    entries_.emplace(key, std::move(entry));

    const auto cancelled = std::make_shared<std::atomic_bool>(false);
    if (!configuration_.asyncDecode) {
        std::promise<EditorThumbnailDecodedImage> promise;
        promise.set_value(decoder_(request));
        jobs_.push_back({key, cancelled, promise.get_future()});
        return;
    }
    const auto decoder = decoder_;
    jobs_.push_back({key, cancelled, std::async(std::launch::async, [decoder, request, cancelled] {
                         if (cancelled->load()) {
                             return EditorThumbnailDecodedImage{false, "thumbnail_request_cancelled", 0, 0, 1, 0, {}};
                         }
                         auto image = decoder(request);
                         return cancelled->load()
                                    ? EditorThumbnailDecodedImage{false, "thumbnail_request_cancelled", 0, 0, 1, 0, {}}
                                    : image;
                     })});
}

void EditorThumbnailCache::setVisibleRequests(const std::vector<EditorThumbnailRequest>& requests) {
    std::unordered_set<std::string> nextVisible;
    for (const auto& request : requests) {
        if (request.sourcePath.empty()) {
            continue;
        }
        const auto key = cacheKey(request);
        nextVisible.insert(key);
        const auto existing = entries_.find(key);
        if (existing == entries_.end()) {
            beginDecode(request, key);
        } else {
            touch(existing->second);
        }
    }
    for (auto& job : jobs_) {
        if (!nextVisible.contains(job.key)) {
            job.cancelled->store(true);
        }
    }
    visible_keys_ = std::move(nextVisible);
}

void EditorThumbnailCache::eraseEntry(const std::string& key) {
    const auto entry = entries_.find(key);
    if (entry == entries_.end()) {
        return;
    }
    resident_bytes_ -= std::min(resident_bytes_, entry->second.bytes);
    lru_.erase(entry->second.lruIt);
    entries_.erase(entry);
}

void EditorThumbnailCache::evictToBudget() {
    while (resident_bytes_ > configuration_.memoryBudgetBytes && !lru_.empty()) {
        const auto key = lru_.back();
        eraseEntry(key);
    }
}

void EditorThumbnailCache::pumpUploads() {
    for (auto job = jobs_.begin(); job != jobs_.end();) {
        if (job->future.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
            ++job;
            continue;
        }
        const auto image = job->future.get();
        const bool cancelled = job->cancelled->load() || !visible_keys_.contains(job->key);
        const auto entry = entries_.find(job->key);
        if (entry != entries_.end()) {
            if (cancelled) {
                eraseEntry(job->key);
            } else if (!image.success) {
                entry->second.snapshot = {job->key, EditorThumbnailState::Fallback, image.diagnosticCode, 0, 0, 0, 1, 0};
                touch(entry->second);
            } else {
                entry->second.texture = uploader_(image);
                if (!entry->second.texture) {
                    entry->second.snapshot = {job->key, EditorThumbnailState::Fallback, "thumbnail_texture_upload_failed", 0, 0, 0, 1, 0};
                } else {
                    entry->second.bytes = image.rgba.size();
                    resident_bytes_ += entry->second.bytes;
                    entry->second.snapshot = {job->key, EditorThumbnailState::Ready, "thumbnail_ready", image.width,
                                              image.height, entry->second.texture->getId(), image.frameCount,
                                              image.frameDurationMs};
                }
                touch(entry->second);
            }
        }
        job = jobs_.erase(job);
    }
    evictToBudget();
}

EditorThumbnailSnapshot EditorThumbnailCache::snapshotFor(const EditorThumbnailRequest& request) const {
    const auto key = cacheKey(request);
    const auto entry = entries_.find(key);
    if (entry == entries_.end()) {
        return {key, EditorThumbnailState::Fallback, "thumbnail_not_requested", 0, 0, 0, 1, 0};
    }
    return entry->second.snapshot;
}

void EditorThumbnailCache::shutdown() {
    for (auto& job : jobs_) {
        job.cancelled->store(true);
    }
    jobs_.clear();
    entries_.clear();
    lru_.clear();
    visible_keys_.clear();
    resident_bytes_ = 0;
}

} // namespace urpg::editor
