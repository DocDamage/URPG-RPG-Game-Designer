#include "engine/core/perf/primary_route_work.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <optional>
#include <set>
#include <utility>

namespace urpg::perf {
namespace {

constexpr std::array<PrimaryRoute, 8> kRequiredRoutes = {
    PrimaryRoute::ProjectOpen, PrimaryRoute::AssetBrowse, PrimaryRoute::Search, PrimaryRoute::MapEdit,
    PrimaryRoute::Save, PrimaryRoute::PlaytestLaunch, PrimaryRoute::Package, PrimaryRoute::ProjectGraph};

std::string normalized(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](const unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

std::optional<PrimaryRoute> primaryRoute(const std::string_view value) {
    for (const auto route : kRequiredRoutes) {
        if (value == primaryRouteName(route)) return route;
    }
    return std::nullopt;
}

uint32_t percentile95(std::vector<uint32_t> values) {
    if (values.empty()) return 0;
    std::sort(values.begin(), values.end());
    const auto rank = (values.size() * 95 + 99) / 100;
    return values[rank - 1];
}

} // namespace

const char* primaryRouteName(const PrimaryRoute route) {
    switch (route) {
    case PrimaryRoute::ProjectOpen: return "project_open";
    case PrimaryRoute::AssetBrowse: return "asset_browse";
    case PrimaryRoute::Search: return "search";
    case PrimaryRoute::MapEdit: return "map_edit";
    case PrimaryRoute::Save: return "save";
    case PrimaryRoute::PlaytestLaunch: return "playtest_launch";
    case PrimaryRoute::Package: return "package";
    case PrimaryRoute::ProjectGraph: return "project_graph";
    }
    return "unknown";
}

PrimaryRouteAuditResult PrimaryRouteWorkAudit::evaluate(const std::vector<PrimaryRouteTrace>& traces) const {
    PrimaryRouteAuditResult result;
    std::set<PrimaryRoute> covered;
    for (const auto& trace : traces) {
        if (!covered.insert(trace.route).second) {
            result.diagnostics.push_back(std::string("primary_route_duplicate:") + primaryRouteName(trace.route));
            continue;
        }
        const auto route = std::string(primaryRouteName(trace.route));
        if (trace.main_thread_scan) result.diagnostics.push_back("primary_route_main_thread_scan:" + route);
        if (trace.blocking_io) result.diagnostics.push_back("primary_route_blocking_io:" + route);
        if (!trace.bounded_job || trace.maximum_items_per_slice == 0) {
            result.diagnostics.push_back("primary_route_job_unbounded:" + route);
        }
        if (!trace.incremental_index && (trace.route == PrimaryRoute::AssetBrowse || trace.route == PrimaryRoute::Search ||
                                         trace.route == PrimaryRoute::ProjectGraph)) {
            result.diagnostics.push_back("primary_route_incremental_index_missing:" + route);
        }
        if (trace.acknowledgement_us == 0 || trace.acknowledgement_us > 50000) {
            result.diagnostics.push_back("primary_route_acknowledgement_slow:" + route);
        }
        if (trace.interaction_budget_us == 0 || trace.interaction_p95_us == 0 ||
            trace.interaction_p95_us > trace.interaction_budget_us) {
            result.diagnostics.push_back("primary_route_latency_regression:" + route);
        }
    }
    for (const auto route : kRequiredRoutes) {
        if (!covered.contains(route)) result.diagnostics.push_back(std::string("primary_route_missing:") + primaryRouteName(route));
    }
    result.complete = covered.size() == kRequiredRoutes.size();
    result.within_budgets = result.complete && result.diagnostics.empty();
    return result;
}

PrimaryRoutePlan PrimaryRouteWorkAudit::parsePlan(const nlohmann::json& value) {
    PrimaryRoutePlan plan;
    if (!value.is_object() || value.value("schema", "") != "urpg.primary_route_latency_plan.v1") {
        plan.diagnostics.push_back("primary_route_plan_schema_invalid");
        return plan;
    }
    plan.version = value.value("version", "");
    plan.budget_status = value.value("budget_status", "");
    if (plan.version.empty()) plan.diagnostics.push_back("primary_route_plan_version_missing");
    if (plan.budget_status.empty()) plan.diagnostics.push_back("primary_route_budget_status_missing");

    const auto rows = value.value("routes", nlohmann::json::array());
    std::set<PrimaryRoute> covered;
    if (!rows.is_array()) {
        plan.diagnostics.push_back("primary_route_plan_routes_invalid");
    } else {
        for (const auto& row : rows) {
            if (!row.is_object()) {
                plan.diagnostics.push_back("primary_route_plan_row_invalid");
                continue;
            }
            const auto route = primaryRoute(row.value("id", ""));
            if (!route) {
                plan.diagnostics.push_back("primary_route_plan_route_unknown:" + row.value("id", ""));
                continue;
            }
            if (!covered.insert(*route).second) {
                plan.diagnostics.push_back("primary_route_plan_route_duplicate:" + row.value("id", ""));
                continue;
            }
            PrimaryRoutePolicy policy;
            policy.route = *route;
            policy.owner = row.value("owner", "");
            policy.source = row.value("source", "");
            policy.requires_incremental_index = row.value("requires_incremental_index", false);
            policy.acknowledgement_budget_us = row.value("acknowledgement_budget_us", 0U);
            policy.interaction_budget_us = row.value("interaction_budget_us", 0U);
            policy.maximum_items_per_slice = row.value("maximum_items_per_slice", 0U);
            policy.minimum_samples = row.value("minimum_samples", 0U);
            if (policy.owner.empty() || policy.source.empty() || policy.acknowledgement_budget_us == 0 ||
                policy.acknowledgement_budget_us > 50000 || policy.interaction_budget_us == 0 ||
                policy.maximum_items_per_slice == 0 || policy.minimum_samples == 0) {
                plan.diagnostics.push_back("primary_route_plan_policy_invalid:" + row.value("id", ""));
            }
            const bool indexRequired = *route == PrimaryRoute::AssetBrowse || *route == PrimaryRoute::Search ||
                                       *route == PrimaryRoute::ProjectGraph;
            if (policy.requires_incremental_index != indexRequired) {
                plan.diagnostics.push_back("primary_route_plan_index_policy_invalid:" + row.value("id", ""));
            }
            plan.policies.push_back(std::move(policy));
        }
    }
    for (const auto route : kRequiredRoutes) {
        if (!covered.contains(route)) {
            plan.diagnostics.push_back(std::string("primary_route_plan_route_missing:") + primaryRouteName(route));
        }
    }
    std::sort(plan.diagnostics.begin(), plan.diagnostics.end());
    plan.diagnostics.erase(std::unique(plan.diagnostics.begin(), plan.diagnostics.end()), plan.diagnostics.end());
    plan.valid = plan.diagnostics.empty() && plan.policies.size() == kRequiredRoutes.size();
    return plan;
}

PrimaryRouteAuditResult PrimaryRouteWorkAudit::evaluate(
    const PrimaryRoutePlan& plan, const std::vector<PrimaryRouteInteractionSample>& samples) const {
    if (!plan.valid) {
        PrimaryRouteAuditResult result;
        result.diagnostics = plan.diagnostics;
        result.report = {{"schema", "urpg.primary_route_latency_report.v1"}, {"version", plan.version},
                         {"budget_status", plan.budget_status}, {"complete", false},
                         {"within_budgets", false}, {"routes", nlohmann::json::array()},
                         {"diagnostics", result.diagnostics}};
        return result;
    }

    std::vector<PrimaryRouteTrace> traces;
    std::vector<std::string> captureDiagnostics;
    bool captureComplete = true;
    std::set<std::string> sampleIds;
    nlohmann::json routeReports = nlohmann::json::array();
    for (const auto& policy : plan.policies) {
        std::vector<uint32_t> interactionValues;
        uint32_t maximumAcknowledgement = 0;
        bool mainThreadScan = false;
        bool blockingIo = false;
        bool bounded = true;
        bool indexed = true;
        nlohmann::json sampleReports = nlohmann::json::array();
        for (const auto& sample : samples) {
            if (sample.route != policy.route) continue;
            if (sample.id.empty() || !sampleIds.insert(sample.id).second) {
                captureDiagnostics.push_back(std::string("primary_route_sample_id_invalid:") +
                                             primaryRouteName(policy.route));
                captureComplete = false;
            }
            maximumAcknowledgement = std::max(maximumAcknowledgement, sample.acknowledgement_us);
            interactionValues.push_back(sample.interaction_us);
            mainThreadScan = mainThreadScan || sample.main_thread_scan;
            blockingIo = blockingIo || sample.blocking_io;
            bounded = bounded && sample.bounded_job && sample.processed_items > 0 &&
                      sample.processed_items <= policy.maximum_items_per_slice;
            if (policy.requires_incremental_index) {
                indexed = indexed && sample.incremental_index && sample.index_revision > 0;
            }
            sampleReports.push_back({{"id", sample.id}, {"acknowledgement_us", sample.acknowledgement_us},
                                     {"interaction_us", sample.interaction_us},
                                     {"processed_items", sample.processed_items},
                                     {"main_thread_scan", sample.main_thread_scan},
                                     {"blocking_io", sample.blocking_io},
                                     {"bounded_job", sample.bounded_job},
                                     {"incremental_index", sample.incremental_index},
                                     {"index_revision", sample.index_revision}});
        }
        if (interactionValues.size() < policy.minimum_samples) {
            captureDiagnostics.push_back(std::string("primary_route_samples_insufficient:") +
                                         primaryRouteName(policy.route));
            captureComplete = false;
        }
        if (maximumAcknowledgement > policy.acknowledgement_budget_us) {
            captureDiagnostics.push_back(std::string("primary_route_acknowledgement_budget_exceeded:") +
                                         primaryRouteName(policy.route));
        }
        const auto p95 = percentile95(interactionValues);
        traces.push_back({policy.route, mainThreadScan, blockingIo, bounded, indexed, maximumAcknowledgement, p95,
                          policy.interaction_budget_us, policy.maximum_items_per_slice});
        routeReports.push_back({{"id", primaryRouteName(policy.route)}, {"owner", policy.owner},
                                {"source", policy.source}, {"sample_count", interactionValues.size()},
                                {"minimum_samples", policy.minimum_samples},
                                {"acknowledgement_max_us", maximumAcknowledgement},
                                {"acknowledgement_budget_us", policy.acknowledgement_budget_us},
                                {"interaction_p95_us", p95},
                                {"interaction_budget_us", policy.interaction_budget_us},
                                {"maximum_items_per_slice", policy.maximum_items_per_slice},
                                {"samples", std::move(sampleReports)}});
    }

    auto result = evaluate(traces);
    result.diagnostics.insert(result.diagnostics.end(), captureDiagnostics.begin(), captureDiagnostics.end());
    std::sort(result.diagnostics.begin(), result.diagnostics.end());
    result.diagnostics.erase(std::unique(result.diagnostics.begin(), result.diagnostics.end()), result.diagnostics.end());
    result.complete = result.complete && captureComplete;
    result.within_budgets = result.complete && result.diagnostics.empty();
    result.report = {{"schema", "urpg.primary_route_latency_report.v1"}, {"version", plan.version},
                     {"budget_status", plan.budget_status}, {"complete", result.complete},
                     {"within_budgets", result.within_budgets}, {"routes", std::move(routeReports)},
                     {"diagnostics", result.diagnostics}};
    return result;
}

BoundedWorkQueue::BoundedWorkQueue(const size_t maximum_jobs) : maximum_jobs_(std::max<size_t>(1, maximum_jobs)) {}

bool BoundedWorkQueue::enqueue(BoundedWorkItem item) {
    if (item.id.empty() || item.remaining_units == 0 || jobs_.size() >= maximum_jobs_ ||
        std::any_of(jobs_.begin(), jobs_.end(), [&](const auto& row) { return row.id == item.id; })) return false;
    jobs_.push_back(std::move(item));
    return true;
}

std::vector<std::string> BoundedWorkQueue::advance(uint32_t maximum_units) {
    std::vector<std::string> completed;
    while (maximum_units > 0 && !jobs_.empty()) {
        auto& job = jobs_.front();
        const auto consumed = std::min(maximum_units, job.remaining_units);
        job.remaining_units -= consumed;
        maximum_units -= consumed;
        consumed_units_ += consumed;
        if (job.remaining_units == 0) {
            completed.push_back(job.id);
            jobs_.pop_front();
        }
    }
    return completed;
}

bool BoundedWorkQueue::cancel(const std::string& id) {
    const auto row = std::find_if(jobs_.begin(), jobs_.end(), [&](const auto& job) { return job.id == id; });
    if (row == jobs_.end()) return false;
    jobs_.erase(row);
    return true;
}

bool IncrementalRouteIndex::upsert(std::string key, std::string searchable_text) {
    if (key.empty() || searchable_text.empty()) return false;
    const auto normalizedText = normalized(std::move(searchable_text));
    const auto existing = entries_.find(key);
    if (existing != entries_.end() && existing->second == normalizedText) return true;
    entries_[std::move(key)] = normalizedText;
    ++revision_;
    return true;
}

bool IncrementalRouteIndex::remove(const std::string& key) {
    if (entries_.erase(key) == 0) return false;
    ++revision_;
    return true;
}

std::vector<std::string> IncrementalRouteIndex::search(std::string query, const size_t maximum_results) const {
    std::vector<std::string> results;
    if (query.empty() || maximum_results == 0) return results;
    query = normalized(std::move(query));
    for (const auto& [key, text] : entries_) {
        if (text.find(query) != std::string::npos) {
            results.push_back(key);
            if (results.size() == maximum_results) break;
        }
    }
    return results;
}

} // namespace urpg::perf
