#include "engine/core/timeline/timeline_document.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <utility>

namespace urpg::timeline {

namespace {

bool requiresActor(TimelineCommandKind kind) {
    return kind == TimelineCommandKind::Movement || kind == TimelineCommandKind::Camera;
}

TimelineCommand commandFromJson(const nlohmann::json& json) {
    TimelineCommand command;
    command.id = json.value("id", "");
    command.kind = timelineCommandKindFromString(json.value("kind", ""));
    command.tick = json.value("tick", int64_t{0});
    command.duration = json.value("duration", int64_t{0});
    command.actor_id = json.value("actor_id", "");
    command.target = json.value("target", "");
    command.payload = json.value("payload", nlohmann::json::object());
    return command;
}

nlohmann::json commandToJson(const TimelineCommand& command) {
    nlohmann::json json;
    json["id"] = command.id;
    json["kind"] = toString(command.kind);
    json["tick"] = command.tick;
    json["duration"] = command.duration;
    if (!command.actor_id.empty()) {
        json["actor_id"] = command.actor_id;
    }
    if (!command.target.empty()) {
        json["target"] = command.target;
    }
    if (!command.payload.empty()) {
        json["payload"] = command.payload;
    }
    return json;
}

} // namespace

void TimelineDocument::addActor(std::string actor_id) {
    if (!actor_id.empty()) {
        actors_.insert(std::move(actor_id));
    }
}

void TimelineDocument::addCommand(TimelineCommand command) {
    commands_.push_back(std::move(command));
}

std::vector<TimelineCommand> TimelineDocument::sortedCommands() const {
    auto sorted = commands_;
    std::stable_sort(sorted.begin(), sorted.end(), [](const auto& lhs, const auto& rhs) {
        return std::tie(lhs.tick, lhs.id, lhs.target) < std::tie(rhs.tick, rhs.id, rhs.target);
    });
    return sorted;
}

std::vector<TimelineDiagnostic> TimelineDocument::validate() const {
    std::vector<TimelineDiagnostic> diagnostics;
    std::map<std::string, int> ids;

    for (const auto& command : commands_) {
        if (command.id.empty()) {
            diagnostics.push_back({"missing_command_id", "Timeline command is missing an id.", command.id});
        } else if (++ids[command.id] > 1) {
            diagnostics.push_back({"duplicate_command_id", "Timeline command id is duplicated.", command.id});
        }

        if (command.tick < 0) {
            diagnostics.push_back({"negative_tick", "Timeline command tick cannot be negative.", command.id});
        }
        if (requiresActor(command.kind) && (command.actor_id.empty() || actors_.count(command.actor_id) == 0)) {
            diagnostics.push_back({"missing_actor", "Timeline command references an unknown actor.", command.id});
        }
        if (command.kind == TimelineCommandKind::Wait && command.duration <= 0) {
            diagnostics.push_back({"non_positive_wait", "Wait commands must have a positive duration.", command.id});
        }
        if (command.kind == TimelineCommandKind::Unsupported) {
            diagnostics.push_back({"unsupported_timeline_command", "Timeline command kind is unsupported and preserved as payload.", command.id});
        }
    }

    return diagnostics;
}

nlohmann::json TimelineDocument::toJson() const {
    nlohmann::json json;
    json["schema_version"] = "urpg.timeline.v1";
    json["actors"] = actors_;
    json["commands"] = nlohmann::json::array();
    for (const auto& command : sortedCommands()) {
        json["commands"].push_back(commandToJson(command));
    }
    return json;
}

TimelineDocument TimelineDocument::fromJson(const nlohmann::json& json) {
    TimelineDocument document;
    for (const auto& actor : json.value("actors", nlohmann::json::array())) {
        document.addActor(actor.get<std::string>());
    }
    for (const auto& command : json.value("commands", nlohmann::json::array())) {
        document.addCommand(commandFromJson(command));
    }
    return document;
}

std::string toString(TimelineCommandKind kind) {
    switch (kind) {
    case TimelineCommandKind::Message:
        return "message";
    case TimelineCommandKind::Movement:
        return "movement";
    case TimelineCommandKind::Audio:
        return "audio";
    case TimelineCommandKind::Fade:
        return "fade";
    case TimelineCommandKind::Tint:
        return "tint";
    case TimelineCommandKind::Camera:
        return "camera";
    case TimelineCommandKind::Wait:
        return "wait";
    case TimelineCommandKind::EventCall:
        return "event_call";
    case TimelineCommandKind::BattleCue:
        return "battle_cue";
    case TimelineCommandKind::Unsupported:
        return "unsupported";
    }
    return "unsupported";
}

TimelineCommandKind timelineCommandKindFromString(const std::string& value) {
    if (value == "message") {
        return TimelineCommandKind::Message;
    }
    if (value == "movement") {
        return TimelineCommandKind::Movement;
    }
    if (value == "audio") {
        return TimelineCommandKind::Audio;
    }
    if (value == "fade") {
        return TimelineCommandKind::Fade;
    }
    if (value == "tint") {
        return TimelineCommandKind::Tint;
    }
    if (value == "camera") {
        return TimelineCommandKind::Camera;
    }
    if (value == "wait") {
        return TimelineCommandKind::Wait;
    }
    if (value == "event_call") {
        return TimelineCommandKind::EventCall;
    }
    if (value == "battle_cue") {
        return TimelineCommandKind::BattleCue;
    }
    return TimelineCommandKind::Unsupported;
}

} // namespace urpg::timeline
