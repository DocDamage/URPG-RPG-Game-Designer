#include "engine/core/export/governed_package_manifest.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

urpg::exporting::GovernedPackageInput packageInput() {
    const std::string a(64, 'a');
    const std::string b(64, 'b');
    return {"demo", "1.2.3",
            {{"content/maps/field.json", b, {"map:field", "asset:tiles"}, "CC0-1.0", "notice-assets"},
             {"urpg_runtime.exe", a, {"runtime"}, "MIT", "notice-engine"}},
            {"storage", "input_devices", "locale"},
            {{"compiler", "gcc-15.2.0"}, {"preset", "release"}, {"source_commit", "abc123"}}};
}

} // namespace

TEST_CASE("Governed package manifests are deterministic across equivalent input order", "[export][manifest][pcq755]") {
    auto input = packageInput();
    auto reversed = input;
    std::reverse(reversed.files.begin(), reversed.files.end());
    std::reverse(reversed.platform_capabilities.begin(), reversed.platform_capabilities.end());
    const urpg::exporting::GovernedPackageManifestBuilder builder;
    const auto first = builder.build(input);
    const auto second = builder.build(reversed);
    REQUIRE(first.valid);
    REQUIRE(first.canonical_json == second.canonical_json);
    REQUIRE(builder.explainDifference(first.manifest, second.manifest).empty());

    auto changed = second.manifest;
    changed["reproducibility_inputs"]["compiler"] = "clang-20";
    REQUIRE(builder.explainDifference(first.manifest, changed) == std::vector<std::string>{"reproducibility_inputs"});
}

TEST_CASE("Governed package manifests reject unsafe incomplete content", "[export][manifest][pcq755]") {
    auto input = packageInput();
    input.files[0].path = "../outside.json";
    input.files[0].sha256 = "bad";
    input.files[0].license_id.clear();
    input.files[0].notice_id.clear();
    const auto result = urpg::exporting::GovernedPackageManifestBuilder{}.build(std::move(input));
    REQUIRE_FALSE(result.valid);
    REQUIRE(result.diagnostics.size() == 4);
}
