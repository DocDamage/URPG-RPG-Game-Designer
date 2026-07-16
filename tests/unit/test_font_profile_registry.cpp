#include "engine/core/localization/font_profile_registry.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/localization/project_localization_audit.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {

class TemporaryFontProject {
public:
    explicit TemporaryFontProject(const std::string& fixtureName) {
        const auto unique = std::chrono::steady_clock::now().time_since_epoch().count();
        root = std::filesystem::temp_directory_path() / ("urpg_font_profile_" + std::to_string(unique));
        std::filesystem::create_directories(root / "content" / "localization");
        std::filesystem::create_directories(root / "content" / "fonts");
        const auto sourceRoot = std::filesystem::path(URPG_SOURCE_DIR);
        const auto corpusRoot = sourceRoot / "imports" / "normalized" / "sibling_bulk_assets" /
                                "sf3000-700zx1-cubegm" / "font" / "cubegm";
        std::filesystem::copy_file(corpusRoot / "arial-en-945309e4914c.ttf",
                                   root / "content" / "fonts" / "ui_latin_arabic.ttf",
                                   std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy_file(corpusRoot / "font-d4345020451f.ttf",
                                   root / "content" / "fonts" / "ui_cjk.ttf",
                                   std::filesystem::copy_options::overwrite_existing);
        std::filesystem::copy_file(
            sourceRoot / "tests" / "fixtures" / "localization" / "pcq652_font_profiles" / fixtureName,
            root / "content" / "localization" / "font_profiles.json",
            std::filesystem::copy_options::overwrite_existing);
    }

    ~TemporaryFontProject() {
        std::error_code error;
        std::filesystem::remove_all(root, error);
    }

    std::filesystem::path root;
};

} // namespace

TEST_CASE("Governed font profiles validate coverage and rasterize through one authority",
          "[localization][font_profile][glyphs][pcq652]") {
    TemporaryFontProject project("good.json");
    const auto loaded = urpg::localization::FontProfileRegistry::loadProjectManifest(project.root);
    REQUIRE(loaded.ok());
    REQUIRE(loaded.registry->profileCount() == 1);
    CHECK(loaded.registry->hasProfile("font.profile.ui.test"));
    CHECK(loaded.registry->missingGlyphs("font.profile.ui.test", "Save 42 مرحبا 日本語").empty());

    const auto rasterized = loaded.registry->rasterize("font.profile.ui.test", "Save 42\nLoad", 24);
    REQUIRE(rasterized.valid);
    CHECK(rasterized.width > 0);
    CHECK(rasterized.height >= 48);
    CHECK(rasterized.rgba.size() == static_cast<size_t>(rasterized.width * rasterized.height * 4));
    CHECK(std::ranges::any_of(rasterized.rgba, [](uint8_t value) { return value != 0; }));
    CHECK(rasterized.missingCodepoints.empty());

    const auto rtlLayout = urpg::localization::layoutLocaleText(
        "مرحبا 123", urpg::localization::LocaleTextDirection::RightToLeft);
    REQUIRE(rtlLayout.lines.size() == 1);
    const auto rtlRasterized = loaded.registry->rasterize(
        "font.profile.ui.test", rtlLayout.lines.front().visualText, 28);
    REQUIRE(rtlRasterized.valid);
    CHECK(rtlRasterized.missingCodepoints.empty());
    CHECK(std::ranges::any_of(rtlRasterized.rgba, [](uint8_t value) { return value != 0; }));

    std::ofstream(project.root / "content" / "localization" / "en.json", std::ios::binary | std::ios::trunc)
        << nlohmann::json{{"locale", "en"}, {"font_profile_id", "font.profile.ui.test"},
                          {"keys", {{"menu.save", "Save"}}}}
               .dump(2)
        << '\n';
    const auto projectAudit = urpg::localization::buildProjectLocalizationAudit(project.root);
    CHECK(projectAudit.unresolved_font_profile_locales.empty());
    CHECK(projectAudit.font_profile_diagnostics.empty());

    std::ofstream(project.root / "content" / "localization" / "en.json", std::ios::binary | std::ios::trunc)
        << nlohmann::json{{"locale", "en"}, {"font_profile_id", "font.profile.missing"},
                          {"keys", {{"menu.save", "Save"}}}}
               .dump(2)
        << '\n';
    const auto unresolvedAudit = urpg::localization::buildProjectLocalizationAudit(project.root);
    CHECK(unresolvedAudit.unresolved_font_profile_locales == std::vector<std::string>{"en"});

    const auto unknown = loaded.registry->rasterize("font.profile.unknown", "Save", 24);
    CHECK_FALSE(unknown.valid);
}

TEST_CASE("Font profile corpus reports missing glyphs unsafe paths and invalid faces",
          "[localization][font_profile][glyphs][diagnostics][pcq652]") {
    TemporaryFontProject badGlyphProject("bad_missing_glyph.json");
    const auto missingGlyph = urpg::localization::FontProfileRegistry::loadProjectManifest(badGlyphProject.root);
    REQUIRE(missingGlyph.registry);
    REQUIRE(missingGlyph.diagnostics.size() == 1);
    CHECK(missingGlyph.diagnostics.front().code == "font_profile_required_sample_missing_glyph");
    CHECK(missingGlyph.diagnostics.front().sampleId == "impossible_scalar");
    CHECK(missingGlyph.diagnostics.front().codepoints == std::vector<char32_t>{0x10FFFF});

    const auto traversal = nlohmann::json{
        {"schema_version", "urpg.font_profiles.v1"},
        {"profiles", {{{"id", "font.profile.bad"}, {"faces", {"../outside.ttf"}}}}}};
    CHECK_FALSE(urpg::localization::FontProfileRegistry::validateManifestJson(traversal));

    std::ofstream(badGlyphProject.root / "content" / "fonts" / "ui_latin_arabic.ttf",
                  std::ios::binary | std::ios::trunc)
        << "not a font";
    const auto invalidFace = urpg::localization::FontProfileRegistry::loadProjectManifest(badGlyphProject.root);
    REQUIRE(invalidFace.registry);
    REQUIRE(invalidFace.diagnostics.size() == 2);
    CHECK(invalidFace.diagnostics[0].code == "font_profile_face_invalid");
    CHECK(invalidFace.diagnostics[1].code == "font_profile_required_sample_missing_glyph");
}
