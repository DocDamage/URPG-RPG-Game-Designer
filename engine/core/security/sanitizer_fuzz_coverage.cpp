#include "engine/core/security/sanitizer_fuzz_coverage.h"

#include <array>
#include <exception>
#include <set>

namespace urpg::security {
namespace {

constexpr std::array<SanitizerKind, 4> kSanitizers = {
    SanitizerKind::Address, SanitizerKind::UndefinedBehavior, SanitizerKind::Thread, SanitizerKind::Memory};
constexpr std::array<FuzzBoundary, 4> kBoundaries = {
    FuzzBoundary::Schema, FuzzBoundary::Archive, FuzzBoundary::EventStream, FuzzBoundary::Compatibility};

} // namespace

const char* sanitizerKindName(const SanitizerKind kind) {
    switch (kind) {
    case SanitizerKind::Address: return "address";
    case SanitizerKind::UndefinedBehavior: return "undefined_behavior";
    case SanitizerKind::Thread: return "thread";
    case SanitizerKind::Memory: return "memory";
    }
    return "unknown";
}

const char* fuzzBoundaryName(const FuzzBoundary boundary) {
    switch (boundary) {
    case FuzzBoundary::Schema: return "schema";
    case FuzzBoundary::Archive: return "archive";
    case FuzzBoundary::EventStream: return "event_stream";
    case FuzzBoundary::Compatibility: return "compatibility";
    }
    return "unknown";
}

SanitizerFuzzResult SanitizerFuzzCoverage::evaluate(const std::vector<SanitizerLane>& sanitizer_lanes,
                                                    const std::vector<FuzzTarget>& fuzz_targets,
                                                    const std::vector<FuzzCorpusCase>& corpus) const {
    SanitizerFuzzResult result;
    std::set<SanitizerKind> sanitizerCoverage;
    for (const auto& lane : sanitizer_lanes) {
        const auto name = std::string(sanitizerKindName(lane.sanitizer));
        if (lane.toolchain.empty() || !sanitizerCoverage.insert(lane.sanitizer).second) {
            result.diagnostics.push_back("sanitizer_lane_invalid_or_duplicate:" + name);
        } else if (lane.supported && !lane.enabled) {
            result.diagnostics.push_back("sanitizer_supported_but_disabled:" + name);
        } else if (!lane.supported && (lane.blocker.empty() || lane.alternative_coverage.empty())) {
            result.diagnostics.push_back("sanitizer_blocker_or_alternative_missing:" + name);
        }
    }
    for (const auto kind : kSanitizers) {
        if (!sanitizerCoverage.contains(kind)) result.diagnostics.push_back(std::string("sanitizer_lane_missing:") + sanitizerKindName(kind));
    }

    std::map<FuzzBoundary, const FuzzTarget*> targets;
    std::set<std::string> targetIds;
    for (const auto& target : fuzz_targets) {
        if (target.id.empty() || target.maximum_input_bytes == 0 || !target.consume ||
            !targetIds.insert(target.id).second || targets.contains(target.boundary)) {
            result.diagnostics.push_back("fuzz_target_invalid_or_duplicate:" + target.id);
            continue;
        }
        targets[target.boundary] = &target;
    }
    std::set<FuzzBoundary> corpusCoverage;
    std::set<std::string> corpusIds;
    for (const auto& testCase : corpus) {
        const auto boundaryName = std::string(fuzzBoundaryName(testCase.boundary));
        const auto target = targets.find(testCase.boundary);
        if (testCase.id.empty() || !corpusIds.insert(testCase.id).second || testCase.bytes.empty() || target == targets.end()) {
            result.diagnostics.push_back("fuzz_case_invalid_or_unrouted:" + testCase.id);
            continue;
        }
        corpusCoverage.insert(testCase.boundary);
        if (testCase.bytes.size() > target->second->maximum_input_bytes) {
            result.diagnostics.push_back("fuzz_case_exceeds_bound:" + testCase.id);
            continue;
        }
        try {
            if (!target->second->consume(testCase.bytes)) result.diagnostics.push_back("fuzz_case_unhandled:" + testCase.id);
        } catch (const std::exception& exception) {
            result.diagnostics.push_back("fuzz_target_threw:" + testCase.id + ":" + exception.what());
        } catch (...) {
            result.diagnostics.push_back("fuzz_target_threw:" + testCase.id);
        }
        ++result.fuzz_executions;
    }
    for (const auto boundary : kBoundaries) {
        if (!targets.contains(boundary)) result.diagnostics.push_back(std::string("fuzz_target_missing:") + fuzzBoundaryName(boundary));
        if (!corpusCoverage.contains(boundary)) result.diagnostics.push_back(std::string("fuzz_corpus_missing:") + fuzzBoundaryName(boundary));
    }
    result.complete = sanitizerCoverage.size() == kSanitizers.size() && targets.size() == kBoundaries.size() &&
                      corpusCoverage.size() == kBoundaries.size();
    result.safe = result.complete && result.diagnostics.empty();
    return result;
}

} // namespace urpg::security
