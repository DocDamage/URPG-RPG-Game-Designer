#pragma once

#include "engine/core/presentation/effects/effect_cue.h"
#include "engine/core/timeline/timeline_document.h"

#include <cstdint>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::scene {
class BattleScene;
}

namespace urpg::battle {

struct BattleVfxTimelineEvent {
    std::string id;
    std::string track_id;
    int32_t frame = 0;
    std::string label;
    presentation::effects::EffectCueKind kind = presentation::effects::EffectCueKind::Gameplay;
    presentation::effects::EffectAnchorMode anchor = presentation::effects::EffectAnchorMode::Target;
    uint64_t source_id = 0;
    uint64_t owner_id = 0;
    float intensity = 1.0f;
    float overlay_emphasis = 0.0f;
    nlohmann::json payload = nlohmann::json::object();

    bool operator==(const BattleVfxTimelineEvent& other) const = default;
};

struct BattleVfxTimelineTrack {
    std::string id;
    std::string label;
    std::string kind;
    bool visible = true;
    bool locked = false;

    bool operator==(const BattleVfxTimelineTrack& other) const = default;
};

struct BattleVfxTimelineDiagnostic {
    std::string code;
    std::string message;
    std::string event_id;
};

class BattleVfxTimelineDocument {
public:
    std::string id = "battle_vfx_timeline";
    int32_t fps = 60;
    int32_t duration_frames = 120;

    void addTrack(BattleVfxTimelineTrack track);
    void addEvent(BattleVfxTimelineEvent event);
    bool setTrackVisible(std::string track_id, bool visible);
    void clearEvents();

    const std::vector<BattleVfxTimelineTrack>& tracks() const { return tracks_; }
    const std::vector<BattleVfxTimelineEvent>& events() const { return events_; }
    std::vector<BattleVfxTimelineEvent> sortedEvents() const;
    std::vector<BattleVfxTimelineEvent> eventsAtFrame(int32_t frame) const;
    std::vector<BattleVfxTimelineDiagnostic> validate() const;
    std::vector<std::string> runtimeCommandsAtFrame(int32_t frame) const;

    timeline::TimelineDocument toTimelineDocument() const;
    std::vector<presentation::effects::EffectCue> toEffectCues() const;
    nlohmann::json toJson() const;

    static BattleVfxTimelineDocument fromJson(const nlohmann::json& json);
    static BattleVfxTimelineDocument fromPresentationJson(const nlohmann::json& json);

private:
    std::vector<BattleVfxTimelineTrack> tracks_;
    std::vector<BattleVfxTimelineEvent> events_;
};

std::string toString(presentation::effects::EffectCueKind kind);
presentation::effects::EffectCueKind effectCueKindFromString(const std::string& value);
std::string toString(presentation::effects::EffectAnchorMode anchor);
presentation::effects::EffectAnchorMode effectAnchorModeFromString(const std::string& value);
std::size_t applyBattleVfxTimeline(scene::BattleScene& scene, const BattleVfxTimelineDocument& document);

} // namespace urpg::battle
