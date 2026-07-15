#include "engine/core/assets/archive_catalog.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
void put16(std::vector<unsigned char>& out, uint16_t value) { out.push_back(value & 0xffU); out.push_back((value >> 8U) & 0xffU); }
void put32(std::vector<unsigned char>& out, uint32_t value) { for (int shift = 0; shift < 32; shift += 8) out.push_back((value >> shift) & 0xffU); }

std::filesystem::path writeStoredZip(const std::string& name, uint16_t flags = 0, uint32_t attributes = 0) {
    const auto path = std::filesystem::temp_directory_path() /
                      ("urpg_archive_catalog_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".zip");
    const std::string contents = "safe";
    std::vector<unsigned char> bytes;
    put32(bytes, 0x04034b50U); put16(bytes, 20); put16(bytes, flags); put16(bytes, 0); put16(bytes, 0); put16(bytes, 0); put32(bytes, 0); put32(bytes, contents.size()); put32(bytes, contents.size()); put16(bytes, name.size()); put16(bytes, 0);
    bytes.insert(bytes.end(), name.begin(), name.end()); bytes.insert(bytes.end(), contents.begin(), contents.end());
    const auto centralOffset = static_cast<uint32_t>(bytes.size());
    put32(bytes, 0x02014b50U); put16(bytes, 0x0314); put16(bytes, 20); put16(bytes, flags); put16(bytes, 0); put16(bytes, 0); put16(bytes, 0); put32(bytes, 0); put32(bytes, contents.size()); put32(bytes, contents.size()); put16(bytes, name.size()); put16(bytes, 0); put16(bytes, 0); put16(bytes, 0); put16(bytes, 0); put32(bytes, attributes); put32(bytes, 0);
    bytes.insert(bytes.end(), name.begin(), name.end());
    const auto centralSize = static_cast<uint32_t>(bytes.size()) - centralOffset;
    put32(bytes, 0x06054b50U); put16(bytes, 0); put16(bytes, 0); put16(bytes, 1); put16(bytes, 1); put32(bytes, centralSize); put32(bytes, centralOffset); put16(bytes, 0);
    std::ofstream output(path, std::ios::binary); output.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
    return path;
}
}

TEST_CASE("ArchiveCatalog lists safe ZIP entries without extraction", "[assets][archive_catalog]") {
    const auto path = writeStoredZip("sprites/hero.png");
    const auto result = urpg::assets::ArchiveCatalog{}.list(path);
    REQUIRE(result.success);
    REQUIRE(result.code == "archive_listed");
    REQUIRE(result.entries.size() == 1);
    REQUIRE(result.entries.front().path == "sprites/hero.png");
    std::filesystem::remove(path);
}

TEST_CASE("ArchiveCatalog rejects unsafe and encrypted ZIP entries", "[assets][archive_catalog]") {
    const auto traversal = writeStoredZip("../outside.png");
    const auto encrypted = writeStoredZip("safe.png", 1);
    REQUIRE(urpg::assets::ArchiveCatalog{}.list(traversal).code == "unsafe_archive_path");
    REQUIRE(urpg::assets::ArchiveCatalog{}.list(encrypted).code == "archive_encryption_unsupported");
    std::filesystem::remove(traversal);
    std::filesystem::remove(encrypted);
}

TEST_CASE("ArchiveCatalog lists RAR and 7z through a configured extractor without extraction", "[assets][archive_catalog]") {
    const auto path = std::filesystem::temp_directory_path() /
                      ("urpg_archive_catalog_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()) + ".7z");
    std::ofstream(path, std::ios::binary) << "fixture";
    std::vector<std::string> received;
    const auto result = urpg::assets::ArchiveCatalog{}.list(
        path, {}, {"7z"}, [&](const std::vector<std::string>& command) {
            received = command;
            return urpg::assets::ArchiveCatalogProcessResult{
                0,
                "Listing archive: fixture.7z\n----------\nPath = sprites/hero.png\nSize = 64\nPacked Size = 12\nAttributes = A\nEncrypted = -\n\n",
                ""};
        });
    REQUIRE(result.success);
    REQUIRE(result.code == "archive_listed_external");
    REQUIRE(received == std::vector<std::string>{"7z", "l", "-slt", path.string()});
    REQUIRE(result.entries.size() == 1);
    REQUIRE(result.entries.front().path == "sprites/hero.png");
    REQUIRE(result.entries.front().expandedBytes == 64);
    std::filesystem::remove(path);
}
