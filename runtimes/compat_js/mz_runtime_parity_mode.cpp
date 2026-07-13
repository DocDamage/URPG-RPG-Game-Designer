#include "runtimes/compat_js/mz_runtime_parity_mode.h"

#include <nlohmann/json.hpp>

namespace urpg::compat_js {

namespace {

constexpr const char* kRuntimeParityExperimentalDiagnostic =
    "mz_runtime_parity_is_experimental_and_not_part_of_compat_bridge_exit_ready_scope";

}  // namespace

const char* ToString(MzRuntimeParityMode mode) {
    switch (mode) {
        case MzRuntimeParityMode::BridgeOnly:
            return "bridge_only";
        case MzRuntimeParityMode::RuntimeParityExperimental:
            return "runtime_parity_experimental";
    }

    return "bridge_only";
}

bool MzRuntimeParityConfig::runtimeParityEnabled() const {
    return mode == MzRuntimeParityMode::RuntimeParityExperimental;
}

nlohmann::json MzRuntimeParityConfig::toJson() const {
    return nlohmann::json{{"mode", ToString(mode)},
                          {"release_authoritative", false},
                          {"diagnostic", kRuntimeParityExperimentalDiagnostic}};
}

}  // namespace urpg::compat_js
