#include "engine/core/presentation/exploration_feedback.h"

#include <algorithm>
#include <cmath>
#include <set>
#include <utility>

namespace urpg::presentation {
namespace {

uint32_t priorityFor(const ExplorationFeedbackKind kind) {
    switch (kind) {
    case ExplorationFeedbackKind::Transfer:
    case ExplorationFeedbackKind::ScreenTransition: return 300;
    case ExplorationFeedbackKind::Reward:
    case ExplorationFeedbackKind::QuestUpdate:
    case ExplorationFeedbackKind::DialogueLine: return 200;
    case ExplorationFeedbackKind::InteractionAcknowledged:
    case ExplorationFeedbackKind::Pickup:
    case ExplorationFeedbackKind::Door: return 150;
    case ExplorationFeedbackKind::InteractionPrompt: return 100;
    }
    return 100;
}

RuntimeFeedbackPriority runtimePriorityFor(const ExplorationFeedbackKind kind) {
    switch (kind) {
    case ExplorationFeedbackKind::Transfer:
    case ExplorationFeedbackKind::ScreenTransition: return RuntimeFeedbackPriority::Primary;
    case ExplorationFeedbackKind::Reward:
    case ExplorationFeedbackKind::QuestUpdate:
    case ExplorationFeedbackKind::DialogueLine: return RuntimeFeedbackPriority::Primary;
    case ExplorationFeedbackKind::InteractionAcknowledged:
    case ExplorationFeedbackKind::Pickup:
    case ExplorationFeedbackKind::Door: return RuntimeFeedbackPriority::Secondary;
    case ExplorationFeedbackKind::InteractionPrompt: return RuntimeFeedbackPriority::Ambient;
    }
    return RuntimeFeedbackPriority::Ambient;
}

} // namespace

const char* explorationFeedbackKindName(const ExplorationFeedbackKind kind) {
    switch (kind) {
    case ExplorationFeedbackKind::InteractionPrompt: return "interaction_prompt";
    case ExplorationFeedbackKind::InteractionAcknowledged: return "interaction_acknowledged";
    case ExplorationFeedbackKind::Pickup: return "pickup";
    case ExplorationFeedbackKind::Reward: return "reward";
    case ExplorationFeedbackKind::Door: return "door";
    case ExplorationFeedbackKind::Transfer: return "transfer";
    case ExplorationFeedbackKind::QuestUpdate: return "quest_update";
    case ExplorationFeedbackKind::DialogueLine: return "dialogue_line";
    case ExplorationFeedbackKind::ScreenTransition: return "screen_transition";
    }
    return "unknown";
}

ExplorationFeedbackDirector::ExplorationFeedbackDirector(RuntimePresentationBible bible) : bible_(std::move(bible)) {}

void ExplorationFeedbackDirector::setSettings(ExplorationFeedbackSettings settings) {
    settings.dialogue_speed = std::clamp(settings.dialogue_speed, 0.5F, 3.0F);
    settings_ = settings;
}

ExplorationFeedbackResult ExplorationFeedbackDirector::submit(ExplorationFeedbackRequest request) {
    if (request.id.empty() || request.source_id.empty() || request.text.empty() || request.id.size() > 128 ||
        request.text.size() > 4096) {
        return {false, "exploration_feedback_invalid", "Feedback requires bounded ID, source, and visible text.", std::nullopt};
    }
    const auto duplicate = [&](const ExplorationFeedbackCue& cue) { return cue.request_id == request.id; };
    if ((active_ && duplicate(*active_)) || std::any_of(queue_.begin(), queue_.end(), duplicate)) {
        return {false, "exploration_feedback_duplicate", "Feedback request ID is already active or queued.", std::nullopt};
    }
    if (queue_.size() >= 31) {
        return {false, "exploration_feedback_queue_full", "Exploration feedback queue is full.", std::nullopt};
    }
    auto cue = buildCue(request);
    const auto sequence = cue.sequence;
    if (active_ && priorityFor(cue.kind) > priorityFor(active_->kind)) {
        queue_.push_back(std::move(*active_));
        active_ = std::move(cue);
    } else {
        queue_.push_back(std::move(cue));
    }
    std::stable_sort(queue_.begin(), queue_.end(), [](const auto& left, const auto& right) {
        const auto left_priority = priorityFor(left.kind);
        const auto right_priority = priorityFor(right.kind);
        return left_priority == right_priority ? left.sequence < right.sequence : left_priority > right_priority;
    });
    startNext();
    return {true, "exploration_feedback_queued", "Exploration feedback accepted.", sequence};
}

void ExplorationFeedbackDirector::advance(uint32_t delta_ms) {
    while (delta_ms > 0 && active_) {
        const auto remaining = active_->duration_ms - active_->elapsed_ms;
        const auto consumed = std::min(delta_ms, remaining);
        active_->elapsed_ms += consumed;
        delta_ms -= consumed;
        if (active_->elapsed_ms >= active_->duration_ms) {
            active_->completed = true;
            timeline_.push_back(*active_);
            if (timeline_.size() > 256) timeline_.erase(timeline_.begin());
            active_.reset();
            startNext();
        }
    }
}

void ExplorationFeedbackDirector::clear() {
    active_.reset();
    queue_.clear();
    timeline_.clear();
    next_sequence_ = 1;
}

ExplorationFeedbackSnapshot ExplorationFeedbackDirector::snapshot() const {
    return {settings_, active_, queue_, timeline_};
}

ExplorationFeedbackCue ExplorationFeedbackDirector::buildCue(const ExplorationFeedbackRequest& request) {
    ExplorationFeedbackCue cue;
    cue.sequence = next_sequence_++;
    cue.kind = request.kind;
    cue.request_id = request.id;
    cue.source_id = request.source_id;
    cue.text = request.text;
    cue.non_color_cue = true;
    switch (request.kind) {
    case ExplorationFeedbackKind::InteractionPrompt:
        cue.icon = "urpg.icon.interact"; cue.feedback_tier = "ambient"; cue.duration_ms = 900; break;
    case ExplorationFeedbackKind::InteractionAcknowledged:
        cue.icon = "urpg.icon.confirm"; cue.feedback_tier = "secondary"; cue.duration_ms = 180;
        cue.sound = bible_.sound_motifs.at("confirm"); cue.haptic_cue = "urpg.haptic.confirm_tick"; break;
    case ExplorationFeedbackKind::Pickup:
        cue.icon = "urpg.icon.pickup"; cue.feedback_tier = "secondary"; cue.duration_ms = 650;
        cue.sound = bible_.sound_motifs.at("confirm"); cue.particle_motif = "paper_spark"; break;
    case ExplorationFeedbackKind::Reward:
        cue.icon = "urpg.icon.reward"; cue.feedback_tier = "primary"; cue.duration_ms = 900;
        cue.sound = bible_.sound_motifs.at("reward"); cue.haptic_cue = "urpg.haptic.reward_pulse";
        cue.particle_motif = "star_notch"; cue.camera_cue = "bounded_emphasis"; cue.screen_shake_pixels = 2.0F; break;
    case ExplorationFeedbackKind::Door:
        cue.icon = "urpg.icon.door"; cue.feedback_tier = "secondary"; cue.duration_ms = 500;
        cue.sound = request.contextual_sound.empty() ? "urpg.sound.door" : request.contextual_sound; break;
    case ExplorationFeedbackKind::Transfer:
        cue.icon = "urpg.icon.transfer"; cue.feedback_tier = "primary"; cue.duration_ms = bible_.camera.transition_ms;
        cue.sound = request.contextual_sound; cue.camera_cue = "player_led_transfer"; break;
    case ExplorationFeedbackKind::QuestUpdate:
        cue.icon = "urpg.icon.quest"; cue.feedback_tier = "primary"; cue.duration_ms = 1100;
        cue.sound = bible_.sound_motifs.at("quest"); cue.particle_motif = "ink_arc"; break;
    case ExplorationFeedbackKind::DialogueLine:
        cue.icon = "urpg.icon.dialogue"; cue.feedback_tier = "secondary"; cue.duration_ms = dialogueDuration(request.text);
        cue.sound = request.contextual_sound; break;
    case ExplorationFeedbackKind::ScreenTransition:
        cue.icon = "urpg.icon.transition"; cue.feedback_tier = "primary"; cue.duration_ms = bible_.camera.transition_ms;
        cue.camera_cue = "bounded_transition"; break;
    }
    RuntimeFeedbackTreatment treatment{cue.sound, cue.haptic_cue, cue.particle_motif, cue.camera_cue, {},
                                       cue.screen_shake_pixels, cue.hit_stop_ms, 0};
    const RuntimeFeedbackSettings shared_settings{settings_.audio_enabled, settings_.haptics_enabled,
                                                   settings_.particles_enabled, settings_.screen_shake_enabled,
                                                   settings_.hit_stop_enabled, settings_.reduced_motion, 1.0F};
    treatment = applyRuntimeFeedbackConstraints(std::move(treatment), shared_settings, bible_,
                                                runtimePriorityFor(request.kind));
    cue.sound = std::move(treatment.sound);
    cue.haptic_cue = std::move(treatment.haptic);
    cue.particle_motif = std::move(treatment.particle);
    cue.camera_cue = std::move(treatment.camera);
    cue.screen_shake_pixels = treatment.screen_shake_pixels;
    cue.hit_stop_ms = treatment.hit_stop_ms;
    return cue;
}

void ExplorationFeedbackDirector::startNext() {
    if (active_ || queue_.empty()) return;
    active_ = std::move(queue_.front());
    queue_.erase(queue_.begin());
}

uint32_t ExplorationFeedbackDirector::dialogueDuration(const std::string& text) const {
    const auto characters_per_second = 32.0F * settings_.dialogue_speed;
    const auto milliseconds = static_cast<uint32_t>(std::ceil(static_cast<float>(text.size()) / characters_per_second * 1000.0F));
    return std::clamp(milliseconds, uint32_t{400}, uint32_t{4000});
}

} // namespace urpg::presentation
