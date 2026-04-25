#include "engine/core/battle/party_tactics_profile.h"

#include <algorithm>

namespace urpg::battle {

PartyTacticsDecision ChoosePartyTacticsAction(const PartyTacticsProfile& profile,
                                              const PartyTacticsMemberState& member) {
    const int32_t max_hp = std::max(1, member.max_hp);
    const int32_t hp_percent = std::clamp((member.hp * 100) / max_hp, 0, 100);
    if (hp_percent < profile.heal_below_percent) {
        if (member.has_heal_skill) {
            return {member.actor_id, profile.heal_action, "hp_below_threshold"};
        }
        return {member.actor_id, profile.defend_action, "low_hp_no_heal_available"};
    }
    return {member.actor_id, profile.attack_action, "default_attack"};
}

} // namespace urpg::battle
