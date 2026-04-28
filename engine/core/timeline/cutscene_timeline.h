#pragma once

#include "engine/core/timeline/timeline_document.h"
#include "engine/core/timeline/timeline_player.h"

#include <nlohmann/json.hpp>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::timeline {

enum class CutsceneTrackKind : uint8_t {
    Dialogue,
    Choice,
    Camera,
    Audio,
    Variable,
    Event,
    Wait,
    Unknown
};

struct CutsceneTrack {
    std::string id;
    CutsceneTrackKind kind = CutsceneTrackKind::Unknown;
    std::string actor_id;
};

struct CutsceneCue {
    std::string id;
    std::string track_id;
    int64_t tick = 0;
    int64_t duration = 0;
    std::string target;
    std::string localization_key;
    nlohmann::json payload = nlohmann::json::object();
};

struct CutsceneTimelineDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct CutsceneTimelinePreview {
    std::string cutscene_id;
    int64_t cursor_tick = 0;
    std::vector<CutsceneCue> active_cues;
    std::vector<PlayedTimelineCommand> played_commands;
    std::map<std::string, std::string> variable_writes;
    std::vector<CutsceneTimelineDiagnostic> diagnostics;
};

class CutsceneTimelineDocument {
public:
    std::string id;
    std::vector<std::string> actors;
    std::vector<CutsceneTrack> tracks;
    std::vector<CutsceneCue> cues;

    [[nodiscard]] std::vector<CutsceneTimelineDiagnostic> validate(const std::set<std::string>& localization_keys = {}) const;
    [[nodiscard]] TimelineDocument toRuntimeTimeline() const;
    [[nodiscard]] CutsceneTimelinePreview preview(int64_t cursor_tick,
                                                  uint64_t seed,
                                                  const std::set<std::string>& localization_keys = {}) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static CutsceneTimelineDocument fromJson(const nlohmann::json& json);
};

std::string toString(CutsceneTrackKind kind);
CutsceneTrackKind cutsceneTrackKindFromString(const std::string& value);
nlohmann::json cutsceneTimelinePreviewToJson(const CutsceneTimelinePreview& preview);

} // namespace urpg::timeline
