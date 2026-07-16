#include "engine/core/perf/primary_route_work.h"

#include <algorithm>
#include <array>
#include <cctype>
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
