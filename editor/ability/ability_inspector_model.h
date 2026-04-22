#pragma once

#include "engine/core/ability/gameplay_tags.h"
#include "engine/core/ability/pattern_field.h"
#include <vector>
#include <string>
#include <memory>

namespace urpg::ability {
    class AbilitySystemComponent;
}

namespace urpg::editor {

using namespace urpg::ability;

struct AbilityInfo {
    std::string name;
    bool can_activate;
    float cooldown_remaining;
    std::string blocking_reason;
    std::shared_ptr<urpg::PatternField> pattern;
};

struct ActiveTagInfo {
    std::string tag;
    int count;
};

struct AbilityDiagnosticsAbilityState {
    std::string id;
    bool can_activate = false;
    float cooldown_remaining = 0.0f;
    std::string blocking_reason;
};

struct AbilityDiagnosticsEffectState {
    std::string id;
    float duration = 0.0f;
    float elapsed = 0.0f;
    int32_t stack_count = 1;
};

struct AbilityDiagnosticsSnapshot {
    size_t ability_count = 0;
    size_t active_effect_count = 0;
    size_t active_cooldown_count = 0;
    size_t last_execution_sequence_id = 0;
    std::vector<AbilityDiagnosticsAbilityState> ability_states;
    std::vector<AbilityDiagnosticsEffectState> active_effects;
};

class AbilityInspectorModel {
public:
    void refresh(const AbilitySystemComponent& asc);
    void clear();

    const std::vector<AbilityInfo>& getAbilities() const { return m_abilities; }
    const std::vector<ActiveTagInfo>& getActiveTags() const { return m_active_tags; }

    AbilityDiagnosticsSnapshot buildDiagnosticsSnapshot(const AbilitySystemComponent& asc) const;

private:
    std::vector<AbilityInfo> m_abilities;
    std::vector<ActiveTagInfo> m_active_tags;
};

} // namespace urpg::editor
