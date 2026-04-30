#include "engine/core/localization/locale_catalog.h"

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
