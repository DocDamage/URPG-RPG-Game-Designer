#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace urpg::localization {

struct FontProfileDiagnostic {
    std::string code;
    std::string profileId;
    std::string sampleId;
    std::filesystem::path path;
    std::vector<char32_t> codepoints;
};

struct FontProfileLoadResult;

struct RasterizedFontText {
    bool valid = false;
    int32_t width = 0;
    int32_t height = 0;
    int32_t baseline = 0;
    std::vector<uint8_t> rgba;
    std::vector<char32_t> missingCodepoints;
};

/**
 * Project-owned, ordered font fallback profiles. The same registry provides
 * release diagnostics and renderer rasterization so their coverage authority
 * cannot drift.
 */
class FontProfileRegistry {
public:
    FontProfileRegistry();
    ~FontProfileRegistry();
    FontProfileRegistry(FontProfileRegistry&&) noexcept;
    FontProfileRegistry& operator=(FontProfileRegistry&&) noexcept;
    FontProfileRegistry(const FontProfileRegistry&) = delete;
    FontProfileRegistry& operator=(const FontProfileRegistry&) = delete;

    static bool validateManifestJson(const nlohmann::json& manifest);
    static FontProfileLoadResult loadProjectManifest(const std::filesystem::path& projectRoot);

    bool hasProfile(std::string_view profileId) const;
    size_t profileCount() const;
    std::vector<std::string> profileIds() const;
    std::vector<char32_t> missingGlyphs(std::string_view profileId, std::string_view utf8Text) const;
    RasterizedFontText rasterize(std::string_view profileId, std::string_view utf8Text,
                                 int32_t pixelHeight) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    explicit FontProfileRegistry(std::unique_ptr<Impl> impl);
};

struct FontProfileLoadResult {
    std::shared_ptr<FontProfileRegistry> registry;
    std::vector<FontProfileDiagnostic> diagnostics;

    bool ok() const { return registry != nullptr && diagnostics.empty(); }
};

} // namespace urpg::localization
