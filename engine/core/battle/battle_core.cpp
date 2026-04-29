#include "engine/core/battle/battle_core.h"

#include <algorithm>
#include <utility>

namespace urpg::battle {

namespace {

bool IsBattleTerminal(BattleFlowPhase phase) {
    return phase == BattleFlowPhase::Victory || phase == BattleFlowPhase::Defeat || phase == BattleFlowPhase::Abort ||
           phase == BattleFlowPhase::None;
}

bool ActionSortLess(const BattleQueuedAction& lhs, const BattleQueuedAction& rhs) {
    if (lhs.speed != rhs.speed) {
        return lhs.speed > rhs.speed;
    }
    if (lhs.priority != rhs.priority) {
        return lhs.priority < rhs.priority;
    }
    if (lhs.subject_id != rhs.subject_id) {
        return lhs.subject_id < rhs.subject_id;
    }
    if (lhs.target_id != rhs.target_id) {
        return lhs.target_id < rhs.target_id;
    }
    return lhs.command < rhs.command;
}

int32_t ClampPositive(int32_t value) {
    return std::max(1, value);
}

int32_t ClampRatioPercent(int32_t value) {
    return std::clamp(value, 0, 100);
}

} // namespace

void BattleFlowController::beginBattle(bool can_escape) {
    phase_ = BattleFlowPhase::Start;
    allow_escape_ = can_escape;
    turn_count_ = 1;
    escape_failures_ = 0;
}

void BattleFlowController::enterInput() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::Input;
}

void BattleFlowController::enterAction() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::Action;
}

void BattleFlowController::endTurn() {
    if (IsBattleTerminal(phase_)) {
        return;
    }
    phase_ = BattleFlowPhase::TurnEnd;
    ++turn_count_;
}

void BattleFlowController::markVictory() {
    phase_ = BattleFlowPhase::Victory;
}

void BattleFlowController::markDefeat() {
    phase_ = BattleFlowPhase::Defeat;
}

void BattleFlowController::abort() {
    phase_ = BattleFlowPhase::Abort;
}

bool BattleFlowController::isActive() const {
    return !IsBattleTerminal(phase_);
}

bool BattleFlowController::canEscape() const {
    return isActive() && allow_escape_;
}

void BattleFlowController::noteEscapeFailure() {
    if (canEscape()) {
        ++escape_failures_;
    }
}

void BattleActionQueue::enqueue(BattleQueuedAction action) {
    queue_.push_back(std::move(action));
}

std::optional<BattleQueuedAction> BattleActionQueue::popNext() {
    if (queue_.empty()) {
        return std::nullopt;
    }

    auto it = std::min_element(queue_.begin(), queue_.end(), [](const BattleQueuedAction& lhs, const BattleQueuedAction& rhs) {
        return ActionSortLess(lhs, rhs);
    });
    BattleQueuedAction action = *it;
    queue_.erase(it);
    return action;
}

void BattleActionQueue::clear() {
    queue_.clear();
}

std::vector<BattleQueuedAction> BattleActionQueue::snapshotOrdered() const {
    std::vector<BattleQueuedAction> snapshot = queue_;
    std::sort(snapshot.begin(), snapshot.end(), ActionSortLess);
    return snapshot;
}

int32_t BattleRuleResolver::resolveDamage(const BattleDamageContext& context) {
    const int32_t atk_like = context.magical ? context.subject.mat : context.subject.atk;
    const int32_t def_like = context.magical ? context.target.mdf : context.target.def;

    int32_t damage = std::max(0, context.power + (atk_like * 2) - def_like);

    if (context.critical) {
        damage = static_cast<int32_t>(damage * 1.5);
    }

    if (context.variance_percent > 0) {
        const int32_t variance = (damage * std::min(context.variance_percent, 100)) / 100;
        // Deterministic midpoint variance adjustment so replay stays stable.
        damage += variance / 2;
    }

    if (context.target.guarding && damage > 0) {
        damage /= 2;
    }

    return std::clamp(damage, 0, ClampPositive(context.target.hp));
}

BattleFeedbackPreview BattleRuleResolver::resolveFeedbackPreview(int32_t damage,
                                                                 int32_t healing,
                                                                 int32_t current_buff_level,
                                                                 int32_t buff_delta,
                                                                 const BattleFeedbackPolicy& policy) {
    BattleFeedbackPreview preview;
    const int32_t safe_damage_percent = std::clamp(policy.chip_damage_percent, 0, 100);
    const int32_t safe_healing_percent = std::clamp(policy.chip_healing_percent, 0, 100);
    if (damage > 0) {
        preview.chip_damage = std::max(policy.min_chip_damage, (damage * safe_damage_percent) / 100);
        preview.chip_damage = std::min(preview.chip_damage, damage);
    }
    if (healing > 0) {
        preview.chip_healing = std::max(policy.min_chip_healing, (healing * safe_healing_percent) / 100);
        preview.chip_healing = std::min(preview.chip_healing, healing);
    }
    preview.buff_level = std::clamp(current_buff_level + buff_delta, -std::max(0, policy.max_buff_level),
                                    std::max(0, policy.max_buff_level));
    preview.zero_damage_label = toString(policy.zero_damage_policy);
    return preview;
}

TroopPositionReuseResult BattleRuleResolver::resolveTroopPositions(
    const std::vector<TroopMemberPosition>& authored_positions,
    const std::vector<TroopMemberPosition>& reusable_positions,
    const BattleFeedbackPolicy& policy) {
    TroopPositionReuseResult result;
    result.positions = authored_positions;
    if (!policy.reuse_troop_positions) {
        return result;
    }

    for (auto& authored : result.positions) {
        const auto reusable = std::find_if(reusable_positions.begin(), reusable_positions.end(),
                                           [&](const TroopMemberPosition& candidate) {
                                               return candidate.enemy_id == authored.enemy_id;
                                           });
        if (reusable != reusable_positions.end()) {
            authored.x = reusable->x;
            authored.y = reusable->y;
            ++result.reused_count;
        }
    }
    return result;
}

std::string BattleRuleResolver::toString(ZeroDamagePresentationPolicy policy) {
    switch (policy) {
        case ZeroDamagePresentationPolicy::Evasion: return "evasion";
        case ZeroDamagePresentationPolicy::Immune: return "immune";
        case ZeroDamagePresentationPolicy::NoEffect: return "no_effect";
        case ZeroDamagePresentationPolicy::Miss: return "miss";
    }
    return "miss";
}

int32_t BattleRuleResolver::resolveEscapeRatio(int32_t party_agi, int32_t troop_agi, int32_t fail_count) {
    const int32_t safe_party_agi = ClampPositive(party_agi);
    const int32_t safe_troop_agi = ClampPositive(troop_agi);
    const int32_t base_ratio = (safe_party_agi * 100) / safe_troop_agi;
    const int32_t fail_bonus = std::max(0, fail_count) * 10;
    return ClampRatioPercent(base_ratio + fail_bonus);
}

} // namespace urpg::battle
