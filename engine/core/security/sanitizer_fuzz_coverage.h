#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace urpg::security {

enum class SanitizerKind : uint8_t { Address, UndefinedBehavior, Thread, Memory };
enum class FuzzBoundary : uint8_t { Schema, Archive, EventStream, Compatibility };

struct SanitizerLane {
    SanitizerKind sanitizer = SanitizerKind::Address;
    std::string toolchain;
    bool supported = false;
    bool enabled = false;
    std::string blocker;
    std::string alternative_coverage;
};

struct FuzzTarget {
    std::string id;
    FuzzBoundary boundary = FuzzBoundary::Schema;
    size_t maximum_input_bytes = 0;
    std::function<bool(std::span<const uint8_t>)> consume;
};

struct FuzzCorpusCase {
    std::string id;
    FuzzBoundary boundary = FuzzBoundary::Schema;
    std::vector<uint8_t> bytes;
};

struct SanitizerFuzzResult {
    bool complete = false;
    bool safe = false;
    uint32_t fuzz_executions = 0;
    std::vector<std::string> diagnostics;
};

class SanitizerFuzzCoverage {
public:
    SanitizerFuzzResult evaluate(const std::vector<SanitizerLane>& sanitizer_lanes,
                                 const std::vector<FuzzTarget>& fuzz_targets,
                                 const std::vector<FuzzCorpusCase>& corpus) const;
};

const char* sanitizerKindName(SanitizerKind kind);
const char* fuzzBoundaryName(FuzzBoundary boundary);

} // namespace urpg::security
