#pragma once

// QuickJS Compat Harness Contract Kernel
// Phase 2 - Compat Layer
//
// This defines the fixture-backed QuickJS contract harness used by URPG's MZ compatibility layer.
// It is intentionally scoped as a compat/import verification harness until a real QuickJS runtime is integrated.
// The harness is isolated from native URPG systems per Invariant 1: "Compat never degrades Native."

#include "engine/runtimes/bridge/value.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg {
namespace compat {

// Severity levels for compat sandbox violations
enum class CompatSeverity : uint8_t {
    WARN = 0,           // API missing but game continues normally
    SOFT_FAIL = 1,      // System fallback activated (e.g., MZ battle → URPG battle)
    HARD_FAIL = 2,      // Plugin isolated, game continues without it
    CRASH_PREVENTED = 3 // Would have crashed; logged with full stack trace
};

// Compatibility surface status tags per Section 4 - Compatibility Surface Registry
enum class CompatStatus : uint8_t {
    FULL = 0,       // Behaves identically to MZ
    PARTIAL = 1,    // Documented deviations listed
    STUB = 2,       // Present but no-ops (optional warnings)
    UNSUPPORTED = 3 // Throws gracefully; logged in Compat Report
};

// Result of a script execution
struct ScriptResult {
    bool success = false;
    Value value;                         // Return value (if any)
    std::string error;                   // Error message (if failed)
    CompatSeverity severity = CompatSeverity::WARN;
    std::string sourceLocation;          // file:line for diagnostics
};

// Memory/CPU budget tracking for compat lane
struct CompatBudget {
    uint32_t cpu_slice_us = 4000;        // Max CPU time per frame for compat lane
    uint32_t memory_limit_mb = 64;       // Memory ceiling for QuickJS heap
    bool exceeded_cpu = false;
    bool exceeded_memory = false;
};

// Virtual filesystem mount point for sandboxed file access
struct VFSMount {
    std::string virtualPath;             // Path as seen by JS (e.g., "/data/")
    std::string realPath;                // Actual filesystem path
    bool readOnly = true;                // Compat lane is read-only by default
};

// QuickJS harness configuration
struct QuickJSConfig {
    bool enableMemoryLimit = true;
    uint32_t memoryLimitMB = 64;
    bool enableCPUBudget = true;
    uint32_t cpuBudgetUs = 4000;
    std::vector<VFSMount> vfsMounts;
    std::string moduleBasePath = "/js/";
};

// Forward declaration - real QuickJS state remains opaque to the contract when/if integrated
class QuickJSContextImpl;

// QuickJSContext - Contract kernel for the fixture-backed QuickJS compat harness
// 
// Design principles:
// 1. All JS→C++ calls go through the Value bridge (no direct type exposure)
// 2. File access is virtualized through VFSMounts (no direct disk access)
// 3. Memory and CPU are budgeted and enforceable
// 4. Errors are always caught and converted to ScriptResult (no exceptions cross boundary)
// 5. Until real QuickJS integration lands, eval/call semantics are harness-driven rather than live-engine driven
//
class QuickJSContext {
public:
    QuickJSContext();
    ~QuickJSContext();

    // Non-copyable, movable
    QuickJSContext(const QuickJSContext&) = delete;
    QuickJSContext& operator=(const QuickJSContext&) = delete;
    QuickJSContext(QuickJSContext&&) noexcept;
    QuickJSContext& operator=(QuickJSContext&&) noexcept;

    // Initialize the compat harness with configuration
    bool initialize(const QuickJSConfig& config);

    // Evaluate harness-supported script directives or passthrough fixture text
    ScriptResult eval(const std::string& code, const std::string& filename = "<eval>");

    // Evaluate a script string within a specific local scope for harness/testing flows
    ScriptResult evalWithScope(const std::string& code, const std::map<std::string, Value>& scope, const std::string& filename = "<eval>");

    // Load and evaluate a module through the harness loader hook
    ScriptResult evalModule(const std::string& filename);

    // Call a global function with arguments
    ScriptResult call(const std::string& functionName, const std::vector<Value>& args);

    // Call a method on an object
    ScriptResult callMethod(const std::string& objectName, 
                           const std::string& methodName,
                           const std::vector<Value>& args);

    // Get/set global variable
    std::optional<Value> getGlobal(const std::string& name);
    bool setGlobal(const std::string& name, const Value& value);

    // Register a C++ function callable from JS
    // The function receives Value arguments and returns a Value
    using HostFunction = std::function<Value(const std::vector<Value>&)>;
    bool registerFunction(const std::string& name, HostFunction fn);

    // Register an object with methods (for API surfaces like Window_Base)
    struct MethodDef {
        std::string name;
        HostFunction fn;
        CompatStatus status = CompatStatus::STUB;
        std::string deviationNote;      // Required if status == PARTIAL
    };
    bool registerObject(const std::string& name, const std::vector<MethodDef>& methods);

    // Budget and resource management
    CompatBudget getBudgetStatus() const;
    void resetBudgetCounters();
    bool isMemoryExceeded() const;
    bool isCPUExceeded() const;

    // Garbage collection
    void runGC();
    size_t getHeapSize() const;

    // Error handling
    std::string getLastError() const;
    void clearLastError();

    // Module loading hook
    using ModuleLoader = std::function<std::optional<std::string>(const std::string& moduleId)>;
    void setModuleLoader(ModuleLoader loader);

    // Compat status registry - tracks which MZ APIs are implemented
    struct APIStatus {
        std::string apiName;
        CompatStatus status;
        std::string deviationNote;
        uint32_t callCount = 0;
        uint32_t failCount = 0;
    };
    void registerAPIStatus(const std::string& apiName, CompatStatus status, 
                          const std::string& deviationNote = "");
    APIStatus getAPIStatus(const std::string& apiName) const;
    std::vector<APIStatus> getAllAPIStatuses() const;

private:
    std::unique_ptr<QuickJSContextImpl> impl_;
    std::unordered_map<std::string, APIStatus> apiRegistry_;
    QuickJSConfig config_;
    CompatBudget budget_;
    std::string lastError_;
    ModuleLoader moduleLoader_;
};

// QuickJSRuntime - Manages multiple isolated compat-harness contexts
//
// Each MZ plugin can run in its own isolated harness context for fixture-backed testing/import flows.
// This does not imply live production JS execution support yet.
//
class QuickJSRuntime {
public:
    QuickJSRuntime();
    ~QuickJSRuntime();

    // Non-copyable
    QuickJSRuntime(const QuickJSRuntime&) = delete;
    QuickJSRuntime& operator=(const QuickJSRuntime&) = delete;

    // Initialize the compat harness runtime wrapper
    bool initialize();

    // Create a new isolated context for a plugin
    // pluginId is used for diagnostics and budget tracking
    std::optional<uint32_t> createContext(const std::string& pluginId, 
                                          const QuickJSConfig& config);

    // Get context by ID
    QuickJSContext* getContext(uint32_t contextId);
    const QuickJSContext* getContext(uint32_t contextId) const;

    // Destroy a context (when plugin is disabled/unloaded)
    void destroyContext(uint32_t contextId);

    // Get context ID by plugin ID
    std::optional<uint32_t> getContextId(const std::string& pluginId) const;

    // Aggregate budget status across all contexts
    CompatBudget getAggregateBudgetStatus() const;

    // Run GC on all contexts
    void runGCAll();

    // Get total heap size across all contexts
    size_t getTotalHeapSize() const;

    // Get all registered API statuses across contexts
    std::unordered_map<std::string, QuickJSContext::APIStatus> getAllAPIStatuses() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace compat
} // namespace urpg
