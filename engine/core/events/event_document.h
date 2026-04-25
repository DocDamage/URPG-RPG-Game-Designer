#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace urpg::events {

enum class EventTrigger : uint8_t {
    ActionButton,
    PlayerTouch,
    Autorun,
    Parallel
};

enum class EventCommandKind : uint8_t {
    Message,
    Switch,
    Variable,
    Transfer,
    CommonEvent,
    Battle,
    Item,
    Gold,
    Wait,
    Fade,
    Sound,
    Plugin,
    Unsupported
};

struct EventCondition {
    std::map<std::string, bool> switches;
    std::map<std::string, int64_t> min_variables;
    std::set<std::string> quest_flags;

    bool matches(const std::map<std::string, bool>& switch_values,
                 const std::map<std::string, int64_t>& variable_values,
                 const std::set<std::string>& active_quest_flags) const;
    bool empty() const;
};

struct EventCommand {
    EventCommand() = default;
    EventCommand(std::string command_id,
                 EventCommandKind command_kind,
                 std::string command_target = {},
                 std::string command_value = {},
                 int64_t command_amount = 0,
                 std::set<std::string> command_read_save_fields = {},
                 std::set<std::string> command_write_save_fields = {},
                 nlohmann::json command_payload = nlohmann::json::object(),
                 nlohmann::json command_compat_fallback = nlohmann::json::object())
        : id(std::move(command_id)),
          kind(command_kind),
          target(std::move(command_target)),
          value(std::move(command_value)),
          amount(command_amount),
          read_save_fields(std::move(command_read_save_fields)),
          write_save_fields(std::move(command_write_save_fields)),
          payload(std::move(command_payload)),
          compat_fallback(std::move(command_compat_fallback)) {}

    std::string id;
    EventCommandKind kind = EventCommandKind::Unsupported;
    std::string target;
    std::string value;
    int64_t amount = 0;
    std::set<std::string> read_save_fields;
    std::set<std::string> write_save_fields;
    nlohmann::json payload = nlohmann::json::object();
    nlohmann::json compat_fallback = nlohmann::json::object();
};

struct EventPage {
    std::string id;
    int32_t priority = 0;
    EventTrigger trigger = EventTrigger::ActionButton;
    EventCondition conditions;
    std::vector<EventCommand> commands;
};

struct EventDefinition {
    std::string id;
    std::string map_id;
    int32_t x = 0;
    int32_t y = 0;
    std::vector<EventPage> pages;
};

struct CommonEventDefinition {
    std::string id;
    std::vector<EventCommand> commands;
};

struct MapDefinition {
    std::string id;
    int32_t width = 0;
    int32_t height = 0;
};

struct EventWorldState {
    std::map<std::string, bool> switches;
    std::map<std::string, int64_t> variables;
    std::set<std::string> quest_flags;
    std::set<std::string> enabled_plugins;
};

enum class EventDiagnosticSeverity : uint8_t {
    Info,
    Warning,
    Error
};

struct EventDiagnostic {
    EventDiagnosticSeverity severity = EventDiagnosticSeverity::Info;
    std::string code;
    std::string message;
    std::string event_id;
    std::string page_id;
    std::string command_id;
};

class EventDocument {
public:
    void addMap(MapDefinition map);
    void addEvent(EventDefinition event);
    void addCommonEvent(CommonEventDefinition common_event);
    void setAvailablePlugins(std::set<std::string> plugin_ids);
    void setKnownSwitches(std::set<std::string> switch_ids);
    void setKnownVariables(std::set<std::string> variable_ids);
    void setKnownSaveFields(std::set<std::string> save_field_ids);

    const std::vector<EventDefinition>& events() const { return events_; }
    const std::map<std::string, CommonEventDefinition>& commonEvents() const { return common_events_; }
    const std::map<std::string, MapDefinition>& maps() const { return maps_; }
    const std::set<std::string>& availablePlugins() const { return available_plugins_; }

    std::optional<EventPage> resolveActivePage(const std::string& event_id, const EventWorldState& state) const;
    std::vector<EventDiagnostic> validate(const EventWorldState& state = {}) const;
    nlohmann::json toJson() const;

    static EventDocument fromJson(const nlohmann::json& json);

private:
    std::vector<EventDefinition> events_;
    std::map<std::string, CommonEventDefinition> common_events_;
    std::map<std::string, MapDefinition> maps_;
    std::set<std::string> available_plugins_;
    std::set<std::string> known_switches_;
    std::set<std::string> known_variables_;
    std::set<std::string> known_save_fields_;
};

std::string toString(EventCommandKind kind);
std::string toString(EventDiagnosticSeverity severity);
EventCommandKind eventCommandKindFromString(const std::string& value);

} // namespace urpg::events
