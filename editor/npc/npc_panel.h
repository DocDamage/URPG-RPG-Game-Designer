#pragma once

#include "engine/core/npc/npc_schedule.h"

#include <nlohmann/json.hpp>
#include <set>
#include <string>

namespace urpg::editor::npc {

struct NpcSchedulePanelSnapshot {
    std::string npc_id;
    int hour = 0;
    std::string map_id;
    bool used_fallback = false;
    std::size_t diagnostic_count = 0;
};

class NpcPanel {
public:
    [[nodiscard]] static std::string snapshot(const urpg::npc::NpcResolvedState& state);
};

class NpcSchedulePanel {
public:
    void loadDocument(urpg::npc::NpcScheduleDocument document);
    void setPreviewContext(std::string npc_id, int hour, std::set<std::string> available_maps);
    void render();

    [[nodiscard]] nlohmann::json saveProjectData() const;
    [[nodiscard]] const urpg::npc::NpcSchedulePreview& preview() const { return preview_; }
    [[nodiscard]] const NpcSchedulePanelSnapshot& snapshot() const { return snapshot_; }

private:
    void refresh();

    urpg::npc::NpcScheduleDocument document_;
    std::string npc_id_;
    int hour_ = 0;
    std::set<std::string> available_maps_;
    urpg::npc::NpcSchedulePreview preview_{};
    NpcSchedulePanelSnapshot snapshot_{};
};

} // namespace urpg::editor::npc
