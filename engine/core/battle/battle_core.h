#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::battle {

enum class BattleFlowPhase : uint8_t {
    None = 0,
    Start = 1,
    Input = 2,
    Action = 3,
    TurnEnd = 4,
    Victory = 5,
    Defeat = 6,
    Abort = 7,
};

class BattleFlowController {
public:
    void beginBattle(bool can_escape);
    void enterInput();
    void enterAction();
    void endTurn();
    void markVictory();
    void markDefeat();
    void abort();

    [[nodiscard]] BattleFlowPhase phase() const { return phase_; }
    [[nodiscard]] bool isActive() const;
    [[nodiscard]] bool canEscape() const;
    [[nodiscard]] int32_t turnCount() const { return turn_count_; }
    [[nodiscard]] int32_t escapeFailures() const { return escape_failures_; }
    void noteEscapeFailure();

private:
    BattleFlowPhase phase_ = BattleFlowPhase::None;
    bool allow_escape_ = false;
    int32_t turn_count_ = 0;
    int32_t escape_failures_ = 0;
};

struct BattleQueuedAction {
    std::string subject_id;
    std::string target_id;
    std::string command;
    int32_t speed = 0;
    int32_t priority = 0;
};

class BattleActionQueue {
public:
    void enqueue(BattleQueuedAction action);
    std::optional<BattleQueuedAction> popNext();
    void clear();

    [[nodiscard]] bool empty() const { return queue_.empty(); }
    [[nodiscard]] size_t size() const { return queue_.size(); }
    [[nodiscard]] std::vector<BattleQueuedAction> snapshotOrdered() const;

private:
    std::vector<BattleQueuedAction> queue_;
};

struct BattleRuleStatBlock {
    int32_t hp = 1;
    int32_t mhp = 1;
    int32_t atk = 1;
    int32_t def = 1;
    int32_t mat = 1;
    int32_t mdf = 1;
    int32_t agi = 1;
    int32_t luk = 1;
    bool guarding = false;
};

struct BattleDamageContext {
    BattleRuleStatBlock subject;
    BattleRuleStatBlock target;
    int32_t power = 0;
    bool magical = false;
    bool critical = false;
    int32_t variance_percent = 0;
};

class BattleRuleResolver {
public:
    static int32_t resolveDamage(const BattleDamageContext& context);
    static int32_t resolveEscapeRatio(int32_t party_agi, int32_t troop_agi, int32_t fail_count);
};

} // namespace urpg::battle
