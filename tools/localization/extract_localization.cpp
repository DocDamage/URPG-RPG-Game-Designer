#include "engine/core/localization/localization_document_tools.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

namespace {

nlohmann::json readJson(const std::filesystem::path& path) {
    std::ifstream stream(path);
    if (!stream.good()) {
        throw std::runtime_error("Failed to open input JSON: " + path.string());
    }
    nlohmann::json json;
    stream >> json;
    return json;
}

void writeJson(const std::filesystem::path& path, const nlohmann::json& json) {
    if (!path.parent_path().empty()) {
        std::filesystem::create_directories(path.parent_path());
    }
    std::ofstream stream(path, std::ios::trunc);
    if (!stream.good()) {
        throw std::runtime_error("Failed to open output JSON: " + path.string());
    }
    stream << json.dump(2) << '\n';
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cerr << "usage: extract_localization <dialogue_preview.json> <bundle.json>\n";
        return 1;
    }

    try {
        writeJson(argv[2], urpg::localization::extractDialoguePreviewLocalizationBundle(readJson(argv[1])));
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 2;
    }
    return 0;
}
