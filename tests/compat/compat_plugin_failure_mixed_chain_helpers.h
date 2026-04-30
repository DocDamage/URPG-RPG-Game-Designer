#pragma once

#include "runtimes/compat_js/plugin_manager.h"

#include <cstddef>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace urpg::tests::compat_mixed_chain {

struct FixtureSpec {
    std::string pluginName;
    std::string happyPathCommand;
};

using DiagnosticRows = std::vector<nlohmann::json>;

std::filesystem::path fixturePath(const std::string& pluginName);
const std::vector<FixtureSpec>& fixtureSpecs();
DiagnosticRows parseJsonl(std::string_view jsonl);
std::filesystem::path uniqueTempFixturePath(std::string_view stem);
void writeTextFile(const std::filesystem::path& path, std::string_view contents);

void verifyOperationCounts(const DiagnosticRows& diagnostics, size_t dependencyProbeCount);
void verifyDependencyGateDiagnosticRows(
    const DiagnosticRows& diagnostics,
    const std::vector<FixtureSpec>& specs
);
void verifyRuntimeDiagnosticRows(const DiagnosticRows& diagnostics);
void verifyLoadPluginDiagnosticRows(
    const DiagnosticRows& diagnostics,
    const std::filesystem::path& missingFixture,
    const std::filesystem::path& emptyNameFixture
);
void verifyCompatReportModelAndPanel(
    urpg::compat::PluginManager& pm,
    const std::string& diagnosticsJsonl,
    const std::vector<FixtureSpec>& specs,
    const std::filesystem::path& missingFixture,
    const std::filesystem::path& malformedFixture,
    const std::filesystem::path& emptyNameFixture
);

void loadCuratedFixturePluginsAndVerifyHappyPaths(urpg::compat::PluginManager& pm);
void registerDependencyGateProbesAndVerifyBlocking(urpg::compat::PluginManager& pm);
std::filesystem::path createAndExerciseWeeklyLifecycleFixture(urpg::compat::PluginManager& pm);

} // namespace urpg::tests::compat_mixed_chain
