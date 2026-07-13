#include "engine/core/assets/archive_catalog.h"
#include "engine/core/platform/process_runner.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <sstream>

namespace urpg::assets {
namespace {

uint16_t read16(const std::vector<unsigned char>& data, size_t offset) {
    return static_cast<uint16_t>(data[offset]) | (static_cast<uint16_t>(data[offset + 1]) << 8U);
}
uint32_t read32(const std::vector<unsigned char>& data, size_t offset) {
    return static_cast<uint32_t>(data[offset]) | (static_cast<uint32_t>(data[offset + 1]) << 8U) |
           (static_cast<uint32_t>(data[offset + 2]) << 16U) | (static_cast<uint32_t>(data[offset + 3]) << 24U);
}

bool unsafePath(const std::string& path) {
    if (path.empty() || path.front() == '/' || path.front() == '\\') return true;
    if (path.size() >= 2 && std::isalpha(static_cast<unsigned char>(path[0])) && path[1] == ':') return true;
    std::string component;
    for (const char character : path) {
        if (character == '/' || character == '\\') {
            if (component == "..") return true;
            component.clear();
        } else component.push_back(character);
    }
    if (component == "..") return true;
    const auto slash = path.find_last_of("/\\");
    std::string filename = path.substr(slash == std::string::npos ? 0 : slash + 1);
    std::transform(filename.begin(), filename.end(), filename.begin(), [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    static constexpr std::array<const char*, 22> devices = {"CON", "PRN", "AUX", "NUL", "COM1", "COM2", "COM3", "COM4", "COM5", "COM6", "COM7", "COM8", "COM9", "LPT1", "LPT2", "LPT3", "LPT4", "LPT5", "LPT6", "LPT7", "LPT8", "LPT9"};
    const auto dot = filename.find('.');
    filename.resize(dot == std::string::npos ? filename.size() : dot);
    return std::find(devices.begin(), devices.end(), filename) != devices.end();
}

ArchiveCatalogResult failure(std::string code, std::string message) {
    return {false, std::move(code), std::move(message), {}, {}};
}

uint64_t parseBytes(const std::string& value) {
    try {
        return static_cast<uint64_t>(std::stoull(value));
    } catch (...) {
        return 0;
    }
}

ArchiveCatalogResult listWithExternalExtractor(const std::filesystem::path& archivePath, const ArchiveCatalogLimits& limits,
                                               const std::vector<std::string>& commandPrefix,
                                               ArchiveCatalogCommandExecutor executor) {
    if (!std::filesystem::is_regular_file(archivePath)) {
        return failure("archive_unreadable", "Archive could not be opened for listing.");
    }
    if (commandPrefix.empty()) {
        return failure("archive_extractor_unavailable",
                       "RAR/7z listing requires a configured 7z-compatible extractor; ZIP remains available natively.");
    }
    std::vector<std::string> command = commandPrefix;
    command.push_back("l");
    command.push_back("-slt");
    command.push_back(archivePath.string());
    ArchiveCatalogProcessResult process;
    if (executor) {
        process = executor(command);
    } else {
        urpg::platform::ProcessCommand processCommand;
        processCommand.executable = command.front();
        processCommand.arguments.assign(command.begin() + 1, command.end());
        processCommand.captureStdout = true;
        processCommand.captureStderr = true;
        const auto result = urpg::platform::runProcess(processCommand);
        process = {result.exitCode, result.stdoutText, result.stderrText + result.error};
    }
    if (process.exitCode != 0) {
        return failure("archive_extractor_failed", "The configured archive extractor could not list this archive.");
    }

    ArchiveCatalogResult result;
    result.success = true;
    result.code = "archive_listed_external";
    result.message = "Archive entries were listed through the configured extractor without extraction.";
    bool inEntries = false;
    std::string path;
    uint64_t expanded = 0;
    uint64_t compressed = 0;
    std::string attributes;
    bool encrypted = false;
    uint64_t totalExpanded = 0;
    const auto flush = [&] {
        if (path.empty()) {
            return true;
        }
        if (result.entries.size() >= limits.maxEntryCount) {
            result = failure("archive_entry_limit_exceeded", "Archive entry count exceeds the configured safety limit.");
            return false;
        }
        if (encrypted) {
            result = failure("archive_encryption_unsupported", "Encrypted archive entries cannot be listed for import.");
            return false;
        }
        if (unsafePath(path)) {
            result = failure("unsafe_archive_path", "Archive contains an absolute, traversal, or device-name entry.");
            return false;
        }
        if (attributes.find('L') != std::string::npos || attributes.find("symlink") != std::string::npos) {
            result = failure("archive_link_unsupported", "Archive contains a symbolic link entry.");
            return false;
        }
        totalExpanded += expanded;
        if (totalExpanded > limits.maxExpandedBytes) {
            result = failure("archive_expansion_limit_exceeded", "Archive expanded size exceeds the configured safety limit.");
            return false;
        }
        result.entries.push_back({path, compressed, expanded, !path.empty() && (path.back() == '/' || path.back() == '\\')});
        path.clear(); expanded = 0; compressed = 0; attributes.clear(); encrypted = false;
        return true;
    };

    std::istringstream lines(process.stdoutText);
    std::string line;
    while (std::getline(lines, line)) {
        if (line.rfind("----------", 0) == 0) {
            inEntries = true;
            continue;
        }
        if (!inEntries) {
            continue;
        }
        if (line.empty()) {
            if (!flush()) return result;
            continue;
        }
        const auto separator = line.find(" = ");
        if (separator == std::string::npos) {
            continue;
        }
        const auto key = line.substr(0, separator);
        const auto value = line.substr(separator + 3);
        if (key == "Path") path = value;
        else if (key == "Size") expanded = parseBytes(value);
        else if (key == "Packed Size") compressed = parseBytes(value);
        else if (key == "Attributes") attributes = value;
        else if (key == "Encrypted") encrypted = value == "+" || value == "true";
    }
    if (!flush()) return result;
    if (result.entries.empty()) {
        return failure("archive_extractor_output_invalid", "The configured extractor returned no parseable archive entries.");
    }
    return result;
}

} // namespace

ArchiveCatalogResult ArchiveCatalog::list(const std::filesystem::path& archivePath, const ArchiveCatalogLimits& limits,
                                          const std::vector<std::string>& externalExtractorCommand,
                                          ArchiveCatalogCommandExecutor executor) const {
    auto extension = archivePath.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char value) {
        return static_cast<char>(std::tolower(value));
    });
    if (extension != ".zip") {
        if (extension == ".rar" || extension == ".7z") {
            return listWithExternalExtractor(archivePath, limits, externalExtractorCommand, std::move(executor));
        }
        return failure("archive_listing_unsupported", "Archive listing supports ZIP natively and RAR/7z through a configured extractor.");
    }
    std::ifstream input(archivePath, std::ios::binary);
    if (!input) return failure("archive_unreadable", "Archive could not be opened for listing.");
    input.seekg(0, std::ios::end);
    const auto size = input.tellg();
    if (size < 22) return failure("archive_invalid", "ZIP archive is too short to contain a central directory.");
    const auto tailSize = static_cast<size_t>(std::min<std::streamoff>(size, 65'557));
    std::vector<unsigned char> tail(tailSize);
    input.seekg(size - static_cast<std::streamoff>(tailSize));
    input.read(reinterpret_cast<char*>(tail.data()), static_cast<std::streamsize>(tail.size()));
    size_t eocd = tailSize;
    for (size_t index = tailSize - 22;; --index) {
        if (read32(tail, index) == 0x06054b50U) { eocd = index; break; }
        if (index == 0) break;
    }
    if (eocd == tailSize) return failure("archive_invalid", "ZIP end-of-central-directory record was not found.");
    const auto entries = read16(tail, eocd + 10);
    const auto centralSize = read32(tail, eocd + 12);
    const auto centralOffset = read32(tail, eocd + 16);
    if (entries > limits.maxEntryCount) return failure("archive_entry_limit_exceeded", "Archive entry count exceeds the configured safety limit.");
    if (static_cast<uint64_t>(centralOffset) + centralSize > static_cast<uint64_t>(size)) return failure("archive_invalid", "ZIP central directory range is invalid.");
    std::vector<unsigned char> central(centralSize);
    input.seekg(centralOffset);
    input.read(reinterpret_cast<char*>(central.data()), static_cast<std::streamsize>(central.size()));
    if (!input) return failure("archive_unreadable", "ZIP central directory could not be read.");
    ArchiveCatalogResult result; result.success = true; result.code = "archive_listed"; result.message = "Archive entries listed without extraction.";
    size_t offset = 0; uint64_t totalExpanded = 0;
    for (size_t index = 0; index < entries; ++index) {
        if (offset + 46 > central.size() || read32(central, offset) != 0x02014b50U) return failure("archive_invalid", "ZIP central directory entry is malformed.");
        const auto flags = read16(central, offset + 8);
        const auto compressed = read32(central, offset + 20);
        const auto expanded = read32(central, offset + 24);
        const auto nameLength = read16(central, offset + 28);
        const auto extraLength = read16(central, offset + 30);
        const auto commentLength = read16(central, offset + 32);
        const auto externalAttributes = read32(central, offset + 38);
        if (offset + 46 + nameLength + extraLength + commentLength > central.size()) return failure("archive_invalid", "ZIP entry name range is invalid.");
        const std::string name(reinterpret_cast<const char*>(central.data() + offset + 46), nameLength);
        if ((flags & 0x0001U) != 0) return failure("archive_encryption_unsupported", "Encrypted archive entries cannot be listed for import.");
        if (unsafePath(name)) return failure("unsafe_archive_path", "Archive contains an absolute, traversal, or device-name entry.");
        const bool isSymlink = ((externalAttributes >> 16U) & 0170000U) == 0120000U;
        if (isSymlink) return failure("archive_link_unsupported", "Archive contains a symbolic link entry.");
        totalExpanded += expanded;
        if (totalExpanded > limits.maxExpandedBytes) return failure("archive_expansion_limit_exceeded", "Archive expanded size exceeds the configured safety limit.");
        result.entries.push_back({name, compressed, expanded, !name.empty() && (name.back() == '/' || name.back() == '\\')});
        offset += 46 + nameLength + extraLength + commentLength;
    }
    return result;
}

} // namespace urpg::assets
