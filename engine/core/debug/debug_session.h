#pragma once

#include <optional>
#include <string>
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

} // namespace urpg
