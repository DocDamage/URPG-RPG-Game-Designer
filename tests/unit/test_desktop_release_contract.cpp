#include "engine/core/export/desktop_release_contract.h"

#include <catch2/catch_test_macros.hpp>

namespace {

urpg::exporting::DesktopInstallPolicy qualifiedWindowsPolicy() {
    using urpg::exporting::DesktopPackageRole;
    const std::string hash(64, 'a');
    return {"windows-x64",
            {{DesktopPackageRole::Executable, "urpg_runtime.exe", hash},
             {DesktopPackageRole::RuntimeLibrary, "bin/SDL2.dll", hash},
             {DesktopPackageRole::Content, "content/project.json", hash},
             {DesktopPackageRole::Licenses, "licenses/THIRD_PARTY.md", hash},
             {DesktopPackageRole::Notices, "NOTICE.txt", hash}},
            true, true, true, "user_data/saves", "portable/saves", true, true};
}

std::vector<urpg::exporting::TargetSupportRecord> truthfulTargets() {
    using namespace urpg::exporting;
    return {{ReleaseTarget::Windows, TargetSupportStatus::Qualified, TargetSupportStatus::Qualified,
             TargetSupportStatus::Qualified, TargetSupportStatus::Qualified, "clean-vm-evidence"},
            {ReleaseTarget::MacOS, TargetSupportStatus::Unavailable, TargetSupportStatus::Unavailable,
             TargetSupportStatus::Unavailable, TargetSupportStatus::Unavailable, {}},
            {ReleaseTarget::Linux, TargetSupportStatus::Experimental, TargetSupportStatus::Experimental,
             TargetSupportStatus::Experimental, TargetSupportStatus::Experimental, {}},
            {ReleaseTarget::Web, TargetSupportStatus::Unsupported, TargetSupportStatus::Unsupported,
             TargetSupportStatus::Unsupported, TargetSupportStatus::Unsupported, {}}};
}

} // namespace

TEST_CASE("Desktop release contract governs layout install portable repair and truthful targets",
          "[export][desktop][pcq752][pcq754]") {
    const auto result = urpg::exporting::DesktopReleaseContract{}.evaluate(
        {qualifiedWindowsPolicy()}, truthfulTargets());
    REQUIRE(result.complete);
    REQUIRE(result.release_ready);
    REQUIRE(result.diagnostics.empty());
}

TEST_CASE("Desktop release contract rejects traversal lifecycle gaps and unsupported web claims",
          "[export][desktop][pcq752][pcq754]") {
    auto policy = qualifiedWindowsPolicy();
    policy.entries[0].relative_path = "../runtime.exe";
    policy.repair_supported = false;
    auto targets = truthfulTargets();
    targets.back().ui_status = urpg::exporting::TargetSupportStatus::Experimental;
    const auto result = urpg::exporting::DesktopReleaseContract{}.evaluate({policy}, targets);
    REQUIRE(result.complete);
    REQUIRE_FALSE(result.release_ready);
    REQUIRE(result.diagnostics.size() == 4);
}
