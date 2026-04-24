#include "tools/audit/project_audit_template_spec.h"

#include <algorithm>
#include <optional>
#include <sstream>

namespace urpg::tools::audit {

namespace {

std::string trim(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) {
        return "";
    }

    const auto end = text.find_last_not_of(" \t\r\n");
    return text.substr(begin, end - begin + 1);
}

bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

std::optional<std::string> extractBacktickValue(const std::string& line) {
    const auto firstTick = line.find('`');
    if (firstTick == std::string::npos) {
        return std::nullopt;
    }

    const auto secondTick = line.find('`', firstTick + 1);
    if (secondTick == std::string::npos || secondTick <= firstTick + 1) {
        return std::nullopt;
    }

    return line.substr(firstTick + 1, secondTick - firstTick - 1);
}

} // namespace

std::string templateBarDisplayName(const std::string& barName) {
    if (barName == "accessibility") {
        return "Accessibility";
    }
    if (barName == "audio") {
        return "Audio";
    }
    if (barName == "input") {
        return "Input";
    }
    if (barName == "localization") {
        return "Localization";
    }
    if (barName == "performance") {
        return "Performance";
    }
    return barName;
}

std::vector<std::string> extractTemplateSpecRequiredSubsystems(const std::string& text) {
    std::vector<std::string> subsystems;
    std::istringstream stream(text);
    std::string line;
    bool inRequiredSubsystems = false;

    while (std::getline(stream, line)) {
        const std::string trimmedLine = trim(line);
        if (startsWith(trimmedLine, "## ")) {
            inRequiredSubsystems = trimmedLine == "## Required Subsystems";
            continue;
        }

        if (!inRequiredSubsystems || !startsWith(trimmedLine, "| `")) {
            continue;
        }

        const auto subsystem = extractBacktickValue(trimmedLine);
        if (subsystem.has_value()) {
            subsystems.push_back(*subsystem);
        }
    }

    std::sort(subsystems.begin(), subsystems.end());
    subsystems.erase(std::unique(subsystems.begin(), subsystems.end()), subsystems.end());
    return subsystems;
}

nlohmann::json extractTemplateSpecBars(const std::string& text) {
    nlohmann::json bars = nlohmann::json::object();
    std::istringstream stream(text);
    std::string line;
    bool inCrossCuttingBars = false;

    while (std::getline(stream, line)) {
        const std::string trimmedLine = trim(line);
        if (startsWith(trimmedLine, "## ")) {
            inCrossCuttingBars = trimmedLine == "## Cross-Cutting Minimum Bars";
            continue;
        }

        if (!inCrossCuttingBars || !startsWith(trimmedLine, "| ")) {
            continue;
        }

        const auto firstSeparator = trimmedLine.find('|', 2);
        if (firstSeparator == std::string::npos) {
            continue;
        }

        const std::string barLabel = trim(trimmedLine.substr(2, firstSeparator - 2));
        const auto statusValue = extractBacktickValue(trimmedLine);
        if (!statusValue.has_value()) {
            continue;
        }

        if (barLabel == "Accessibility") {
            bars["accessibility"] = *statusValue;
        } else if (barLabel == "Audio") {
            bars["audio"] = *statusValue;
        } else if (barLabel == "Input") {
            bars["input"] = *statusValue;
        } else if (barLabel == "Localization") {
            bars["localization"] = *statusValue;
        } else if (barLabel == "Performance") {
            bars["performance"] = *statusValue;
        }
    }

    return bars;
}

std::string joinItems(const std::vector<std::string>& items) {
    std::ostringstream buffer;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            buffer << ", ";
        }
        buffer << items[i];
    }
    return buffer.str();
}

} // namespace urpg::tools::audit
