#include "engine/core/perf/product_benchmark_suite.h"

#include <algorithm>
#include <array>
#include <optional>
#include <set>
#include <tuple>
#include <utility>

namespace urpg::perf {
namespace {

constexpr std::array<BenchmarkMetric, 9> kRequiredMetrics = {
    BenchmarkMetric::Startup, BenchmarkMetric::ProjectOpen, BenchmarkMetric::Save,
    BenchmarkMetric::Search, BenchmarkMetric::MapEdit, BenchmarkMetric::PlaytestLaunch,
    BenchmarkMetric::FramePacing, BenchmarkMetric::Package, BenchmarkMetric::Memory};

bool validVersion(const std::string& version) {
    return version.starts_with("pcq700.v") && version.size() > 8 &&
           std::all_of(version.begin() + 8, version.end(), [](const unsigned char value) { return value >= '0' && value <= '9'; });
}

std::optional<BenchmarkProjectScale> projectScale(const std::string_view value) {
    if (value == "tiny") return BenchmarkProjectScale::Tiny;
    if (value == "medium") return BenchmarkProjectScale::Medium;
    if (value == "large") return BenchmarkProjectScale::Large;
    return std::nullopt;
}

std::optional<BenchmarkHardwareClass> hardwareClass(const std::string_view value) {
    if (value == "minimum_desktop") return BenchmarkHardwareClass::MinimumDesktop;
    if (value == "recommended_desktop") return BenchmarkHardwareClass::RecommendedDesktop;
    if (value == "high_end_desktop") return BenchmarkHardwareClass::HighEndDesktop;
    return std::nullopt;
}

} // namespace

const char* benchmarkProjectScaleName(const BenchmarkProjectScale scale) {
    switch (scale) {
    case BenchmarkProjectScale::Tiny: return "tiny";
    case BenchmarkProjectScale::Medium: return "medium";
    case BenchmarkProjectScale::Large: return "large";
    }
    return "unknown";
}

const char* benchmarkHardwareClassName(const BenchmarkHardwareClass hardware_class) {
    switch (hardware_class) {
    case BenchmarkHardwareClass::MinimumDesktop: return "minimum_desktop";
    case BenchmarkHardwareClass::RecommendedDesktop: return "recommended_desktop";
    case BenchmarkHardwareClass::HighEndDesktop: return "high_end_desktop";
    }
    return "unknown";
}

const char* benchmarkMetricName(const BenchmarkMetric metric) {
    switch (metric) {
    case BenchmarkMetric::Startup: return "startup";
    case BenchmarkMetric::ProjectOpen: return "project_open";
    case BenchmarkMetric::Save: return "save";
    case BenchmarkMetric::Search: return "search";
    case BenchmarkMetric::MapEdit: return "map_edit";
    case BenchmarkMetric::PlaytestLaunch: return "playtest_launch";
    case BenchmarkMetric::FramePacing: return "frame_pacing";
    case BenchmarkMetric::Package: return "package";
    case BenchmarkMetric::Memory: return "memory";
    }
    return "unknown";
}

const char* benchmarkMetricUnit(const BenchmarkMetric metric) {
    return metric == BenchmarkMetric::Memory ? "bytes" : "microseconds";
}

std::vector<BenchmarkProjectFixture> ProductBenchmarkSuite::representativeFixtures() {
    return {{"tiny_journey", BenchmarkProjectScale::Tiny, 3, 24, 80, 120, 32ULL * 1024 * 1024},
            {"medium_journey", BenchmarkProjectScale::Medium, 30, 600, 2400, 1800, 1024ULL * 1024 * 1024},
            {"large_journey", BenchmarkProjectScale::Large, 120, 6000, 18000, 12000, 8ULL * 1024 * 1024 * 1024}};
}

std::vector<BenchmarkHardwareProfile> ProductBenchmarkSuite::targetHardwareClasses() {
    return {{"minimum_win_x64", BenchmarkHardwareClass::MinimumDesktop, 4, 8ULL * 1024 * 1024 * 1024, "integrated"},
            {"recommended_win_x64", BenchmarkHardwareClass::RecommendedDesktop, 8, 16ULL * 1024 * 1024 * 1024, "entry_discrete"},
            {"high_end_win_x64", BenchmarkHardwareClass::HighEndDesktop, 16, 32ULL * 1024 * 1024 * 1024, "modern_discrete"}};
}

ProductBenchmarkPlan ProductBenchmarkSuite::parsePlan(const nlohmann::json& value) {
    ProductBenchmarkPlan plan;
    if (!value.is_object() || value.value("schema", "") != "urpg.product_benchmark_plan.v1") {
        plan.diagnostics.push_back("benchmark_plan_schema_invalid");
        return plan;
    }
    plan.baseline_version = value.value("baseline_version", "");
    plan.threshold_status = value.value("threshold_status", "");
    if (!validVersion(plan.baseline_version)) plan.diagnostics.push_back("benchmark_version_invalid");
    if (plan.threshold_status.empty()) plan.diagnostics.push_back("benchmark_threshold_status_missing");

    const auto fixtureRows = value.value("fixtures", nlohmann::json::array());
    if (!fixtureRows.is_array()) {
        plan.diagnostics.push_back("benchmark_fixtures_invalid");
    } else {
        for (const auto& row : fixtureRows) {
            if (!row.is_object()) {
                plan.diagnostics.push_back("benchmark_fixture_plan_row_invalid");
                continue;
            }
            const auto scale = projectScale(row.value("scale", ""));
            if (!scale) {
                plan.diagnostics.push_back("benchmark_fixture_plan_row_invalid");
                continue;
            }
            plan.fixtures.push_back({row.value("id", ""), *scale, row.value("maps", 0U),
                                     row.value("events", 0U), row.value("assets", 0U),
                                     row.value("database_records", 0U), row.value("content_bytes", 0ULL)});
        }
    }
    const auto hardwareRows = value.value("hardware", nlohmann::json::array());
    if (!hardwareRows.is_array()) {
        plan.diagnostics.push_back("benchmark_hardware_plan_invalid");
    } else {
        for (const auto& row : hardwareRows) {
            if (!row.is_object()) {
                plan.diagnostics.push_back("benchmark_hardware_plan_row_invalid");
                continue;
            }
            const auto hardware_class = hardwareClass(row.value("class", ""));
            if (!hardware_class) {
                plan.diagnostics.push_back("benchmark_hardware_plan_row_invalid");
                continue;
            }
            plan.hardware.push_back({row.value("id", ""), *hardware_class, row.value("logical_cores", 0U),
                                     row.value("memory_bytes", 0ULL), row.value("graphics_class", "")});
        }
    }

    const auto thresholdRows = value.value("thresholds", nlohmann::json::object());
    if (!thresholdRows.is_object()) {
        plan.diagnostics.push_back("benchmark_thresholds_invalid");
    } else {
        for (const auto& profile : plan.hardware) {
            if (!thresholdRows.contains(profile.id) || !thresholdRows[profile.id].is_object()) {
                plan.diagnostics.push_back("benchmark_hardware_threshold_missing:" + profile.id);
                continue;
            }
            const auto& profileThresholds = thresholdRows[profile.id];
            for (const auto metric : kRequiredMetrics) {
                const auto metricName = benchmarkMetricName(metric);
                if (!profileThresholds.contains(metricName) || !profileThresholds[metricName].is_object()) {
                    plan.diagnostics.push_back("benchmark_metric_threshold_missing:" + profile.id + ":" + metricName);
                    continue;
                }
                for (const auto& fixture : plan.fixtures) {
                    const auto scaleName = benchmarkProjectScaleName(fixture.scale);
                    const auto& metricThresholds = profileThresholds[metricName];
                    const auto threshold = metricThresholds.value(scaleName, 0ULL);
                    if (threshold == 0) {
                        plan.diagnostics.push_back("benchmark_scale_threshold_missing:" + profile.id + ":" +
                                                   metricName + ":" + scaleName);
                        continue;
                    }
                    plan.thresholds.push_back({fixture.id, profile.id, metric, threshold});
                }
            }
        }
    }

    const auto structural = ProductBenchmarkSuite{}.evaluate(plan.baseline_version, plan.fixtures, plan.hardware, {});
    for (const auto& diagnostic : structural.diagnostics) {
        if (!diagnostic.starts_with("benchmark_measurement_missing:")) plan.diagnostics.push_back(diagnostic);
    }
    std::sort(plan.diagnostics.begin(), plan.diagnostics.end());
    plan.diagnostics.erase(std::unique(plan.diagnostics.begin(), plan.diagnostics.end()), plan.diagnostics.end());
    plan.valid = plan.diagnostics.empty() && plan.thresholds.size() ==
                     plan.fixtures.size() * plan.hardware.size() * kRequiredMetrics.size();
    if (!plan.valid && plan.diagnostics.empty()) plan.diagnostics.push_back("benchmark_threshold_matrix_incomplete");
    return plan;
}

ProductBenchmarkResult ProductBenchmarkSuite::evaluate(const ProductBenchmarkPlan& plan,
                                                        std::vector<BenchmarkMeasurement> measurements) const {
    if (!plan.valid) {
        ProductBenchmarkResult result;
        result.baseline_version = plan.baseline_version;
        result.diagnostics = plan.diagnostics;
        result.report = {{"schema", "urpg.product_benchmark.v1"}, {"baseline_version", plan.baseline_version},
                         {"complete", false}, {"within_thresholds", false},
                         {"threshold_status", plan.threshold_status}, {"measurements", nlohmann::json::array()},
                         {"diagnostics", result.diagnostics}};
        return result;
    }
    for (auto& measurement : measurements) {
        const auto threshold = std::find_if(plan.thresholds.begin(), plan.thresholds.end(), [&](const auto& row) {
            return row.fixture_id == measurement.fixture_id && row.hardware_id == measurement.hardware_id &&
                   row.metric == measurement.metric;
        });
        measurement.regression_threshold = threshold == plan.thresholds.end() ? 0 : threshold->regression_threshold;
    }
    auto result = evaluate(plan.baseline_version, plan.fixtures, plan.hardware, std::move(measurements));
    result.report["threshold_status"] = plan.threshold_status;
    return result;
}

ProductBenchmarkResult ProductBenchmarkSuite::evaluate(std::string baseline_version,
                                                        std::vector<BenchmarkProjectFixture> fixtures,
                                                        std::vector<BenchmarkHardwareProfile> hardware,
                                                        std::vector<BenchmarkMeasurement> measurements) const {
    ProductBenchmarkResult result;
    result.baseline_version = std::move(baseline_version);
    if (!validVersion(result.baseline_version)) result.diagnostics.push_back("benchmark_version_invalid");

    std::set<std::string> fixture_ids;
    std::set<BenchmarkProjectScale> scales;
    for (const auto& fixture : fixtures) {
        if (fixture.id.empty() || !fixture_ids.insert(fixture.id).second || fixture.maps == 0 || fixture.events == 0 ||
            fixture.assets == 0 || fixture.database_records == 0 || fixture.content_bytes == 0) {
            result.diagnostics.push_back("benchmark_fixture_invalid:" + fixture.id);
        }
        scales.insert(fixture.scale);
    }
    for (const auto scale : {BenchmarkProjectScale::Tiny, BenchmarkProjectScale::Medium, BenchmarkProjectScale::Large}) {
        if (!scales.contains(scale)) result.diagnostics.push_back(std::string("benchmark_scale_missing:") + benchmarkProjectScaleName(scale));
    }

    std::set<std::string> hardware_ids;
    std::set<BenchmarkHardwareClass> classes;
    for (const auto& profile : hardware) {
        if (profile.id.empty() || !hardware_ids.insert(profile.id).second || profile.logical_cores == 0 ||
            profile.memory_bytes == 0 || profile.graphics_class.empty()) {
            result.diagnostics.push_back("benchmark_hardware_invalid:" + profile.id);
        }
        classes.insert(profile.hardware_class);
    }
    for (const auto hardware_class : {BenchmarkHardwareClass::MinimumDesktop,
                                      BenchmarkHardwareClass::RecommendedDesktop,
                                      BenchmarkHardwareClass::HighEndDesktop}) {
        if (!classes.contains(hardware_class)) {
            result.diagnostics.push_back(std::string("benchmark_hardware_class_missing:") +
                                         benchmarkHardwareClassName(hardware_class));
        }
    }

    std::stable_sort(measurements.begin(), measurements.end(), [](const auto& left, const auto& right) {
        return std::tie(left.fixture_id, left.hardware_id, left.metric) <
               std::tie(right.fixture_id, right.hardware_id, right.metric);
    });
    std::set<std::tuple<std::string, std::string, BenchmarkMetric>> measurement_keys;
    nlohmann::json rows = nlohmann::json::array();
    bool thresholdsPass = true;
    for (const auto& measurement : measurements) {
        const auto key = std::make_tuple(measurement.fixture_id, measurement.hardware_id, measurement.metric);
        const bool known = fixture_ids.contains(measurement.fixture_id) && hardware_ids.contains(measurement.hardware_id);
        const bool valid = known && measurement_keys.insert(key).second && measurement.measured_value > 0 &&
                           measurement.regression_threshold > 0 && measurement.sample_count >= 3;
        if (!valid) result.diagnostics.push_back("benchmark_measurement_invalid:" + measurement.fixture_id + ":" +
                                                 measurement.hardware_id + ":" + benchmarkMetricName(measurement.metric));
        const bool passes = valid && measurement.measured_value <= measurement.regression_threshold;
        thresholdsPass = thresholdsPass && passes;
        rows.push_back({{"fixture_id", measurement.fixture_id}, {"hardware_id", measurement.hardware_id},
                        {"metric", benchmarkMetricName(measurement.metric)},
                        {"unit", benchmarkMetricUnit(measurement.metric)},
                        {"measured_value", measurement.measured_value},
                        {"regression_threshold", measurement.regression_threshold},
                        {"sample_count", measurement.sample_count}, {"passes", passes}});
    }
    for (const auto& fixture : fixtures) {
        for (const auto& profile : hardware) {
            for (const auto metric : kRequiredMetrics) {
                if (!measurement_keys.contains({fixture.id, profile.id, metric})) {
                    result.diagnostics.push_back("benchmark_measurement_missing:" + fixture.id + ":" + profile.id + ":" +
                                                 benchmarkMetricName(metric));
                }
            }
        }
    }

    nlohmann::json fixtureRows = nlohmann::json::array();
    for (const auto& fixture : fixtures) {
        fixtureRows.push_back({{"id", fixture.id}, {"scale", benchmarkProjectScaleName(fixture.scale)},
                               {"maps", fixture.maps}, {"events", fixture.events}, {"assets", fixture.assets},
                               {"database_records", fixture.database_records}, {"content_bytes", fixture.content_bytes}});
    }
    nlohmann::json hardwareRows = nlohmann::json::array();
    for (const auto& profile : hardware) {
        hardwareRows.push_back({{"id", profile.id}, {"class", benchmarkHardwareClassName(profile.hardware_class)},
                                {"logical_cores", profile.logical_cores}, {"memory_bytes", profile.memory_bytes},
                                {"graphics_class", profile.graphics_class}});
    }
    result.complete = result.diagnostics.empty();
    result.within_thresholds = result.complete && thresholdsPass;
    result.report = {{"schema", "urpg.product_benchmark.v1"}, {"baseline_version", result.baseline_version},
                     {"complete", result.complete}, {"within_thresholds", result.within_thresholds},
                     {"fixtures", std::move(fixtureRows)}, {"hardware", std::move(hardwareRows)},
                     {"measurements", std::move(rows)}, {"diagnostics", result.diagnostics}};
    return result;
}

} // namespace urpg::perf
