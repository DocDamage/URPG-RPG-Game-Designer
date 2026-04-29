#include "engine/core/battle/battle_vfx_timeline.h"

#include "engine/core/scene/battle_scene.h"

#include <algorithm>
#include <map>
#include <tuple>
#include <utility>

namespace urpg::battle {

namespace {

using presentation::effects::EffectAnchorMode;
using presentation::effects::EffectCue;
using presentation::effects::EffectCueKind;

void addDiagnostic(std::vector<BattleVfxTimelineDiagnostic>& diagnostics,
                   std::string code,
                   std::string message,
                   std::string event_id) {
    diagnostics.push_back({std::move(code), std::move(message), std::move(event_id)});
}

BattleVfxTimelineEvent eventFromJson(const nlohmann::json& json) {
    BattleVfxTimelineEvent event;
    event.id = json.value("id", "");
    event.track_id = json.value("track_id", json.value("trackId", ""));
    event.frame = json.value("frame", 0);
    event.label = json.value("label", "");
    event.kind = effectCueKindFromString(json.value("kind", json.value("type", "gameplay")));
    event.anchor = effectAnchorModeFromString(json.value("anchor", "target"));
    event.source_id = json.value("source_id", json.value("sourceId", uint64_t{0}));
    event.owner_id = json.value("owner_id", json.value("ownerId", uint64_t{0}));
    event.intensity = json.value("intensity", 1.0f);
    event.overlay_emphasis = json.value("overlay_emphasis", json.value("overlayEmphasis", 0.0f));
    event.payload = json.value("payload", nlohmann::json::object());
    if (event.label.empty() && event.payload.is_string()) {
        event.label = event.payload.get<std::string>();
    }
    return event;
}

BattleVfxTimelineTrack trackFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("label", ""),
            json.value("kind", ""),
            json.value("visible", true),
            json.value("locked", false)};
}

nlohmann::json eventToJson(const BattleVfxTimelineEvent& event) {
    nlohmann::json json;
    json["id"] = event.id;
    json["track_id"] = event.track_id;
    json["frame"] = event.frame;
    json["kind"] = toString(event.kind);
    json["anchor"] = toString(event.anchor);
    json["source_id"] = event.source_id;
    json["owner_id"] = event.owner_id;
    json["intensity"] = event.intensity;
    json["overlay_emphasis"] = event.overlay_emphasis;
    if (!event.label.empty()) {
        json["label"] = event.label;
    }
    if (!event.payload.empty()) {
        json["payload"] = event.payload;
    }
    return json;
}

nlohmann::json trackToJson(const BattleVfxTimelineTrack& track) {
    return {{"id", track.id},
            {"label", track.label},
            {"kind", track.kind},
            {"visible", track.visible},
            {"locked", track.locked}};
}

EffectCue eventToCue(const BattleVfxTimelineEvent& event) {
    EffectCue cue;
    cue.frameTick = static_cast<uint64_t>(std::max(event.frame, 0));
    cue.sourceId = event.source_id;
    cue.ownerId = event.owner_id;
    cue.kind = event.kind;
    cue.anchorMode = event.anchor;
    cue.intensity = {std::max(0.0f, event.intensity)};
    cue.overlayEmphasis = {std::clamp(event.overlay_emphasis, 0.0f, 1.0f)};
    return cue;
}

} // namespace

void BattleVfxTimelineDocument::addTrack(BattleVfxTimelineTrack track) {
    tracks_.push_back(std::move(track));
}

void BattleVfxTimelineDocument::addEvent(BattleVfxTimelineEvent event) {
    events_.push_back(std::move(event));
}

bool BattleVfxTimelineDocument::setTrackVisible(std::string track_id, bool visible) {
    const auto found = std::find_if(tracks_.begin(), tracks_.end(), [&](const auto& track) {
        return track.id == track_id;
    });
    if (found == tracks_.end()) {
        return false;
    }
    found->visible = visible;
    return true;
}

void BattleVfxTimelineDocument::clearEvents() {
    events_.clear();
}

std::vector<BattleVfxTimelineEvent> BattleVfxTimelineDocument::sortedEvents() const {
    auto sorted = events_;
    std::stable_sort(sorted.begin(), sorted.end(), [](const auto& lhs, const auto& rhs) {
        return std::tie(lhs.frame, lhs.id, lhs.owner_id, lhs.source_id) <
               std::tie(rhs.frame, rhs.id, rhs.owner_id, rhs.source_id);
    });
    return sorted;
}

std::vector<BattleVfxTimelineEvent> BattleVfxTimelineDocument::eventsAtFrame(int32_t frame) const {
    std::vector<BattleVfxTimelineEvent> result;
    for (const auto& event : sortedEvents()) {
        if (event.frame == frame) {
            result.push_back(event);
        }
    }
    return result;
}

std::vector<BattleVfxTimelineDiagnostic> BattleVfxTimelineDocument::validate() const {
    std::vector<BattleVfxTimelineDiagnostic> diagnostics;
    std::map<std::string, int> ids;

    if (id.empty()) {
        addDiagnostic(diagnostics, "missing_timeline_id", "Battle VFX timeline is missing an id.", {});
    }
    if (fps <= 0) {
        addDiagnostic(diagnostics, "invalid_fps", "Battle VFX timeline FPS must be positive.", {});
    }
    if (duration_frames <= 0) {
        addDiagnostic(diagnostics, "invalid_duration", "Battle VFX timeline duration must be positive.", {});
    }

    std::map<std::string, int> track_ids;
    for (const auto& track : tracks_) {
        if (track.id.empty()) {
            addDiagnostic(diagnostics, "missing_track_id", "Battle VFX track is missing an id.", {});
        } else if (++track_ids[track.id] > 1) {
            addDiagnostic(diagnostics, "duplicate_track_id", "Battle VFX track id is duplicated.", track.id);
        }
        if (track.label.empty()) {
            addDiagnostic(diagnostics, "missing_track_label", "Battle VFX track is missing a label.", track.id);
        }
    }

    for (const auto& event : events_) {
        if (event.id.empty()) {
            addDiagnostic(diagnostics, "missing_event_id", "Battle VFX event is missing an id.", event.id);
        } else if (++ids[event.id] > 1) {
            addDiagnostic(diagnostics, "duplicate_event_id", "Battle VFX event id is duplicated.", event.id);
        }
        if (event.frame < 0) {
            addDiagnostic(diagnostics, "negative_event_frame", "Battle VFX event frame cannot be negative.", event.id);
        }
        if (event.frame > duration_frames) {
            addDiagnostic(diagnostics, "event_after_duration", "Battle VFX event falls after the timeline duration.", event.id);
        }
        if (!event.track_id.empty() && !track_ids.contains(event.track_id)) {
            addDiagnostic(diagnostics, "unknown_event_track", "Battle VFX event references an unknown track.", event.id);
        }
        if ((event.anchor == EffectAnchorMode::Owner || event.anchor == EffectAnchorMode::Target) &&
            event.source_id == 0 && event.owner_id == 0) {
            addDiagnostic(diagnostics, "missing_anchor_participant",
                          "Owner/target VFX events require a source_id or owner_id.", event.id);
        }
        if (event.intensity < 0.0f) {
            addDiagnostic(diagnostics, "negative_intensity", "Battle VFX event intensity cannot be negative.", event.id);
        }
        if (event.overlay_emphasis < 0.0f || event.overlay_emphasis > 1.0f) {
            addDiagnostic(diagnostics, "overlay_emphasis_out_of_range",
                          "Battle VFX event overlay emphasis must be between 0 and 1.", event.id);
        }
    }

    std::stable_sort(diagnostics.begin(), diagnostics.end(), [](const auto& lhs, const auto& rhs) {
        return std::tie(lhs.code, lhs.event_id) < std::tie(rhs.code, rhs.event_id);
    });
    return diagnostics;
}

std::vector<std::string> BattleVfxTimelineDocument::runtimeCommandsAtFrame(int32_t frame) const {
    std::vector<std::string> commands;
    const auto clamped_frame = std::clamp(frame, 0, std::max(0, duration_frames));
    commands.push_back("battle_vfx_timeline:" + id + ":frame:" + std::to_string(clamped_frame));
    for (const auto& event : eventsAtFrame(clamped_frame)) {
        std::string asset;
        if (event.payload.is_object()) {
            asset = event.payload.value("asset", "");
        }
        auto command = "battle_vfx_cue:" + event.id + ":" + toString(event.kind);
        if (!event.track_id.empty()) {
            command += ":track=" + event.track_id;
        }
        if (!asset.empty()) {
            command += ":asset=" + asset;
        }
        commands.push_back(std::move(command));
    }
    return commands;
}

timeline::TimelineDocument BattleVfxTimelineDocument::toTimelineDocument() const {
    timeline::TimelineDocument document;
    for (const auto& event : sortedEvents()) {
        timeline::TimelineCommand command;
        command.id = event.id;
        command.kind = timeline::TimelineCommandKind::BattleCue;
        command.tick = event.frame;
        command.target = event.label.empty() ? toString(event.kind) : event.label;
        command.payload = eventToJson(event);
        document.addCommand(std::move(command));
    }
    return document;
}

std::vector<EffectCue> BattleVfxTimelineDocument::toEffectCues() const {
    std::vector<EffectCue> cues;
    for (const auto& event : sortedEvents()) {
        cues.push_back(eventToCue(event));
    }
    return cues;
}

nlohmann::json BattleVfxTimelineDocument::toJson() const {
    nlohmann::json json;
    json["schema_version"] = "urpg.battle_vfx_timeline.v1";
    json["id"] = id;
    json["fps"] = fps;
    json["duration_frames"] = duration_frames;
    json["tracks"] = nlohmann::json::array();
    for (const auto& track : tracks_) {
        json["tracks"].push_back(trackToJson(track));
    }
    json["events"] = nlohmann::json::array();
    for (const auto& event : sortedEvents()) {
        json["events"].push_back(eventToJson(event));
    }
    return json;
}

BattleVfxTimelineDocument BattleVfxTimelineDocument::fromJson(const nlohmann::json& json) {
    BattleVfxTimelineDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", document.id);
    document.fps = json.value("fps", document.fps);
    document.duration_frames = json.value("duration_frames", json.value("durationFrames", document.duration_frames));
    for (const auto& row : json.value("tracks", nlohmann::json::array())) {
        if (row.is_object()) {
            document.addTrack(trackFromJson(row));
        }
    }
    for (const auto& row : json.value("events", nlohmann::json::array())) {
        if (row.is_object()) {
            document.addEvent(eventFromJson(row));
        }
    }
    return document;
}

BattleVfxTimelineDocument BattleVfxTimelineDocument::fromPresentationJson(const nlohmann::json& json) {
    BattleVfxTimelineDocument document;
    document.id = json.value("id", "battle_presentation_vfx");
    int32_t maxFrame = 0;
    for (const auto& row : json.value("cue_timeline", nlohmann::json::array())) {
        if (!row.is_object()) {
            continue;
        }
        auto event = eventFromJson(row);
        event.frame = row.value("frame", event.frame);
        event.label = row.value("payload", event.label);
        maxFrame = std::max(maxFrame, event.frame);
        document.addEvent(std::move(event));
    }
    document.duration_frames = std::max(document.duration_frames, maxFrame);
    return document;
}

std::string toString(EffectCueKind kind) {
    switch (kind) {
    case EffectCueKind::Gameplay: return "gameplay";
    case EffectCueKind::Status: return "status";
    case EffectCueKind::System: return "system";
    case EffectCueKind::CastStart: return "cast";
    case EffectCueKind::HitConfirm: return "hit";
    case EffectCueKind::CriticalHit: return "critical";
    case EffectCueKind::BloodSplatter: return "blood";
    case EffectCueKind::GuardClash: return "guard";
    case EffectCueKind::MissSweep: return "miss";
    case EffectCueKind::HealPulse: return "heal";
    case EffectCueKind::DefeatFade: return "death";
    case EffectCueKind::PhaseBanner: return "phase_transition";
    }
    return "gameplay";
}

EffectCueKind effectCueKindFromString(const std::string& value) {
    if (value == "status") return EffectCueKind::Status;
    if (value == "system") return EffectCueKind::System;
    if (value == "cast" || value == "cast_start") return EffectCueKind::CastStart;
    if (value == "hit" || value == "hit_confirm") return EffectCueKind::HitConfirm;
    if (value == "critical" || value == "critical_hit") return EffectCueKind::CriticalHit;
    if (value == "blood" || value == "blood_splatter") return EffectCueKind::BloodSplatter;
    if (value == "guard" || value == "guard_clash") return EffectCueKind::GuardClash;
    if (value == "miss" || value == "miss_sweep") return EffectCueKind::MissSweep;
    if (value == "heal" || value == "heal_pulse") return EffectCueKind::HealPulse;
    if (value == "death" || value == "defeat_fade") return EffectCueKind::DefeatFade;
    if (value == "phase_transition" || value == "phase_banner" || value == "victory" || value == "defeat") {
        return EffectCueKind::PhaseBanner;
    }
    return EffectCueKind::Gameplay;
}

std::string toString(EffectAnchorMode anchor) {
    switch (anchor) {
    case EffectAnchorMode::World: return "world";
    case EffectAnchorMode::Owner: return "owner";
    case EffectAnchorMode::Target: return "target";
    case EffectAnchorMode::Screen: return "screen";
    case EffectAnchorMode::Overlay: return "overlay";
    }
    return "world";
}

EffectAnchorMode effectAnchorModeFromString(const std::string& value) {
    if (value == "owner") return EffectAnchorMode::Owner;
    if (value == "target") return EffectAnchorMode::Target;
    if (value == "screen") return EffectAnchorMode::Screen;
    if (value == "overlay") return EffectAnchorMode::Overlay;
    return EffectAnchorMode::World;
}

std::size_t applyBattleVfxTimeline(scene::BattleScene& scene, const BattleVfxTimelineDocument& document) {
    std::size_t applied = 0;
    for (const auto& cue : document.toEffectCues()) {
        scene.enqueueEffectCue(cue);
        ++applied;
    }
    return applied;
}

} // namespace urpg::battle
