#pragma once

#include "engine/core/presentation/runtime_presentation_bible.h"
#include "engine/core/presentation/runtime_feedback_stack.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::presentation {

enum class ExplorationFeedbackKind : uint8_t {
    InteractionPrompt,
    InteractionAcknowledged,
    Pickup,
    Reward,
    Door,
    Transfer,
    QuestUpdate,
    DialogueLine,
    ScreenTransition
};

struct ExplorationFeedbackSettings {
    bool audio_enabled = true;
    bool reduced_motion = false;
    bool captions_enabled = true;
    float dialogue_speed = 1.0F;
    bool haptics_enabled = true;
    bool particles_enabled = true;
    bool screen_shake_enabled = true;
    bool hit_stop_enabled = true;
};

struct ExplorationFeedbackRequest {
    std::string id;
    ExplorationFeedbackKind kind = ExplorationFeedbackKind::InteractionPrompt;
    std::string text;
    std::string source_id;
    std::string contextual_sound;
};

struct ExplorationFeedbackCue {
    uint64_t sequence = 0;
    ExplorationFeedbackKind kind = ExplorationFeedbackKind::InteractionPrompt;
    std::string request_id;
    std::string source_id;
    std::string text;
    std::string icon;
    std::string sound;
    std::string particle_motif;
    std::string camera_cue;
    std::string haptic_cue;
    std::string feedback_tier;
    float screen_shake_pixels = 0.0F;
    uint32_t hit_stop_ms = 0;
    uint32_t duration_ms = 0;
    uint32_t elapsed_ms = 0;
    bool non_color_cue = true;
    bool control_blocking = false;
    bool completed = false;
};

struct ExplorationFeedbackResult {
    bool success = false;
    std::string code;
    std::string message;
    std::optional<uint64_t> sequence;
};

struct ExplorationFeedbackSnapshot {
    ExplorationFeedbackSettings settings;
    std::optional<ExplorationFeedbackCue> active;
    std::vector<ExplorationFeedbackCue> queued;
    std::vector<ExplorationFeedbackCue> timeline;
};

class ExplorationFeedbackDirector {
public:
    explicit ExplorationFeedbackDirector(RuntimePresentationBible bible = makeRuntimePresentationBible());

    void setSettings(ExplorationFeedbackSettings settings);
    ExplorationFeedbackResult submit(ExplorationFeedbackRequest request);
    void advance(uint32_t delta_ms);
    void clear();

    const std::optional<ExplorationFeedbackCue>& activeCue() const { return active_; }
    ExplorationFeedbackSnapshot snapshot() const;

private:
    ExplorationFeedbackCue buildCue(const ExplorationFeedbackRequest& request);
    void startNext();
    uint32_t dialogueDuration(const std::string& text) const;

    RuntimePresentationBible bible_;
    ExplorationFeedbackSettings settings_;
    std::optional<ExplorationFeedbackCue> active_;
    std::vector<ExplorationFeedbackCue> queue_;
    std::vector<ExplorationFeedbackCue> timeline_;
    uint64_t next_sequence_ = 1;
};

const char* explorationFeedbackKindName(ExplorationFeedbackKind kind);

} // namespace urpg::presentation
