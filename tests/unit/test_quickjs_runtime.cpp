// Unit tests for QuickJS Runtime Integration Contract Kernel
// Phase 2 - Compat Layer
//
// These tests verify the contract kernel behavior without requiring
// actual QuickJS linking. The stubs validate API surface and budget tracking.

#include "runtimes/compat_js/quickjs_runtime.h"
#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

TEST_CASE("QuickJSContext initializes with default config", "[compat][quickjs]") {
    QuickJSContext ctx;
    QuickJSConfig config;
    
    REQUIRE(ctx.initialize(config));
}

TEST_CASE("QuickJSContext eval returns success for stub", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    auto result = ctx.eval("1 + 1", "test.js");
    
    REQUIRE(result.success);
}

TEST_CASE("QuickJSContext eval fails without initialization", "[compat][quickjs]") {
    QuickJSContext ctx;
    // Not initialized
    
    auto result = ctx.eval("1 + 1", "test.js");
    
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::HARD_FAIL);
}

TEST_CASE("QuickJSContext registers host function", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    bool called = false;
    auto fn = [&called](const std::vector<urpg::Value>&) -> urpg::Value {
        called = true;
        return urpg::Value::Int(42);
    };
    
    REQUIRE(ctx.registerFunction("testFunc", fn));
}

TEST_CASE("QuickJSContext registers object with methods", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    std::vector<QuickJSContext::MethodDef> methods;
    methods.push_back({"method1", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Nil();
    }, CompatStatus::FULL});
    methods.push_back({"method2", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Int(10);
    }, CompatStatus::PARTIAL, "Minor deviation"});
    
    REQUIRE(ctx.registerObject("TestObject", methods));
}

TEST_CASE("QuickJSContext tracks API status", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    ctx.registerAPIStatus("Window_Base.drawText", CompatStatus::FULL);
    ctx.registerAPIStatus("Window_Base.drawActorFace", CompatStatus::PARTIAL, "Scaling differs");
    
    auto status1 = ctx.getAPIStatus("Window_Base.drawText");
    REQUIRE(status1.apiName == "Window_Base.drawText");
    REQUIRE(status1.status == CompatStatus::FULL);
    
    auto status2 = ctx.getAPIStatus("Window_Base.drawActorFace");
    REQUIRE(status2.status == CompatStatus::PARTIAL);
    REQUIRE(status2.deviationNote == "Scaling differs");
}

TEST_CASE("QuickJSContext returns UNSUPPORTED for unknown API", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    auto status = ctx.getAPIStatus("Unknown.API");
    REQUIRE(status.status == CompatStatus::UNSUPPORTED);
}

TEST_CASE("QuickJSContext getAllAPIStatuses returns sorted list", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    ctx.registerAPIStatus("B.method", CompatStatus::FULL);
    ctx.registerAPIStatus("A.method", CompatStatus::FULL);
    ctx.registerAPIStatus("C.method", CompatStatus::FULL);
    
    auto statuses = ctx.getAllAPIStatuses();
    
    REQUIRE(statuses.size() == 3);
    REQUIRE(statuses[0].apiName == "A.method");
    REQUIRE(statuses[1].apiName == "B.method");
    REQUIRE(statuses[2].apiName == "C.method");
}

TEST_CASE("QuickJSContext budget tracking", "[compat][quickjs]") {
    QuickJSContext ctx;
    QuickJSConfig config;
    config.memoryLimitMB = 32;
    config.cpuBudgetUs = 2000;
    
    REQUIRE(ctx.initialize(config));
    
    auto budget = ctx.getBudgetStatus();
    REQUIRE(budget.cpu_slice_us == 2000);
    REQUIRE(budget.memory_limit_mb == 32);
    REQUIRE_FALSE(budget.exceeded_cpu);
    REQUIRE_FALSE(budget.exceeded_memory);
}

TEST_CASE("QuickJSContext resetBudgetCounters clears flags", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    ctx.resetBudgetCounters();
    
    auto budget = ctx.getBudgetStatus();
    REQUIRE_FALSE(budget.exceeded_cpu);
    REQUIRE_FALSE(budget.exceeded_memory);
}

TEST_CASE("QuickJSContext module loader", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    bool loaderCalled = false;
    ctx.setModuleLoader([&loaderCalled](const std::string& moduleId) -> std::optional<std::string> {
        loaderCalled = true;
        if (moduleId == "test") {
            return "export const value = 42;";
        }
        return std::nullopt;
    });
    
    auto result = ctx.evalModule("test");
    REQUIRE(loaderCalled);
}

TEST_CASE("QuickJSContext evalModule fails for missing module", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    ctx.setModuleLoader([](const std::string&) -> std::optional<std::string> {
        return std::nullopt;
    });
    
    auto result = ctx.evalModule("nonexistent");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::SOFT_FAIL);
}

TEST_CASE("QuickJSRuntime initializes", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
}

TEST_CASE("QuickJSRuntime creates isolated contexts", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    auto ctx1 = runtime.createContext("plugin1", QuickJSConfig{});
    auto ctx2 = runtime.createContext("plugin2", QuickJSConfig{});
    
    REQUIRE(ctx1.has_value());
    REQUIRE(ctx2.has_value());
    REQUIRE(ctx1 != ctx2);
}

TEST_CASE("QuickJSRuntime getContext returns valid pointer", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    auto ctxId = runtime.createContext("plugin1", QuickJSConfig{});
    REQUIRE(ctxId.has_value());
    
    auto* ctx = runtime.getContext(*ctxId);
    REQUIRE(ctx != nullptr);
}

TEST_CASE("QuickJSRuntime getContextId by plugin name", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    auto ctxId = runtime.createContext("myPlugin", QuickJSConfig{});
    REQUIRE(ctxId.has_value());
    
    auto found = runtime.getContextId("myPlugin");
    REQUIRE(found.has_value());
    REQUIRE(*found == *ctxId);
}

TEST_CASE("QuickJSRuntime destroyContext removes context", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    auto ctxId = runtime.createContext("plugin1", QuickJSConfig{});
    REQUIRE(ctxId.has_value());
    
    runtime.destroyContext(*ctxId);
    
    auto* ctx = runtime.getContext(*ctxId);
    REQUIRE(ctx == nullptr);
    
    auto found = runtime.getContextId("plugin1");
    REQUIRE_FALSE(found.has_value());
}

TEST_CASE("QuickJSRuntime duplicate createContext returns same ID", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    auto ctx1 = runtime.createContext("plugin1", QuickJSConfig{});
    auto ctx2 = runtime.createContext("plugin1", QuickJSConfig{});
    
    REQUIRE(ctx1 == ctx2);
}

TEST_CASE("QuickJSRuntime aggregate budget status", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    runtime.createContext("plugin1", QuickJSConfig{});
    runtime.createContext("plugin2", QuickJSConfig{});
    
    auto budget = runtime.getAggregateBudgetStatus();
    REQUIRE(budget.cpu_slice_us > 0);
}

TEST_CASE("QuickJSRuntime getTotalHeapSize aggregates", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    runtime.createContext("plugin1", QuickJSConfig{});
    runtime.createContext("plugin2", QuickJSConfig{});
    
    size_t total = runtime.getTotalHeapSize();
    // Stubs return 0 heap size
    REQUIRE(total == 0);
}

TEST_CASE("QuickJSContext is movable", "[compat][quickjs]") {
    QuickJSContext ctx1;
    REQUIRE(ctx1.initialize(QuickJSConfig{}));
    
    QuickJSContext ctx2 = std::move(ctx1);
    // ctx2 should be valid after move
    auto result = ctx2.eval("1", "test.js");
    REQUIRE(result.success);
}

TEST_CASE("CompatSeverity values are ordered", "[compat][quickjs]") {
    // Verify severity ordering for escalation logic
    REQUIRE(static_cast<int>(CompatSeverity::WARN) < static_cast<int>(CompatSeverity::SOFT_FAIL));
    REQUIRE(static_cast<int>(CompatSeverity::SOFT_FAIL) < static_cast<int>(CompatSeverity::HARD_FAIL));
    REQUIRE(static_cast<int>(CompatSeverity::HARD_FAIL) < static_cast<int>(CompatSeverity::CRASH_PREVENTED));
}

TEST_CASE("CompatStatus values are ordered", "[compat][quickjs]") {
    // Verify status ordering for compat reports
    REQUIRE(static_cast<int>(CompatStatus::FULL) < static_cast<int>(CompatStatus::PARTIAL));
    REQUIRE(static_cast<int>(CompatStatus::PARTIAL) < static_cast<int>(CompatStatus::STUB));
    REQUIRE(static_cast<int>(CompatStatus::STUB) < static_cast<int>(CompatStatus::UNSUPPORTED));
}
