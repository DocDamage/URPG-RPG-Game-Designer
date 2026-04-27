// Unit tests for the QuickJS compat runtime contract kernel
// Phase 2 - Compat Layer
//
// These tests verify live QuickJS execution, fixture directive compatibility,
// API surface registration, and budget tracking.

#include "runtimes/compat_js/quickjs_runtime.h"
#include <catch2/catch_test_macros.hpp>
#include <algorithm>

using namespace urpg::compat;

TEST_CASE("QuickJSContext initializes with default config", "[compat][quickjs]") {
    QuickJSContext ctx;
    QuickJSConfig config;

    REQUIRE(ctx.initialize(config));
}

TEST_CASE("QuickJSContext eval executes real JavaScript expressions", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    auto result = ctx.eval("1 + 1", "test.js");

    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 2);
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

TEST_CASE("QuickJSContext call dispatches registered host function", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    REQUIRE(ctx.registerFunction("sumArgs", [](const std::vector<urpg::Value>& args) -> urpg::Value {
        int64_t total = 0;
        for (const auto& arg : args) {
            if (const auto* integer = std::get_if<int64_t>(&arg.v)) {
                total += *integer;
            }
        }
        return urpg::Value::Int(total);
    }));

    const auto result = ctx.call("sumArgs", {urpg::Value::Int(2), urpg::Value::Int(5)});
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 7);
}

TEST_CASE("QuickJSContext call fails for unknown function", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto result = ctx.call("missingFn", {});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::SOFT_FAIL);
}

TEST_CASE("QuickJSContext registers object with methods", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    std::vector<QuickJSContext::MethodDef> methods;
    methods.push_back({"method1", [](const std::vector<urpg::Value>&) -> urpg::Value { return urpg::Value::Nil(); },
                       CompatStatus::FULL});
    methods.push_back({"method2", [](const std::vector<urpg::Value>&) -> urpg::Value { return urpg::Value::Int(10); },
                       CompatStatus::PARTIAL, "Minor deviation"});

    REQUIRE(ctx.registerObject("TestObject", methods));
}

TEST_CASE("QuickJSContext callMethod invokes registered object method", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    std::vector<QuickJSContext::MethodDef> methods;
    methods.push_back({"mul2",
                       [](const std::vector<urpg::Value>& args) -> urpg::Value {
                           if (!args.empty()) {
                               if (const auto* integer = std::get_if<int64_t>(&args[0].v)) {
                                   return urpg::Value::Int(*integer * 2);
                               }
                           }
                           return urpg::Value::Int(0);
                       },
                       CompatStatus::FULL});
    REQUIRE(ctx.registerObject("MathObj", methods));

    const auto result = ctx.callMethod("MathObj", "mul2", {urpg::Value::Int(21)});
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 42);

    const auto status = ctx.getAPIStatus("MathObj.mul2");
    REQUIRE(status.callCount == 1);
    REQUIRE(status.failCount == 0);
}

TEST_CASE("QuickJSContext stub API calls emit diagnostics instead of silent no-op", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    std::vector<QuickJSContext::MethodDef> methods;
    methods.push_back({"optionalHook",
                       [](const std::vector<urpg::Value>&) -> urpg::Value {
                           return urpg::Value::Int(99);
                       },
                       CompatStatus::STUB, "Optional plugin hook is not release-required."});
    REQUIRE(ctx.registerObject("OptionalCompat", methods));

    const auto result = ctx.callMethod("OptionalCompat", "optionalHook", {});
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<std::monostate>(result.value.v));

    const auto diagnostics = ctx.getRuntimeDiagnostics();
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.operation == "quickjs_stub_api_call" &&
               diagnostic.message == "Stub API called: OptionalCompat.optionalHook" &&
               diagnostic.sourceLocation == "OptionalCompat.optionalHook";
    }));
}

TEST_CASE("QuickJSContext live JS can call registered host functions", "[compat][quickjs][runtime]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    REQUIRE(ctx.registerFunction("nativeAdd", [](const std::vector<urpg::Value>& args) -> urpg::Value {
        int64_t total = 0;
        for (const auto& arg : args) {
            if (const auto* integer = std::get_if<int64_t>(&arg.v)) {
                total += *integer;
            }
        }
        return urpg::Value::Int(total);
    }));

    const auto result = ctx.eval("nativeAdd(10, 32);", "host_call.js");
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 42);
}

TEST_CASE("QuickJSContext live JS object methods are callable from C++", "[compat][quickjs][runtime]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto evalResult = ctx.eval(
        "globalThis.Plugin = { run: function(a, b) { return { sum: a + b, ok: true }; } };", "plugin_object.js");
    REQUIRE(evalResult.success);

    const auto result = ctx.callMethod("Plugin", "run", {urpg::Value::Int(12), urpg::Value::Int(30)});
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<urpg::Object>(result.value.v));
    const auto& object = std::get<urpg::Object>(result.value.v);
    REQUIRE(std::get<int64_t>(object.at("sum").v) == 42);
    REQUIRE(std::get<bool>(object.at("ok").v));
}

TEST_CASE("QuickJSContext global set/get roundtrip", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    urpg::Value answer = urpg::Value::Int(42);
    REQUIRE(ctx.setGlobal("answer", answer));

    const auto found = ctx.getGlobal("answer");
    REQUIRE(found.has_value());
    REQUIRE(std::holds_alternative<int64_t>(found->v));
    REQUIRE(std::get<int64_t>(found->v) == 42);

    const auto missing = ctx.getGlobal("missing");
    REQUIRE_FALSE(missing.has_value());
}

TEST_CASE("QuickJSContext eval binds directive exports", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const std::string code = R"(// @urpg-export countArgs arg_count
// @urpg-export secondArg arg 1
// @urpg-export fixedValue const "ready")";
    const auto evalResult = ctx.eval(code, "fixture_plugin.js");
    REQUIRE(evalResult.success);

    const auto countResult = ctx.call("countArgs", {urpg::Value::Int(1), urpg::Value::Int(2)});
    REQUIRE(countResult.success);
    REQUIRE(std::holds_alternative<int64_t>(countResult.value.v));
    REQUIRE(std::get<int64_t>(countResult.value.v) == 2);

    const auto argResult = ctx.call("secondArg", {urpg::Value::Int(7), urpg::Value::Int(42)});
    REQUIRE(argResult.success);
    REQUIRE(std::holds_alternative<int64_t>(argResult.value.v));
    REQUIRE(std::get<int64_t>(argResult.value.v) == 42);

    const auto constResult = ctx.call("fixedValue", {});
    REQUIRE(constResult.success);
    REQUIRE(std::holds_alternative<std::string>(constResult.value.v));
    REQUIRE(std::get<std::string>(constResult.value.v) == "ready");
}

TEST_CASE("QuickJSContext eval supports explicit failure directive", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto result = ctx.eval(R"(// @urpg-fail-eval simulated fixture eval failure)", "fixture_failure.js");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::HARD_FAIL);
    REQUIRE(result.error == "simulated fixture eval failure");
    REQUIRE(result.sourceLocation == "fixture_failure.js:1");
}

TEST_CASE("QuickJSContext eval supports explicit call failure directive", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto evalResult =
        ctx.eval(R"(// @urpg-fail-call brokenRuntime simulated fixture runtime failure)", "fixture_runtime_failure.js");
    REQUIRE(evalResult.success);

    const auto callResult = ctx.call("brokenRuntime", {});
    REQUIRE_FALSE(callResult.success);
    REQUIRE(callResult.severity == CompatSeverity::HARD_FAIL);
    REQUIRE(callResult.error == "Host function error: simulated fixture runtime failure");
}

TEST_CASE("QuickJSContext call marks unknown host exceptions as crash prevented", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    REQUIRE(ctx.registerFunction("unknownThrow", [](const std::vector<urpg::Value>&) -> urpg::Value { throw 7; }));

    const auto callResult = ctx.call("unknownThrow", {});
    REQUIRE_FALSE(callResult.success);
    REQUIRE(callResult.severity == CompatSeverity::CRASH_PREVENTED);
    REQUIRE(callResult.error == "Unknown host function error");
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

TEST_CASE("QuickJSContext module loader delegates through loader hook", "[compat][quickjs]") {
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

    ctx.setModuleLoader([](const std::string&) -> std::optional<std::string> { return std::nullopt; });

    auto result = ctx.evalModule("nonexistent");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::SOFT_FAIL);
}

TEST_CASE("QuickJSContext evalModule propagates module eval failure", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    ctx.setModuleLoader([](const std::string& moduleId) -> std::optional<std::string> {
        if (moduleId == "broken_module") {
            return "// @urpg-fail-eval module fixture failure";
        }
        return std::nullopt;
    });

    const auto result = ctx.evalModule("broken_module");
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::HARD_FAIL);
    REQUIRE(result.error == "module fixture failure");
    REQUIRE(result.sourceLocation == "broken_module:1");
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

TEST_CASE("QuickJSRuntime createContext supports deterministic fixture init failure marker", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());

    const auto failedContext = runtime.createContext("broken__urpg_fail_context_init__plugin", QuickJSConfig{});
    REQUIRE_FALSE(failedContext.has_value());
    REQUIRE_FALSE(runtime.getContextId("broken__urpg_fail_context_init__plugin").has_value());
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
    REQUIRE(total > 0);
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

TEST_CASE("MethodDef default status is STUB to prevent silent status inflation", "[compat][quickjs]") {
    QuickJSContext::MethodDef def;
    REQUIRE(def.status == CompatStatus::STUB);
}

TEST_CASE("QuickJSContext eval returns real arithmetic result", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    auto result = ctx.eval("1 + 1", "test.js");

    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 2);
}

TEST_CASE("QuickJSContext eval of plain JS without directives executes globals", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    auto result = ctx.eval("globalThis.foo = function() { return 42; }; foo();", "plain.js");

    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 42);

    auto callResult = ctx.call("foo", {});
    REQUIRE(callResult.success);
    REQUIRE(std::get<int64_t>(callResult.value.v) == 42);
}

TEST_CASE("QuickJSContext module loader evaluates real JS source", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    ctx.setModuleLoader(
        [](const std::string&) -> std::optional<std::string> { return "globalThis.moduleAnswer = 42; moduleAnswer;"; });

    auto result = ctx.evalModule("fixture_module");
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 42);
}

TEST_CASE("QuickJSContext call counts are runtime API status features", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    ctx.registerAPIStatus("HarnessAPI.testCall", CompatStatus::STUB);

    // Calling a stub method increments callCount at the harness level
    auto result = ctx.callMethod("HarnessAPI", "testCall", {});
    REQUIRE(result.success);

    auto status = ctx.getAPIStatus("HarnessAPI.testCall");
    REQUIRE(status.callCount == 1);
    REQUIRE(status.failCount == 0);
}

TEST_CASE("QuickJSContext CPU budget is enforced at harness level", "[compat][quickjs]") {
    QuickJSContext ctx;
    QuickJSConfig config;
    config.enableCPUBudget = true;
    config.cpuBudgetUs = 0; // Force immediate exceeded

    REQUIRE(ctx.initialize(config));
    REQUIRE(ctx.registerFunction("dummy",
                                 [](const std::vector<urpg::Value>&) -> urpg::Value { return urpg::Value::Nil(); }));

    auto result = ctx.call("dummy", {});
    REQUIRE_FALSE(result.success);
    REQUIRE(result.error == "CPU budget exceeded");
    REQUIRE(ctx.isCPUExceeded());
}

TEST_CASE("QuickJSContext compat timers are deterministic and clearable", "[compat][quickjs][async]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto setup = ctx.eval(R"JS(
        globalThis.timerHits = 0;
        const timeoutId = setTimeout(function(delta) { globalThis.timerHits += delta; }, 0, 2);
        clearTimeout(timeoutId);
        setTimeout(function(delta) { globalThis.timerHits += delta; }, 0, 5);
        __urpgPendingTimerCount();
    )JS",
                                "timer_cleanup.js");
    REQUIRE(setup.success);
    REQUIRE(std::holds_alternative<int64_t>(setup.value.v));
    REQUIRE(std::get<int64_t>(setup.value.v) == 1);

    const auto run = ctx.eval("__urpgRunDueTimers();", "timer_cleanup.js");
    REQUIRE(run.success);
    REQUIRE(std::holds_alternative<int64_t>(run.value.v));
    REQUIRE(std::get<int64_t>(run.value.v) == 1);

    const auto hits = ctx.getGlobal("timerHits");
    REQUIRE(hits.has_value());
    REQUIRE(std::holds_alternative<int64_t>(hits->v));
    REQUIRE(std::get<int64_t>(hits->v) == 5);

    const auto remaining = ctx.eval("__urpgPendingTimerCount();", "timer_cleanup.js");
    REQUIRE(remaining.success);
    REQUIRE(std::get<int64_t>(remaining.value.v) == 0);
}

TEST_CASE("QuickJSContext compat intervals persist until cleared", "[compat][quickjs][async]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto setup = ctx.eval(R"JS(
        globalThis.intervalHits = 0;
        globalThis.intervalId = setInterval(function() { globalThis.intervalHits += 1; }, 0);
        __urpgRunDueTimers();
        __urpgRunDueTimers();
        clearInterval(globalThis.intervalId);
        __urpgPendingTimerCount();
    )JS",
                                "interval_cleanup.js");
    REQUIRE(setup.success);
    REQUIRE(std::holds_alternative<int64_t>(setup.value.v));
    REQUIRE(std::get<int64_t>(setup.value.v) == 0);

    const auto hits = ctx.getGlobal("intervalHits");
    REQUIRE(hits.has_value());
    REQUIRE(std::holds_alternative<int64_t>(hits->v));
    REQUIRE(std::get<int64_t>(hits->v) == 2);
}

TEST_CASE("QuickJSContext compat event listeners can be removed", "[compat][quickjs][async]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto result = ctx.eval(R"JS(
        globalThis.eventHits = 0;
        function onReady(event) {
          globalThis.eventHits += event.amount;
        }
        addEventListener("ready", onReady);
        dispatchEvent({ type: "ready", amount: 3 });
        removeEventListener("ready", onReady);
        dispatchEvent({ type: "ready", amount: 7 });
        __urpgListenerCount("ready");
    )JS",
                                 "event_listener_cleanup.js");
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 0);

    const auto hits = ctx.getGlobal("eventHits");
    REQUIRE(hits.has_value());
    REQUIRE(std::holds_alternative<int64_t>(hits->v));
    REQUIRE(std::get<int64_t>(hits->v) == 3);
}

TEST_CASE("QuickJSRuntime context teardown drops compat async resources", "[compat][quickjs][async]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());

    const auto firstId = runtime.createContext("async-plugin", QuickJSConfig{});
    REQUIRE(firstId.has_value());
    auto* firstContext = runtime.getContext(*firstId);
    REQUIRE(firstContext != nullptr);
    REQUIRE(firstContext->eval("setTimeout(function() {}, 0); addEventListener('boot', function() {});",
                               "async_teardown.js")
                .success);

    runtime.destroyContext(*firstId);
    REQUIRE(runtime.getContext(*firstId) == nullptr);

    const auto secondId = runtime.createContext("async-plugin", QuickJSConfig{});
    REQUIRE(secondId.has_value());
    auto* secondContext = runtime.getContext(*secondId);
    REQUIRE(secondContext != nullptr);

    const auto timerCount = secondContext->eval("__urpgPendingTimerCount();", "async_teardown.js");
    REQUIRE(timerCount.success);
    REQUIRE(std::get<int64_t>(timerCount.value.v) == 0);

    const auto listenerCount = secondContext->eval("__urpgListenerCount('boot');", "async_teardown.js");
    REQUIRE(listenerCount.success);
    REQUIRE(std::get<int64_t>(listenerCount.value.v) == 0);
}

TEST_CASE("QuickJSContext drains promise jobs and reports unhandled rejections", "[compat][quickjs][async]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto result = ctx.eval(R"JS(
        Promise.resolve().then(function() {
          globalThis.promiseValue = 42;
        });
        Promise.reject("plugin async failure");
        "queued";
    )JS",
                                 "promise_rejection_plugin.js");
    REQUIRE(result.success);

    REQUIRE(ctx.drainPendingJobs() >= 1);

    const auto value = ctx.getGlobal("promiseValue");
    REQUIRE(value.has_value());
    REQUIRE(std::holds_alternative<int64_t>(value->v));
    REQUIRE(std::get<int64_t>(value->v) == 42);

    const auto diagnostics = ctx.getRuntimeDiagnostics();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(diagnostics.back().operation == "quickjs_unhandled_promise_rejection");
    REQUIRE(diagnostics.back().message == "plugin async failure");
    REQUIRE(diagnostics.back().severity == CompatSeverity::HARD_FAIL);
    REQUIRE(diagnostics.back().sourceLocation == "promise_rejection_plugin.js");

    ctx.clearRuntimeDiagnostics();
    REQUIRE(ctx.getRuntimeDiagnostics().empty());
}
