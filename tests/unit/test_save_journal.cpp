#include "engine/core/save/save_journal.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <string>

namespace {
std::string ReadAll(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}
}

TEST_CASE("SaveJournal writes atomically and creates backup", "[save]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_save_journal_test";
    std::filesystem::create_directories(base);
    const auto slot_file = base / "slot_001.json";

    std::string error;
    REQUIRE(urpg::SaveJournal::WriteAtomically(slot_file, "{\"v\":1}", &error));
    REQUIRE(ReadAll(slot_file) == "{\"v\":1}");

    REQUIRE(urpg::SaveJournal::WriteAtomically(slot_file, "{\"v\":2}", &error));
    REQUIRE(ReadAll(slot_file) == "{\"v\":2}");
    REQUIRE(std::filesystem::exists(slot_file.string() + ".backup"));
    REQUIRE(ReadAll(slot_file.string() + ".backup") == "{\"v\":1}");

    std::filesystem::remove_all(base);
}
