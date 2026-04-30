#pragma once

// PluginManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Plugin Command Registry + Execution

#include "runtimes/compat_js/plugin_manager.h"
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

using namespace urpg::compat;

namespace {

[[maybe_unused]] std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 && sourceRoot.front() == '"' && sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

[[maybe_unused]] std::filesystem::path fixturePath(const std::string& pluginName) {
    std::vector<std::filesystem::path> candidateRoots;
    const auto sourceRoot = sourceRootFromMacro();
    if (!sourceRoot.empty()) {
        candidateRoots.push_back(sourceRoot / "tests" / "compat" / "fixtures" / "plugins");
    }
    candidateRoots.push_back(std::filesystem::current_path() / "tests" / "compat" / "fixtures" / "plugins");
    candidateRoots.push_back(std::filesystem::current_path().parent_path() / "tests" / "compat" / "fixtures" /
                             "plugins");
    candidateRoots.push_back(std::filesystem::path("tests") / "compat" / "fixtures" / "plugins");

    for (const auto& root : candidateRoots) {
        const auto candidate = root / (pluginName + ".json");
        if (std::filesystem::exists(candidate)) {
            return candidate;
        }
    }

    return candidateRoots.front() / (pluginName + ".json");
}

[[maybe_unused]] std::filesystem::path uniqueTempDirectoryPath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (std::string(stem) + "_" + std::to_string(ticks));
}

[[maybe_unused]] PluginInfo makePluginInfo(std::string name,
                                           std::string version = "1.0",
                                           std::string author = {},
                                           std::string description = {}) {
    PluginInfo info;
    info.name = std::move(name);
    info.version = std::move(version);
    info.author = std::move(author);
    info.description = std::move(description);
    return info;
}

[[maybe_unused]] void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

[[maybe_unused]] std::vector<nlohmann::json> parseJsonl(std::string_view jsonl) {
    std::vector<nlohmann::json> rows;
    if (jsonl.empty()) {
        return rows;
    }

    std::istringstream stream{std::string(jsonl)};
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }
        auto row = nlohmann::json::parse(line, nullptr, false);
        if (!row.is_discarded() && row.is_object()) {
            rows.push_back(std::move(row));
        }
    }
    return rows;
}

} // namespace
