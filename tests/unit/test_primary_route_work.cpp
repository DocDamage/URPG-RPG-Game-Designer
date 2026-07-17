#include "engine/core/perf/primary_route_work.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

TEST_CASE("Primary route audit requires bounded jobs indexes and latency traces", "[perf][primary_routes][pcq701]") {
    using urpg::perf::PrimaryRoute;
    std::vector<urpg::perf::PrimaryRouteTrace> traces;
    for (const auto route : {PrimaryRoute::ProjectOpen, PrimaryRoute::AssetBrowse, PrimaryRoute::Search,
                             PrimaryRoute::MapEdit, PrimaryRoute::Save, PrimaryRoute::PlaytestLaunch,
                             PrimaryRoute::Package, PrimaryRoute::ProjectGraph}) {
        const bool indexed = route == PrimaryRoute::AssetBrowse || route == PrimaryRoute::Search ||
                             route == PrimaryRoute::ProjectGraph;
        traces.push_back({route, false, false, true, indexed, 1000, 8000, 10000, 64});
    }
    const auto result = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(traces);
    REQUIRE(result.complete);
    REQUIRE(result.within_budgets);
    REQUIRE(result.diagnostics.empty());

    traces[0].blocking_io = true;
    traces[1].incremental_index = false;
    traces[2].interaction_p95_us = 10001;
    const auto failed = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(traces);
    REQUIRE_FALSE(failed.within_budgets);
    REQUIRE(failed.diagnostics.size() == 3);
}

TEST_CASE("Primary route jobs and indexes remain incrementally bounded", "[perf][primary_routes][pcq701]") {
    urpg::perf::BoundedWorkQueue queue(2);
    REQUIRE(queue.enqueue({"scan-a", 5}));
    REQUIRE(queue.enqueue({"scan-b", 3}));
    REQUIRE_FALSE(queue.enqueue({"scan-c", 1}));
    REQUIRE(queue.advance(4).empty());
    REQUIRE(queue.size() == 2);
    REQUIRE(queue.advance(2) == std::vector<std::string>{"scan-a"});
    REQUIRE(queue.size() == 1);
    REQUIRE(queue.cancel("scan-b"));
    REQUIRE(queue.size() == 0);
    REQUIRE(queue.consumedUnits() == 6);

    urpg::perf::IncrementalRouteIndex index;
    REQUIRE(index.upsert("map:2", "Forest Shrine"));
    REQUIRE(index.upsert("map:1", "Forest Entrance"));
    REQUIRE(index.search("FOREST", 1) == std::vector<std::string>{"map:1"});
    REQUIRE(index.revision() == 2);
    REQUIRE(index.remove("map:1"));
    REQUIRE(index.search("forest", 4) == std::vector<std::string>{"map:2"});
}

TEST_CASE("Primary route latency plan aggregates raw samples into a versioned report",
          "[perf][primary_routes][pcq701]") {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "benchmarks" /
                        "primary_route_latency_plan_v1.json");
    REQUIRE(input.good());
    const auto plan = urpg::perf::PrimaryRouteWorkAudit::parsePlan(nlohmann::json::parse(input));
    REQUIRE(plan.valid);
    REQUIRE(plan.diagnostics.empty());
    REQUIRE(plan.version == "pcq701.v1");
    REQUIRE(plan.policies.size() == 8);

    std::vector<urpg::perf::PrimaryRouteInteractionSample> samples;
    for (const auto& policy : plan.policies) {
        for (uint32_t index = 0; index < policy.minimum_samples; ++index) {
            urpg::perf::PrimaryRouteInteractionSample sample;
            sample.id = std::string(urpg::perf::primaryRouteName(policy.route)) + ":" + std::to_string(index);
            sample.route = policy.route;
            sample.bounded_job = true;
            sample.incremental_index = policy.requires_incremental_index;
            sample.index_revision = policy.requires_incremental_index ? index + 1 : 0;
            sample.acknowledgement_us = 1000 + index;
            sample.interaction_us = policy.interaction_budget_us - policy.minimum_samples + index;
            sample.processed_items = policy.maximum_items_per_slice;
            samples.push_back(std::move(sample));
        }
    }

    const auto result = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(plan, samples);
    REQUIRE(result.complete);
    REQUIRE(result.within_budgets);
    REQUIRE(result.diagnostics.empty());
    REQUIRE(result.report.at("schema") == "urpg.primary_route_latency_report.v1");
    REQUIRE(result.report.at("version") == "pcq701.v1");
    REQUIRE(result.report.at("routes").size() == 8);
    REQUIRE(result.report.at("routes")[0].at("sample_count") == 20);
    REQUIRE(result.report.at("routes")[0].at("interaction_p95_us") == 499998);

    auto budget_failure_samples = samples;
    budget_failure_samples.front().acknowledgement_us = plan.policies.front().acknowledgement_budget_us + 1;
    const auto budget_failure = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(plan, budget_failure_samples);
    REQUIRE(budget_failure.complete);
    REQUIRE_FALSE(budget_failure.within_budgets);
    REQUIRE(std::find(budget_failure.diagnostics.begin(), budget_failure.diagnostics.end(),
                      "primary_route_acknowledgement_budget_exceeded:project_open") !=
            budget_failure.diagnostics.end());

    samples.front().blocking_io = true;
    samples[20].processed_items = plan.policies[1].maximum_items_per_slice + 1;
    samples.pop_back();
    const auto failed = urpg::perf::PrimaryRouteWorkAudit{}.evaluate(plan, samples);
    REQUIRE_FALSE(failed.complete);
    REQUIRE_FALSE(failed.within_budgets);
    REQUIRE(std::find(failed.diagnostics.begin(), failed.diagnostics.end(),
                      "primary_route_blocking_io:project_open") != failed.diagnostics.end());
    REQUIRE(std::find(failed.diagnostics.begin(), failed.diagnostics.end(),
                      "primary_route_job_unbounded:asset_browse") != failed.diagnostics.end());
    REQUIRE(std::find(failed.diagnostics.begin(), failed.diagnostics.end(),
                      "primary_route_samples_insufficient:project_graph") != failed.diagnostics.end());
}

TEST_CASE("Primary route latency plan rejects incomplete or unsafe governance",
          "[perf][primary_routes][pcq701]") {
    nlohmann::json value = {
        {"schema", "urpg.primary_route_latency_plan.v1"},
        {"version", "pcq701.v1"},
        {"budget_status", "provisional"},
        {"routes", nlohmann::json::array({{{"id", "project_open"},
                                            {"owner", "editor/project"},
                                            {"source", "editor/project/editor_project_session.cpp"},
                                            {"requires_incremental_index", true},
                                            {"acknowledgement_budget_us", 60000},
                                            {"interaction_budget_us", 500000},
                                            {"maximum_items_per_slice", 32},
                                            {"minimum_samples", 20}}})}};
    const auto plan = urpg::perf::PrimaryRouteWorkAudit::parsePlan(value);
    REQUIRE_FALSE(plan.valid);
    REQUIRE_FALSE(plan.diagnostics.empty());
    REQUIRE(std::find(plan.diagnostics.begin(), plan.diagnostics.end(),
                      "primary_route_plan_policy_invalid:project_open") != plan.diagnostics.end());
    REQUIRE(std::find(plan.diagnostics.begin(), plan.diagnostics.end(),
                      "primary_route_plan_index_policy_invalid:project_open") != plan.diagnostics.end());
    REQUIRE(std::find(plan.diagnostics.begin(), plan.diagnostics.end(),
                      "primary_route_plan_route_missing:package") != plan.diagnostics.end());
}
