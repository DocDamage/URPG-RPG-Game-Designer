#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <deque>
#include <map>
#include <string>
#include <vector>

namespace urpg::perf {

enum class PrimaryRoute : uint8_t {
    ProjectOpen,
    AssetBrowse,
    Search,
    MapEdit,
    Save,
    PlaytestLaunch,
    Package,
    ProjectGraph
};

struct PrimaryRouteTrace {
    PrimaryRoute route = PrimaryRoute::ProjectOpen;
    bool main_thread_scan = false;
    bool blocking_io = false;
    bool bounded_job = false;
    bool incremental_index = false;
    uint32_t acknowledgement_us = 0;
    uint32_t interaction_p95_us = 0;
    uint32_t interaction_budget_us = 0;
    uint32_t maximum_items_per_slice = 0;
};

struct PrimaryRouteAuditResult {
    bool complete = false;
    bool within_budgets = false;
    std::vector<std::string> diagnostics;
    nlohmann::json report = nlohmann::json::object();
};

struct PrimaryRoutePolicy {
    PrimaryRoute route = PrimaryRoute::ProjectOpen;
    std::string owner;
    std::string source;
    bool requires_incremental_index = false;
    uint32_t acknowledgement_budget_us = 0;
    uint32_t interaction_budget_us = 0;
    uint32_t maximum_items_per_slice = 0;
    uint32_t minimum_samples = 0;
};

struct PrimaryRoutePlan {
    bool valid = false;
    std::string version;
    std::string budget_status;
    std::vector<PrimaryRoutePolicy> policies;
    std::vector<std::string> diagnostics;
};

struct PrimaryRouteInteractionSample {
    std::string id;
    PrimaryRoute route = PrimaryRoute::ProjectOpen;
    bool main_thread_scan = false;
    bool blocking_io = false;
    bool bounded_job = false;
    bool incremental_index = false;
    uint64_t index_revision = 0;
    uint32_t acknowledgement_us = 0;
    uint32_t interaction_us = 0;
    uint32_t processed_items = 0;
};

class PrimaryRouteWorkAudit {
public:
    PrimaryRouteAuditResult evaluate(const std::vector<PrimaryRouteTrace>& traces) const;
    PrimaryRouteAuditResult evaluate(const PrimaryRoutePlan& plan,
                                     const std::vector<PrimaryRouteInteractionSample>& samples) const;
    static PrimaryRoutePlan parsePlan(const nlohmann::json& value);
};

struct BoundedWorkItem {
    std::string id;
    uint32_t remaining_units = 0;
};

class BoundedWorkQueue {
public:
    explicit BoundedWorkQueue(size_t maximum_jobs = 32);
    bool enqueue(BoundedWorkItem item);
    std::vector<std::string> advance(uint32_t maximum_units);
    bool cancel(const std::string& id);
    size_t size() const { return jobs_.size(); }
    uint64_t consumedUnits() const { return consumed_units_; }

private:
    size_t maximum_jobs_;
    std::deque<BoundedWorkItem> jobs_;
    uint64_t consumed_units_ = 0;
};

class IncrementalRouteIndex {
public:
    bool upsert(std::string key, std::string searchable_text);
    bool remove(const std::string& key);
    std::vector<std::string> search(std::string query, size_t maximum_results) const;
    uint64_t revision() const { return revision_; }
    size_t size() const { return entries_.size(); }

private:
    std::map<std::string, std::string> entries_;
    uint64_t revision_ = 0;
};

const char* primaryRouteName(PrimaryRoute route);

} // namespace urpg::perf
