#include "engine/core/export/runtime_bundle_loader.h"
#include "engine/core/tools/export_packager_bundle_writer.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace {

std::filesystem::path testTempDir(const std::string& name) {
    auto path = std::filesystem::temp_directory_path() / ("urpg_ffs07_" + name);
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
    return path;
}

std::vector<std::uint8_t> bytes(std::initializer_list<std::uint8_t> values) {
    return std::vector<std::uint8_t>(values);
}

} // namespace

TEST_CASE("runtime bundle loader accepts signed bundles", "[export][runtime_bundle][ffs07]") {
    const auto dir = testTempDir("valid_bundle");
    std::vector<urpg::tools::export_packager_detail::BundlePayload> payloads = {
        {"data/Actors.json", "database", bytes({1, 2, 3, 4}), 4, false, false, "actors-tag"},
    };

    const auto write_result = urpg::tools::export_packager_detail::writeBundleFile(
        dir, urpg::tools::ExportTarget::Windows_x64, false, "test_fixture", payloads);
    REQUIRE(write_result.success);

    const auto result = urpg::exporting::LoadRuntimeBundle(dir / "data.pck", urpg::tools::ExportTarget::Windows_x64);

    REQUIRE(result.loaded);
    REQUIRE(result.errors.empty());
    REQUIRE(result.manifest.value("signatureMode", "") == "sha256_keyed_bundle_v1");
}

TEST_CASE("runtime bundle loader rejects tampered bundles before content load", "[export][runtime_bundle][ffs07]") {
    const auto dir = testTempDir("tampered_bundle");
    std::vector<urpg::tools::export_packager_detail::BundlePayload> payloads = {
        {"data/Map001.json", "database", bytes({9, 8, 7, 6}), 4, false, false, "map-tag"},
    };
    const auto write_result = urpg::tools::export_packager_detail::writeBundleFile(
        dir, urpg::tools::ExportTarget::Windows_x64, false, "test_fixture", payloads);
    REQUIRE(write_result.success);

    {
        std::fstream bundle(dir / "data.pck", std::ios::binary | std::ios::in | std::ios::out);
        bundle.seekp(-1, std::ios::end);
        const char tampered = '\x42';
        bundle.write(&tampered, 1);
    }

    const auto result = urpg::exporting::LoadRuntimeBundle(dir / "data.pck", urpg::tools::ExportTarget::Windows_x64);

    REQUIRE_FALSE(result.loaded);
    REQUIRE(std::find(result.errors.begin(), result.errors.end(), "bundle_signature_mismatch") != result.errors.end());
}

TEST_CASE("runtime bundle atomic publish leaves existing bundle intact on failed publish", "[export][runtime_bundle][ffs07]") {
    const auto dir = testTempDir("atomic_publish");
    const auto destination = dir / "data.pck";
    {
        std::ofstream out(destination, std::ios::binary);
        out << "previous";
    }

    std::string error;
    const bool published = urpg::exporting::PublishRuntimeBundleAtomic(dir / "missing.pck", destination, &error);

    REQUIRE_FALSE(published);
    REQUIRE_FALSE(error.empty());
    std::ifstream in(destination, std::ios::binary);
    const std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    REQUIRE(contents == "previous");
}
