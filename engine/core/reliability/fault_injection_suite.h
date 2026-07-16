#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace urpg::reliability {

enum class FaultInjectionPoint : uint8_t {
    DiskFull,
    PermissionLoss,
    InterruptedWrite,
    MalformedDocument,
    MissingAsset,
    BadArchive,
    FailedMigration,
    RendererDeviceLoss,
    ChildProcessFailure
};

struct FaultInjectionOutcome {
    FaultInjectionPoint point = FaultInjectionPoint::DiskFull;
    bool fault_observed = false;
    bool bounded = false;
    bool project_data_preserved = false;
    bool partial_state_contained = false;
    bool retry_or_recovery_available = false;
    uint32_t elapsed_ms = 0;
    uint32_t budget_ms = 0;
    std::string diagnostic;
    std::string recovery_path;
};

struct FaultInjectionResult {
    bool complete = false;
    bool safe = false;
    std::vector<FaultInjectionOutcome> outcomes;
    std::vector<std::string> diagnostics;
};

class FaultInjectionSuite {
public:
    using Handler = std::function<FaultInjectionOutcome(FaultInjectionPoint)>;

    bool registerHandler(FaultInjectionPoint point, Handler handler);
    FaultInjectionResult run() const;

private:
    std::map<FaultInjectionPoint, Handler> handlers_;
};

const char* faultInjectionPointName(FaultInjectionPoint point);

} // namespace urpg::reliability
