#include "engine/core/presentation/battle_feedback.h"

#include <algorithm>
#include <utility>

namespace urpg::presentation {
namespace {

uint32_t priorityFor(const BattleFeedbackKind kind) {
    switch (kind) {
    case BattleFeedbackKind::Victory:
    case BattleFeedbackKind::Defeat: return 400;
    case BattleFeedbackKind::Results: return 350;
    case BattleFeedbackKind::Impact:
    case BattleFeedbackKind::Damage:
    case BattleFeedbackKind::Heal:
    case BattleFeedbackKind::Status: return 250;
    case BattleFeedbackKind::Anticipation: return 200;
    case BattleFeedbackKind::TurnState: return 150;
    case BattleFeedbackKind::Selection:
    case BattleFeedbackKind::Targeting: return 100;
    }
    return 100;
}

RuntimeFeedbackPriority runtimePriorityFor(const BattleFeedbackKind kind) {
    switch (kind) {
    case BattleFeedbackKind::Victory:
    case BattleFeedbackKind::Defeat: return RuntimeFeedbackPriority::Critical;
    case BattleFeedbackKind::Results: return RuntimeFeedbackPriority::Critical;
    case BattleFeedbackKind::Impact:
    case BattleFeedbackKind::Damage:
    case BattleFeedbackKind::Heal:
    case BattleFeedbackKind::Status: return RuntimeFeedbackPriority::Primary;
    case BattleFeedbackKind::Anticipation:
    case BattleFeedbackKind::TurnState: return RuntimeFeedbackPriority::Secondary;
    case BattleFeedbackKind::Selection:
    case BattleFeedbackKind::Targeting: return RuntimeFeedbackPriority::Ambient;
    }
    return RuntimeFeedbackPriority::Ambient;
}

} // namespace

const char* battleFeedbackKindName(const BattleFeedbackKind kind) {
    switch (kind) {
    case BattleFeedbackKind::Selection: return "selection";
    case BattleFeedbackKind::Targeting: return "targeting";
    case BattleFeedbackKind::TurnState: return "turn_state";
    case BattleFeedbackKind::Anticipation: return "anticipation";
    case BattleFeedbackKind::Impact: return "impact";
    case BattleFeedbackKind::Damage: return "damage";
    case BattleFeedbackKind::Heal: return "heal";
    case BattleFeedbackKind::Status: return "status";
    case BattleFeedbackKind::Victory: return "victory";
    case BattleFeedbackKind::Defeat: return "defeat";
    case BattleFeedbackKind::Results: return "results";
    }
    return "unknown";
}

BattleFeedbackDirector::BattleFeedbackDirector(RuntimePresentationBible bible) : bible_(std::move(bible)) {}

void BattleFeedbackDirector::setSettings(const BattleFeedbackSettings settings) {
    settings_ = settings;
}

BattleFeedbackResult BattleFeedbackDirector::submit(BattleFeedbackRequest request) {
    if (request.id.empty() || request.source_id.empty() || request.text.empty() || request.id.size() > 128 ||
        request.source_id.size() > 128 || request.target_id.size() > 128 || request.text.size() > 512) {
        return {false, "battle_feedback_invalid", "Feedback requires bounded ID, source, and visible text.", std::nullopt};
    }
    const auto duplicate = [&](const BattleFeedbackCue& cue) { return cue.request_id == request.id; };
    if ((active_ && duplicate(*active_)) || std::any_of(queue_.begin(), queue_.end(), duplicate)) {
        return {false, "battle_feedback_duplicate", "Feedback request ID is already active or queued.", std::nullopt};
    }
    if (queue_.size() >= 47) {
        return {false, "battle_feedback_queue_full", "Battle feedback queue is full.", std::nullopt};
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
    return {true, "battle_feedback_queued", "Battle feedback accepted.", sequence};
}

void BattleFeedbackDirector::advance(uint32_t delta_ms) {
    while (delta_ms > 0 && active_) {
        const auto remaining = active_->duration_ms - active_->elapsed_ms;
        const auto consumed = std::min(delta_ms, remaining);
        active_->elapsed_ms += consumed;
        delta_ms -= consumed;
        if (active_->elapsed_ms >= active_->duration_ms) {
            active_->completed = true;
            timeline_.push_back(*active_);
            if (timeline_.size() > 384) timeline_.erase(timeline_.begin());
            active_.reset();
            startNext();
        }
    }
}

void BattleFeedbackDirector::clear() {
    active_.reset();
    queue_.clear();
    timeline_.clear();
    next_sequence_ = 1;
}

BattleFeedbackSnapshot BattleFeedbackDirector::snapshot() const {
    return {settings_, active_, queue_, timeline_};
}

BattleFeedbackCue BattleFeedbackDirector::buildCue(const BattleFeedbackRequest& request) {
    BattleFeedbackCue cue;
    cue.sequence = next_sequence_++;
    cue.kind = request.kind;
    cue.request_id = request.id;
    cue.source_id = request.source_id;
    cue.target_id = request.target_id;
    cue.text = request.text;
    cue.amount = request.amount;
    cue.critical = request.critical;
    switch (request.kind) {
    case BattleFeedbackKind::Selection:
        cue.icon = "urpg.icon.battle.command"; cue.shape_cue = "focus_brackets"; cue.feedback_tier = "ambient";
        cue.duration_ms = 180; cue.sound = bible_.sound_motifs.at("focus"); break;
    case BattleFeedbackKind::Targeting:
        cue.icon = "urpg.icon.battle.target"; cue.shape_cue = "target_reticle"; cue.feedback_tier = "ambient";
        cue.duration_ms = 220; cue.sound = bible_.sound_motifs.at("focus"); break;
    case BattleFeedbackKind::TurnState:
        cue.icon = "urpg.icon.battle.turn"; cue.shape_cue = "turn_banner"; cue.feedback_tier = "secondary";
        cue.duration_ms = 500; break;
    case BattleFeedbackKind::Anticipation:
        cue.icon = "urpg.icon.battle.action"; cue.shape_cue = "windup_arc"; cue.feedback_tier = "secondary";
        cue.duration_ms = 300; cue.sound = "urpg.sound.battle.windup"; cue.particle_motif = "ink_arc"; break;
    case BattleFeedbackKind::Impact:
        cue.icon = "urpg.icon.battle.impact"; cue.shape_cue = "impact_burst"; cue.feedback_tier = "primary";
        cue.duration_ms = 180; cue.hit_stop_ms = std::min(bible_.camera.maximum_hit_stop_ms, uint32_t{60});
        cue.sound = "urpg.sound.battle.impact"; cue.haptic_cue = "urpg.haptic.impact_pulse";
        cue.particle_motif = "paper_spark"; cue.camera_cue = "bounded_impact";
        cue.screen_shake_pixels = bible_.camera.maximum_shake_pixels; break;
    case BattleFeedbackKind::Damage:
        cue.icon = "urpg.icon.battle.damage"; cue.shape_cue = request.critical ? "critical_star" : "damage_notch";
        cue.feedback_tier = "primary"; cue.duration_ms = 650; cue.sound = request.critical ? "urpg.sound.battle.critical" : "";
        break;
    case BattleFeedbackKind::Heal:
        cue.icon = "urpg.icon.battle.heal"; cue.shape_cue = "heal_plus"; cue.feedback_tier = "primary";
        cue.duration_ms = 650; cue.sound = "urpg.sound.battle.heal"; cue.particle_motif = "star_notch"; break;
    case BattleFeedbackKind::Status:
        cue.icon = "urpg.icon.battle.status"; cue.shape_cue = "status_badge"; cue.feedback_tier = "primary";
        cue.duration_ms = 800; cue.sound = "urpg.sound.battle.status"; break;
    case BattleFeedbackKind::Victory:
        cue.icon = "urpg.icon.battle.victory"; cue.shape_cue = "victory_ribbon"; cue.feedback_tier = "critical";
        cue.duration_ms = 1400; cue.sound = bible_.sound_motifs.at("reward"); cue.particle_motif = "star_notch";
        cue.camera_cue = "bounded_victory"; cue.skippable = false; break;
    case BattleFeedbackKind::Defeat:
        cue.icon = "urpg.icon.battle.defeat"; cue.shape_cue = "defeat_frame"; cue.feedback_tier = "critical";
        cue.duration_ms = 1400; cue.sound = "urpg.sound.battle.defeat"; cue.camera_cue = "bounded_defeat";
        cue.skippable = false; break;
    case BattleFeedbackKind::Results:
        cue.icon = "urpg.icon.battle.results"; cue.shape_cue = "results_ledger"; cue.feedback_tier = "critical";
        cue.duration_ms = 1500; cue.sound = bible_.sound_motifs.at("reward"); cue.skippable = false; break;
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
    if (settings_.playback_mode == BattlePlaybackMode::FastForward && cue.skippable) {
        cue.duration_ms = std::max(uint32_t{90}, cue.duration_ms / 2);
        cue.hit_stop_ms = std::min(cue.hit_stop_ms, uint32_t{24});
    } else if (settings_.playback_mode == BattlePlaybackMode::Skip && cue.skippable) {
        cue.duration_ms = 1;
        cue.hit_stop_ms = 0;
        cue.particle_motif.clear();
        cue.camera_cue.clear();
    }
    return cue;
}

void BattleFeedbackDirector::startNext() {
    if (active_ || queue_.empty()) return;
    active_ = std::move(queue_.front());
    queue_.erase(queue_.begin());
}

} // namespace urpg::presentation
