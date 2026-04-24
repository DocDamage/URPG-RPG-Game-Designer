#include "engine/core/tools/export_packager_executable_staging.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <system_error>

namespace urpg::tools::export_packager_detail {

namespace {

std::string makeWebBootstrapHtml() {
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>URPG Web Export</title>
</head>
<body>
  <main id="urpg-root">URPG bounded web export bootstrap</main>
  <script type="module" src="./game.js"></script>
</body>
</html>
)";
}

std::string makeWebBootstrapLoader() {
    return R"(const bundlePath = "./data.pck";
const wasmPath = "./game.wasm";

async function loadUrpgExport() {
  const [bundleResponse, wasmResponse] = await Promise.all([
    fetch(bundlePath),
    fetch(wasmPath),
  ]);

  if (!bundleResponse.ok) {
    throw new Error(`Failed to fetch bundle: ${bundleResponse.status}`);
  }
  if (!wasmResponse.ok) {
    throw new Error(`Failed to fetch wasm: ${wasmResponse.status}`);
  }

  const bundleBytes = await bundleResponse.arrayBuffer();
  const wasmBytes = await wasmResponse.arrayBuffer();
  const wasmModule = await WebAssembly.instantiate(wasmBytes, {});

  globalThis.__URPG_EXPORT_BOOTSTRAP__ = {
    bundleByteLength: bundleBytes.byteLength,
    exports: Object.keys(wasmModule.instance.exports),
  };

  const root = document.getElementById("urpg-root");
  if (root) {
    root.textContent = "URPG bounded web export bootstrap ready";
  }
}

loadUrpgExport().catch((error) => {
  console.error("URPG web export bootstrap failed", error);
  const root = document.getElementById("urpg-root");
  if (root) {
    root.textContent = "URPG bounded web export bootstrap failed";
  }
});
)";
}

std::vector<std::uint8_t> makeEmptyWasmModule() {
    return {
        0x00, 0x61, 0x73, 0x6D,
        0x01, 0x00, 0x00, 0x00,
    };
}

std::string makeLinuxBootstrapScript() {
    return R"URPGSCRIPT(#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
BUNDLE_PATH="$SCRIPT_DIR/data.pck"

if [ ! -f "$BUNDLE_PATH" ]; then
  echo "URPG Linux bootstrap missing data.pck" >&2
  exit 1
fi

echo "URPG bounded Linux export bootstrap ready"
exit 0
)URPGSCRIPT";
}

std::string makeMacBootstrapScript() {
    return R"URPGMAC(#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
APP_ROOT="$(CDPATH= cd -- "$SCRIPT_DIR/../.." && pwd)"
BUNDLE_PATH="$APP_ROOT/data.pck"

if [ ! -f "$BUNDLE_PATH" ]; then
  echo "URPG macOS bootstrap missing data.pck" >&2
  exit 1
fi

echo "URPG bounded macOS export bootstrap ready"
exit 0
)URPGMAC";
}

std::string makeMacInfoPlist() {
    return R"(<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>MyGame</string>
  <key>CFBundleIdentifier</key>
  <string>com.urpg.export.mygame</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>MyGame</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>1.0</string>
</dict>
</plist>
)";
}

bool writeTextFile(const std::filesystem::path& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    out << text;
    out.close();
    return out.good();
}

bool writeBytesFile(const std::filesystem::path& path, const std::vector<std::uint8_t>& bytes) {
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
    out.close();
    return out.good();
}

void makeExecutable(const std::filesystem::path& path) {
    std::error_code chmodError;
    std::filesystem::permissions(
        path,
        std::filesystem::perms::owner_read |
            std::filesystem::perms::owner_write |
            std::filesystem::perms::owner_exec |
            std::filesystem::perms::group_read |
            std::filesystem::perms::group_exec |
            std::filesystem::perms::others_read |
            std::filesystem::perms::others_exec,
        std::filesystem::perm_options::replace,
        chmodError);
}

ExecutableStageResult stageRealWindowsRuntime(const ExportConfig& config) {
    ExecutableStageResult result;
    const std::filesystem::path sourceBinary(config.runtimeBinaryPath);
    const std::filesystem::path outDir(config.outputDir);
    const std::filesystem::path stagedBinary = outDir / "game.exe";

    std::error_code copyError;
    std::filesystem::copy_file(
        sourceBinary,
        stagedBinary,
        std::filesystem::copy_options::overwrite_existing,
        copyError);
    if (copyError) {
        result.log += "Failed to copy real Windows runtime binary: " + copyError.message() + "\n";
        return result;
    }
    result.files.push_back("game.exe");

    for (const auto& entry : std::filesystem::directory_iterator(sourceBinary.parent_path())) {
        if (!entry.is_regular_file() || entry.path().extension() != ".dll") {
            continue;
        }

        const std::filesystem::path destination = outDir / entry.path().filename();
        std::error_code dllCopyError;
        std::filesystem::copy_file(
            entry.path(),
            destination,
            std::filesystem::copy_options::overwrite_existing,
            dllCopyError);
        if (dllCopyError) {
            result.log += "Failed to copy runtime dependency " + entry.path().filename().string() + ": " +
                          dllCopyError.message() + "\n";
            result.files.clear();
            return result;
        }
        result.files.push_back(entry.path().filename().string());
    }

    result.log += "Staged real Windows smoke runtime from " + sourceBinary.string() + ".\n";
    result.log += "Staged " + std::to_string(result.files.size()) + " Windows runtime artifact(s).\n";
    return result;
}

} // namespace

ExecutableStageResult stageExecutableArtifacts(const ExportConfig& config) {
    if (config.target == ExportTarget::Windows_x64 && !config.runtimeBinaryPath.empty()) {
        return stageRealWindowsRuntime(config);
    }

    ExecutableStageResult result;
    const std::filesystem::path outDir(config.outputDir);

    switch (config.target) {
        case ExportTarget::Windows_x64: {
            if (!writeTextFile(outDir / "game.exe", "MZ synthetic windows executable\n")) {
                result.log += "Failed to synthesize bounded Windows executable placeholder.\n";
                return result;
            }
            result.files.push_back("game.exe");
            break;
        }
        case ExportTarget::Linux_x64: {
            const std::filesystem::path exePath = outDir / "game";
            if (!writeTextFile(exePath, makeLinuxBootstrapScript())) {
                result.log += "Failed to synthesize bounded Linux bootstrap launcher script.\n";
                return result;
            }
            makeExecutable(exePath);
            result.files.push_back("game");
            result.log += "Synthesized bounded Linux bootstrap launcher script.\n";
            break;
        }
        case ExportTarget::macOS_Universal: {
            const std::filesystem::path appPath = outDir / "MyGame.app";
            std::error_code appDirError;
            std::filesystem::create_directories(appPath, appDirError);
            const std::filesystem::path exePath = appPath / "Contents" / "MacOS" / "MyGame";
            std::error_code exeDirError;
            std::filesystem::create_directories(exePath.parent_path(), exeDirError);
            if (appDirError || exeDirError) {
                result.log += "Failed to prepare bounded macOS app bootstrap structure.\n";
                return result;
            }

            const std::filesystem::path infoPlistPath = appPath / "Contents" / "Info.plist";
            if (!writeTextFile(exePath, makeMacBootstrapScript()) ||
                !writeTextFile(infoPlistPath, makeMacInfoPlist())) {
                result.log += "Failed to synthesize bounded macOS app bootstrap structure.\n";
                return result;
            }
            makeExecutable(exePath);
            result.files.push_back("MyGame.app");
            result.log += "Synthesized bounded macOS app bootstrap structure.\n";
            break;
        }
        case ExportTarget::Web_WASM: {
            if (!writeTextFile(outDir / "index.html", makeWebBootstrapHtml()) ||
                !writeBytesFile(outDir / "game.wasm", makeEmptyWasmModule()) ||
                !writeTextFile(outDir / "game.js", makeWebBootstrapLoader())) {
                result.log += "Failed to synthesize bounded Web bootstrap artifacts.\n";
                return result;
            }
            result.files.push_back("index.html");
            result.files.push_back("game.wasm");
            result.files.push_back("game.js");
            result.log += "Synthesized bounded Web bootstrap artifacts with real HTML/JS/Wasm structure.\n";
            break;
        }
    }

    result.log += "Synthesized " + std::to_string(result.files.size()) + " executable artifact(s).\n";
    return result;
}

} // namespace urpg::tools::export_packager_detail
