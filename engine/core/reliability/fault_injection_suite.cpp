#include "engine/core/reliability/fault_injection_suite.h"

#include <array>
#include <exception>
#include <utility>

namespace urpg::reliability {
namespace {

constexpr std::array<FaultInjectionPoint, 9> kRequiredPoints = {
    FaultInjectionPoint::DiskFull, FaultInjectionPoint::PermissionLoss, FaultInjectionPoint::InterruptedWrite,
    FaultInjectionPoint::MalformedDocument, FaultInjectionPoint::MissingAsset, FaultInjectionPoint::BadArchive,
    FaultInjectionPoint::FailedMigration, FaultInjectionPoint::RendererDeviceLoss,
    FaultInjectionPoint::ChildProcessFailure};

} // namespace

const char* faultInjectionPointName(const FaultInjectionPoint point) {
    switch (point) {
    case FaultInjectionPoint::DiskFull: return "disk_full";
    case FaultInjectionPoint::PermissionLoss: return "permission_loss";
    case FaultInjectionPoint::InterruptedWrite: return "interrupted_write";
    case FaultInjectionPoint::MalformedDocument: return "malformed_document";
    case FaultInjectionPoint::MissingAsset: return "missing_asset";
    case FaultInjectionPoint::BadArchive: return "bad_archive";
    case FaultInjectionPoint::FailedMigration: return "failed_migration";
    case FaultInjectionPoint::RendererDeviceLoss: return "renderer_device_loss";
    case FaultInjectionPoint::ChildProcessFailure: return "child_process_failure";
    }
    return "unknown";
}

bool FaultInjectionSuite::registerHandler(const FaultInjectionPoint point, Handler handler) {
    if (!handler || handlers_.contains(point)) return false;
    handlers_.emplace(point, std::move(handler));
    return true;
}

FaultInjectionResult FaultInjectionSuite::run() const {
    FaultInjectionResult result;
    for (const auto point : kRequiredPoints) {
        const auto handler = handlers_.find(point);
        if (handler == handlers_.end()) {
            result.diagnostics.push_back(std::string("fault_handler_missing:") + faultInjectionPointName(point));
            continue;
        }
        FaultInjectionOutcome outcome;
        try {
            outcome = handler->second(point);
        } catch (const std::exception& exception) {
            result.diagnostics.push_back(std::string("fault_handler_threw:") + faultInjectionPointName(point) + ":" + exception.what());
            continue;
        } catch (...) {
            result.diagnostics.push_back(std::string("fault_handler_threw:") + faultInjectionPointName(point));
            continue;
        }
        outcome.point = point;
        const auto name = std::string(faultInjectionPointName(point));
        if (!outcome.fault_observed) result.diagnostics.push_back("fault_not_observed:" + name);
        if (!outcome.bounded || outcome.budget_ms == 0 || outcome.elapsed_ms > outcome.budget_ms) {
            result.diagnostics.push_back("fault_behavior_unbounded:" + name);
        }
        if (!outcome.project_data_preserved) result.diagnostics.push_back("fault_project_data_at_risk:" + name);
        if (!outcome.partial_state_contained) result.diagnostics.push_back("fault_partial_state_uncontained:" + name);
        if (!outcome.retry_or_recovery_available) result.diagnostics.push_back("fault_recovery_missing:" + name);
        if (outcome.diagnostic.empty()) result.diagnostics.push_back("fault_diagnostic_missing:" + name);
        if (outcome.recovery_path.empty()) result.diagnostics.push_back("fault_recovery_path_missing:" + name);
        result.outcomes.push_back(std::move(outcome));
    }
    result.complete = result.outcomes.size() == kRequiredPoints.size();
    result.safe = result.complete && result.diagnostics.empty();
    return result;
}

} // namespace urpg::reliability
