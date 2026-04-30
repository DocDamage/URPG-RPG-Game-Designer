#include "export_packager_test_helpers.h"

#include "engine/core/security/resource_protector.h"
#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::tests::export_packager {

using urpg::tools::ExportTarget;

extern const char kBundleMagic[] = "URPGPCK1";
extern const char kAssetDiscoveryManifestPath[] = "export/asset_discovery_manifest.json";
extern const char kDevBootstrapMetadataPath[] = "DevBootstrap/export_mode.json";

void WriteFile(const std::filesystem::path& path, const std::string& content) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << content;
}

void WritePromotedSourceManifest(const std::filesystem::path& root, const std::string& sourceId,
                                 const std::string& legalDisposition) {
    WriteFile(root / "asset_sources" / (sourceId + ".json"),
              "{\n"
              "  \"source_id\": \"" +
                  sourceId +
                  "\",\n"
                  "  \"repo_name\": \"test/source\",\n"
                  "  \"source_url\": \"https://example.invalid/assets\",\n"
                  "  \"legal_disposition\": \"" +
                  legalDisposition +
                  "\",\n"
                  "  \"promotion_status\": \"promoted\"\n"
                  "}\n");
}

void WriteAssetLicenseManifest(const std::filesystem::path& root, const std::vector<std::string>& paths) {
    nlohmann::json manifest;
    manifest["format"] = "URPG_ASSET_LICENSES_V1";
    manifest["assets"] = nlohmann::json::array();
    for (const auto& path : paths) {
        manifest["assets"].push_back({
            {"path", path},
            {"license", "MIT"},
            {"attribution", "URPG test fixture"},
            {"sourceUrl", "https://example.invalid/assets"},
        });
    }
    WriteFile(root / "asset_licenses.json", manifest.dump(2) + "\n");
}

std::vector<uint8_t> ReadFileBytes(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::vector<uint8_t>(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::vector<uint8_t> ReadBundleEntryBytes(const std::filesystem::path& path, const nlohmann::json& entry);

std::string ReadFileText(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
}

std::uint32_t ReadUint32LE(const std::vector<uint8_t>& bytes, std::size_t offset) {
    return static_cast<std::uint32_t>(bytes[offset]) | (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
}

std::string BundleObfuscationKeyForTarget(ExportTarget target) {
    switch (target) {
    case ExportTarget::Windows_x64:
        return "urpg-export-bundle-win";
    case ExportTarget::Linux_x64:
        return "urpg-export-bundle-linux";
    case ExportTarget::macOS_Universal:
        return "urpg-export-bundle-macos";
    case ExportTarget::Web_WASM:
        return "urpg-export-bundle-web";
    default:
        return "urpg-export-bundle";
    }
}

std::string BundleSignatureKeyForTarget(ExportTarget target) {
    switch (target) {
    case ExportTarget::Windows_x64:
        return "urpg-export-signature-win-v1";
    case ExportTarget::Linux_x64:
        return "urpg-export-signature-linux-v1";
    case ExportTarget::macOS_Universal:
        return "urpg-export-signature-macos-v1";
    case ExportTarget::Web_WASM:
        return "urpg-export-signature-web-v1";
    default:
        return "urpg-export-signature-v1";
    }
}

std::string BundleIntegrityScope(const nlohmann::json& entry) {
    return entry.value("path", "") + "|" + entry.value("kind", "") + "|" +
           (entry.value("compressed", false) ? "1" : "0") + "|" + (entry.value("obfuscated", false) ? "1" : "0") + "|" +
           std::to_string(entry.value("rawSize", 0u));
}

std::string ComputeBundleIntegrityTag(const nlohmann::json& entry, const std::vector<uint8_t>& storedBytes,
                                      ExportTarget target) {
    urpg::security::ResourceProtector protector;
    return protector.computeIntegrityTag(BundleIntegrityScope(entry), storedBytes,
                                         BundleObfuscationKeyForTarget(target));
}

nlohmann::json BuildBundleSignatureView(const nlohmann::json& manifest) {
    auto signatureView = manifest;
    signatureView.erase("payloadOffset");
    signatureView.erase("bundleSignature");
    return signatureView;
}

std::string ComputeBundleSignature(const std::filesystem::path& path, const nlohmann::json& manifest,
                                   ExportTarget target) {
    std::vector<uint8_t> storedPayloadBytes;
    for (const auto& entry : manifest["entries"]) {
        const auto entryBytes = ReadBundleEntryBytes(path, entry);
        storedPayloadBytes.insert(storedPayloadBytes.end(), entryBytes.begin(), entryBytes.end());
    }

    urpg::security::ResourceProtector protector;
    return protector.computeCryptographicSignature(BuildBundleSignatureView(manifest).dump(), storedPayloadBytes,
                                                   BundleSignatureKeyForTarget(target));
}

nlohmann::json ReadBundleManifest(const std::filesystem::path& path) {
    const auto bytes = ReadFileBytes(path);
    REQUIRE(bytes.size() > sizeof(kBundleMagic) - 1 + sizeof(std::uint32_t));
    REQUIRE(std::string(bytes.begin(), bytes.begin() + (sizeof(kBundleMagic) - 1)) == kBundleMagic);

    const auto manifestSize = ReadUint32LE(bytes, sizeof(kBundleMagic) - 1);
    const auto manifestOffset = (sizeof(kBundleMagic) - 1) + sizeof(std::uint32_t);
    REQUIRE(bytes.size() >= manifestOffset + manifestSize);

    const std::string manifestText(bytes.begin() + manifestOffset, bytes.begin() + manifestOffset + manifestSize);
    return nlohmann::json::parse(manifestText);
}

std::vector<uint8_t> ReadBundleEntryBytes(const std::filesystem::path& path, const nlohmann::json& entry) {
    const auto bytes = ReadFileBytes(path);
    const auto manifest = ReadBundleManifest(path);
    const auto payloadOffset = manifest["payloadOffset"].get<std::size_t>();
    const auto offset = entry["offset"].get<std::size_t>();
    const auto storedSize = entry["storedSize"].get<std::size_t>();

    REQUIRE(bytes.size() >= payloadOffset + offset + storedSize);
    return std::vector<uint8_t>(bytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset),
                                bytes.begin() + static_cast<std::ptrdiff_t>(payloadOffset + offset + storedSize));
}

std::vector<uint8_t> DecodeBundleEntryBytes(const std::filesystem::path& path, ExportTarget target,
                                            const nlohmann::json& entry) {
    auto payload = ReadBundleEntryBytes(path, entry);
    if (entry.value("obfuscated", false)) {
        urpg::security::ResourceProtector protector;
        protector.obfuscate(payload, BundleObfuscationKeyForTarget(target));
    }
    if (entry.value("compressed", false)) {
        payload = urpg::core::AssetCompressor::instance().decompress(payload);
    }
    return payload;
}

#if defined(_WIN32) && defined(URPG_EXPORT_SMOKE_APP_PATH)
std::filesystem::path GetExportSmokeAppPath() {
    return std::filesystem::path(URPG_EXPORT_SMOKE_APP_PATH);
}
#endif

#ifdef URPG_PACK_CLI_PATH
std::filesystem::path GetPackCliPath() {
    return std::filesystem::path(URPG_PACK_CLI_PATH);
}

ProcessResult RunPackCli(const std::vector<std::string>& args) {
    const auto tempRoot =
        std::filesystem::temp_directory_path() /
        ("urpg_pack_cli_process_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    std::filesystem::remove_all(tempRoot);
    std::filesystem::create_directories(tempRoot);

    const auto stdoutPath = tempRoot / "stdout.txt";
    const auto stderrPath = tempRoot / "stderr.txt";
    const auto exePath = GetPackCliPath();
    REQUIRE(std::filesystem::exists(exePath));

    ProcessResult result;

#ifdef _WIN32
    auto toWide = [](const std::string& text) { return std::wstring(text.begin(), text.end()); };

    auto quote = [](const std::wstring& text) { return L"\"" + text + L"\""; };

    std::wstring commandLine = quote(exePath.wstring());
    for (const auto& arg : args) {
        commandLine += L" ";
        commandLine += quote(toWide(arg));
    }

    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE stdoutHandle = CreateFileW(stdoutPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &securityAttributes,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(stdoutHandle != INVALID_HANDLE_VALUE);

    HANDLE stderrHandle = CreateFileW(stderrPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &securityAttributes,
                                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    REQUIRE(stderrHandle != INVALID_HANDLE_VALUE);

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = stdoutHandle;
    startupInfo.hStdError = stderrHandle;

    PROCESS_INFORMATION processInfo{};
    std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
    mutableCommandLine.push_back(L'\0');

    const BOOL launched = CreateProcessW(nullptr, mutableCommandLine.data(), nullptr, nullptr, TRUE, 0, nullptr,
                                         nullptr, &startupInfo, &processInfo);

    CloseHandle(stdoutHandle);
    CloseHandle(stderrHandle);

    REQUIRE(launched == TRUE);

    WaitForSingleObject(processInfo.hProcess, 30000);

    DWORD exitCode = 1;
    REQUIRE(GetExitCodeProcess(processInfo.hProcess, &exitCode) == TRUE);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    result.exitCode = static_cast<int>(exitCode);
#else
    std::ostringstream command;
    command << "\"" << exePath.string() << "\"";
    for (const auto& arg : args) {
        command << " \"" << arg << "\"";
    }
    command << " > \"" << stdoutPath.string() << "\"";
    command << " 2> \"" << stderrPath.string() << "\"";

    result.exitCode = std::system(command.str().c_str());
#endif

    if (std::filesystem::exists(stdoutPath)) {
        result.stdoutText = ReadFileText(stdoutPath);
    }
    if (std::filesystem::exists(stderrPath)) {
        result.stderrText = ReadFileText(stderrPath);
    }

    std::filesystem::remove_all(tempRoot);
    return result;
}
#endif

#ifdef _WIN32
bool LaunchProcessAndCaptureOutput(const std::filesystem::path& executable, const std::filesystem::path& markerPath,
                                   const std::filesystem::path& logPath) {
    SECURITY_ATTRIBUTES securityAttributes{};
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    HANDLE logHandle = CreateFileW(logPath.c_str(), GENERIC_WRITE, FILE_SHARE_READ, &securityAttributes, CREATE_ALWAYS,
                                   FILE_ATTRIBUTE_NORMAL, nullptr);
    if (logHandle == INVALID_HANDLE_VALUE) {
        return false;
    }

    STARTUPINFOW startupInfo{};
    startupInfo.cb = sizeof(startupInfo);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = logHandle;
    startupInfo.hStdError = logHandle;

    PROCESS_INFORMATION processInfo{};
    std::wstring commandLine = L"\"" + executable.wstring() + L"\" --write-marker \"" + markerPath.wstring() + L"\"";

    const BOOL launched = CreateProcessW(nullptr, commandLine.data(), nullptr, nullptr, TRUE, 0, nullptr, nullptr,
                                         &startupInfo, &processInfo);

    CloseHandle(logHandle);

    if (!launched) {
        return false;
    }

    WaitForSingleObject(processInfo.hProcess, 30000);

    DWORD exitCode = 1;
    GetExitCodeProcess(processInfo.hProcess, &exitCode);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

    return exitCode == 0;
}
#endif

} // namespace urpg::tests::export_packager`n
