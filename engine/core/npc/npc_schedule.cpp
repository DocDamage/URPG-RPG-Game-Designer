#include "engine/core/npc/npc_schedule.h"

#include <algorithm>
#include <utility>

namespace urpg::npc {

namespace {

GridPosition positionFromJson(const nlohmann::json& json) {
    return {json.value("x", 0), json.value("y", 0)};
}

nlohmann::json positionToJson(const GridPosition& position) {
    return {{"x", position.x}, {"y", position.y}};
}

NpcRoutine routineFromJson(const nlohmann::json& json) {
    NpcRoutine routine;
    routine.npc_id = json.value("npc_id", "");
    routine.map_id = json.value("map_id", "");
    routine.start_hour = json.value("start_hour", 0);
    routine.end_hour = json.value("end_hour", 23);
    routine.position = positionFromJson(json.value("position", nlohmann::json::object()));
    routine.animation = json.value("animation", "");
    routine.dialogue_state = json.value("dialogue_state", "");
    return routine;
}

nlohmann::json routineToJson(const NpcRoutine& routine) {
    return {{"npc_id", routine.npc_id},
            {"map_id", routine.map_id},
            {"start_hour", routine.start_hour},
            {"end_hour", routine.end_hour},
            {"position", positionToJson(routine.position)},
            {"animation", routine.animation},
            {"dialogue_state", routine.dialogue_state}};
}

NpcFallback fallbackFromJson(const nlohmann::json& json) {
    NpcFallback fallback;
    fallback.map_id = json.value("map_id", "");
    fallback.position = positionFromJson(json.value("position", nlohmann::json::object()));
    fallback.animation = json.value("animation", "");
    fallback.dialogue_state = json.value("dialogue_state", "");
    return fallback;
}

nlohmann::json fallbackToJson(const NpcFallback& fallback) {
    return {{"map_id", fallback.map_id},
            {"position", positionToJson(fallback.position)},
            {"animation", fallback.animation},
            {"dialogue_state", fallback.dialogue_state}};
}

bool overlaps(const NpcRoutine& lhs, const NpcRoutine& rhs) {
    return lhs.npc_id == rhs.npc_id && lhs.start_hour <= rhs.end_hour && rhs.start_hour <= lhs.end_hour;
}

} // namespace

void NpcSchedule::addRoutine(NpcRoutine routine) {
    routines_.push_back(std::move(routine));
}

void NpcSchedule::setFallback(NpcFallback fallback) {
    fallback_ = std::move(fallback);
}

NpcResolvedState NpcSchedule::resolve(const std::string& npc_id, int hour, const std::set<std::string>& available_maps) const {
    for (const auto& routine : routines_) {
        if (routine.npc_id == npc_id && hour >= routine.start_hour && hour <= routine.end_hour && available_maps.contains(routine.map_id)) {
            return {false, routine.map_id, routine.position, routine.animation, routine.dialogue_state};
        }
    }
    return {true, fallback_.map_id, fallback_.position, fallback_.animation, fallback_.dialogue_state};
}

std::vector<NpcScheduleDiagnostic> NpcScheduleDocument::validate(const std::set<std::string>& known_maps) const {
    std::vector<NpcScheduleDiagnostic> diagnostics;
    if (fallback.map_id.empty()) {
        diagnostics.push_back({"missing_fallback_map", "NPC schedule fallback is missing a map id.", {}});
    } else if (!known_maps.empty() && !known_maps.contains(fallback.map_id)) {
        diagnostics.push_back({"unknown_fallback_map", "NPC schedule fallback references an unknown map: " + fallback.map_id, {}});
    }

    for (std::size_t index = 0; index < routines.size(); ++index) {
        const auto& routine = routines[index];
        if (routine.npc_id.empty()) {
            diagnostics.push_back({"missing_npc_id", "NPC routine is missing an npc id.", routine.npc_id});
        }
        if (routine.map_id.empty()) {
            diagnostics.push_back({"missing_routine_map", "NPC routine is missing a map id.", routine.npc_id});
        } else if (!known_maps.empty() && !known_maps.contains(routine.map_id)) {
            diagnostics.push_back({"unknown_routine_map", "NPC routine references an unknown map: " + routine.map_id, routine.npc_id});
        }
        if (routine.start_hour < 0 || routine.start_hour > 23 || routine.end_hour < 0 || routine.end_hour > 23 || routine.start_hour > routine.end_hour) {
            diagnostics.push_back({"invalid_routine_hours", "NPC routine has invalid daily hours.", routine.npc_id});
        }
        for (std::size_t other = index + 1; other < routines.size(); ++other) {
            if (overlaps(routine, routines[other])) {
                diagnostics.push_back({"overlapping_routine", "NPC routines overlap in the daily schedule.", routine.npc_id});
            }
        }
    }
    return diagnostics;
}

NpcSchedule NpcScheduleDocument::toRuntimeSchedule() const {
    NpcSchedule schedule;
    for (const auto& routine : routines) {
        schedule.addRoutine(routine);
    }
    schedule.setFallback(fallback);
    return schedule;
}

NpcSchedulePreview NpcScheduleDocument::preview(const std::string& npc_id, int hour, const std::set<std::string>& available_maps) const {
    NpcSchedulePreview preview;
    preview.npc_id = npc_id;
    preview.hour = hour;
    preview.diagnostics = validate(available_maps);
    preview.state = toRuntimeSchedule().resolve(npc_id, hour, available_maps);
    return preview;
}

nlohmann::json NpcScheduleDocument::toJson() const {
    nlohmann::json json;
    json["version"] = version;
    json["routines"] = nlohmann::json::array();
    for (const auto& routine : routines) {
        json["routines"].push_back(routineToJson(routine));
    }
    json["fallback"] = fallbackToJson(fallback);
    return json;
}

NpcScheduleDocument NpcScheduleDocument::fromJson(const nlohmann::json& json) {
    NpcScheduleDocument document;
    document.version = json.value("version", "1.0.0");
    for (const auto& routine_json : json.value("routines", nlohmann::json::array())) {
        document.routines.push_back(routineFromJson(routine_json));
    }
    document.fallback = fallbackFromJson(json.value("fallback", nlohmann::json::object()));
    return document;
}

nlohmann::json npcSchedulePreviewToJson(const NpcSchedulePreview& preview) {
    nlohmann::json json{{"npc_id", preview.npc_id},
                        {"hour", preview.hour},
                        {"state",
                         {{"used_fallback", preview.state.usedFallback},
                          {"map_id", preview.state.mapId},
                          {"position", positionToJson(preview.state.position)},
                          {"animation", preview.state.animation},
                          {"dialogue_state", preview.state.dialogueState}}}};
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"npc_id", diagnostic.npc_id}});
    }
    return json;
}

} // namespace urpg::npc
