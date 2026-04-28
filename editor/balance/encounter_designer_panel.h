#pragma once

#include "engine/core/balance/encounter_table.h"

#include <nlohmann/json.hpp>
#include <set>
#include <string>

namespace urpg::editor::balance {

struct EncounterDesignerPanelSnapshot {
    std::string selected_region_id;
    int32_t party_level = 1;
    uint64_t seed = 0;
    std::size_t preview_count = 0;
    std::size_t diagnostic_count = 0;
};

class EncounterDesignerPanel {
public:
    void loadDocument(urpg::balance::EncounterDesignerDocument document);
    void setPreviewContext(std::string region_id, int32_t party_level, std::set<std::string> active_flags, uint64_t seed, std::size_t count);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const EncounterDesignerPanelSnapshot& snapshot() const { return snapshot_; }
    [[nodiscard]] const urpg::balance::EncounterDesignerPreview& preview() const { return preview_; }

private:
    void refresh();

    urpg::balance::EncounterDesignerDocument document_;
    std::string selected_region_id_;
    int32_t party_level_ = 1;
    std::set<std::string> active_flags_;
    uint64_t seed_ = 0;
    std::size_t count_ = 4;
    urpg::balance::EncounterDesignerPreview preview_{};
    EncounterDesignerPanelSnapshot snapshot_{};
};

} // namespace urpg::editor::balance
