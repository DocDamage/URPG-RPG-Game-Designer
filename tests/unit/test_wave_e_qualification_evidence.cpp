#include "engine/core/export/version_compatibility_policy.h"
#include "engine/core/platform/platform_services.h"
#include "engine/core/reliability/fault_injection_suite.h"
#include "engine/core/reliability/stress_scenario_suite.h"
#include "engine/core/security/release_sbom.h"
#include "engine/core/security/release_security_audit.h"
#include "engine/core/security/sanitizer_fuzz_coverage.h"
#include "engine/core/tools/export_packager.h"
#include "engine/core/tools/export_packager_payload_builder.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <set>
#include <string>

namespace {

nlohmann::json loadWaveEQualification() {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) /
                            "content/readiness/wave_e_qualification_matrix.json",
                        std::ios::binary);
    REQUIRE(input.is_open());
    return nlohmann::json::parse(input);
}

std::set<std::string> asSet(const nlohmann::json& values) {
    return values.get<std::set<std::string>>();
}

} // namespace

TEST_CASE("Wave E reliability and security evidence matches every native authority",
          "[wave_e][qualification][reliability][security]") {
    const auto matrix = loadWaveEQualification();

    std::set<std::string> stress;
    for (const auto value : {urpg::reliability::StressScenario::RepeatedOpenClose,
                             urpg::reliability::StressScenario::RepeatedPlaytest,
                             urpg::reliability::StressScenario::RepeatedPackage,
                             urpg::reliability::StressScenario::LargeUndo,
                             urpg::reliability::StressScenario::AssetChurn,
                             urpg::reliability::StressScenario::DeviceChurn,
                             urpg::reliability::StressScenario::SaveCycles,
                             urpg::reliability::StressScenario::SuspendMinimize}) {
        stress.insert(urpg::reliability::stressScenarioName(value));
    }
    REQUIRE(asSet(matrix.at("stressScenarios")) == stress);

    std::set<std::string> faults;
    for (const auto value : {urpg::reliability::FaultInjectionPoint::DiskFull,
                             urpg::reliability::FaultInjectionPoint::PermissionLoss,
                             urpg::reliability::FaultInjectionPoint::InterruptedWrite,
                             urpg::reliability::FaultInjectionPoint::MalformedDocument,
                             urpg::reliability::FaultInjectionPoint::MissingAsset,
                             urpg::reliability::FaultInjectionPoint::BadArchive,
                             urpg::reliability::FaultInjectionPoint::FailedMigration,
                             urpg::reliability::FaultInjectionPoint::RendererDeviceLoss,
                             urpg::reliability::FaultInjectionPoint::ChildProcessFailure}) {
        faults.insert(urpg::reliability::faultInjectionPointName(value));
    }
    REQUIRE(asSet(matrix.at("faultClasses")) == faults);

    std::set<std::string> security;
    for (const auto value : {urpg::security::SecuritySurface::ExternalProcess,
                             urpg::security::SecuritySurface::ArchiveExtraction,
                             urpg::security::SecuritySurface::PathContainment,
                             urpg::security::SecuritySurface::PluginModTrust,
                             urpg::security::SecuritySurface::Secrets,
                             urpg::security::SecuritySurface::NetworkDefaults,
                             urpg::security::SecuritySurface::Logs,
                             urpg::security::SecuritySurface::SupportBundles}) {
        security.insert(urpg::security::securitySurfaceName(value));
    }
    REQUIRE(asSet(matrix.at("securitySurfaces")) == security);

    REQUIRE(asSet(matrix.at("sanitizerLanes")) ==
            std::set<std::string>{"address", "undefined_behavior", "thread", "memory"});
    REQUIRE(asSet(matrix.at("fuzzBoundaries")) ==
            std::set<std::string>{"schema", "archive", "event_stream", "compatibility"});
    REQUIRE(asSet(matrix.at("sbomComponentClasses")) ==
            std::set<std::string>{"binary", "tool", "packaged_asset", "optional_provider", "research_output"});
}

TEST_CASE("Wave E platform, compatibility, and package evidence is bounded and fail closed",
          "[wave_e][qualification][platform][package]") {
    const auto matrix = loadWaveEQualification();

    std::set<std::string> capabilities;
    for (const auto value : {urpg::platform::PlatformCapability::Lifecycle,
                             urpg::platform::PlatformCapability::Users,
                             urpg::platform::PlatformCapability::Storage,
                             urpg::platform::PlatformCapability::InputDevices,
                             urpg::platform::PlatformCapability::DisplayModes,
                             urpg::platform::PlatformCapability::Achievements,
                             urpg::platform::PlatformCapability::Presence,
                             urpg::platform::PlatformCapability::NetworkStatus,
                             urpg::platform::PlatformCapability::VirtualKeyboard,
                             urpg::platform::PlatformCapability::Locale,
                             urpg::platform::PlatformCapability::Clock,
                             urpg::platform::PlatformCapability::PowerSuspend,
                             urpg::platform::PlatformCapability::ErrorPresentation}) {
        capabilities.insert(urpg::platform::platformCapabilityName(value));
    }
    REQUIRE(asSet(matrix.at("platformCapabilities")) == capabilities);

    std::set<std::string> compatibility;
    for (const auto value : {urpg::exporting::CompatibilitySurface::ProjectSchema,
                             urpg::exporting::CompatibilitySurface::SaveData,
                             urpg::exporting::CompatibilitySurface::Runtime,
                             urpg::exporting::CompatibilitySurface::PluginMod,
                             urpg::exporting::CompatibilitySurface::Package,
                             urpg::exporting::CompatibilitySurface::UpdateChannel}) {
        compatibility.insert(urpg::exporting::compatibilitySurfaceName(value));
    }
    REQUIRE(asSet(matrix.at("compatibilityDomains")) == compatibility);
    REQUIRE(matrix.at("releaseTargets").at("web") == "unsupported");
    REQUIRE(matrix.at("promotedAssetSelection").at("default") == "none");

    urpg::tools::ExportConfig config{};
    config.target = urpg::tools::ExportTarget::Windows_x64;
    config.enableAutoAssetDiscovery = false;
    config.promotedAssetBundleIds = {"../escape"};
    const auto payloads = urpg::tools::export_packager_detail::buildBundlePayloads(config);
    REQUIRE_FALSE(payloads.errors.empty());
    REQUIRE(std::ranges::none_of(payloads.payloads, [](const auto& payload) {
        return payload.path.rfind("imports/manifests/asset_bundles/", 0) == 0;
    }));
}
