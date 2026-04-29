#pragma once

#include "engine/core/battle/battle_core.h"

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::editor {

enum class BattlePreviewIssueSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct BattlePreviewIssue {
    BattlePreviewIssueSeverity severity = BattlePreviewIssueSeverity::Info;
    std::string code;
    std::string message;
};

struct BattlePreviewSnapshot {
    std::string phase = "none";
    bool can_escape = false;
    int32_t physical_damage = 0;
    int32_t guarded_damage = 0;
    int32_t critical_damage = 0;
    int32_t magical_damage = 0;
    int32_t chip_damage = 0;
    int32_t chip_healing = 0;
    int32_t buff_level_preview = 0;
    size_t reused_troop_position_count = 0;
    std::string zero_damage_label = "miss";
    int32_t escape_ratio_now = 0;
    int32_t escape_ratio_next_fail = 0;
    size_t issue_count = 0;
};

class BattlePreviewPanel {
public:
    BattlePreviewPanel();

    void bindRuntime(const urpg::battle::BattleFlowController& flow_controller);
    void clearRuntime();

    void setPhysicalPreviewContext(const urpg::battle::BattleDamageContext& context);
    void setMagicalPreviewContext(const urpg::battle::BattleDamageContext& context);
    void setFeedbackPolicy(const urpg::battle::BattleFeedbackPolicy& policy);
    void setTroopPositionPreview(std::vector<urpg::battle::TroopMemberPosition> authored_positions,
                                 std::vector<urpg::battle::TroopMemberPosition> reusable_positions);
    void setEscapePreviewAgility(int32_t party_agi, int32_t troop_agi);

    const BattlePreviewSnapshot& snapshot() const;
    const std::vector<BattlePreviewIssue>& issues() const;

    void setVisible(bool visible);
    bool isVisible() const;

    void render();
    void refresh();
    void update();

private:
    void resetSnapshot();

    const urpg::battle::BattleFlowController* flow_controller_ = nullptr;
    urpg::battle::BattleDamageContext physical_preview_context_;
    urpg::battle::BattleDamageContext magical_preview_context_;
    urpg::battle::BattleFeedbackPolicy feedback_policy_;
    std::vector<urpg::battle::TroopMemberPosition> authored_troop_positions_;
    std::vector<urpg::battle::TroopMemberPosition> reusable_troop_positions_;
    int32_t preview_party_agi_ = 100;
    int32_t preview_troop_agi_ = 100;
    BattlePreviewSnapshot snapshot_;
    std::vector<BattlePreviewIssue> issues_;
    bool visible_ = true;
};

} // namespace urpg::editor
