#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::perf {

enum class BenchmarkProjectScale : uint8_t { Tiny, Medium, Large };
enum class BenchmarkHardwareClass : uint8_t { MinimumDesktop, RecommendedDesktop, HighEndDesktop };
enum class BenchmarkMetric : uint8_t {
    Startup,
    ProjectOpen,
    Save,
    Search,
    MapEdit,
    PlaytestLaunch,
    FramePacing,
    Package,
    Memory
};

struct BenchmarkProjectFixture {
    std::string id;
    BenchmarkProjectScale scale = BenchmarkProjectScale::Tiny;
    uint32_t maps = 0;
    uint32_t events = 0;
    uint32_t assets = 0;
    uint32_t database_records = 0;
    uint64_t content_bytes = 0;
};

struct BenchmarkHardwareProfile {
    std::string id;
    BenchmarkHardwareClass hardware_class = BenchmarkHardwareClass::MinimumDesktop;
    uint32_t logical_cores = 0;
    uint64_t memory_bytes = 0;
    std::string graphics_class;
};

struct BenchmarkMeasurement {
    std::string fixture_id;
    std::string hardware_id;
    BenchmarkMetric metric = BenchmarkMetric::Startup;
    uint64_t measured_value = 0;
    uint64_t regression_threshold = 0;
    uint32_t sample_count = 0;
};

struct ProductBenchmarkResult {
    bool complete = false;
    bool within_thresholds = false;
    std::string baseline_version;
    std::vector<std::string> diagnostics;
    nlohmann::json report;
};

class ProductBenchmarkSuite {
public:
    ProductBenchmarkResult evaluate(std::string baseline_version,
                                    std::vector<BenchmarkProjectFixture> fixtures,
                                    std::vector<BenchmarkHardwareProfile> hardware,
                                    std::vector<BenchmarkMeasurement> measurements) const;

    static std::vector<BenchmarkProjectFixture> representativeFixtures();
    static std::vector<BenchmarkHardwareProfile> targetHardwareClasses();
};

const char* benchmarkProjectScaleName(BenchmarkProjectScale scale);
const char* benchmarkHardwareClassName(BenchmarkHardwareClass hardware_class);
const char* benchmarkMetricName(BenchmarkMetric metric);
const char* benchmarkMetricUnit(BenchmarkMetric metric);

} // namespace urpg::perf
