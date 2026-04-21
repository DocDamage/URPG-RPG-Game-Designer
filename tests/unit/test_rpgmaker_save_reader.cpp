#include "engine/core/save/rpgmaker_save_reader.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>

using namespace urpg::save;

namespace {

void WriteFile(const std::filesystem::path& path, const std::string& content) {
    std::ofstream out(path, std::ios::binary);
    out << content;
}

} // namespace

TEST_CASE("RPGMakerSaveFileReader detects Base64 as MV format", "[save][rpgmaker]") {
    std::string base64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/==";
    std::vector<uint8_t> bytes(base64.begin(), base64.end());

    auto format = RPGMakerSaveFileReader::detectFormat(bytes);
    REQUIRE(format == RPGMakerSaveFormat::RPGMakerMV);
}

TEST_CASE("RPGMakerSaveFileReader detects binary as Unknown", "[save][rpgmaker]") {
    std::vector<uint8_t> bytes = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    auto format = RPGMakerSaveFileReader::detectFormat(bytes);
    REQUIRE(format == RPGMakerSaveFormat::Unknown);
}

TEST_CASE("RPGMakerSaveFileReader detects empty as Unknown", "[save][rpgmaker]") {
    std::vector<uint8_t> bytes;
    auto format = RPGMakerSaveFileReader::detectFormat(bytes);
    REQUIRE(format == RPGMakerSaveFormat::Unknown);
}

TEST_CASE("RPGMakerSaveFileReader XOR decryption round-trips", "[save][rpgmaker]") {
    std::string key = "testkey";
    std::vector<uint8_t> original = {0x10, 0x20, 0x30, 0x40, 0x50};

    auto encrypted = RPGMakerSaveFileReader::decryptXOR(original, key);
    auto decrypted = RPGMakerSaveFileReader::decryptXOR(encrypted, key);

    REQUIRE(decrypted == original);
}

TEST_CASE("RPGMakerSaveFileReader XOR with empty key is no-op", "[save][rpgmaker]") {
    std::vector<uint8_t> original = {0x10, 0x20, 0x30};
    auto result = RPGMakerSaveFileReader::decryptXOR(original, "");
    REQUIRE(result == original);
}

TEST_CASE("RPGMakerSaveFileReader reads uncompressed LZString save file", "[save][rpgmaker]") {
    const auto tempDir = std::filesystem::temp_directory_path() / "urpg_save_test";
    std::filesystem::create_directories(tempDir);
    auto filePath = tempDir / "file1.rpgsave";

    // Write a known LZString-compressed JSON payload
    // "N4IgdghgtgpiBcIAuMDOSQBoQBsYDcYcEBWAXyA=" compresses '{"name":"test","level":5}'
    WriteFile(filePath, "N4IgdghgtgpiBcIAuMDOSQBoQBsYDcYcEBWAXyA=");

    auto result = RPGMakerSaveFileReader::readFile(filePath.string());

    REQUIRE(result.success == true);
    REQUIRE(result.format == RPGMakerSaveFormat::RPGMakerMV);
    REQUIRE(result.errors.empty());
    REQUIRE(result.data.is_object());
    REQUIRE(result.data["name"] == "test");
    REQUIRE(result.data["level"] == 5);

    std::filesystem::remove_all(tempDir);
}

TEST_CASE("RPGMakerSaveFileReader returns error for missing file", "[save][rpgmaker]") {
    auto result = RPGMakerSaveFileReader::readFile("/nonexistent/path/file.rpgsave");
    REQUIRE(result.success == false);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("not found") != std::string::npos);
}

TEST_CASE("RPGMakerSaveFileReader returns error for invalid format", "[save][rpgmaker]") {
    const auto tempDir = std::filesystem::temp_directory_path() / "urpg_save_test_invalid";
    std::filesystem::create_directories(tempDir);
    auto filePath = tempDir / "file1.rpgsave";

    // Write raw binary that is not Base64
    WriteFile(filePath, std::string("\x00\x01\x02\xFF\xFE", 5));

    auto result = RPGMakerSaveFileReader::readFile(filePath.string());

    REQUIRE(result.success == false);
    REQUIRE_FALSE(result.errors.empty());
    REQUIRE(result.errors[0].find("format") != std::string::npos);

    std::filesystem::remove_all(tempDir);
}

TEST_CASE("RPGMakerSaveFileReader reads larger JSON payload", "[save][rpgmaker]") {
    const auto tempDir = std::filesystem::temp_directory_path() / "urpg_save_test_large";
    std::filesystem::create_directories(tempDir);
    auto filePath = tempDir / "global.rpgsave";

    // LZString-compressed JSON: {"party":[{"actorId":1},{"actorId":2}],"gold":500,"mapId":3}
    WriteFile(filePath, "N4IgDghgTgLgniAXAbVBAxjA9lAkgEyQEYBfAGjUxwKQCYSBdMkAcywBtDEBWABl+YBbCGBqIAzCSA==");

    auto result = RPGMakerSaveFileReader::readFile(filePath.string());

    REQUIRE(result.success == true);
    REQUIRE(result.data.is_object());
    REQUIRE(result.data["gold"] == 500);
    REQUIRE(result.data["mapId"] == 3);
    REQUIRE(result.data["party"].is_array());
    REQUIRE(result.data["party"].size() == 2);

    std::filesystem::remove_all(tempDir);
}
