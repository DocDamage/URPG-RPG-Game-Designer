#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>

#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

TEST_CASE("Hugging Face curated manifest and vendored fixtures stay aligned", "[assets][huggingface]") {
    const fs::path repoRoot = fs::path(URPG_SOURCE_DIR);
    const fs::path manifestPath = repoRoot / "tools" / "assets" / "huggingface_curated_manifest.json";
    const fs::path readmePath = repoRoot / "imports" / "raw" / "third_party_assets" / "huggingface" / "README.md";

    REQUIRE(fs::exists(manifestPath));
    REQUIRE(fs::exists(readmePath));

    nlohmann::json manifest;
    {
        std::ifstream in(manifestPath);
        REQUIRE(in.good());
        in >> manifest;
    }

    REQUIRE(manifest.contains("datasets"));
    REQUIRE(manifest["datasets"].is_array());
    REQUIRE(manifest["datasets"].size() == 5);

    bool foundManifestOnly = false;
    bool foundVendorDirect = false;
    for (const auto& dataset : manifest["datasets"]) {
        REQUIRE(dataset.contains("repo"));
        REQUIRE(dataset.contains("license"));
        REQUIRE(dataset.contains("ingestMode"));
        REQUIRE(dataset.contains("targetRoot"));
        REQUIRE(dataset.contains("files"));
        REQUIRE(dataset["files"].is_array());
        REQUIRE_FALSE(dataset["files"].empty());

        const auto mode = dataset["ingestMode"].get<std::string>();
        if (mode == "manifest_only") {
            foundManifestOnly = true;
        }
        if (mode == "vendor_direct") {
            foundVendorDirect = true;
            const fs::path targetRoot = repoRoot / fs::path(dataset["targetRoot"].get<std::string>());
            for (const auto& file : dataset["files"]) {
                fs::path relative = fs::path(file.get<std::string>());
                REQUIRE(fs::exists(targetRoot / relative));
            }
        }
    }

    REQUIRE(foundManifestOnly);
    REQUIRE(foundVendorDirect);
}
