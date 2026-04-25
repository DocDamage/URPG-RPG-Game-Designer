#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace urpg::battle {

struct PartyTacticsMemberState {
    std::string actor_id;
    int32_t hp = 1;
    int32_t max_hp = 1;
    bool has_heal_skill = false;
};

struct PartyTacticsProfile {
    std::string id;
    int32_t heal_below_percent = 35;
    std::string heal_action = "heal";
    std::string attack_action = "attack";
    std::string defend_action = "defend";
};

struct PartyTacticsDecision {
    std::string actor_id;
    std::string action_id;
    std::string reason;
};

PartyTacticsDecision ChoosePartyTacticsAction(const PartyTacticsProfile& profile,
                                              const PartyTacticsMemberState& member);

} // namespace urpg::battle
