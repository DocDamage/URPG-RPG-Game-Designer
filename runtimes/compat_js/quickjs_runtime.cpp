// QuickJS Compat Harness Contract Kernel - Live QuickJS + Fixture Directive Support
// Phase 2 - Compat Layer

#include "quickjs_runtime.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <exception>
#include <nlohmann/json.hpp>
#include <quickjs.h>
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
        ((text.front() == '"' && text.back() == '"') || (text.front() == '\'' && text.back() == '\''))) {
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

Value jsonToBridgeValue(const nlohmann::json& node) {
    if (node.is_null()) {
        return Value::Nil();
    }
    if (node.is_boolean()) {
        Value value;
        value.v = node.get<bool>();
        return value;
    }
    if (node.is_number_integer() || node.is_number_unsigned()) {
        return Value::Int(node.get<int64_t>());
    }
    if (node.is_number_float()) {
        Value value;
        value.v = node.get<double>();
        return value;
    }
    if (node.is_string()) {
        Value value;
        value.v = node.get<std::string>();
        return value;
    }
    if (node.is_array()) {
        Array array;
        array.reserve(node.size());
        for (const auto& item : node) {
            array.push_back(jsonToBridgeValue(item));
        }
        return Value::Arr(std::move(array));
    }

    Object object;
    for (const auto& [key, item] : node.items()) {
        object[key] = jsonToBridgeValue(item);
    }
    return Value::Obj(std::move(object));
}

JSValue bridgeValueToJS(JSContext* ctx, const Value& value) {
    if (std::holds_alternative<std::monostate>(value.v)) {
        return JS_UNDEFINED;
    }
    if (const auto* flag = std::get_if<bool>(&value.v)) {
        return JS_NewBool(ctx, *flag);
    }
    if (const auto* integer = std::get_if<int64_t>(&value.v)) {
        return JS_NewInt64(ctx, *integer);
    }
    if (const auto* real = std::get_if<double>(&value.v)) {
        return JS_NewFloat64(ctx, *real);
    }
    if (const auto* text = std::get_if<std::string>(&value.v)) {
        return JS_NewStringLen(ctx, text->data(), text->size());
    }
    if (const auto* array = std::get_if<Array>(&value.v)) {
        JSValue out = JS_NewArray(ctx);
        for (uint32_t index = 0; index < array->size(); ++index) {
            JS_SetPropertyUint32(ctx, out, index, bridgeValueToJS(ctx, (*array)[index]));
        }
        return out;
    }

    const auto& object = std::get<Object>(value.v);
    JSValue out = JS_NewObject(ctx);
    for (const auto& [key, item] : object) {
        JS_SetPropertyStr(ctx, out, key.c_str(), bridgeValueToJS(ctx, item));
    }
    return out;
}

std::string exceptionToString(JSContext* ctx) {
    JSValue exception = JS_GetException(ctx);
    const char* text = JS_ToCString(ctx, exception);
    std::string out = text ? text : "JavaScript exception";
    if (text) {
        JS_FreeCString(ctx, text);
    }
    JS_FreeValue(ctx, exception);
    constexpr std::string_view internalErrorPrefix = "InternalError: ";
    if (out.rfind(internalErrorPrefix, 0) == 0) {
        out.erase(0, internalErrorPrefix.size());
    }
    return out;
}

Value jsValueToBridgeValue(JSContext* ctx, JSValueConst value) {
    if (JS_IsUndefined(value) || JS_IsNull(value)) {
        return Value::Nil();
    }
    if (JS_IsBool(value)) {
        Value out;
        out.v = static_cast<bool>(JS_ToBool(ctx, value));
        return out;
    }
    if (JS_IsNumber(value)) {
        int64_t integer = 0;
        if (JS_ToInt64(ctx, &integer, value) == 0) {
            double real = 0.0;
            if (JS_ToFloat64(ctx, &real, value) == 0 && std::fabs(real - static_cast<double>(integer)) <= 1e-9) {
                return Value::Int(integer);
            }
        }
        double real = 0.0;
        JS_ToFloat64(ctx, &real, value);
        Value out;
        out.v = real;
        return out;
    }
    if (JS_IsString(value)) {
        const char* text = JS_ToCString(ctx, value);
        Value out;
        out.v = text ? std::string(text) : std::string{};
        if (text) {
            JS_FreeCString(ctx, text);
        }
        return out;
    }

    const bool isArray = JS_IsArray(value);
    if (isArray > 0) {
        Array array;
        JSValue lengthValue = JS_GetPropertyStr(ctx, value, "length");
        uint32_t length = 0;
        JS_ToUint32(ctx, &length, lengthValue);
        JS_FreeValue(ctx, lengthValue);
        array.reserve(length);
        for (uint32_t index = 0; index < length; ++index) {
            JSValue item = JS_GetPropertyUint32(ctx, value, index);
            array.push_back(jsValueToBridgeValue(ctx, item));
            JS_FreeValue(ctx, item);
        }
        return Value::Arr(std::move(array));
    }

    JSPropertyEnum* properties = nullptr;
    uint32_t propertyCount = 0;
    if (JS_GetOwnPropertyNames(ctx, &properties, &propertyCount, value, JS_GPN_STRING_MASK | JS_GPN_ENUM_ONLY) == 0) {
        Object object;
        for (uint32_t index = 0; index < propertyCount; ++index) {
            const char* keyText = JS_AtomToCString(ctx, properties[index].atom);
            if (!keyText) {
                JS_FreeAtom(ctx, properties[index].atom);
                continue;
            }
            JSValue propertyValue = JS_GetProperty(ctx, value, properties[index].atom);
            object[keyText] = jsValueToBridgeValue(ctx, propertyValue);
            JS_FreeValue(ctx, propertyValue);
            JS_FreeCString(ctx, keyText);
            JS_FreeAtom(ctx, properties[index].atom);
        }
        js_free(ctx, properties);
        return Value::Obj(std::move(object));
    }

    JSValue global = JS_GetGlobalObject(ctx);
    JSValue json = JS_GetPropertyStr(ctx, global, "JSON");
    JSValue stringify = JS_GetPropertyStr(ctx, json, "stringify");
    JSValue jsonTextValue = JS_Call(ctx, stringify, json, 1, &value);
    Value out = Value::Nil();
    if (!JS_IsException(jsonTextValue) && JS_IsString(jsonTextValue)) {
        const char* jsonText = JS_ToCString(ctx, jsonTextValue);
        if (jsonText) {
            const auto parsed = nlohmann::json::parse(jsonText, nullptr, false);
            if (!parsed.is_discarded()) {
                out = jsonToBridgeValue(parsed);
            }
            JS_FreeCString(ctx, jsonText);
        }
    }
    JS_FreeValue(ctx, jsonTextValue);
    JS_FreeValue(ctx, stringify);
    JS_FreeValue(ctx, json);
    JS_FreeValue(ctx, global);
    return out;
}

} // namespace

// ============================================================================
// QuickJSContext Implementation
// ============================================================================

// Opaque implementation struct - currently stores harness state; real QuickJS state can slot in later
class QuickJSContextImpl {
  public:
    bool initialized = false;
    QuickJSConfig config;
    std::unordered_map<std::string, QuickJSContext::HostFunction> hostFunctions;
    std::unordered_map<std::string, Value> globals;
    size_t heapSize = 0;
    uint64_t cpuUsedUs = 0;
    JSRuntime* runtime = nullptr;
    JSContext* context = nullptr;

    ~QuickJSContextImpl() {
        if (context) {
            JS_FreeContext(context);
            context = nullptr;
        }
        if (runtime) {
            JS_FreeRuntime(runtime);
            runtime = nullptr;
        }
    }
};

JSValue hostFunctionThunk(JSContext* ctx, JSValueConst, int argc, JSValueConst* argv, int, JSValueConst* funcData) {
    auto* impl = static_cast<QuickJSContextImpl*>(JS_GetContextOpaque(ctx));
    if (!impl) {
        return JS_ThrowInternalError(ctx, "QuickJS host context is unavailable");
    }

    const char* functionNameText = JS_ToCString(ctx, funcData[0]);
    const std::string functionName = functionNameText ? functionNameText : "";
    if (functionNameText) {
        JS_FreeCString(ctx, functionNameText);
    }

    const auto it = impl->hostFunctions.find(functionName);
    if (it == impl->hostFunctions.end()) {
        return JS_ThrowReferenceError(ctx, "Host function not found: %s", functionName.c_str());
    }

    std::vector<Value> args;
    args.reserve(static_cast<size_t>(argc));
    for (int index = 0; index < argc; ++index) {
        args.push_back(jsValueToBridgeValue(ctx, argv[index]));
    }

    try {
        return bridgeValueToJS(ctx, it->second(args));
    } catch (const std::exception& e) {
        return JS_ThrowInternalError(ctx, "Host function error: %s", e.what());
    } catch (...) {
        return JS_ThrowInternalError(ctx, "Unknown host function error");
    }
}

QuickJSContext::QuickJSContext() : impl_(std::make_unique<QuickJSContextImpl>()) {}

QuickJSContext::~QuickJSContext() = default;

QuickJSContext::QuickJSContext(QuickJSContext&& other) noexcept
    : impl_(std::move(other.impl_)), apiRegistry_(std::move(other.apiRegistry_)), config_(std::move(other.config_)),
      budget_(other.budget_), lastError_(std::move(other.lastError_)), moduleLoader_(std::move(other.moduleLoader_)) {}

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

    if (impl_->context) {
        JS_FreeContext(impl_->context);
        impl_->context = nullptr;
    }
    if (impl_->runtime) {
        JS_FreeRuntime(impl_->runtime);
        impl_->runtime = nullptr;
    }

    impl_->runtime = JS_NewRuntime();
    if (!impl_->runtime) {
        lastError_ = "Failed to create QuickJS runtime";
        return false;
    }
    if (config.enableMemoryLimit) {
        JS_SetMemoryLimit(impl_->runtime, static_cast<size_t>(config.memoryLimitMB) * 1024 * 1024);
    }

    impl_->context = JS_NewContext(impl_->runtime);
    if (!impl_->context) {
        JS_FreeRuntime(impl_->runtime);
        impl_->runtime = nullptr;
        lastError_ = "Failed to create QuickJS context";
        return false;
    }
    JS_SetContextOpaque(impl_->context, impl_.get());

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

    const bool hasFixtureDirective = code.find("@urpg-") != std::string::npos;

    // Fixture bridge: parse lightweight export directives and bind functions.
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
            std::string payload = trim(line.substr(failPos + std::string("@urpg-fail-eval").size()));
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
            std::istringstream directive(line.substr(failCallPos + std::string("@urpg-fail-call").size()));
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

            registerFunction(functionName,
                             [payload](const std::vector<Value>&) -> Value { throw std::runtime_error(payload); });
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
            registerFunction(functionName, [](const std::vector<Value>& args) -> Value {
                return Value::Int(static_cast<int64_t>(args.size()));
            });
            continue;
        }

        if (mode == "arg") {
            int64_t index = 0;
            (void)(directive >> index);
            registerFunction(functionName, [index](const std::vector<Value>& args) -> Value {
                if (index >= 0 && static_cast<size_t>(index) < args.size()) {
                    return args[static_cast<size_t>(index)];
                }
                return Value::Nil();
            });
            continue;
        }

        if (mode == "const") {
            std::string payload;
            std::getline(directive, payload);
            const Value constantValue = parseDirectiveConstValue(payload);
            registerFunction(functionName,
                             [constantValue](const std::vector<Value>&) -> Value { return constantValue; });
        }
    }

    if (hasFixtureDirective) {
        result.success = true;
        result.value = Value::Nil();
        result.sourceLocation = filename + ":1";
        lastError_.clear();
        return result;
    }

    JSValue evaluated = JS_Eval(impl_->context, code.c_str(), code.size(), filename.c_str(), JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(evaluated)) {
        result.success = false;
        result.error = exceptionToString(impl_->context);
        result.severity = CompatSeverity::HARD_FAIL;
        result.sourceLocation = filename;
        lastError_ = result.error;
        JS_FreeValue(impl_->context, evaluated);
        return result;
    }

    result.success = true;
    result.value = jsValueToBridgeValue(impl_->context, evaluated);
    result.sourceLocation = filename + ":1";
    JS_FreeValue(impl_->context, evaluated);
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

ScriptResult QuickJSContext::evalWithScope(const std::string& code, const std::map<std::string, Value>& scope,
                                           const std::string& filename) {
    for (const auto& [name, value] : scope) {
        setGlobal(name, value);
    }
    return eval(code, filename);
}

ScriptResult QuickJSContext::call(const std::string& functionName, const std::vector<Value>& args) {
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

    JSValue global = JS_GetGlobalObject(impl_->context);
    JSValue fn = JS_GetPropertyStr(impl_->context, global, functionName.c_str());
    if (JS_IsUndefined(fn) || JS_IsNull(fn)) {
        JS_FreeValue(impl_->context, fn);
        JS_FreeValue(impl_->context, global);
        result.success = false;
        result.error = "Function not found: " + functionName;
        result.severity = CompatSeverity::SOFT_FAIL;
        lastError_ = result.error;
        return result;
    }

    if (!JS_IsFunction(impl_->context, fn)) {
        JS_FreeValue(impl_->context, fn);
        JS_FreeValue(impl_->context, global);
        result.success = false;
        result.error = "Global is not callable: " + functionName;
        result.severity = CompatSeverity::SOFT_FAIL;
        lastError_ = result.error;
        return result;
    }

    std::vector<JSValue> jsArgs;
    jsArgs.reserve(args.size());
    for (const auto& arg : args) {
        jsArgs.push_back(bridgeValueToJS(impl_->context, arg));
    }

    JSValue callValue = JS_Call(impl_->context, fn, JS_UNDEFINED, static_cast<int>(jsArgs.size()), jsArgs.data());
    for (auto& arg : jsArgs) {
        JS_FreeValue(impl_->context, arg);
    }
    JS_FreeValue(impl_->context, fn);
    JS_FreeValue(impl_->context, global);

    if (JS_IsException(callValue)) {
        result.success = false;
        result.error = exceptionToString(impl_->context);
        result.severity =
            result.error == "Unknown host function error" ? CompatSeverity::CRASH_PREVENTED : CompatSeverity::HARD_FAIL;
        lastError_ = result.error;
        JS_FreeValue(impl_->context, callValue);
        return result;
    }

    result.value = jsValueToBridgeValue(impl_->context, callValue);
    result.success = true;
    result.sourceLocation = functionName;
    lastError_.clear();
    JS_FreeValue(impl_->context, callValue);
    return result;
}

ScriptResult QuickJSContext::callMethod(const std::string& objectName, const std::string& methodName,
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
            return result; // No-op for stubs
        }
    }

    JSValue global = JS_GetGlobalObject(impl_->context);
    JSValue object = JS_GetPropertyStr(impl_->context, global, objectName.c_str());
    JSValue method = JS_GetPropertyStr(impl_->context, object, methodName.c_str());
    if (!JS_IsUndefined(object) && !JS_IsNull(object) && JS_IsFunction(impl_->context, method)) {
        std::vector<JSValue> jsArgs;
        jsArgs.reserve(args.size());
        for (const auto& arg : args) {
            jsArgs.push_back(bridgeValueToJS(impl_->context, arg));
        }
        JSValue callValue = JS_Call(impl_->context, method, object, static_cast<int>(jsArgs.size()), jsArgs.data());
        for (auto& arg : jsArgs) {
            JS_FreeValue(impl_->context, arg);
        }
        if (JS_IsException(callValue)) {
            result.success = false;
            result.error = exceptionToString(impl_->context);
            result.severity = result.error == "Unknown host function error" ? CompatSeverity::CRASH_PREVENTED
                                                                            : CompatSeverity::HARD_FAIL;
            lastError_ = result.error;
        } else {
            result.success = true;
            result.value = jsValueToBridgeValue(impl_->context, callValue);
            result.sourceLocation = fullName;
            lastError_.clear();
        }
        JS_FreeValue(impl_->context, callValue);
    } else {
        result = call(fullName, args);
    }
    JS_FreeValue(impl_->context, method);
    JS_FreeValue(impl_->context, object);
    JS_FreeValue(impl_->context, global);

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

    JSValue global = JS_GetGlobalObject(impl_->context);
    JSValue value = JS_GetPropertyStr(impl_->context, global, name.c_str());
    JS_FreeValue(impl_->context, global);
    if (JS_IsUndefined(value) || JS_IsNull(value)) {
        JS_FreeValue(impl_->context, value);
        return std::nullopt;
    }
    Value out = jsValueToBridgeValue(impl_->context, value);
    JS_FreeValue(impl_->context, value);
    return out;
}

bool QuickJSContext::setGlobal(const std::string& name, const Value& value) {
    assert(impl_ != nullptr);

    if (!impl_->initialized) {
        return false;
    }

    JSValue global = JS_GetGlobalObject(impl_->context);
    JS_SetPropertyStr(impl_->context, global, name.c_str(), bridgeValueToJS(impl_->context, value));
    JS_FreeValue(impl_->context, global);
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
    if (impl_->initialized && impl_->context) {
        JSValue functionName = JS_NewString(impl_->context, name.c_str());
        JSValue function = JS_NewCFunctionData(impl_->context, hostFunctionThunk, 0, 0, 1, &functionName);
        JS_FreeValue(impl_->context, functionName);
        JSValue global = JS_GetGlobalObject(impl_->context);
        JS_SetPropertyStr(impl_->context, global, name.c_str(), function);
        JS_FreeValue(impl_->context, global);
    }
    return true;
}

bool QuickJSContext::registerObject(const std::string& name, const std::vector<MethodDef>& methods) {
    assert(impl_ != nullptr);

    JSValue object = JS_UNDEFINED;
    if (impl_->initialized && impl_->context) {
        JSValue global = JS_GetGlobalObject(impl_->context);
        object = JS_GetPropertyStr(impl_->context, global, name.c_str());
        if (JS_IsUndefined(object) || JS_IsNull(object)) {
            JS_FreeValue(impl_->context, object);
            object = JS_NewObject(impl_->context);
        }
        JS_FreeValue(impl_->context, global);
    }

    for (const auto& method : methods) {
        if (method.name.empty() || !method.fn) {
            continue;
        }
        std::string fullName = name + "." + method.name;
        impl_->hostFunctions[fullName] = method.fn;
        if (impl_->initialized && impl_->context) {
            JSValue functionName = JS_NewString(impl_->context, fullName.c_str());
            JSValue function = JS_NewCFunctionData(impl_->context, hostFunctionThunk, 0, 0, 1, &functionName);
            JS_FreeValue(impl_->context, functionName);
            JS_SetPropertyStr(impl_->context, object, method.name.c_str(), function);
        }

        // Register API status
        registerAPIStatus(fullName, method.status, method.deviationNote);
    }

    if (impl_->initialized && impl_->context) {
        JSValue global = JS_GetGlobalObject(impl_->context);
        JS_SetPropertyStr(impl_->context, global, name.c_str(), object);
        JS_FreeValue(impl_->context, global);
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
    if (impl_->runtime) {
        JS_RunGC(impl_->runtime);
    }
}

size_t QuickJSContext::getHeapSize() const {
    if (!impl_ || !impl_->runtime) {
        return 0;
    }
    JSMemoryUsage usage{};
    JS_ComputeMemoryUsage(impl_->runtime, &usage);
    return static_cast<size_t>(usage.malloc_size);
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

void QuickJSContext::registerAPIStatus(const std::string& apiName, CompatStatus status,
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
              [](const APIStatus& a, const APIStatus& b) { return a.apiName < b.apiName; });
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

QuickJSRuntime::QuickJSRuntime() : impl_(std::make_unique<Impl>()) {}

QuickJSRuntime::~QuickJSRuntime() = default;

bool QuickJSRuntime::initialize() {
    assert(impl_ != nullptr);
    impl_->initialized = true;
    return true;
}

std::optional<uint32_t> QuickJSRuntime::createContext(const std::string& pluginId, const QuickJSConfig& config) {
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
        for (auto mapIt = impl_->pluginToContextId.begin(); mapIt != impl_->pluginToContextId.end();) {
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
    return it != impl_->pluginToContextId.end() ? std::optional<uint32_t>(it->second) : std::nullopt;
}

CompatBudget QuickJSRuntime::getAggregateBudgetStatus() const {
    assert(impl_ != nullptr);

    CompatBudget aggregate;
    uint32_t cpuUsed = 0;
    for (const auto& [id, context] : impl_->contexts) {
        auto budget = context->getBudgetStatus();
        if (budget.exceeded_cpu) {
            aggregate.exceeded_cpu = true;
        }
        if (budget.exceeded_memory) {
            aggregate.exceeded_memory = true;
        }
        cpuUsed += budget.cpu_slice_us;
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

std::unordered_map<std::string, QuickJSContext::APIStatus> QuickJSRuntime::getAllAPIStatuses() const {
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
