#pragma once

#include "engine/core/platform/gl_texture.h"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <future>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace urpg::editor {

// A render-thread-owned thumbnail cache. Image decoding is deliberately kept
// separate from texture creation so catalog scrolling never invokes OpenGL from
// a worker thread.
struct EditorThumbnailRequest {
    std::filesystem::path sourcePath;
    uint64_t sizeBytes = 0;
    int64_t modifiedTimeNs = 0;
    uint32_t requestedWidth = 96;
    uint32_t requestedHeight = 96;
    uint32_t gifFrameIndex = 0;
    bool hashPending = false;
};

enum class EditorThumbnailState {
    Queued,
    Ready,
    Fallback,
};

struct EditorThumbnailSnapshot {
    std::string key;
    EditorThumbnailState state = EditorThumbnailState::Fallback;
    std::string diagnosticCode;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t textureId = 0;
    uint32_t frameCount = 1;
    uint32_t frameDurationMs = 0;
};

struct EditorThumbnailDecodedImage {
    bool success = false;
    std::string diagnosticCode;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t frameCount = 1;
    uint32_t frameDurationMs = 0;
    std::vector<uint8_t> rgba;
};

class EditorThumbnailCache {
  public:
    struct Configuration {
        size_t memoryBudgetBytes = 64u * 1024u * 1024u;
        bool asyncDecode = true;
    };

    using Decoder = std::function<EditorThumbnailDecodedImage(const EditorThumbnailRequest&)>;
    using TextureUploader = std::function<std::shared_ptr<urpg::Texture>(const EditorThumbnailDecodedImage&)>;

    EditorThumbnailCache();
    explicit EditorThumbnailCache(Configuration configuration, Decoder decoder = {}, TextureUploader uploader = {});
    ~EditorThumbnailCache();

    EditorThumbnailCache(const EditorThumbnailCache&) = delete;
    EditorThumbnailCache& operator=(const EditorThumbnailCache&) = delete;

    static std::string cacheKey(const EditorThumbnailRequest& request);
    static EditorThumbnailDecodedImage decodeImage(const EditorThumbnailRequest& request);

    // Starts requests only for the rows visible in the current viewport. Jobs
    // for rows that leave the viewport are cancelled/discarded before upload.
    void setVisibleRequests(const std::vector<EditorThumbnailRequest>& requests);
    // Must be called on the OpenGL/ImGui thread. It uploads completed decodes
    // and releases evicted Texture instances on that same thread.
    void pumpUploads();
    EditorThumbnailSnapshot snapshotFor(const EditorThumbnailRequest& request) const;
    void shutdown();

    size_t residentBytes() const { return resident_bytes_; }
    size_t residentCount() const { return entries_.size(); }

  private:
    struct Entry {
        EditorThumbnailSnapshot snapshot;
        std::shared_ptr<urpg::Texture> texture;
        size_t bytes = 0;
        std::list<std::string>::iterator lruIt;
    };

    struct Job {
        std::string key;
        std::shared_ptr<std::atomic_bool> cancelled;
        std::future<EditorThumbnailDecodedImage> future;
    };

    void touch(Entry& entry);
    void beginDecode(const EditorThumbnailRequest& request, const std::string& key);
    void evictToBudget();
    void eraseEntry(const std::string& key);

    Configuration configuration_;
    Decoder decoder_;
    TextureUploader uploader_;
    std::unordered_map<std::string, Entry> entries_;
    std::list<std::string> lru_;
    std::vector<Job> jobs_;
    std::unordered_set<std::string> visible_keys_;
    size_t resident_bytes_ = 0;
};

} // namespace urpg::editor
