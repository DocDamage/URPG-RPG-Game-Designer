#include "engine/core/export/export_bundle_contract.h"
#include "engine/core/export/runtime_bundle_loader.h"
#include "engine/core/security/resource_protector.h"
#include "engine/core/tools/export_packager_bundle_writer.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <string>
#include <utility>
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

urpg::tools::export_packager_detail::BundlePayload payload(std::string path, std::string kind,
                                                           std::vector<std::uint8_t> storedBytes) {
    urpg::tools::export_packager_detail::BundlePayload result;
    result.path = std::move(path);
    result.kind = std::move(kind);
    result.rawSize = storedBytes.size();
    result.bytes = std::move(storedBytes);

    nlohmann::json entry = {
        {"path", result.path},
        {"kind", result.kind},
        {"compressed", result.compressed},
        {"obfuscated", result.obfuscated},
        {"rawSize", result.rawSize},
    };
    urpg::security::ResourceProtector protector;
    result.integrityTag = protector.computeIntegrityTag(
        urpg::exporting::bundle_contract::makePayloadIntegrityScope(entry), result.bytes,
        urpg::exporting::bundle_contract::bundleObfuscationKey(urpg::tools::ExportTarget::Windows_x64));
    return result;
}

std::vector<std::uint8_t> readFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

void writeFileBytes(const std::filesystem::path& path, const std::vector<std::uint8_t>& contents) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out.write(reinterpret_cast<const char*>(contents.data()), static_cast<std::streamsize>(contents.size()));
}

void rewriteManifestWithoutResigning(const std::filesystem::path& bundlePath,
                                     const std::function<void(nlohmann::json&)>& mutate) {
    auto bundle = readFileBytes(bundlePath);
    const auto manifestSize = urpg::exporting::bundle_contract::readUint32LE(
        bundle, sizeof(urpg::exporting::bundle_contract::kBundleMagic) - 1);
    const auto manifestOffset = (sizeof(urpg::exporting::bundle_contract::kBundleMagic) - 1) + sizeof(std::uint32_t);
    const auto payloadOffset = manifestOffset + manifestSize;
    const std::string manifestText(bundle.begin() + static_cast<std::ptrdiff_t>(manifestOffset),
                                   bundle.begin() + static_cast<std::ptrdiff_t>(payloadOffset));
    auto manifest = nlohmann::json::parse(manifestText);
    std::vector<std::uint8_t> payloadBytes(
        bundle.begin() + static_cast<std::ptrdiff_t>(manifest["payloadOffset"].get<std::size_t>()), bundle.end());

    mutate(manifest);
    std::string rewrittenManifest = manifest.dump();
    for (int i = 0; i < 2; ++i) {
        manifest["payloadOffset"] = manifestOffset + rewrittenManifest.size();
        rewrittenManifest = manifest.dump();
    }

    const auto rewrittenManifestSize = static_cast<std::uint32_t>(rewrittenManifest.size());
    std::vector<std::uint8_t> rewritten;
    for (char c : std::string(urpg::exporting::bundle_contract::kBundleMagic,
                              sizeof(urpg::exporting::bundle_contract::kBundleMagic) - 1)) {
        rewritten.push_back(static_cast<std::uint8_t>(c));
    }
    rewritten.push_back(static_cast<std::uint8_t>(rewrittenManifestSize & 0xFFu));
    rewritten.push_back(static_cast<std::uint8_t>((rewrittenManifestSize >> 8u) & 0xFFu));
    rewritten.push_back(static_cast<std::uint8_t>((rewrittenManifestSize >> 16u) & 0xFFu));
    rewritten.push_back(static_cast<std::uint8_t>((rewrittenManifestSize >> 24u) & 0xFFu));
    rewritten.insert(rewritten.end(), rewrittenManifest.begin(), rewrittenManifest.end());
    rewritten.insert(rewritten.end(), payloadBytes.begin(), payloadBytes.end());
    writeFileBytes(bundlePath, rewritten);
}

} // namespace

TEST_CASE("runtime bundle loader accepts signed bundles", "[export][runtime_bundle][ffs07]") {
    const auto dir = testTempDir("valid_bundle");
    std::vector<urpg::tools::export_packager_detail::BundlePayload> payloads = {
        payload("data/Actors.json", "database", bytes({1, 2, 3, 4})),
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
        payload("data/Map001.json", "database", bytes({9, 8, 7, 6})),
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
    REQUIRE(std::find(result.errors.begin(), result.errors.end(), "entry_integrity_mismatch:data/Map001.json") !=
            result.errors.end());
}

TEST_CASE("runtime bundle loader rejects tampered manifest metadata", "[export][runtime_bundle]") {
    const auto dir = testTempDir("tampered_manifest");
    std::vector<urpg::tools::export_packager_detail::BundlePayload> payloads = {
        payload("data/Map002.json", "database", bytes({1, 3, 5, 7})),
    };
    const auto write_result = urpg::tools::export_packager_detail::writeBundleFile(
        dir, urpg::tools::ExportTarget::Windows_x64, false, "test_fixture", payloads);
    REQUIRE(write_result.success);

    rewriteManifestWithoutResigning(
        dir / "data.pck", [](nlohmann::json& manifest) { manifest["assetDiscoveryMode"] = "tampered_fixture"; });

    const auto result = urpg::exporting::LoadRuntimeBundle(dir / "data.pck", urpg::tools::ExportTarget::Windows_x64);

    REQUIRE_FALSE(result.loaded);
    REQUIRE(std::find(result.errors.begin(), result.errors.end(), "bundle_signature_mismatch") != result.errors.end());
}

TEST_CASE("runtime bundle loader rejects missing bundle signature", "[export][runtime_bundle]") {
    const auto dir = testTempDir("missing_signature");
    std::vector<urpg::tools::export_packager_detail::BundlePayload> payloads = {
        payload("data/Items.json", "database", bytes({2, 4, 6, 8})),
    };
    const auto write_result = urpg::tools::export_packager_detail::writeBundleFile(
        dir, urpg::tools::ExportTarget::Windows_x64, false, "test_fixture", payloads);
    REQUIRE(write_result.success);

    rewriteManifestWithoutResigning(dir / "data.pck",
                                    [](nlohmann::json& manifest) { manifest.erase("bundleSignature"); });

    const auto result = urpg::exporting::LoadRuntimeBundle(dir / "data.pck", urpg::tools::ExportTarget::Windows_x64);

    REQUIRE_FALSE(result.loaded);
    REQUIRE(std::find(result.errors.begin(), result.errors.end(), "missing_bundle_signature") != result.errors.end());
}

TEST_CASE("runtime bundle loader rejects unsupported signature mode", "[export][runtime_bundle]") {
    const auto dir = testTempDir("unsupported_signature_mode");
    std::vector<urpg::tools::export_packager_detail::BundlePayload> payloads = {
        payload("data/Skills.json", "database", bytes({3, 6, 9, 12})),
    };
    const auto write_result = urpg::tools::export_packager_detail::writeBundleFile(
        dir, urpg::tools::ExportTarget::Windows_x64, false, "test_fixture", payloads);
    REQUIRE(write_result.success);

    rewriteManifestWithoutResigning(dir / "data.pck",
                                    [](nlohmann::json& manifest) { manifest["signatureMode"] = "sha1_legacy_test"; });

    const auto result = urpg::exporting::LoadRuntimeBundle(dir / "data.pck", urpg::tools::ExportTarget::Windows_x64);

    REQUIRE_FALSE(result.loaded);
    REQUIRE(std::find(result.errors.begin(), result.errors.end(), "unsupported_signature_mode") != result.errors.end());
}

TEST_CASE("runtime bundle atomic publish leaves existing bundle intact on failed publish",
          "[export][runtime_bundle][ffs07]") {
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
