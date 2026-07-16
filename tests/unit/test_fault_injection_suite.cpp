#include "engine/core/reliability/fault_injection_suite.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::reliability::FaultInjectionSuite completeSuite() {
    using urpg::reliability::FaultInjectionPoint;
    urpg::reliability::FaultInjectionSuite suite;
    for (const auto point : {FaultInjectionPoint::DiskFull, FaultInjectionPoint::PermissionLoss,
                             FaultInjectionPoint::InterruptedWrite, FaultInjectionPoint::MalformedDocument,
                             FaultInjectionPoint::MissingAsset, FaultInjectionPoint::BadArchive,
                             FaultInjectionPoint::FailedMigration, FaultInjectionPoint::RendererDeviceLoss,
                             FaultInjectionPoint::ChildProcessFailure}) {
        REQUIRE(suite.registerHandler(point, [](const FaultInjectionPoint injected) {
            return urpg::reliability::FaultInjectionOutcome{
                injected, true, true, true, true, true, 12, 100,
                std::string("Injected ") + urpg::reliability::faultInjectionPointName(injected),
                "Preserve authority, discard partial state, and expose retry or recovery."};
        }));
    }
    return suite;
}

} // namespace

TEST_CASE("Fault injection matrix requires bounded diagnostics and recovery for every adverse class",
          "[reliability][fault_injection][pcq703]") {
    const auto result = completeSuite().run();
    REQUIRE(result.complete);
    REQUIRE(result.safe);
    REQUIRE(result.outcomes.size() == 9);
    REQUIRE(result.diagnostics.empty());
    for (const auto& outcome : result.outcomes) {
        REQUIRE(outcome.project_data_preserved);
        REQUIRE(outcome.partial_state_contained);
        REQUIRE(outcome.elapsed_ms <= outcome.budget_ms);
        REQUIRE_FALSE(outcome.diagnostic.empty());
        REQUIRE_FALSE(outcome.recovery_path.empty());
    }
}

TEST_CASE("Fault injection matrix fails missing throwing and unsafe handlers",
          "[reliability][fault_injection][pcq703]") {
    auto suite = completeSuite();
    REQUIRE_FALSE(suite.registerHandler(urpg::reliability::FaultInjectionPoint::DiskFull, {}));

    urpg::reliability::FaultInjectionSuite incomplete;
    REQUIRE(incomplete.registerHandler(urpg::reliability::FaultInjectionPoint::DiskFull,
        [](const auto point) {
            return urpg::reliability::FaultInjectionOutcome{point, true, false, false, false, false, 200, 100, {}, {}};
        }));
    const auto result = incomplete.run();
    REQUIRE_FALSE(result.complete);
    REQUIRE_FALSE(result.safe);
    REQUIRE(result.diagnostics.size() == 14);
}
