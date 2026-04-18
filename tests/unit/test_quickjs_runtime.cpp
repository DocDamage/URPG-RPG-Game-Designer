// Unit tests for the QuickJS compat harness contract kernel
// Phase 2 - Compat Layer
//
// These tests verify the fixture-backed harness behavior without requiring
// actual QuickJS linking. They validate API surface, directive semantics, and budget tracking.

#include "runtimes/compat_js/quickjs_runtime.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/audio_manager.h"
#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

TEST_CASE("QuickJSContext harness initializes with default config", "[compat][quickjs]") {
    QuickJSContext ctx;
    QuickJSConfig config;
    
    REQUIRE(ctx.initialize(config));
}

TEST_CASE("QuickJSContext eval returns harness success for non-directive input", "[compat][quickjs]") {
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
    methods.push_back({"method1", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Nil();
    }, CompatStatus::FULL});
    methods.push_back({"method2", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Int(10);
    }, CompatStatus::PARTIAL, "Minor deviation"});
    
    REQUIRE(ctx.registerObject("TestObject", methods));
}

TEST_CASE("QuickJSContext callMethod invokes registered object method", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    std::vector<QuickJSContext::MethodDef> methods;
    methods.push_back({"mul2", [](const std::vector<urpg::Value>& args) -> urpg::Value {
        if (!args.empty()) {
            if (const auto* integer = std::get_if<int64_t>(&args[0].v)) {
                return urpg::Value::Int(*integer * 2);
            }
        }
        return urpg::Value::Int(0);
    }, CompatStatus::FULL});
    REQUIRE(ctx.registerObject("MathObj", methods));

    const auto result = ctx.callMethod("MathObj", "mul2", {urpg::Value::Int(21)});
    REQUIRE(result.success);
    REQUIRE(std::holds_alternative<int64_t>(result.value.v));
    REQUIRE(std::get<int64_t>(result.value.v) == 42);

    const auto status = ctx.getAPIStatus("MathObj.mul2");
    REQUIRE(status.callCount == 1);
    REQUIRE(status.failCount == 0);
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

    const auto result = ctx.eval(
        R"(// @urpg-fail-eval simulated fixture eval failure)",
        "fixture_failure.js"
    );
    REQUIRE_FALSE(result.success);
    REQUIRE(result.severity == CompatSeverity::HARD_FAIL);
    REQUIRE(result.error == "simulated fixture eval failure");
    REQUIRE(result.sourceLocation == "fixture_failure.js:1");
}

TEST_CASE("QuickJSContext eval supports explicit call failure directive", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    const auto evalResult = ctx.eval(
        R"(// @urpg-fail-call brokenRuntime simulated fixture runtime failure)",
        "fixture_runtime_failure.js"
    );
    REQUIRE(evalResult.success);

    const auto callResult = ctx.call("brokenRuntime", {});
    REQUIRE_FALSE(callResult.success);
    REQUIRE(callResult.severity == CompatSeverity::HARD_FAIL);
    REQUIRE(callResult.error == "Host function error: simulated fixture runtime failure");
}

TEST_CASE("QuickJSContext call marks unknown host exceptions as crash prevented", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    REQUIRE(ctx.registerFunction(
        "unknownThrow",
        [](const std::vector<urpg::Value>&) -> urpg::Value {
            throw 7;
        }
    ));

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

TEST_CASE("QuickJSContext harness module loader delegates through loader hook", "[compat][quickjs]") {
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

TEST_CASE("QuickJSRuntime harness initializes", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
}

TEST_CASE("QuickJSRuntime harness creates isolated contexts", "[compat][quickjs]") {
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

TEST_CASE("QuickJSRuntime createContext supports deterministic fixture init failure marker",
          "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());

    const auto failedContext =
        runtime.createContext("broken__urpg_fail_context_init__plugin", QuickJSConfig{});
    REQUIRE_FALSE(failedContext.has_value());
    REQUIRE_FALSE(
        runtime.getContextId("broken__urpg_fail_context_init__plugin").has_value()
    );
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
    // Each stub context has a 1KB base overhead
    REQUIRE(total == 2048);
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

TEST_CASE("QuickJSContext runGC reduces simulated heap", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    // Register functions and set globals to build up heap
    REQUIRE(ctx.registerFunction("fn1", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Nil();
    }));
    REQUIRE(ctx.registerFunction("fn2", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Nil();
    }));
    REQUIRE(ctx.setGlobal("alive", urpg::Value::Int(1)));
    REQUIRE(ctx.setGlobal("dead", urpg::Value::Nil()));
    REQUIRE(ctx.setGlobal("alsoDead", urpg::Value::Nil()));
    
    const size_t heapBeforeGC = ctx.getHeapSize();
    REQUIRE(heapBeforeGC > 0);
    
    // First GC should remove Nil globals
    ctx.runGC();
    const size_t heapAfterFirstGC = ctx.getHeapSize();
    REQUIRE(heapAfterFirstGC <= heapBeforeGC);
    
    // Set another global to Nil and GC again
    REQUIRE(ctx.setGlobal("alive", urpg::Value::Nil()));
    ctx.runGC();
    const size_t heapAfterSecondGC = ctx.getHeapSize();
    REQUIRE(heapAfterSecondGC <= heapAfterFirstGC);
    
    // Verify unreachable globals were collected
    REQUIRE(ctx.getGlobal("dead") == std::nullopt);
    REQUIRE(ctx.getGlobal("alsoDead") == std::nullopt);
    REQUIRE(ctx.getGlobal("alive") == std::nullopt);
}

TEST_CASE("QuickJSContext heap size grows with operations", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));
    
    const size_t initialHeap = ctx.getHeapSize();
    REQUIRE(initialHeap == 1024);
    
    auto evalResult = ctx.eval("// some code\n1 + 1;", "test.js");
    REQUIRE(evalResult.success);
    const size_t heapAfterEval = ctx.getHeapSize();
    REQUIRE(heapAfterEval > initialHeap);
    
    REQUIRE(ctx.registerFunction("growFn", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Nil();
    }));
    const size_t heapAfterRegister = ctx.getHeapSize();
    REQUIRE(heapAfterRegister > heapAfterEval);
}

TEST_CASE("QuickJSRuntime runGCAll runs GC on all contexts", "[compat][quickjs]") {
    QuickJSRuntime runtime;
    REQUIRE(runtime.initialize());
    
    auto id1 = runtime.createContext("plugin1", QuickJSConfig{});
    auto id2 = runtime.createContext("plugin2", QuickJSConfig{});
    REQUIRE(id1.has_value());
    REQUIRE(id2.has_value());
    
    auto* ctx1 = runtime.getContext(*id1);
    auto* ctx2 = runtime.getContext(*id2);
    REQUIRE(ctx1 != nullptr);
    REQUIRE(ctx2 != nullptr);
    
    // Build up heap in both contexts
    REQUIRE(ctx1->registerFunction("a", [](const std::vector<urpg::Value>&) -> urpg::Value {
        return urpg::Value::Nil();
    }));
    REQUIRE(ctx2->setGlobal("x", urpg::Value::Nil()));
    
    const size_t totalBefore = runtime.getTotalHeapSize();
    REQUIRE(totalBefore > 0);
    
    // Should not crash and should collect unreachable state
    runtime.runGCAll();
    
    const size_t totalAfter = runtime.getTotalHeapSize();
    REQUIRE(totalAfter <= totalBefore);
    REQUIRE(ctx1->getHeapSize() > 0);
    REQUIRE(ctx2->getHeapSize() > 0);
}

TEST_CASE("QuickJSContext memory budget triggers when heap exceeds limit", "[compat][quickjs]") {
    QuickJSContext ctx;
    QuickJSConfig config;
    config.memoryLimitMB = 0; // Very small limit
    config.enableMemoryLimit = true;
    REQUIRE(ctx.initialize(config));
    REQUIRE(ctx.isMemoryExceeded());
    
    QuickJSContext ctx2;
    QuickJSConfig config2;
    config2.memoryLimitMB = 64; // Default generous limit
    config2.enableMemoryLimit = true;
    REQUIRE(ctx2.initialize(config2));
    REQUIRE_FALSE(ctx2.isMemoryExceeded());
}

TEST_CASE("QuickJSContext DataManager API roundtrip", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    DataManager::instance().clearDatabase();
    DataManager::instance().setupNewGame();

    DataManager::registerAPI(ctx);

    const auto goldResult = ctx.callMethod("DataManager", "getGold", {});
    REQUIRE(goldResult.success);
    REQUIRE(std::holds_alternative<int64_t>(goldResult.value.v));
    REQUIRE(std::get<int64_t>(goldResult.value.v) == DataManager::instance().getGold());

    const auto setSwitchResult = ctx.callMethod("DataManager", "setSwitch", {urpg::Value::Int(5), urpg::Value::Int(1)});
    REQUIRE(setSwitchResult.success);
    REQUIRE(DataManager::instance().getSwitch(5) == true);

    const auto getSwitchResult = ctx.callMethod("DataManager", "getSwitch", {urpg::Value::Int(5)});
    REQUIRE(getSwitchResult.success);
    REQUIRE(std::holds_alternative<int64_t>(getSwitchResult.value.v));
    REQUIRE(std::get<int64_t>(getSwitchResult.value.v) == 1);
}

TEST_CASE("QuickJSContext BattleManager API roundtrip", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    DataManager::instance().clearDatabase();
    BattleManager::registerAPI(ctx);

    const auto phaseResult = ctx.callMethod("BattleManager", "getPhase", {});
    REQUIRE(phaseResult.success);
    REQUIRE(std::holds_alternative<int64_t>(phaseResult.value.v));
    REQUIRE(std::get<int64_t>(phaseResult.value.v) == static_cast<int32_t>(BattleManager::instance().getPhase()));

    const auto setupResult = ctx.callMethod("BattleManager", "setup", {urpg::Value::Int(1)});
    REQUIRE(setupResult.success);
    REQUIRE(BattleManager::instance().getPhase() == BattlePhase::INIT);
}

TEST_CASE("QuickJSContext AudioManager API roundtrip", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    AudioManager::registerAPI(ctx);

    const auto playResult = ctx.callMethod("AudioManager", "playBgm", {urpg::Value::Str("Battle1"), urpg::Value::Int(80), urpg::Value::Int(100)});
    REQUIRE(playResult.success);
    REQUIRE(AudioManager::instance().getCurrentBgm().name == "Battle1");
}

TEST_CASE("QuickJSContext $gameParty API roundtrip", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    DataManager::instance().clearDatabase();
    DataManager::instance().setupNewGame();
    DataManager::registerAPI(ctx);

    // Test members
    const auto membersResult = ctx.callMethod("$gameParty", "members", {});
    REQUIRE(membersResult.success);
    REQUIRE(std::holds_alternative<urpg::Array>(membersResult.value.v));
    const auto& membersArr = std::get<urpg::Array>(membersResult.value.v);
    REQUIRE(membersArr.empty());

    // Test addActor
    const auto addResult = ctx.callMethod("$gameParty", "addActor", {urpg::Value::Int(1)});
    REQUIRE(addResult.success);
    REQUIRE(DataManager::instance().getGameParty().exists(1));

    // Test size
    const auto sizeResult = ctx.callMethod("$gameParty", "size", {});
    REQUIRE(sizeResult.success);
    REQUIRE(std::holds_alternative<int64_t>(sizeResult.value.v));
    REQUIRE(std::get<int64_t>(sizeResult.value.v) == 1);

    // Test gold
    const auto goldResult = ctx.callMethod("$gameParty", "gold", {});
    REQUIRE(goldResult.success);
    REQUIRE(std::get<int64_t>(goldResult.value.v) == 0);

    // Test setGold / gainGold
    ctx.callMethod("$gameParty", "setGold", {urpg::Value::Int(500)});
    REQUIRE(DataManager::instance().getGameParty().gold() == 500);

    ctx.callMethod("$gameParty", "gainGold", {urpg::Value::Int(250)});
    REQUIRE(DataManager::instance().getGameParty().gold() == 750);

    // Test items
    ctx.callMethod("$gameParty", "gainItem", {urpg::Value::Int(1), urpg::Value::Int(5)});
    REQUIRE(DataManager::instance().getGameParty().numItems(1) == 5);

    const auto itemResult = ctx.callMethod("$gameParty", "numItems", {urpg::Value::Int(1)});
    REQUIRE(itemResult.success);
    REQUIRE(std::get<int64_t>(itemResult.value.v) == 5);

    // Test steps
    ctx.callMethod("$gameParty", "increaseSteps", {urpg::Value::Int(10)});
    REQUIRE(DataManager::instance().getGameParty().steps() == 10);
}

TEST_CASE("QuickJSContext $gameTroop API roundtrip", "[compat][quickjs]") {
    QuickJSContext ctx;
    REQUIRE(ctx.initialize(QuickJSConfig{}));

    DataManager::instance().clearDatabase();
    DataManager::instance().setupNewGame();

    // Set up mock enemies and troop
    DataManager::instance().addTestEnemy();
    DataManager::instance().addTestEnemy();
    EnemyData* enemy1 = DataManager::instance().getEnemy(1);
    REQUIRE(enemy1 != nullptr);
    enemy1->exp = 30;
    enemy1->gold = 20;
    EnemyData* enemy2 = DataManager::instance().getEnemy(2);
    REQUIRE(enemy2 != nullptr);
    enemy2->exp = 50;
    enemy2->gold = 40;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members = {1, 2};
    
    // Sync GameTroop runtime state (normally done by BattleManager::setup)
    DataManager::instance().getGameTroop().setMembers(troop.members);

    DataManager::registerAPI(ctx);

    // Test totalExp / totalGold
    const auto expResult = ctx.callMethod("$gameTroop", "totalExp", {});
    REQUIRE(expResult.success);
    REQUIRE(std::get<int64_t>(expResult.value.v) == 80);

    const auto goldResult = ctx.callMethod("$gameTroop", "totalGold", {});
    REQUIRE(goldResult.success);
    REQUIRE(std::get<int64_t>(goldResult.value.v) == 60);

    // Test members
    const auto membersResult = ctx.callMethod("$gameTroop", "members", {});
    REQUIRE(membersResult.success);
    REQUIRE(std::holds_alternative<urpg::Array>(membersResult.value.v));
    const auto& membersArr = std::get<urpg::Array>(membersResult.value.v);
    REQUIRE(membersArr.size() == 2);

    // Test size
    const auto sizeResult = ctx.callMethod("$gameTroop", "size", {});
    REQUIRE(sizeResult.success);
    REQUIRE(std::get<int64_t>(sizeResult.value.v) == 2);

    // Test isEmpty
    const auto emptyResult = ctx.callMethod("$gameTroop", "isEmpty", {});
    REQUIRE(emptyResult.success);
    REQUIRE(std::get<int64_t>(emptyResult.value.v) == 0);
}

TEST_CASE("BattleManager setup syncs GameTroop", "[compat][battle]") {
    DataManager::instance().clearDatabase();
    DataManager::instance().setupNewGame();

    DataManager::instance().addTestEnemy();
    DataManager::instance().addTestEnemy();
    EnemyData* enemy1 = DataManager::instance().getEnemy(1);
    REQUIRE(enemy1 != nullptr);
    enemy1->exp = 10;
    enemy1->gold = 5;

    TroopData& troop = DataManager::instance().addTestTroop();
    troop.members = {1, 2};

    BattleManager::instance().setup(1);

    REQUIRE(DataManager::instance().getGameTroop().size() == 2);
    REQUIRE(DataManager::instance().getGameTroop().members() == std::vector<int32_t>{1, 2});
    REQUIRE(DataManager::instance().getGameTroop().totalExp() == 10);
    REQUIRE(DataManager::instance().getGameTroop().totalGold() == 5);
}
