#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace urpg {

struct Breakpoint {
    std::string event_id;
    std::string block_id;
    bool enabled = true;
};

class BreakpointStore {
public:
    bool Add(const Breakpoint& breakpoint);
    bool Remove(std::string_view event_id, std::string_view block_id = "");
    bool Has(std::string_view event_id, std::string_view block_id = "") const;

    size_t Count() const;
    const std::vector<Breakpoint>& All() const;

private:
    std::vector<Breakpoint> breakpoints_;
};

struct WatchValue {
    std::string key;
    std::string value;
};

class WatchTable {
public:
    void Set(std::string_view key, std::string_view value);
    std::optional<std::string> Get(std::string_view key) const;
    bool Remove(std::string_view key);

    std::vector<WatchValue> SnapshotSorted() const;

private:
    std::vector<WatchValue> values_;
};

struct DebugFrame {
    std::string function_name;
    std::string event_id;
    std::string block_id;
};

class CallStack {
public:
    void Push(const DebugFrame& frame);
    bool Pop();
    size_t Depth() const;
    const std::vector<DebugFrame>& Frames() const;

private:
    std::vector<DebugFrame> frames_;
};

enum class StepMode : uint8_t {
    Continue = 0,
    StepInto = 1,
    StepOver = 2,
    StepOut = 3
};

class StepController {
public:
    void Request(StepMode mode, size_t current_depth);
    bool ShouldPause(size_t current_depth);
    StepMode Mode() const;

private:
    void Reset();

    StepMode mode_ = StepMode::Continue;
    size_t start_depth_ = 0;
    bool armed_ = false;
    bool first_tick_ = false;
};

class DebugRuntimeSession {
public:
    bool AddBreakpoint(const Breakpoint& breakpoint);
    bool RemoveBreakpoint(std::string_view event_id, std::string_view block_id = "");
    bool ShouldBreak(std::string_view event_id, std::string_view block_id = "") const;

    void SetWatch(std::string_view key, std::string_view value);
    std::optional<std::string> GetWatch(std::string_view key) const;

    void EnterFrame(const DebugFrame& frame);
    bool ExitFrame();

    void RequestStep(StepMode mode);
    bool ShouldPauseForStep();

    const CallStack& Stack() const;

private:
    BreakpointStore breakpoints_;
    WatchTable watches_;
    CallStack stack_;
    StepController step_controller_;
};

} // namespace urpg
