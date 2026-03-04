#include "engine/core/debug/debug_session.h"

#include <algorithm>

namespace urpg {

namespace {

bool Matches(const Breakpoint& breakpoint, std::string_view event_id, std::string_view block_id) {
    return breakpoint.event_id == event_id && breakpoint.block_id == block_id;
}

} // namespace

bool BreakpointStore::Add(const Breakpoint& breakpoint) {
    if (Has(breakpoint.event_id, breakpoint.block_id)) {
        return false;
    }

    breakpoints_.push_back(breakpoint);
    return true;
}

bool BreakpointStore::Remove(std::string_view event_id, std::string_view block_id) {
    auto it = std::find_if(
        breakpoints_.begin(),
        breakpoints_.end(),
        [&](const Breakpoint& breakpoint) {
            return Matches(breakpoint, event_id, block_id);
        }
    );

    if (it == breakpoints_.end()) {
        return false;
    }

    breakpoints_.erase(it);
    return true;
}

bool BreakpointStore::Has(std::string_view event_id, std::string_view block_id) const {
    return std::any_of(
        breakpoints_.begin(),
        breakpoints_.end(),
        [&](const Breakpoint& breakpoint) {
            return Matches(breakpoint, event_id, block_id);
        }
    );
}

size_t BreakpointStore::Count() const {
    return breakpoints_.size();
}

const std::vector<Breakpoint>& BreakpointStore::All() const {
    return breakpoints_;
}

void WatchTable::Set(std::string_view key, std::string_view value) {
    auto it = std::find_if(
        values_.begin(),
        values_.end(),
        [&](const WatchValue& watch_value) {
            return watch_value.key == key;
        }
    );

    if (it != values_.end()) {
        it->value = std::string(value);
        return;
    }

    values_.push_back(WatchValue{std::string(key), std::string(value)});
}

std::optional<std::string> WatchTable::Get(std::string_view key) const {
    auto it = std::find_if(
        values_.begin(),
        values_.end(),
        [&](const WatchValue& watch_value) {
            return watch_value.key == key;
        }
    );

    if (it == values_.end()) {
        return std::nullopt;
    }

    return it->value;
}

bool WatchTable::Remove(std::string_view key) {
    auto it = std::find_if(
        values_.begin(),
        values_.end(),
        [&](const WatchValue& watch_value) {
            return watch_value.key == key;
        }
    );

    if (it == values_.end()) {
        return false;
    }

    values_.erase(it);
    return true;
}

std::vector<WatchValue> WatchTable::SnapshotSorted() const {
    std::vector<WatchValue> snapshot = values_;
    std::sort(
        snapshot.begin(),
        snapshot.end(),
        [](const WatchValue& left, const WatchValue& right) {
            return left.key < right.key;
        }
    );
    return snapshot;
}

void CallStack::Push(const DebugFrame& frame) {
    frames_.push_back(frame);
}

bool CallStack::Pop() {
    if (frames_.empty()) {
        return false;
    }

    frames_.pop_back();
    return true;
}

size_t CallStack::Depth() const {
    return frames_.size();
}

const std::vector<DebugFrame>& CallStack::Frames() const {
    return frames_;
}

void StepController::Request(StepMode mode, size_t current_depth) {
    mode_ = mode;
    start_depth_ = current_depth;
    armed_ = mode != StepMode::Continue;
    first_tick_ = armed_;
}

bool StepController::ShouldPause(size_t current_depth) {
    if (!armed_) {
        return false;
    }

    if (first_tick_) {
        first_tick_ = false;
        return false;
    }

    bool should_pause = false;
    switch (mode_) {
    case StepMode::Continue:
        should_pause = false;
        break;
    case StepMode::StepInto:
        should_pause = true;
        break;
    case StepMode::StepOver:
        should_pause = current_depth <= start_depth_;
        break;
    case StepMode::StepOut:
        should_pause = current_depth < start_depth_;
        break;
    }

    if (should_pause) {
        Reset();
    }

    return should_pause;
}

StepMode StepController::Mode() const {
    return mode_;
}

void StepController::Reset() {
    mode_ = StepMode::Continue;
    start_depth_ = 0;
    armed_ = false;
    first_tick_ = false;
}

bool DebugRuntimeSession::AddBreakpoint(const Breakpoint& breakpoint) {
    return breakpoints_.Add(breakpoint);
}

bool DebugRuntimeSession::RemoveBreakpoint(std::string_view event_id, std::string_view block_id) {
    return breakpoints_.Remove(event_id, block_id);
}

bool DebugRuntimeSession::ShouldBreak(std::string_view event_id, std::string_view block_id) const {
    return breakpoints_.Has(event_id, block_id);
}

void DebugRuntimeSession::SetWatch(std::string_view key, std::string_view value) {
    watches_.Set(key, value);
}

std::optional<std::string> DebugRuntimeSession::GetWatch(std::string_view key) const {
    return watches_.Get(key);
}

void DebugRuntimeSession::EnterFrame(const DebugFrame& frame) {
    stack_.Push(frame);
}

bool DebugRuntimeSession::ExitFrame() {
    return stack_.Pop();
}

void DebugRuntimeSession::RequestStep(StepMode mode) {
    step_controller_.Request(mode, stack_.Depth());
}

bool DebugRuntimeSession::ShouldPauseForStep() {
    return step_controller_.ShouldPause(stack_.Depth());
}

const CallStack& DebugRuntimeSession::Stack() const {
    return stack_;
}

} // namespace urpg
