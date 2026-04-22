#include "engine/core/save/lzstring_decompressor.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::save::lzstring;

TEST_CASE("LZString decompress empty string", "[save][lzstring]") {
    auto result = decompressFromBase64("");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "");
}

TEST_CASE("LZString decompress single character 'a'", "[save][lzstring]") {
    auto result = decompressFromBase64("IZA=");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "a");
}

TEST_CASE("LZString decompress 'hello'", "[save][lzstring]") {
    auto result = decompressFromBase64("BYUwNmD2Q===");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "hello");
}

TEST_CASE("LZString decompress 'Hello, world!'", "[save][lzstring]") {
    auto result = decompressFromBase64("BIUwNmD2A0AEDukBOYAmBCIA");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "Hello, world!");
}

TEST_CASE("LZString decompress JSON string", "[save][lzstring]") {
    auto result = decompressFromBase64("N4IgdghgtgpiBcIAuMDOSQBoQBsYDcYcEBWAXyA=");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "{\"name\":\"test\",\"level\":5}");
}

TEST_CASE("LZString decompress long sentence", "[save][lzstring]") {
    auto result = decompressFromBase64("CoCwpgBAjgrglgYwNYQEYCcD2B3AdhAM0wA8IArGAWwAcBnCTANzHQgBdwIAbAQwC8AnhAAmmAOYBCIA");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "The quick brown fox jumps over the lazy dog!");
}

TEST_CASE("LZString decompress RPG Maker save header", "[save][lzstring]") {
    auto result = decompressFromBase64("EoBQ4gBAsghg1gUwE7QGoQMowG4IgERgBcYg");
    REQUIRE(result.has_value());
    REQUIRE(result.value() == "RPG Maker MV Save Data");
}

TEST_CASE("LZString decompress invalid data returns nullopt", "[save][lzstring]") {
    // Random invalid Base64 that should fail decompression
    auto result = decompressFromBase64("!!!!!!!!");
    REQUIRE_FALSE(result.has_value());
}
