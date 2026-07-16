#include "engine/core/localization/font_profile_registry.h"

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"
#elif defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <utility>

#include <nlohmann/json.hpp>

namespace urpg::localization {

namespace {

constexpr uintmax_t kMaximumFontBytes = 64U * 1024U * 1024U;

bool safeId(std::string_view value) {
    if (value.empty() || value.size() > 128) return false;
    return std::ranges::all_of(value, [](const unsigned char ch) {
        return std::isalnum(ch) != 0 || ch == '.' || ch == '_' || ch == '-';
    });
}

bool pathInside(const std::filesystem::path& root, const std::filesystem::path& path) {
    std::error_code error;
    const auto canonicalRoot = std::filesystem::weakly_canonical(root, error);
    if (error) return false;
    const auto canonicalPath = std::filesystem::weakly_canonical(path, error);
    if (error) return false;
    const auto relative = canonicalPath.lexically_relative(canonicalRoot);
    return !relative.empty() && !relative.is_absolute() &&
           std::ranges::none_of(relative, [](const auto& part) { return part == ".."; });
}

bool decodeUtf8(std::string_view text, std::vector<char32_t>& output) {
    bool allValid = true;
    for (size_t cursor = 0; cursor < text.size();) {
        const auto lead = static_cast<unsigned char>(text[cursor]);
        char32_t value = U'\uFFFD';
        size_t length = 1;
        bool valid = true;
        if (lead < 0x80) {
            value = lead;
        } else if ((lead >> 5) == 0x6 && cursor + 1 < text.size()) {
            const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
            if ((b1 & 0xC0) == 0x80) {
                value = static_cast<char32_t>(((lead & 0x1F) << 6) | (b1 & 0x3F));
                length = 2;
                valid = value >= 0x80;
            } else valid = false;
        } else if ((lead >> 4) == 0xE && cursor + 2 < text.size()) {
            const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
            const auto b2 = static_cast<unsigned char>(text[cursor + 2]);
            if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
                value = static_cast<char32_t>(((lead & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
                length = 3;
                valid = value >= 0x800 && !(value >= 0xD800 && value <= 0xDFFF);
            } else valid = false;
        } else if ((lead >> 3) == 0x1E && cursor + 3 < text.size()) {
            const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
            const auto b2 = static_cast<unsigned char>(text[cursor + 2]);
            const auto b3 = static_cast<unsigned char>(text[cursor + 3]);
            if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
                value = static_cast<char32_t>(((lead & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                                              ((b2 & 0x3F) << 6) | (b3 & 0x3F));
                length = 4;
                valid = value >= 0x10000 && value <= 0x10FFFF;
            } else valid = false;
        } else valid = false;
        if (!valid) {
            value = U'\uFFFD';
            allValid = false;
        }
        output.push_back(value);
        cursor += length;
    }
    return allValid;
}

bool ignoredCoverageCodepoint(char32_t cp) {
    return cp == U'\n' || cp == U'\r' || cp == U'\t' || cp == 0x200C || cp == 0x200D ||
           (cp >= 0xFE00 && cp <= 0xFE0F);
}

} // namespace

struct FontProfileRegistry::Impl {
    struct Face {
        std::filesystem::path path;
        std::vector<uint8_t> bytes;
        stbtt_fontinfo info{};
    };
    struct Profile {
        std::string id;
        std::vector<std::unique_ptr<Face>> faces;
    };
    std::map<std::string, Profile> profiles;

    const Face* findFace(const Profile& profile, char32_t cp) const {
        for (const auto& face : profile.faces) {
            if (stbtt_FindGlyphIndex(&face->info, static_cast<int>(cp)) != 0) return face.get();
        }
        return nullptr;
    }
};

FontProfileRegistry::FontProfileRegistry() : impl_(std::make_unique<Impl>()) {}
FontProfileRegistry::FontProfileRegistry(std::unique_ptr<Impl> impl) : impl_(std::move(impl)) {}
FontProfileRegistry::~FontProfileRegistry() = default;
FontProfileRegistry::FontProfileRegistry(FontProfileRegistry&&) noexcept = default;
FontProfileRegistry& FontProfileRegistry::operator=(FontProfileRegistry&&) noexcept = default;

bool FontProfileRegistry::validateManifestJson(const nlohmann::json& manifest) {
    if (!manifest.is_object() || manifest.value("schema_version", "") != "urpg.font_profiles.v1" ||
        !manifest.contains("profiles") || !manifest["profiles"].is_array() || manifest["profiles"].empty()) return false;
    std::set<std::string> ids;
    for (const auto& profile : manifest["profiles"]) {
        if (!profile.is_object() || !profile.contains("id") || !profile["id"].is_string() ||
            !safeId(profile["id"].get_ref<const std::string&>()) || !ids.insert(profile["id"]).second ||
            !profile.contains("faces") || !profile["faces"].is_array() || profile["faces"].empty()) return false;
        for (const auto& face : profile["faces"]) {
            if (!face.is_string() || face.get_ref<const std::string&>().empty()) return false;
            const auto path = std::filesystem::path(face.get_ref<const std::string&>());
            if (path.is_absolute() || std::ranges::any_of(path, [](const auto& part) { return part == ".."; }))
                return false;
        }
        if (profile.contains("required_samples")) {
            if (!profile["required_samples"].is_array()) return false;
            std::set<std::string> sampleIds;
            for (const auto& sample : profile["required_samples"]) {
                if (!sample.is_object() || !sample.contains("id") || !sample["id"].is_string() ||
                    !safeId(sample["id"].get_ref<const std::string&>()) || !sampleIds.insert(sample["id"]).second ||
                    !sample.contains("text") || !sample["text"].is_string() || sample["text"].get_ref<const std::string&>().empty())
                    return false;
            }
        }
    }
    return true;
}

FontProfileLoadResult FontProfileRegistry::loadProjectManifest(const std::filesystem::path& projectRoot) {
    FontProfileLoadResult result;
    const auto manifestPath = projectRoot / "content" / "localization" / "font_profiles.json";
    std::ifstream input(manifestPath, std::ios::binary);
    const auto manifest = input ? nlohmann::json::parse(input, nullptr, false) : nlohmann::json{};
    if (!validateManifestJson(manifest)) {
        result.diagnostics.push_back({"font_profile_manifest_invalid", {}, {}, manifestPath, {}});
        return result;
    }

    auto impl = std::make_unique<Impl>();
    for (const auto& profileJson : manifest["profiles"]) {
        Impl::Profile profile;
        profile.id = profileJson["id"].get<std::string>();
        for (const auto& relativeJson : profileJson["faces"]) {
            const auto relative = std::filesystem::path(relativeJson.get<std::string>());
            const auto path = projectRoot / relative;
            std::error_code error;
            const auto byteCount = std::filesystem::file_size(path, error);
            if (error || byteCount == 0 || byteCount > kMaximumFontBytes || !pathInside(projectRoot / "content", path)) {
                result.diagnostics.push_back({"font_profile_face_unavailable", profile.id, {}, path, {}});
                continue;
            }
            auto face = std::make_unique<Impl::Face>();
            face->path = path;
            face->bytes.resize(static_cast<size_t>(byteCount));
            std::ifstream fontInput(path, std::ios::binary);
            fontInput.read(reinterpret_cast<char*>(face->bytes.data()), static_cast<std::streamsize>(face->bytes.size()));
            const int offset = fontInput && !face->bytes.empty()
                                   ? stbtt_GetFontOffsetForIndex(face->bytes.data(), 0)
                                   : -1;
            if (offset < 0 || stbtt_InitFont(&face->info, face->bytes.data(), offset) == 0) {
                result.diagnostics.push_back({"font_profile_face_invalid", profile.id, {}, path, {}});
                continue;
            }
            profile.faces.push_back(std::move(face));
        }
        if (profile.faces.empty()) {
            result.diagnostics.push_back({"font_profile_has_no_valid_face", profile.id, {}, manifestPath, {}});
            continue;
        }
        impl->profiles.emplace(profile.id, std::move(profile));
    }
    if (impl->profiles.empty()) return result;
    result.registry = std::shared_ptr<FontProfileRegistry>(new FontProfileRegistry(std::move(impl)));

    for (const auto& profileJson : manifest["profiles"]) {
        const auto id = profileJson["id"].get<std::string>();
        if (!result.registry->hasProfile(id) || !profileJson.contains("required_samples")) continue;
        for (const auto& sample : profileJson["required_samples"]) {
            const auto missing = result.registry->missingGlyphs(id, sample["text"].get_ref<const std::string&>());
            if (!missing.empty()) {
                result.diagnostics.push_back(
                    {"font_profile_required_sample_missing_glyph", id, sample["id"].get<std::string>(), manifestPath, missing});
            }
        }
    }
    return result;
}

bool FontProfileRegistry::hasProfile(std::string_view profileId) const {
    return impl_ && impl_->profiles.contains(std::string(profileId));
}

size_t FontProfileRegistry::profileCount() const { return impl_ ? impl_->profiles.size() : 0; }

std::vector<std::string> FontProfileRegistry::profileIds() const {
    std::vector<std::string> result;
    if (impl_) for (const auto& [id, profile] : impl_->profiles) { (void)profile; result.push_back(id); }
    return result;
}

std::vector<char32_t> FontProfileRegistry::missingGlyphs(std::string_view profileId, std::string_view utf8Text) const {
    std::vector<char32_t> codepoints;
    decodeUtf8(utf8Text, codepoints);
    std::set<char32_t> missing;
    if (!impl_) {
        for (const auto cp : codepoints) if (!ignoredCoverageCodepoint(cp)) missing.insert(cp);
        return {missing.begin(), missing.end()};
    }
    const auto found = impl_->profiles.find(std::string(profileId));
    if (found == impl_->profiles.end()) {
        for (const auto cp : codepoints) if (!ignoredCoverageCodepoint(cp)) missing.insert(cp);
    } else {
        for (const auto cp : codepoints) {
            if (!ignoredCoverageCodepoint(cp) && !impl_->findFace(found->second, cp)) missing.insert(cp);
        }
    }
    return {missing.begin(), missing.end()};
}

RasterizedFontText FontProfileRegistry::rasterize(std::string_view profileId, std::string_view utf8Text,
                                                  int32_t pixelHeight) const {
    RasterizedFontText result;
    if (!impl_) return result;
    const auto profileIt = impl_->profiles.find(std::string(profileId));
    if (profileIt == impl_->profiles.end() || profileIt->second.faces.empty() || pixelHeight < 4 || pixelHeight > 256)
        return result;

    std::vector<char32_t> codepoints;
    if (!decodeUtf8(utf8Text, codepoints)) result.missingCodepoints.push_back(U'\uFFFD');
    if (codepoints.empty()) { result.valid = true; return result; }

    const auto& profile = profileIt->second;
    const auto* primary = profile.faces.front().get();
    const float primaryScale = stbtt_ScaleForPixelHeight(&primary->info, static_cast<float>(pixelHeight));
    int ascent = 0, descent = 0, lineGap = 0;
    stbtt_GetFontVMetrics(&primary->info, &ascent, &descent, &lineGap);
    const int baseline = std::max(1, static_cast<int>(std::ceil(ascent * primaryScale)));
    const int lineHeight = std::max(pixelHeight, static_cast<int>(std::ceil((ascent - descent + lineGap) * primaryScale)));

    struct Glyph { const Impl::Face* face; char32_t cp; int x; int y; int w; int h; float scale; };
    std::vector<Glyph> glyphs;
    float penX = 0.0F;
    int line = 0;
    int minX = 0, minY = 0, maxX = 0, maxY = lineHeight;
    std::set<char32_t> missing(result.missingCodepoints.begin(), result.missingCodepoints.end());
    for (size_t index = 0; index < codepoints.size(); ++index) {
        auto cp = codepoints[index];
        if (cp == U'\r') continue;
        if (cp == U'\n') { maxX = std::max(maxX, static_cast<int>(std::ceil(penX))); penX = 0.0F; ++line; maxY = std::max(maxY, (line + 1) * lineHeight); continue; }
        if (cp == U'\t') cp = U' ';
        const auto* face = impl_->findFace(profile, cp);
        if (!face) {
            missing.insert(cp);
            cp = U'\uFFFD';
            face = impl_->findFace(profile, cp);
            if (!face) { cp = U'?'; face = impl_->findFace(profile, cp); }
            if (!face) continue;
        }
        const float scale = stbtt_ScaleForPixelHeight(&face->info, static_cast<float>(pixelHeight));
        int advance = 0, bearing = 0;
        stbtt_GetCodepointHMetrics(&face->info, static_cast<int>(cp), &advance, &bearing);
        int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        stbtt_GetCodepointBitmapBox(&face->info, static_cast<int>(cp), scale, scale, &x0, &y0, &x1, &y1);
        const int gx = static_cast<int>(std::floor(penX)) + x0;
        const int gy = line * lineHeight + baseline + y0;
        if (x1 > x0 && y1 > y0) glyphs.push_back({face, cp, gx, gy, x1 - x0, y1 - y0, scale});
        minX = std::min(minX, gx); minY = std::min(minY, gy);
        maxX = std::max(maxX, gx + (x1 - x0)); maxY = std::max(maxY, gy + (y1 - y0));
        penX += advance * scale;
        if (index + 1 < codepoints.size() && codepoints[index + 1] != U'\n') {
            const auto* nextFace = impl_->findFace(profile, codepoints[index + 1]);
            if (nextFace == face) penX += stbtt_GetCodepointKernAdvance(
                &face->info, static_cast<int>(cp), static_cast<int>(codepoints[index + 1])) * scale;
        }
    }
    maxX = std::max(maxX, static_cast<int>(std::ceil(penX)));
    result.width = std::max(1, maxX - minX + 2);
    result.height = std::max(1, maxY - minY + 2);
    result.baseline = baseline + 1 - minY;
    result.rgba.assign(static_cast<size_t>(result.width) * static_cast<size_t>(result.height) * 4, 0);
    for (const auto& glyph : glyphs) {
        std::vector<uint8_t> alpha(static_cast<size_t>(glyph.w) * static_cast<size_t>(glyph.h));
        stbtt_MakeCodepointBitmap(&glyph.face->info, alpha.data(), glyph.w, glyph.h, glyph.w,
                                  glyph.scale, glyph.scale, static_cast<int>(glyph.cp));
        const int destinationX = glyph.x - minX + 1;
        const int destinationY = glyph.y - minY + 1;
        for (int y = 0; y < glyph.h; ++y) for (int x = 0; x < glyph.w; ++x) {
            const auto source = alpha[static_cast<size_t>(y) * glyph.w + x];
            const auto destination = (static_cast<size_t>(destinationY + y) * result.width + destinationX + x) * 4;
            result.rgba[destination] = 255;
            result.rgba[destination + 1] = 255;
            result.rgba[destination + 2] = 255;
            result.rgba[destination + 3] = std::max(result.rgba[destination + 3], source);
        }
    }
    result.missingCodepoints.assign(missing.begin(), missing.end());
    result.valid = true;
    return result;
}

} // namespace urpg::localization
