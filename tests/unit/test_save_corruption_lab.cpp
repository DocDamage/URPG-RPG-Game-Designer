#include "engine/core/save/save_corruption_lab.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>

namespace {

void writeText(const std::filesystem::path& path, const std::string& text) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << text;
}

std::string readText(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

} // namespace

TEST_CASE("save corruption lab never mutates original fixture", "[save][corruption][ffs09]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_ffs09_corruption";
    std::filesystem::remove_all(base);
    const auto source = base / "source" / "save.json";
    const std::string original = R"({"schema_version":"urpg.save.v1","slot_id":1})";
    writeText(source, original);

    const urpg::save::SaveCorruptionLab lab;
    const auto result = lab.corruptFixtureCopy(source, base / "lab", "invalid_json");

    REQUIRE(result.success);
    REQUIRE(readText(source) == original);
    REQUIRE(readText(result.corrupted_copy_path) != original);

    std::filesystem::remove_all(base);
}

TEST_CASE("save corruption lab recovery simulation reports selected tier", "[save][corruption][ffs09]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_ffs09_recovery";
    std::filesystem::remove_all(base);
    writeText(base / "meta.json", R"({"_slot_id":2})");

    urpg::SaveRecoveryRequest request;
    request.metadata_path = base / "meta.json";
    request.variables_path = base / "missing_vars.json";

    const urpg::save::SaveCorruptionLab lab;
    const auto report = lab.simulateRecovery(request);

    REQUIRE(report["ok"] == true);
    REQUIRE(report["tier"] == "level3_safe_skeleton");

    std::filesystem::remove_all(base);
}
