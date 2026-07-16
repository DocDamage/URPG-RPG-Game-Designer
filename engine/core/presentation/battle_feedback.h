#pragma once

#include "engine/core/presentation/runtime_presentation_bible.h"
#include "engine/core/presentation/runtime_feedback_stack.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::presentation {

enum class BattleFeedbackKind : uint8_t {
    Selection,
    Targeting,
    TurnState,
    Anticipation,
    Impact,
    Damage,
    Heal,
    Status,
    Victory,
    Defeat,
    Results
};

enum class BattlePlaybackMode : uint8_t { Normal, FastForward, Skip };
enum class BattleColorFilter : uint8_t { None, Protanopia, Deuteranopia, Tritanopia, Monochrome };

struct BattleFeedbackSettings {
    bool audio_enabled = true;
    bool reduced_motion = false;
    BattleColorFilter color_filter = BattleColorFilter::None;
    BattlePlaybackMode playback_mode = BattlePlaybackMode::Normal;
    bool haptics_enabled = true;
    bool particles_enabled = true;
    bool screen_shake_enabled = true;
    bool hit_stop_enabled = true;
};

struct BattleFeedbackRequest {
    std::string id;
    BattleFeedbackKind kind = BattleFeedbackKind::TurnState;
    std::string text;
    std::string source_id;
    std::string target_id;
    int32_t amount = 0;
    bool critical = false;
};

struct BattleFeedbackCue {
    uint64_t sequence = 0;
    BattleFeedbackKind kind = BattleFeedbackKind::TurnState;
    std::string request_id;
    std::string source_id;
    std::string target_id;
    std::string text;
    std::string icon;
    std::string shape_cue;
    std::string sound;
    std::string particle_motif;
    std::string camera_cue;
    std::string haptic_cue;
    std::string feedback_tier;
    int32_t amount = 0;
    uint32_t duration_ms = 0;
    uint32_t elapsed_ms = 0;
    uint32_t hit_stop_ms = 0;
    float screen_shake_pixels = 0.0F;
    bool critical = false;
    bool non_color_cue = true;
    bool control_blocking = false;
    bool skippable = true;
    bool completed = false;
};

struct BattleFeedbackResult {
    bool success = false;
    std::string code;
    std::string message;
    std::optional<uint64_t> sequence;
};

struct BattleFeedbackSnapshot {
    BattleFeedbackSettings settings;
    std::optional<BattleFeedbackCue> active;
    std::vector<BattleFeedbackCue> queued;
    std::vector<BattleFeedbackCue> timeline;
};

class BattleFeedbackDirector {
public:
    explicit BattleFeedbackDirector(RuntimePresentationBible bible = makeRuntimePresentationBible());

    void setSettings(BattleFeedbackSettings settings);
    BattleFeedbackResult submit(BattleFeedbackRequest request);
    void advance(uint32_t delta_ms);
    void clear();

    const std::optional<BattleFeedbackCue>& activeCue() const { return active_; }
    BattleFeedbackSnapshot snapshot() const;

private:
    BattleFeedbackCue buildCue(const BattleFeedbackRequest& request);
    void startNext();

    RuntimePresentationBible bible_;
    BattleFeedbackSettings settings_;
    std::optional<BattleFeedbackCue> active_;
    std::vector<BattleFeedbackCue> queue_;
    std::vector<BattleFeedbackCue> timeline_;
    uint64_t next_sequence_ = 1;
};

const char* battleFeedbackKindName(BattleFeedbackKind kind);

} // namespace urpg::presentation
