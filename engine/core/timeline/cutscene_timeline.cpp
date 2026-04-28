#include "engine/core/timeline/cutscene_timeline.h"

#include <algorithm>
#include <utility>

namespace urpg::timeline {

namespace {

CutsceneTrack trackFromJson(const nlohmann::json& json) {
    CutsceneTrack track;
    track.id = json.value("id", "");
    track.kind = cutsceneTrackKindFromString(json.value("kind", ""));
    track.actor_id = json.value("actor_id", "");
    return track;
}

nlohmann::json trackToJson(const CutsceneTrack& track) {
    return {{"id", track.id}, {"kind", toString(track.kind)}, {"actor_id", track.actor_id}};
}

CutsceneCue cueFromJson(const nlohmann::json& json) {
    CutsceneCue cue;
    cue.id = json.value("id", "");
    cue.track_id = json.value("track_id", "");
    cue.tick = json.value("tick", 0);
    cue.duration = json.value("duration", 0);
    cue.target = json.value("target", "");
    cue.localization_key = json.value("localization_key", "");
    cue.payload = json.value("payload", nlohmann::json::object());
    return cue;
}

nlohmann::json cueToJson(const CutsceneCue& cue) {
    return {{"id", cue.id},
            {"track_id", cue.track_id},
            {"tick", cue.tick},
            {"duration", cue.duration},
            {"target", cue.target},
            {"localization_key", cue.localization_key},
            {"payload", cue.payload}};
}

TimelineCommandKind timelineKindForTrack(CutsceneTrackKind kind) {
    switch (kind) {
    case CutsceneTrackKind::Dialogue:
    case CutsceneTrackKind::Choice:
        return TimelineCommandKind::Message;
    case CutsceneTrackKind::Camera:
        return TimelineCommandKind::Camera;
    case CutsceneTrackKind::Audio:
        return TimelineCommandKind::Audio;
    case CutsceneTrackKind::Variable:
    case CutsceneTrackKind::Event:
        return TimelineCommandKind::EventCall;
    case CutsceneTrackKind::Wait:
        return TimelineCommandKind::Wait;
    case CutsceneTrackKind::Unknown:
        return TimelineCommandKind::Unsupported;
    }
    return TimelineCommandKind::Unsupported;
}

} // namespace

std::vector<CutsceneTimelineDiagnostic> CutsceneTimelineDocument::validate(const std::set<std::string>& localization_keys) const {
    std::vector<CutsceneTimelineDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_cutscene_id", "Cutscene timeline is missing an id.", id});
    }
    std::set<std::string> actor_ids(actors.begin(), actors.end());
    std::set<std::string> track_ids;
    for (const auto& track : tracks) {
        if (track.id.empty()) {
            diagnostics.push_back({"missing_track_id", "Cutscene track is missing an id.", track.id});
        } else if (!track_ids.insert(track.id).second) {
            diagnostics.push_back({"duplicate_track_id", "Cutscene track id is duplicated.", track.id});
        }
        if (!track.actor_id.empty() && !actor_ids.contains(track.actor_id)) {
            diagnostics.push_back({"unknown_track_actor", "Cutscene track references an unknown actor: " + track.actor_id, track.id});
        }
        if (track.kind == CutsceneTrackKind::Unknown) {
            diagnostics.push_back({"unknown_track_kind", "Cutscene track has an unknown kind.", track.id});
        }
    }

    std::set<std::string> cue_ids;
    for (const auto& cue : cues) {
        if (cue.id.empty()) {
            diagnostics.push_back({"missing_cue_id", "Cutscene cue is missing an id.", cue.id});
        } else if (!cue_ids.insert(cue.id).second) {
            diagnostics.push_back({"duplicate_cue_id", "Cutscene cue id is duplicated.", cue.id});
        }
        if (!track_ids.contains(cue.track_id)) {
            diagnostics.push_back({"unknown_cue_track", "Cutscene cue references an unknown track: " + cue.track_id, cue.id});
        }
        if (cue.tick < 0) {
            diagnostics.push_back({"negative_cue_tick", "Cutscene cue tick cannot be negative.", cue.id});
        }
        if (cue.duration < 0) {
            diagnostics.push_back({"negative_cue_duration", "Cutscene cue duration cannot be negative.", cue.id});
        }
        if (!cue.localization_key.empty() && !localization_keys.empty() && !localization_keys.contains(cue.localization_key)) {
            diagnostics.push_back({"missing_localization_key", "Cutscene cue references a missing localization key: " + cue.localization_key, cue.id});
        }
    }
    return diagnostics;
}

TimelineDocument CutsceneTimelineDocument::toRuntimeTimeline() const {
    TimelineDocument document;
    for (const auto& actor : actors) {
        document.addActor(actor);
    }
    for (const auto& cue : cues) {
        const auto track = std::find_if(tracks.begin(), tracks.end(), [&](const auto& candidate) { return candidate.id == cue.track_id; });
        TimelineCommand command;
        command.id = cue.id;
        command.kind = track == tracks.end() ? TimelineCommandKind::Unsupported : timelineKindForTrack(track->kind);
        command.tick = cue.tick;
        command.duration = cue.duration;
        command.actor_id = track == tracks.end() ? std::string{} : track->actor_id;
        command.target = cue.target.empty() ? cue.localization_key : cue.target;
        command.payload = cue.payload;
        if (!cue.localization_key.empty()) {
            command.payload["localization_key"] = cue.localization_key;
        }
        document.addCommand(std::move(command));
    }
    return document;
}

CutsceneTimelinePreview CutsceneTimelineDocument::preview(int64_t cursor_tick,
                                                          uint64_t seed,
                                                          const std::set<std::string>& localization_keys) const {
    CutsceneTimelinePreview preview;
    preview.cutscene_id = id;
    preview.cursor_tick = cursor_tick;
    preview.diagnostics = validate(localization_keys);
    for (const auto& cue : cues) {
        const auto end_tick = cue.tick + std::max<int64_t>(cue.duration, 0);
        if (cursor_tick >= cue.tick && cursor_tick <= end_tick) {
            preview.active_cues.push_back(cue);
        }
        const auto track = std::find_if(tracks.begin(), tracks.end(), [&](const auto& candidate) { return candidate.id == cue.track_id; });
        if (track != tracks.end() && track->kind == CutsceneTrackKind::Variable && cursor_tick >= cue.tick && cue.payload.contains("name")) {
            preview.variable_writes[cue.payload.value("name", "")] = cue.payload.value("value", "");
        }
    }
    preview.played_commands = TimelinePlayer::play(toRuntimeTimeline(), seed);
    preview.played_commands.erase(std::remove_if(preview.played_commands.begin(),
                                                 preview.played_commands.end(),
                                                 [&](const auto& command) { return command.tick > cursor_tick; }),
                                  preview.played_commands.end());
    return preview;
}

nlohmann::json CutsceneTimelineDocument::toJson() const {
    nlohmann::json json;
    json["id"] = id;
    json["actors"] = actors;
    json["tracks"] = nlohmann::json::array();
    for (const auto& track : tracks) {
        json["tracks"].push_back(trackToJson(track));
    }
    json["cues"] = nlohmann::json::array();
    for (const auto& cue : cues) {
        json["cues"].push_back(cueToJson(cue));
    }
    return json;
}

CutsceneTimelineDocument CutsceneTimelineDocument::fromJson(const nlohmann::json& json) {
    CutsceneTimelineDocument document;
    document.id = json.value("id", "");
    document.actors = json.value("actors", std::vector<std::string>{});
    for (const auto& track_json : json.value("tracks", nlohmann::json::array())) {
        document.tracks.push_back(trackFromJson(track_json));
    }
    for (const auto& cue_json : json.value("cues", nlohmann::json::array())) {
        document.cues.push_back(cueFromJson(cue_json));
    }
    return document;
}

std::string toString(CutsceneTrackKind kind) {
    switch (kind) {
    case CutsceneTrackKind::Dialogue:
        return "dialogue";
    case CutsceneTrackKind::Choice:
        return "choice";
    case CutsceneTrackKind::Camera:
        return "camera";
    case CutsceneTrackKind::Audio:
        return "audio";
    case CutsceneTrackKind::Variable:
        return "variable";
    case CutsceneTrackKind::Event:
        return "event";
    case CutsceneTrackKind::Wait:
        return "wait";
    case CutsceneTrackKind::Unknown:
        return "unknown";
    }
    return "unknown";
}

CutsceneTrackKind cutsceneTrackKindFromString(const std::string& value) {
    if (value == "dialogue") {
        return CutsceneTrackKind::Dialogue;
    }
    if (value == "choice") {
        return CutsceneTrackKind::Choice;
    }
    if (value == "camera") {
        return CutsceneTrackKind::Camera;
    }
    if (value == "audio") {
        return CutsceneTrackKind::Audio;
    }
    if (value == "variable") {
        return CutsceneTrackKind::Variable;
    }
    if (value == "event") {
        return CutsceneTrackKind::Event;
    }
    if (value == "wait") {
        return CutsceneTrackKind::Wait;
    }
    return CutsceneTrackKind::Unknown;
}

nlohmann::json cutsceneTimelinePreviewToJson(const CutsceneTimelinePreview& preview) {
    nlohmann::json json{{"cutscene_id", preview.cutscene_id},
                        {"cursor_tick", preview.cursor_tick},
                        {"variable_writes", preview.variable_writes}};
    json["active_cues"] = nlohmann::json::array();
    for (const auto& cue : preview.active_cues) {
        json["active_cues"].push_back(cueToJson(cue));
    }
    json["played_commands"] = nlohmann::json::array();
    for (const auto& command : preview.played_commands) {
        json["played_commands"].push_back({{"tick", command.tick},
                                           {"seed", command.seed},
                                           {"command_id", command.command_id},
                                           {"kind", toString(command.kind)},
                                           {"actor_id", command.actor_id},
                                           {"target", command.target}});
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return json;
}

} // namespace urpg::timeline
