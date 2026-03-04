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

} // namespace urpg
