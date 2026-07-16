#include "engine/core/security/sanitizer_fuzz_coverage.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Sanitizer and fuzz coverage records supported lanes blockers and parser boundaries",
          "[security][sanitizer][fuzz][pcq704]") {
    using namespace urpg::security;
    const std::vector<SanitizerLane> lanes = {
        {SanitizerKind::Address, "clang-linux", true, true, {}, {}},
        {SanitizerKind::UndefinedBehavior, "clang-linux", true, true, {}, {}},
        {SanitizerKind::Thread, "mingw-windows", false, false, "Runtime unavailable in active MinGW distribution.", "Repeated concurrency stress and race assertions."},
        {SanitizerKind::Memory, "mingw-windows", false, false, "MSan requires an instrumented Clang runtime.", "ASan plus initialized-field fuzz corpus."},
    };
    std::vector<FuzzTarget> targets;
    for (const auto boundary : {FuzzBoundary::Schema, FuzzBoundary::Archive,
                                FuzzBoundary::EventStream, FuzzBoundary::Compatibility}) {
        targets.push_back({std::string(fuzzBoundaryName(boundary)) + "-target", boundary, 1024,
                           [](const std::span<const uint8_t>) { return true; }});
    }
    std::vector<FuzzCorpusCase> corpus;
    for (const auto boundary : {FuzzBoundary::Schema, FuzzBoundary::Archive,
                                FuzzBoundary::EventStream, FuzzBoundary::Compatibility}) {
        corpus.push_back({std::string(fuzzBoundaryName(boundary)) + "-emptyish", boundary, {0, 0xff, '{', '}'}});
    }
    const auto result = SanitizerFuzzCoverage{}.evaluate(lanes, targets, corpus);
    REQUIRE(result.complete);
    REQUIRE(result.safe);
    REQUIRE(result.fuzz_executions == 4);
    REQUIRE(result.diagnostics.empty());
}

TEST_CASE("Sanitizer and fuzz coverage fails unsupported omissions and throwing targets",
          "[security][sanitizer][fuzz][pcq704]") {
    using namespace urpg::security;
    const auto result = SanitizerFuzzCoverage{}.evaluate(
        {{SanitizerKind::Address, "gcc", true, false, {}, {}}},
        {{"schema", FuzzBoundary::Schema, 2, [](const auto) -> bool { throw 7; }}},
        {{"too-large", FuzzBoundary::Schema, {1, 2, 3}}});
    REQUIRE_FALSE(result.complete);
    REQUIRE_FALSE(result.safe);
    REQUIRE_FALSE(result.diagnostics.empty());
}
