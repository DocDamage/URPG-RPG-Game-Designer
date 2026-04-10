// QuickJS Runtime Integration Contract Kernel - Implementation Stubs
// Phase 2 - Compat Layer
//
// This file provides stub implementations for the QuickJS runtime contract.
// The actual QuickJS integration will be completed when linking against QuickJS.

#include "quickjs_runtime.h"
#include <algorithm>
#include <cassert>
#include <exception>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace urpg {
namespace compat {

namespace {

constexpr uint64_t kStubCallCostUs = 100;
constexpr std::string_view kFailContextInitMarker = "__urpg_fail_context_init__";
constexpr std::string_view kFailRegisterFunctionMarker = "__urpg_fail_register_function__";

std::string trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

Value parseDirectiveConstValue(const std::string& payload) {
    const std::string text = trim(payload);
    if (text.empty()) {
        return Value::Nil();
    }

    if (text.size() >= 2 &&
        ((text.front() == '"' && text.back() == '"') ||
         (text.front() == '\'' && text.back() == '\''))) {
        Value value;
        value.v = text.substr(1, text.size() - 2);
        return value;
    }

    if (text == "true" || text == "false") {
        Value value;
        value.v = (text == "true");
        return value;
    }

    try {
        size_t consumed = 0;
        const int64_t asInt = std::stoll(text, &consumed, 10);
        if (consumed == text.size()) {
            return Value::Int(asInt);
        }
    } catch (...) {
        // Fall through to double/string parsing.
    }

    try {
        size_t consumed = 0;
        const double asDouble = std::stod(text, &consumed);
        if (consumed == text.size()) {
            Value value;
            value.v = asDouble;
            return value;
        }
    } catch (...) {
        // Fall through to string fallback.
    }

    Value value;
    value.v = text;
    return value;
}

} // namespace

// ============================================================================
// QuickJSContext Implementation
// ============================================================================

// Opaque implementation struct - will hold actual QuickJS state
class QuickJSContextImpl {
public:
    bool initialized = false;
    QuickJSConfig config;
    std::unordered_map<std::string, QuickJSContext::HostFunction> hostFunctions;
    std::unordered_map<std::string, Value> globals;
    size_t heapSize = 0;
    uint64_t cpuUsedUs = 0;
};

QuickJSContext::QuickJSContext() 
    : impl_(std::make_unique<QuickJSContextImpl>()) 
{
}

QuickJSContext::~QuickJSContext() = default;

QuickJSContext::QuickJSContext(QuickJSContext&& other) noexcept
    : impl_(std::move(other.impl_))
    , apiRegistry_(std::move(other.apiRegistry_))
    , config_(std::move(other.config_))
    , budget_(other.budget_)
    , lastError_(std::move(other.lastError_))
    , moduleLoader_(std::move(other.moduleLoader_))
{
}

QuickJSContext& QuickJSContext::operator=(QuickJSContext&& other) noexcept {
    if (this != &other) {
        impl_ = std::move(other.impl_);
        apiRegistry_ = std::move(other.apiRegistry_);
        config_ = std::move(other.config_);
        budget_ = other.budget_;
        lastError_ = std::move(other.lastError_);
        moduleLoader_ = std::move(other.moduleLoader_);
    }
    return *this;
}

bool QuickJSContext::initialize(const QuickJSConfig& config) {
    assert(impl_ != nullptr);
    
    config_ = config;
    budget_.cpu_slice_us = config.cpuBudgetUs;
    budget_.memory_limit_mb = config.memoryLimitMB;
    
    // TODO: Initialize actual QuickJS runtime here
    // JSRuntime* rt = JS_NewRuntime();
    // JSContext* ctx = JS_NewContext(rt);
    // Configure memory limits, etc.
    
    impl_->initialized = true;
    impl_->config = config;
    return true;
}

ScriptResult QuickJSContext::eval(const std::string& code, const std::string& filename) {
    assert(impl_ != nullptr);
    
    ScriptResult result;
    
    if (!impl_->initialized) {
        result.success = false;
        result.error = "QuickJSContext not initialized";
        result.severity = CompatSeverity::HARD_FAIL;
        return result;
    }
    
    if (isMemoryExceeded()) {
        result.success = false;
        result.error = "Memory budget exceeded";
        result.severity = CompatSeverity::HARD_FAIL;
        budget_.exceeded_memory = true;
        return result;
    }
    
    // Stub fixture bridge: parse lightweight export directives and bind functions.
    // Supported directive format:
    //   // @urpg-export <fnName> arg_count
    //   // @urpg-export <fnName> arg <index>
    //   // @urpg-export <fnName> const <value>
    //   // @urpg-fail-call <fnName> <message>
    std::istringstream stream(code);
    std::string line;
    uint32_t lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber;
        const auto failPos = line.find("@urpg-fail-eval");
        if (failPos != std::string::npos) {
            std::string payload =
                trim(line.substr(failPos + std::string("@urpg-fail-eval").size()));
            if (payload.empty()) {
                payload = "QuickJS eval failure requested by fixture directive";
            }
            result.success = false;
            result.error = payload;
            result.severity = CompatSeverity::HARD_FAIL;
            result.sourceLocation = filename + ":" + std::to_string(lineNumber);
            lastError_ = result.error;
            return result;
        }

        const auto failCallPos = line.find("@urpg-fail-call");
        if (failCallPos != std::string::npos) {
            std::istringstream directive(
                line.substr(failCallPos + std::string("@urpg-fail-call").size())
            );
            std::string functionName;
            if (!(directive >> functionName) || functionName.empty()) {
                continue;
            }

            std::string payload;
            std::getline(directive, payload);
            payload = trim(payload);
            if (payload.empty()) {
                payload = "QuickJS call failure requested by fixture directive";
            }

            registerFunction(
                functionName,
                [payload](const std::vector<Value>&) -> Value {
                    throw std::runtime_error(payload);
                }
            );
            continue;
        }

        const auto markerPos = line.find("@urpg-export");
        if (markerPos == std::string::npos) {
            continue;
        }

        std::istringstream directive(line.substr(markerPos + std::string("@urpg-export").size()));
        std::string functionName;
        std::string mode;
        if (!(directive >> functionName >> mode)) {
            continue;
        }
        if (functionName.empty()) {
            continue;
        }

        if (mode == "arg_count") {
            registerFunction(
                functionName,
                [](const std::vector<Value>& args) -> Value {
                    return Value::Int(static_cast<int64_t>(args.size()));
                }
            );
            continue;
        }

        if (mode == "arg") {
            int64_t index = 0;
            (void)(directive >> index);
            registerFunction(
                functionName,
                [index](const std::vector<Value>& args) -> Value {
                    if (index >= 0 && static_cast<size_t>(index) < args.size()) {
                        return args[static_cast<size_t>(index)];
                    }
                    return Value::Nil();
                }
            );
            continue;
        }

        if (mode == "const") {
            std::string payload;
            std::getline(directive, payload);
            const Value constantValue = parseDirectiveConstValue(payload);
            registerFunction(
                functionName,
                [constantValue](const std::vector<Value>&) -> Value {
                    return constantValue;
                }
            );
        }
    }
    
    // Stub: return success with undefined
    result.success = true;
    result.value = Value::Nil();
    result.sourceLocation = filename + ":1";
    lastError_.clear();
    
    return result;
}

ScriptResult QuickJSContext::evalModule(const std::string& filename) {
    assert(impl_ != nullptr);
    
    ScriptResult result;
    
    if (!impl_->initialized) {
        result.success = false;
        result.error = "QuickJSContext not initialized";
        result.severity = CompatSeverity::HARD_FAIL;
        return result;
    }
    
    // Try to load module via loader
    if (moduleLoader_) {
        auto content = moduleLoader_(filename);
        if (content) {
            return eval(*content, filename);
        }
    }
    
    result.success = false;
    result.error = "Module not found: " + filename;
    result.severity = CompatSeverity::SOFT_FAIL;
    return result;
}

ScriptResult QuickJSContext::call(const std::string& functionName, 
                                   const std::vector<Value>& args) {
    assert(impl_ != nullptr);
    
    ScriptResult result;
    
    if (!impl_->initialized) {
        result.success = false;
        result.error = "QuickJSContext not initialized";
        result.severity = CompatSeverity::HARD_FAIL;
        return result;
    }

    if (isMemoryExceeded()) {
        result.success = false;
        result.error = "Memory budget exceeded";
        result.severity = CompatSeverity::HARD_FAIL;
        budget_.exceeded_memory = true;
        return result;
    }

    if (config_.enableCPUBudget) {
        impl_->cpuUsedUs += kStubCallCostUs;
        if (isCPUExceeded()) {
            budget_.exceeded_cpu = true;
            result.success = false;
            result.error = "CPU budget exceeded";
            result.severity = CompatSeverity::SOFT_FAIL;
            return result;
        }
    }

    auto fnIt = impl_->hostFunctions.find(functionName);
    if (fnIt == impl_->hostFunctions.end()) {
        result.success = false;
        result.error = "Function not found: " + functionName;
        result.severity = CompatSeverity::SOFT_FAIL;
        lastError_ = result.error;
        return result;
    }

    try {
        result.value = fnIt->second(args);
        result.success = true;
        result.sourceLocation = functionName;
        lastError_.clear();
    } catch (const std::exception& e) {
        result.success = false;
        result.error = std::string("Host function error: ") + e.what();
        result.severity = CompatSeverity::HARD_FAIL;
        lastError_ = result.error;
    } catch (...) {
        result.success = false;
        result.error = "Unknown host function error";
        result.severity = CompatSeverity::CRASH_PREVENTED;
        lastError_ = result.error;
    }
    return result;
}

ScriptResult QuickJSContext::callMethod(const std::string& objectName,
                                         const std::string& methodName,
                                         const std::vector<Value>& args) {
    assert(impl_ != nullptr);
    
    ScriptResult result;
    
    if (!impl_->initialized) {
        result.success = false;
        result.error = "QuickJSContext not initialized";
        result.severity = CompatSeverity::HARD_FAIL;
        return result;
    }
    
    // Check API status for deviation tracking
    std::string fullName = objectName + "." + methodName;
    auto it = apiRegistry_.find(fullName);
    if (it != apiRegistry_.end()) {
        it->second.callCount++;
        if (it->second.status == CompatStatus::UNSUPPORTED) {
            result.success = false;
            result.error = "Unsupported API: " + fullName;
            result.severity = CompatSeverity::HARD_FAIL;
            it->second.failCount++;
            lastError_ = result.error;
            return result;
        }
        if (it->second.status == CompatStatus::STUB) {
            result.success = true;
            result.value = Value::Nil();
            lastError_.clear();
            return result;  // No-op for stubs
        }
    }

    const auto callResult = call(fullName, args);
    result = callResult;
    if (!result.success && it != apiRegistry_.end()) {
        it->second.failCount++;
    }
    return result;
}

std::optional<Value> QuickJSContext::getGlobal(const std::string& name) {
    assert(impl_ != nullptr);
    
    if (!impl_->initialized) {
        return std::nullopt;
    }

    auto it = impl_->globals.find(name);
    if (it == impl_->globals.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool QuickJSContext::setGlobal(const std::string& name, const Value& value) {
    assert(impl_ != nullptr);
    
    if (!impl_->initialized) {
        return false;
    }

    impl_->globals[name] = value;
    return true;
}

bool QuickJSContext::registerFunction(const std::string& name, HostFunction fn) {
    assert(impl_ != nullptr);
    if (name.empty() || !fn) {
        return false;
    }
    if (name.find(kFailRegisterFunctionMarker) != std::string::npos) {
        return false;
    }
    impl_->hostFunctions[name] = std::move(fn);
    return true;
}

bool QuickJSContext::registerObject(const std::string& name, 
                                     const std::vector<MethodDef>& methods) {
    assert(impl_ != nullptr);
    
    for (const auto& method : methods) {
        if (method.name.empty() || !method.fn) {
            continue;
        }
        std::string fullName = name + "." + method.name;
        impl_->hostFunctions[fullName] = method.fn;
        
        // Register API status
        registerAPIStatus(fullName, method.status, method.deviationNote);
    }
    
    return true;
}

CompatBudget QuickJSContext::getBudgetStatus() const {
    return budget_;
}

void QuickJSContext::resetBudgetCounters() {
    budget_.exceeded_cpu = false;
    budget_.exceeded_memory = false;
    if (impl_) {
        impl_->cpuUsedUs = 0;
    }
}

bool QuickJSContext::isMemoryExceeded() const {
    if (!config_.enableMemoryLimit) {
        return false;
    }
    return getHeapSize() > static_cast<size_t>(config_.memoryLimitMB) * 1024 * 1024;
}

bool QuickJSContext::isCPUExceeded() const {
    if (!config_.enableCPUBudget) {
        return false;
    }
    return impl_ && impl_->cpuUsedUs > budget_.cpu_slice_us;
}

void QuickJSContext::runGC() {
    assert(impl_ != nullptr);
    // TODO: JS_RunGC(rt);
}

size_t QuickJSContext::getHeapSize() const {
    return impl_ ? impl_->heapSize : 0;
}

std::string QuickJSContext::getLastError() const {
    return lastError_;
}

void QuickJSContext::clearLastError() {
    lastError_.clear();
}

void QuickJSContext::setModuleLoader(ModuleLoader loader) {
    moduleLoader_ = std::move(loader);
}

void QuickJSContext::registerAPIStatus(const std::string& apiName, 
                                        CompatStatus status,
                                        const std::string& deviationNote) {
    APIStatus apiStatus;
    apiStatus.apiName = apiName;
    apiStatus.status = status;
    apiStatus.deviationNote = deviationNote;
    apiRegistry_[apiName] = apiStatus;
}

QuickJSContext::APIStatus QuickJSContext::getAPIStatus(const std::string& apiName) const {
    auto it = apiRegistry_.find(apiName);
    if (it != apiRegistry_.end()) {
        return it->second;
    }
    // Return default status for unknown APIs
    APIStatus unknown;
    unknown.apiName = apiName;
    unknown.status = CompatStatus::UNSUPPORTED;
    return unknown;
}

std::vector<QuickJSContext::APIStatus> QuickJSContext::getAllAPIStatuses() const {
    std::vector<APIStatus> result;
    result.reserve(apiRegistry_.size());
    for (const auto& [name, status] : apiRegistry_) {
        result.push_back(status);
    }
    // Sort by name for deterministic output
    std::sort(result.begin(), result.end(), 
              [](const APIStatus& a, const APIStatus& b) {
                  return a.apiName < b.apiName;
              });
    return result;
}

// ============================================================================
// QuickJSRuntime Implementation
// ============================================================================

struct QuickJSRuntime::Impl {
    bool initialized = false;
    uint32_t nextContextId = 1;
    std::unordered_map<uint32_t, std::unique_ptr<QuickJSContext>> contexts;
    std::unordered_map<std::string, uint32_t> pluginToContextId;
};

QuickJSRuntime::QuickJSRuntime()
    : impl_(std::make_unique<Impl>())
{
}

QuickJSRuntime::~QuickJSRuntime() = default;

bool QuickJSRuntime::initialize() {
    assert(impl_ != nullptr);
    impl_->initialized = true;
    return true;
}

std::optional<uint32_t> QuickJSRuntime::createContext(const std::string& pluginId,
                                                       const QuickJSConfig& config) {
    assert(impl_ != nullptr);
    
    if (!impl_->initialized) {
        return std::nullopt;
    }

    if (pluginId.find(kFailContextInitMarker) != std::string::npos) {
        return std::nullopt;
    }
    
    // Check if plugin already has a context
    auto it = impl_->pluginToContextId.find(pluginId);
    if (it != impl_->pluginToContextId.end()) {
        return it->second;
    }
    
    auto context = std::make_unique<QuickJSContext>();
    if (!context->initialize(config)) {
        return std::nullopt;
    }
    
    uint32_t id = impl_->nextContextId++;
    impl_->contexts[id] = std::move(context);
    impl_->pluginToContextId[pluginId] = id;
    
    return id;
}

QuickJSContext* QuickJSRuntime::getContext(uint32_t contextId) {
    assert(impl_ != nullptr);
    auto it = impl_->contexts.find(contextId);
    return it != impl_->contexts.end() ? it->second.get() : nullptr;
}

const QuickJSContext* QuickJSRuntime::getContext(uint32_t contextId) const {
    assert(impl_ != nullptr);
    auto it = impl_->contexts.find(contextId);
    return it != impl_->contexts.end() ? it->second.get() : nullptr;
}

void QuickJSRuntime::destroyContext(uint32_t contextId) {
    assert(impl_ != nullptr);
    
    auto it = impl_->contexts.find(contextId);
    if (it != impl_->contexts.end()) {
        // Remove from plugin mapping
        for (auto mapIt = impl_->pluginToContextId.begin();
             mapIt != impl_->pluginToContextId.end(); ) {
            if (mapIt->second == contextId) {
                mapIt = impl_->pluginToContextId.erase(mapIt);
            } else {
                ++mapIt;
            }
        }
        impl_->contexts.erase(it);
    }
}

std::optional<uint32_t> QuickJSRuntime::getContextId(const std::string& pluginId) const {
    assert(impl_ != nullptr);
    auto it = impl_->pluginToContextId.find(pluginId);
    return it != impl_->pluginToContextId.end() 
           ? std::optional<uint32_t>(it->second) 
           : std::nullopt;
}

CompatBudget QuickJSRuntime::getAggregateBudgetStatus() const {
    assert(impl_ != nullptr);
    
    CompatBudget aggregate;
    uint32_t cpuUsed = 0;
    size_t memoryUsed = 0;
    
    for (const auto& [id, context] : impl_->contexts) {
        auto budget = context->getBudgetStatus();
        if (budget.exceeded_cpu) {
            aggregate.exceeded_cpu = true;
        }
        if (budget.exceeded_memory) {
            aggregate.exceeded_memory = true;
        }
        cpuUsed += budget.cpu_slice_us;
        memoryUsed += context->getHeapSize();
    }
    
    // Check against total limits
    if (cpuUsed > aggregate.cpu_slice_us * impl_->contexts.size()) {
        aggregate.exceeded_cpu = true;
    }
    
    return aggregate;
}

void QuickJSRuntime::runGCAll() {
    assert(impl_ != nullptr);
    for (auto& [id, context] : impl_->contexts) {
        context->runGC();
    }
}

size_t QuickJSRuntime::getTotalHeapSize() const {
    assert(impl_ != nullptr);
    size_t total = 0;
    for (const auto& [id, context] : impl_->contexts) {
        total += context->getHeapSize();
    }
    return total;
}

std::unordered_map<std::string, QuickJSContext::APIStatus> 
QuickJSRuntime::getAllAPIStatuses() const {
    assert(impl_ != nullptr);
    
    std::unordered_map<std::string, QuickJSContext::APIStatus> all;
    
    for (const auto& [id, context] : impl_->contexts) {
        auto statuses = context->getAllAPIStatuses();
        for (const auto& status : statuses) {
            // Aggregate call counts across contexts
            auto it = all.find(status.apiName);
            if (it != all.end()) {
                it->second.callCount += status.callCount;
                it->second.failCount += status.failCount;
            } else {
                all[status.apiName] = status;
            }
        }
    }
    
    return all;
}

} // namespace compat
} // namespace urpg
