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

class AbilityInspectorModel {
public:
    void refresh(const AbilitySystemComponent& asc);
    void clear();

    const std::vector<AbilityInfo>& getAbilities() const { return m_abilities; }
    const std::vector<ActiveTagInfo>& getActiveTags() const { return m_active_tags; }

private:
    std::vector<AbilityInfo> m_abilities;
    std::vector<ActiveTagInfo> m_active_tags;
};

} // namespace urpg::editor
