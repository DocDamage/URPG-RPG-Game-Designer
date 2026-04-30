#pragma once

#include "editor/compat/compat_report_panel.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using urpg::compat::PluginManager;
using urpg::compat::DataManager;

struct FixtureSpec {
    std::string pluginName;
    std::string happyPathCommand;
};

struct CapturedError {
    std::string pluginName;
    std::string commandName;
    std::string error;
};

[[maybe_unused]] std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 &&
        sourceRoot.front() == '"' &&
        sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

[[maybe_unused]] std::filesystem::path fixtureDir() {
    const auto sourceRoot = sourceRootFromMacro();
    if (!sourceRoot.empty()) {
        return sourceRoot / "tests" / "compat" / "fixtures" / "plugins";
    }
    return std::filesystem::path("tests") / "compat" / "fixtures" / "plugins";
}

[[maybe_unused]] std::filesystem::path fixturePath(const std::string& pluginName) {
    return fixtureDir() / (pluginName + ".json");
}

[[maybe_unused]] const std::vector<FixtureSpec>& fixtureSpecs() {
    static const std::vector<FixtureSpec> specs = {
        {"VisuStella_CoreEngine_MZ", "boot"},
        {"VisuStella_MainMenuCore_MZ", "openMenu"},
        {"VisuStella_OptionsCore_MZ", "openOptions"},
        {"CGMZ_MenuCommandWindow", "refresh"},
        {"CGMZ_Encyclopedia", "openCategory"},
        {"EliMZ_Book", "openBook"},
        {"MOG_CharacterMotion_MZ", "startMotion"},
        {"MOG_BattleHud_MZ", "showHud"},
        {"Galv_QuestLog_MZ", "openQuestLog"},
        {"AltMenuScreen_MZ", "applyLayout"},
    };
    return specs;
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

[[maybe_unused]] std::filesystem::path uniqueTempFixturePath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::filesystem::path temp = std::filesystem::temp_directory_path();
    return temp / (std::string(stem) + "_" + std::to_string(ticks) + ".json");
}

[[maybe_unused]] void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

} // namespace
