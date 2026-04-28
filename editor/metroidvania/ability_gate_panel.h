#pragma once

#include "engine/core/metroidvania/ability_gate_system.h"

#include <nlohmann/json.hpp>
#include <set>

namespace urpg::editor::metroidvania {

struct AbilityGatePanelSnapshot {
    std::string start_region;
    std::size_t reachable_count = 0;
    std::size_t unlocked_ability_count = 0;
    std::size_t blocked_link_count = 0;
    std::size_t diagnostic_count = 0;
};

class AbilityGatePanel {
public:
    void loadDocument(urpg::metroidvania::AbilityGateDocument document);
    void setPreviewContext(std::string start_region, std::set<std::string> initial_abilities);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::metroidvania::AbilityGatePreview& preview() const { return preview_; }
    [[nodiscard]] const AbilityGatePanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::metroidvania::AbilityGateDocument document_;
    std::string start_region_;
    std::set<std::string> initial_abilities_;
    urpg::metroidvania::AbilityGatePreview preview_{};
    AbilityGatePanelSnapshot snapshot_{};
};

} // namespace urpg::editor::metroidvania
