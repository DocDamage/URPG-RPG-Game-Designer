#include "engine/core/localization/locale_catalog.h"
#include "engine/core/localization/localization_document_tools.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Load valid bundle and retrieve keys", "[localization][catalog]") {
    const auto json = nlohmann::json::parse(R"({
        "locale": "en",
        "keys": {
            "GREETING": "Hello",
            "FAREWELL": "Goodbye"
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(json);

    REQUIRE(catalog.getLocaleCode() == "en");
    REQUIRE_FALSE(catalog.hasFontProfile());
    REQUIRE(catalog.keyCount() == 2);
    REQUIRE(catalog.getKey("GREETING").value() == "Hello");
    REQUIRE(catalog.getKey("FAREWELL").value() == "Goodbye");
}

TEST_CASE("LocaleCatalog stores optional font profile id", "[localization][catalog][font]") {
    const auto json = nlohmann::json::parse(R"({
        "locale": "en-US",
        "font_profile_id": "font.profile.ui.latin",
        "keys": {
            "menu.save": "Save"
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(json);

    REQUIRE(catalog.getLocaleCode() == "en-US");
    REQUIRE(catalog.hasFontProfile());
    REQUIRE(catalog.getFontProfileId() == "font.profile.ui.latin");
}

TEST_CASE("LocaleCatalog owns structured plural grammar and runtime locale metadata",
          "[localization][catalog][plural][grammar][pcq652]") {
    const auto json = nlohmann::json::parse(R"({
        "locale": "ar",
        "font_profile_id": "font.profile.ui.arabic",
        "fallback_locale": "en-US",
        "text_direction": "rtl",
        "ime_supported": true,
        "source_revision": 7,
        "keys": {
            "quest.items": {
                "one": {"neutral": "عنصر واحد", "feminine": "عنصر واحدة"},
                "few": "{count} عناصر",
                "many": "{count} عنصرًا",
                "other": "{count} عنصر"
            }
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(json);
    REQUIRE(catalog.getFallbackLocale() == "en-US");
    REQUIRE(catalog.getTextDirection() == "rtl");
    REQUIRE(catalog.supportsIme());
    REQUIRE(catalog.sourceRevision() == 7);
    REQUIRE(catalog.getVariants("quest.items").size() == 5);
    REQUIRE(catalog.getVariant("quest.items", "one", "feminine") == "عنصر واحدة");
    REQUIRE(catalog.getVariant("quest.items", "two") == "{count} عنصر");
    REQUIRE(catalog.getKey("quest.items") == "{count} عنصر");

    catalog.mergeFromJson({{"keys", {{"quest.items", {{"one", {{"feminine", "واحدة"}}}}}}}});
    REQUIRE(catalog.getVariant("quest.items", "one", "feminine") == "واحدة");
    REQUIRE(catalog.getVariant("quest.items", "one", "neutral") == "عنصر واحد");
    REQUIRE_FALSE(urpg::localization::LocaleCatalog::validateBundleJson(
        {{"locale", "en"}, {"keys", {{"bad", {{"singular", "Bad"}}}}}}));
}

TEST_CASE("hasKey returns false for missing keys", "[localization][catalog]") {
    const auto json = nlohmann::json::parse(R"({
        "locale": "ja",
        "keys": {
            "GREETING": "Konnichiwa"
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(json);

    REQUIRE(catalog.hasKey("GREETING"));
    REQUIRE_FALSE(catalog.hasKey("MISSING"));
    REQUIRE(catalog.getKey("MISSING") == std::nullopt);
}

TEST_CASE("validateBundleJson rejects missing locale or keys", "[localization][catalog]") {
    using urpg::localization::LocaleCatalog;

    const auto missingLocale = nlohmann::json::parse(R"({
        "keys": {}
    })");
    REQUIRE_FALSE(LocaleCatalog::validateBundleJson(missingLocale));

    const auto missingKeys = nlohmann::json::parse(R"({
        "locale": "en"
    })");
    REQUIRE_FALSE(LocaleCatalog::validateBundleJson(missingKeys));

    const auto valid = nlohmann::json::parse(R"({
        "locale": "en",
        "keys": {}
    })");
    REQUIRE(LocaleCatalog::validateBundleJson(valid));

    const auto invalidFontProfile = nlohmann::json::parse(R"({
        "locale": "en",
        "font_profile_id": 42,
        "keys": {}
    })");
    REQUIRE_FALSE(LocaleCatalog::validateBundleJson(invalidFontProfile));
}

TEST_CASE("mergeFromJson overlays without clearing", "[localization][catalog]") {
    const auto base = nlohmann::json::parse(R"({
        "locale": "en",
        "keys": {
            "A": "Alpha",
            "B": "Beta"
        }
    })");

    const auto overlay = nlohmann::json::parse(R"({
        "keys": {
            "B": "Bravo",
            "C": "Charlie"
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(base);
    catalog.mergeFromJson(overlay);

    REQUIRE(catalog.keyCount() == 3);
    REQUIRE(catalog.getKey("A").value() == "Alpha");
    REQUIRE(catalog.getKey("B").value() == "Bravo");
    REQUIRE(catalog.getKey("C").value() == "Charlie");
}

TEST_CASE("clear empties the catalog", "[localization][catalog]") {
    const auto json = nlohmann::json::parse(R"({
        "locale": "fr",
        "keys": {
            "GREETING": "Bonjour"
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(json);
    catalog.clear();

    REQUIRE(catalog.keyCount() == 0);
    REQUIRE(catalog.getLocaleCode().empty());
    REQUIRE_FALSE(catalog.hasKey("GREETING"));
}

TEST_CASE("getAllKeys returns all inserted keys", "[localization][catalog]") {
    const auto json = nlohmann::json::parse(R"({
        "locale": "de",
        "keys": {
            "Z": "Zebra",
            "A": "Alpha",
            "M": "Mike"
        }
    })");

    urpg::localization::LocaleCatalog catalog;
    catalog.loadFromJson(json);

    const auto keys = catalog.getAllKeys();
    REQUIRE(keys.size() == 3);
    REQUIRE(keys[0] == "A");
    REQUIRE(keys[1] == "M");
    REQUIRE(keys[2] == "Z");
}

TEST_CASE("Localization extraction emits canonical dialogue preview bundle", "[localization][catalog][extract]") {
    const auto document = nlohmann::json::parse(R"({
        "id": "intro",
        "locale": "en-US",
        "pages": [
            {
                "id": "p1",
                "body": "Welcome home.",
                "localization_key": "dialogue.intro.body",
                "choices": [
                    {
                        "id": "yes",
                        "label": "Yes",
                        "localization_key": "dialogue.intro.yes",
                        "target_page_id": "p2"
                    }
                ]
            }
        ]
    })");

    const auto bundle = urpg::localization::extractDialoguePreviewLocalizationBundle(document);

    REQUIRE(bundle["locale"] == "en-US");
    REQUIRE(bundle["keys"]["dialogue.intro.body"] == "Welcome home.");
    REQUIRE(bundle["keys"]["dialogue.intro.yes"] == "Yes");
}

TEST_CASE("Localization writeback updates only localized dialogue text fields", "[localization][catalog][writeback]") {
    const auto document = nlohmann::json::parse(R"({
        "id": "intro",
        "locale": "en-US",
        "pages": [
            {
                "id": "p1",
                "speaker": "Guide",
                "body": "Welcome home.",
                "localization_key": "dialogue.intro.body",
                "choices": [
                    {
                        "id": "yes",
                        "label": "Yes",
                        "localization_key": "dialogue.intro.yes",
                        "target_page_id": "p2"
                    }
                ]
            }
        ]
    })");
    const auto bundle = nlohmann::json::parse(R"({
        "locale": "ja-JP",
        "keys": {
            "dialogue.intro.body": "Okaeri.",
            "dialogue.intro.yes": "Hai"
        }
    })");

    const auto updated = urpg::localization::writebackDialoguePreviewLocalizationBundle(document, bundle);

    REQUIRE(updated["id"] == document["id"]);
    REQUIRE(updated["pages"][0]["speaker"] == "Guide");
    REQUIRE(updated["pages"][0]["body"] == "Okaeri.");
    REQUIRE(updated["pages"][0]["choices"][0]["label"] == "Hai");
    REQUIRE(updated["pages"][0]["choices"][0]["target_page_id"] == "p2");
}
