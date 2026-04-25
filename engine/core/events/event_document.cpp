#include "engine/core/events/event_document.h"

#include <algorithm>
#include <functional>
#include <map>
#include <set>
#include <tuple>
#include <utility>

namespace urpg::events {

namespace {

std::string stringValue(const nlohmann::json& json, std::string_view key) {
    const auto it = json.find(std::string(key));
    return it != json.end() && it->is_string() ? it->get<std::string>() : std::string{};
}

int64_t intValue(const nlohmann::json& json, std::string_view key, int64_t fallback = 0) {
    const auto it = json.find(std::string(key));
    return it != json.end() && it->is_number_integer() ? it->get<int64_t>() : fallback;
}

EventTrigger triggerFromString(const std::string& value) {
    if (value == "player_touch") {
        return EventTrigger::PlayerTouch;
    }
    if (value == "autorun") {
        return EventTrigger::Autorun;
    }
    if (value == "parallel") {
        return EventTrigger::Parallel;
    }
    return EventTrigger::ActionButton;
}

std::string triggerToString(EventTrigger trigger) {
    switch (trigger) {
        case EventTrigger::PlayerTouch: return "player_touch";
        case EventTrigger::Autorun: return "autorun";
        case EventTrigger::Parallel: return "parallel";
        case EventTrigger::ActionButton: return "action_button";
    }
    return "action_button";
}

EventCondition conditionFromJson(const nlohmann::json& json) {
    EventCondition condition;
    if (!json.is_object()) {
        return condition;
    }

    if (const auto it = json.find("switches"); it != json.end() && it->is_object()) {
        for (const auto& [key, value] : it->items()) {
            condition.switches[key] = value.get<bool>();
        }
    }
    if (const auto it = json.find("min_variables"); it != json.end() && it->is_object()) {
        for (const auto& [key, value] : it->items()) {
            condition.min_variables[key] = value.get<int64_t>();
        }
    }
    if (const auto it = json.find("quest_flags"); it != json.end() && it->is_array()) {
        for (const auto& value : *it) {
            if (value.is_string()) {
                condition.quest_flags.insert(value.get<std::string>());
            }
        }
    }
    return condition;
}

nlohmann::json conditionToJson(const EventCondition& condition) {
    nlohmann::json json = nlohmann::json::object();
    json["switches"] = condition.switches;
    json["min_variables"] = condition.min_variables;
    json["quest_flags"] = nlohmann::json::array();
    for (const auto& flag : condition.quest_flags) {
        json["quest_flags"].push_back(flag);
    }
    return json;
}

EventCommand commandFromJson(const nlohmann::json& json) {
    EventCommand command;
    if (!json.is_object()) {
        return command;
    }
    command.id = stringValue(json, "id");
    command.kind = eventCommandKindFromString(stringValue(json, "kind"));
    command.target = stringValue(json, "target");
    command.value = stringValue(json, "value");
    command.amount = intValue(json, "amount");
    for (const auto& field : json.value("read_save_fields", nlohmann::json::array())) {
        if (field.is_string()) {
            command.read_save_fields.insert(field.get<std::string>());
        }
    }
    for (const auto& field : json.value("write_save_fields", nlohmann::json::array())) {
        if (field.is_string()) {
            command.write_save_fields.insert(field.get<std::string>());
        }
    }
    command.payload = json.value("payload", nlohmann::json::object());
    if (command.kind == EventCommandKind::Unsupported) {
        command.compat_fallback = json.value("_compat_command_fallbacks", json);
    } else {
        command.compat_fallback = json.value("_compat_command_fallbacks", nlohmann::json::object());
    }
    return command;
}

nlohmann::json commandToJson(const EventCommand& command) {
    nlohmann::json json = {
        {"id", command.id},
        {"kind", toString(command.kind)},
        {"target", command.target},
        {"value", command.value},
        {"amount", command.amount},
        {"read_save_fields", command.read_save_fields},
        {"write_save_fields", command.write_save_fields},
        {"payload", command.payload}
    };
    if (!command.compat_fallback.empty()) {
        json["_compat_command_fallbacks"] = command.compat_fallback;
    }
    return json;
}

bool containsAllSwitchConditions(const EventCondition& wider, const EventCondition& narrower) {
    for (const auto& [id, expected] : wider.switches) {
        const auto it = narrower.switches.find(id);
        if (it == narrower.switches.end() || it->second != expected) {
            return false;
        }
    }
    for (const auto& [id, minimum] : wider.min_variables) {
        const auto it = narrower.min_variables.find(id);
        if (it == narrower.min_variables.end() || it->second < minimum) {
            return false;
        }
    }
    for (const auto& flag : wider.quest_flags) {
        if (!narrower.quest_flags.contains(flag)) {
            return false;
        }
    }
    return true;
}

bool pageShadows(const EventPage& higher_priority_page, const EventPage& lower_priority_page) {
    if (higher_priority_page.trigger != lower_priority_page.trigger ||
        higher_priority_page.priority <= lower_priority_page.priority) {
        return false;
    }
    return containsAllSwitchConditions(higher_priority_page.conditions, lower_priority_page.conditions);
}

void pushDiagnostic(std::vector<EventDiagnostic>& diagnostics,
                    EventDiagnosticSeverity severity,
                    std::string code,
                    std::string message,
                    std::string event_id = {},
                    std::string page_id = {},
                    std::string command_id = {}) {
    diagnostics.push_back(EventDiagnostic{
        severity,
        std::move(code),
        std::move(message),
        std::move(event_id),
        std::move(page_id),
        std::move(command_id)
    });
}

} // namespace

bool EventCondition::matches(const std::map<std::string, bool>& switch_values,
                             const std::map<std::string, int64_t>& variable_values,
                             const std::set<std::string>& active_quest_flags) const {
    for (const auto& [id, expected] : switches) {
        const auto it = switch_values.find(id);
        if (it == switch_values.end() || it->second != expected) {
            return false;
        }
    }
    for (const auto& [id, minimum] : min_variables) {
        const auto it = variable_values.find(id);
        if (it == variable_values.end() || it->second < minimum) {
            return false;
        }
    }
    for (const auto& flag : quest_flags) {
        if (!active_quest_flags.contains(flag)) {
            return false;
        }
    }
    return true;
}

bool EventCondition::empty() const {
    return switches.empty() && min_variables.empty() && quest_flags.empty();
}

void EventDocument::addMap(MapDefinition map) {
    maps_[map.id] = std::move(map);
}

void EventDocument::addEvent(EventDefinition event) {
    events_.push_back(std::move(event));
}

void EventDocument::addCommonEvent(CommonEventDefinition common_event) {
    common_events_[common_event.id] = std::move(common_event);
}

void EventDocument::setAvailablePlugins(std::set<std::string> plugin_ids) {
    available_plugins_ = std::move(plugin_ids);
}

void EventDocument::setKnownSwitches(std::set<std::string> switch_ids) {
    known_switches_ = std::move(switch_ids);
}

void EventDocument::setKnownVariables(std::set<std::string> variable_ids) {
    known_variables_ = std::move(variable_ids);
}

void EventDocument::setKnownSaveFields(std::set<std::string> save_field_ids) {
    known_save_fields_ = std::move(save_field_ids);
}

std::optional<EventPage> EventDocument::resolveActivePage(const std::string& event_id, const EventWorldState& state) const {
    const auto event_it = std::find_if(events_.begin(), events_.end(), [&](const EventDefinition& event) {
        return event.id == event_id;
    });
    if (event_it == events_.end()) {
        return std::nullopt;
    }

    std::vector<EventPage> matches;
    for (const auto& page : event_it->pages) {
        if (page.conditions.matches(state.switches, state.variables, state.quest_flags)) {
            matches.push_back(page);
        }
    }
    if (matches.empty()) {
        return std::nullopt;
    }

    std::sort(matches.begin(), matches.end(), [](const EventPage& left, const EventPage& right) {
        if (left.priority != right.priority) {
            return left.priority > right.priority;
        }
        return left.id < right.id;
    });
    return matches.front();
}

std::vector<EventDiagnostic> EventDocument::validate(const EventWorldState& state) const {
    std::vector<EventDiagnostic> diagnostics;
    std::set<std::string> event_ids;
    std::set<std::string> seen_event_ids;

    for (const auto& event : events_) {
        if (!seen_event_ids.insert(event.id).second) {
            pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "duplicate_event_id",
                           "Event ids must be unique.", event.id);
        }
        event_ids.insert(event.id);

        const auto map_it = maps_.find(event.map_id);
        if (map_it == maps_.end()) {
            pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_target_map",
                           "Event references a map that is not present in the event document.", event.id);
        } else if (event.x < 0 || event.y < 0 || event.x >= map_it->second.width || event.y >= map_it->second.height) {
            pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "transfer_coordinates_out_of_bounds",
                           "Event coordinates are outside the owning map bounds.", event.id);
        }

        std::set<std::string> unconditional_signatures;
        for (size_t page_index = 0; page_index < event.pages.size(); ++page_index) {
            const auto& page = event.pages[page_index];
            if (page.conditions.empty()) {
                const auto signature = std::to_string(page.priority) + ":" + triggerToString(page.trigger);
                if (!unconditional_signatures.insert(signature).second) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Warning, "page_shadowing",
                                   "Multiple unconditional pages share the same priority and trigger.", event.id, page.id);
                }
            }

            for (size_t previous_index = 0; previous_index < event.pages.size(); ++previous_index) {
                if (previous_index == page_index) {
                    continue;
                }
                if (pageShadows(event.pages[previous_index], page)) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Warning, "unreachable_page",
                                   "A higher-priority page has conditions that always shadow this page.", event.id, page.id);
                    break;
                }
            }

            if (!known_switches_.empty()) {
                for (const auto& [switch_id, _] : page.conditions.switches) {
                    if (!known_switches_.contains(switch_id)) {
                        pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_switch_reference",
                                       "Event page condition references an unknown switch.", event.id, page.id);
                    }
                }
            }
            if (!known_variables_.empty()) {
                for (const auto& [variable_id, _] : page.conditions.min_variables) {
                    if (!known_variables_.contains(variable_id)) {
                        pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_variable_reference",
                                       "Event page condition references an unknown variable.", event.id, page.id);
                    }
                }
            }

            for (const auto& command : page.commands) {
                if (command.kind == EventCommandKind::Switch &&
                    !known_switches_.empty() &&
                    !known_switches_.contains(command.target)) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_switch_reference",
                                   "Event command references an unknown switch.", event.id, page.id, command.id);
                }
                if (command.kind == EventCommandKind::Variable &&
                    !known_variables_.empty() &&
                    !known_variables_.contains(command.target)) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_variable_reference",
                                   "Event command references an unknown variable.", event.id, page.id, command.id);
                }
                if (!known_save_fields_.empty()) {
                    for (const auto& save_field : command.read_save_fields) {
                        if (!known_save_fields_.contains(save_field)) {
                            pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_save_field_reference",
                                           "Event command reads an unknown save field.", event.id, page.id, command.id);
                        }
                    }
                    for (const auto& save_field : command.write_save_fields) {
                        if (!known_save_fields_.contains(save_field)) {
                            pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_save_field_reference",
                                           "Event command writes an unknown save field.", event.id, page.id, command.id);
                        }
                    }
                }
                if (command.kind == EventCommandKind::CommonEvent && !common_events_.contains(command.target)) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_common_event",
                                   "Event command references a missing common event.", event.id, page.id, command.id);
                }
                if (command.kind == EventCommandKind::Plugin && !state.enabled_plugins.contains(command.target)) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Warning, "disabled_plugin_command",
                                   "Plugin command is present but the plugin is not enabled.", event.id, page.id, command.id);
                }
                if (command.kind == EventCommandKind::Transfer && !maps_.contains(command.target)) {
                    pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "missing_transfer_map",
                                   "Transfer command references a missing map.", event.id, page.id, command.id);
                }
            }
        }
    }

    std::map<std::string, std::vector<std::string>> common_edges;
    for (const auto& [id, common_event] : common_events_) {
        for (const auto& command : common_event.commands) {
            if (command.kind == EventCommandKind::CommonEvent) {
                common_edges[id].push_back(command.target);
            }
        }
    }

    std::set<std::string> visiting;
    std::set<std::string> visited;
    std::function<void(const std::string&)> visit = [&](const std::string& id) {
        if (visited.contains(id)) {
            return;
        }
        if (!visiting.insert(id).second) {
            pushDiagnostic(diagnostics, EventDiagnosticSeverity::Error, "cyclic_common_event",
                           "Common event calls contain a cycle.", id);
            return;
        }
        for (const auto& next : common_edges[id]) {
            if (common_events_.contains(next)) {
                visit(next);
            }
        }
        visiting.erase(id);
        visited.insert(id);
    };
    for (const auto& [id, _] : common_events_) {
        visit(id);
    }

    std::sort(diagnostics.begin(), diagnostics.end(), [](const EventDiagnostic& left, const EventDiagnostic& right) {
        if (left.severity != right.severity) {
            return static_cast<uint8_t>(left.severity) > static_cast<uint8_t>(right.severity);
        }
        if (left.code != right.code) {
            return left.code < right.code;
        }
        return left.event_id < right.event_id;
    });
    return diagnostics;
}

nlohmann::json EventDocument::toJson() const {
    nlohmann::json json;
    json["maps"] = nlohmann::json::array();
    for (const auto& [_, map] : maps_) {
        json["maps"].push_back({{"id", map.id}, {"width", map.width}, {"height", map.height}});
    }
    json["events"] = nlohmann::json::array();
    for (const auto& event : events_) {
        nlohmann::json event_json = {{"id", event.id}, {"map_id", event.map_id}, {"x", event.x}, {"y", event.y}};
        event_json["pages"] = nlohmann::json::array();
        for (const auto& page : event.pages) {
            nlohmann::json page_json = {
                {"id", page.id},
                {"priority", page.priority},
                {"trigger", triggerToString(page.trigger)},
                {"conditions", conditionToJson(page.conditions)}
            };
            page_json["commands"] = nlohmann::json::array();
            for (const auto& command : page.commands) {
                page_json["commands"].push_back(commandToJson(command));
            }
            event_json["pages"].push_back(page_json);
        }
        json["events"].push_back(event_json);
    }
    json["common_events"] = nlohmann::json::array();
    for (const auto& [_, common_event] : common_events_) {
        nlohmann::json common_json = {{"id", common_event.id}, {"commands", nlohmann::json::array()}};
        for (const auto& command : common_event.commands) {
            common_json["commands"].push_back(commandToJson(command));
        }
        json["common_events"].push_back(common_json);
    }
    return json;
}

EventDocument EventDocument::fromJson(const nlohmann::json& json) {
    EventDocument document;
    for (const auto& map_json : json.value("maps", nlohmann::json::array())) {
        document.addMap(MapDefinition{
            stringValue(map_json, "id"),
            static_cast<int32_t>(intValue(map_json, "width")),
            static_cast<int32_t>(intValue(map_json, "height"))
        });
    }
    for (const auto& event_json : json.value("events", nlohmann::json::array())) {
        EventDefinition event;
        event.id = stringValue(event_json, "id");
        event.map_id = stringValue(event_json, "map_id");
        event.x = static_cast<int32_t>(intValue(event_json, "x"));
        event.y = static_cast<int32_t>(intValue(event_json, "y"));
        for (const auto& page_json : event_json.value("pages", nlohmann::json::array())) {
            EventPage page;
            page.id = stringValue(page_json, "id");
            page.priority = static_cast<int32_t>(intValue(page_json, "priority"));
            page.trigger = triggerFromString(stringValue(page_json, "trigger"));
            page.conditions = conditionFromJson(page_json.value("conditions", nlohmann::json::object()));
            for (const auto& command_json : page_json.value("commands", nlohmann::json::array())) {
                page.commands.push_back(commandFromJson(command_json));
            }
            event.pages.push_back(std::move(page));
        }
        document.addEvent(std::move(event));
    }
    for (const auto& common_json : json.value("common_events", nlohmann::json::array())) {
        CommonEventDefinition common_event;
        common_event.id = stringValue(common_json, "id");
        for (const auto& command_json : common_json.value("commands", nlohmann::json::array())) {
            common_event.commands.push_back(commandFromJson(command_json));
        }
        document.addCommonEvent(std::move(common_event));
    }
    return document;
}

std::string toString(EventCommandKind kind) {
    switch (kind) {
        case EventCommandKind::Message: return "message";
        case EventCommandKind::Switch: return "switch";
        case EventCommandKind::Variable: return "variable";
        case EventCommandKind::Transfer: return "transfer";
        case EventCommandKind::CommonEvent: return "common_event";
        case EventCommandKind::Battle: return "battle";
        case EventCommandKind::Item: return "item";
        case EventCommandKind::Gold: return "gold";
        case EventCommandKind::Wait: return "wait";
        case EventCommandKind::Fade: return "fade";
        case EventCommandKind::Sound: return "sound";
        case EventCommandKind::Plugin: return "plugin";
        case EventCommandKind::Unsupported: return "unsupported";
    }
    return "unsupported";
}

std::string toString(EventDiagnosticSeverity severity) {
    switch (severity) {
        case EventDiagnosticSeverity::Error: return "error";
        case EventDiagnosticSeverity::Warning: return "warning";
        case EventDiagnosticSeverity::Info: return "info";
    }
    return "info";
}

EventCommandKind eventCommandKindFromString(const std::string& value) {
    static const std::map<std::string, EventCommandKind> kinds{
        {"message", EventCommandKind::Message},
        {"switch", EventCommandKind::Switch},
        {"variable", EventCommandKind::Variable},
        {"transfer", EventCommandKind::Transfer},
        {"common_event", EventCommandKind::CommonEvent},
        {"battle", EventCommandKind::Battle},
        {"item", EventCommandKind::Item},
        {"gold", EventCommandKind::Gold},
        {"wait", EventCommandKind::Wait},
        {"fade", EventCommandKind::Fade},
        {"sound", EventCommandKind::Sound},
        {"plugin", EventCommandKind::Plugin}
    };
    const auto it = kinds.find(value);
    return it == kinds.end() ? EventCommandKind::Unsupported : it->second;
}

} // namespace urpg::events
