#include "engine/core/ability/authored_ability_asset.h"

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

namespace {

std::string read(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
}

} // namespace

TEST_CASE("Authored ability save publishes atomically and preserves acknowledged content on interruption",
          "[ability][persistence][atomic][project]") {
    const auto root = std::filesystem::temp_directory_path() / "urpg_ability_atomic_save_test";
    std::error_code error;
    std::filesystem::remove_all(root, error);
    const auto path = root / "content" / "abilities" / "fire.json";
    urpg::ability::AuthoredAbilityAsset initial;
    initial.ability_id = "fire";
    initial.effect_value = 10.0F;
    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(initial, path));
    const auto acknowledged = read(path);
    REQUIRE(acknowledged.find("\"effect_value\": 10.0") != std::string::npos);

    auto replacement = initial;
    replacement.effect_value = 99.0F;
    std::string diagnostic;
    REQUIRE_FALSE(urpg::ability::saveAuthoredAbilityAssetToFile(
        replacement, path, &diagnostic, [] { return false; }));
    REQUIRE(diagnostic.find("interrupted") != std::string::npos);
    REQUIRE(read(path) == acknowledged);
    REQUIRE_FALSE(std::filesystem::exists(std::filesystem::path(path.string() + ".tmp")));

    REQUIRE(urpg::ability::saveAuthoredAbilityAssetToFile(replacement, path));
    REQUIRE(read(path).find("\"effect_value\": 99.0") != std::string::npos);
    std::filesystem::remove_all(root, error);
}
