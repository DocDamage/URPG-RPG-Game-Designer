#include "engine/core/export/export_validator.h"

#include <filesystem>

namespace urpg::exporting {

std::vector<std::string> ExportValidator::validateExportDirectory(const std::string& path, tools::ExportTarget target) {
    std::vector<std::string> errors;
    std::filesystem::path dir(path);

    if (!std::filesystem::exists(dir) || !std::filesystem::is_directory(dir)) {
        errors.emplace_back("Path does not exist or is not a directory: " + path);
        return errors;
    }

    auto requirements = getRequirementsForTarget(target);

    for (const auto& req : requirements) {
        bool found = false;

        if (req.filePattern == "*.app") {
            found = checkAnyAppDirectory(dir);
        } else if (req.filePattern == "executable_without_extension") {
            found = checkAnyExecutableWithoutExtension(dir);
        } else if (req.filePattern.find('*') != std::string::npos) {
            found = checkPatternExists(dir, req.filePattern);
        } else {
            found = std::filesystem::exists(dir / req.filePattern);
        }

        if (req.required && !found) {
            errors.emplace_back("Missing required file: " + req.filePattern + " (" + req.description + ")");
        }
    }

    return errors;
}

std::vector<PlatformRequirement> ExportValidator::getRequirementsForTarget(tools::ExportTarget target) const {
    switch (target) {
        case tools::ExportTarget::Windows_x64:
            return {
                {"*.exe", true, "Windows executable"},
                {"data.pck", true, "Asset package"},
                {"*.dll", false, "Dynamic libraries"}
            };
        case tools::ExportTarget::Linux_x64:
            return {
                {"executable_without_extension", true, "Linux executable"},
                {"data.pck", true, "Asset package"},
                {"*.so", false, "Shared libraries"}
            };
        case tools::ExportTarget::macOS_Universal:
            return {
                {"*.app", true, "macOS application bundle"},
                {"data.pck", true, "Asset package"}
            };
        case tools::ExportTarget::Web_WASM:
            return {
                {"index.html", true, "HTML entry point"},
                {"*.wasm", true, "WebAssembly binary"},
                {"*.js", true, "JavaScript loader"},
                {"data.pck", true, "Asset package"}
            };
        default:
            return {};
    }
}

nlohmann::json ExportValidator::buildReportJson(const std::vector<std::string>& errors, tools::ExportTarget target) const {
    nlohmann::json report;
    report["target"] = targetToString(target);
    report["passed"] = errors.empty();
    report["errors"] = errors;
    return report;
}

bool ExportValidator::checkPatternExists(const std::filesystem::path& dir, const std::string& pattern) const {
    if (pattern.size() < 2 || pattern[0] != '*') {
        return std::filesystem::exists(dir / pattern);
    }

    std::string ext = pattern.substr(1);
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file() && entry.path().extension().string() == ext) {
            return true;
        }
    }
    return false;
}

bool ExportValidator::checkAnyAppDirectory(const std::filesystem::path& dir) const {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_directory()) {
            std::string name = entry.path().filename().string();
            if (name.size() > 4 && name.substr(name.size() - 4) == ".app") {
                return true;
            }
        }
    }
    return false;
}

bool ExportValidator::checkAnyExecutableWithoutExtension(const std::filesystem::path& dir) const {
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::string name = entry.path().filename().string();
            if (name.find('.') == std::string::npos) {
                return true;
            }
        }
    }
    return false;
}

std::string ExportValidator::targetToString(tools::ExportTarget target) const {
    switch (target) {
        case tools::ExportTarget::Windows_x64: return "Windows_x64";
        case tools::ExportTarget::Linux_x64: return "Linux_x64";
        case tools::ExportTarget::macOS_Universal: return "macOS_Universal";
        case tools::ExportTarget::Web_WASM: return "Web_WASM";
        default: return "Unknown";
    }
}

} // namespace urpg::exporting
