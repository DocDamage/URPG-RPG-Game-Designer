#pragma once

#include <set>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace urpg::npc {

struct GridPosition {
    int x{0};
    int y{0};
};

struct NpcRoutine {
    std::string npc_id;
    std::string map_id;
    int start_hour{0};
    int end_hour{23};
    GridPosition position;
    std::string animation;
    std::string dialogue_state;
};

struct NpcFallback {
    std::string map_id;
    GridPosition position;
    std::string animation;
    std::string dialogue_state;
};

struct NpcResolvedState {
    bool usedFallback{false};
    std::string mapId;
    GridPosition position;
    std::string animation;
    std::string dialogueState;
};

struct NpcScheduleDiagnostic {
    std::string code;
    std::string message;
    std::string npc_id;
};

struct NpcSchedulePreview {
    std::string npc_id;
    int hour = 0;
    NpcResolvedState state;
    std::vector<NpcScheduleDiagnostic> diagnostics;
};

class NpcSchedule {
public:
    void addRoutine(NpcRoutine routine);
    void setFallback(NpcFallback fallback);
    [[nodiscard]] NpcResolvedState resolve(const std::string& npc_id, int hour, const std::set<std::string>& available_maps) const;

private:
    std::vector<NpcRoutine> routines_;
    NpcFallback fallback_;
};

class NpcScheduleDocument {
public:
    std::string version = "1.0.0";
    std::vector<NpcRoutine> routines;
    NpcFallback fallback;

    [[nodiscard]] std::vector<NpcScheduleDiagnostic> validate(const std::set<std::string>& known_maps = {}) const;
    [[nodiscard]] NpcSchedule toRuntimeSchedule() const;
    [[nodiscard]] NpcSchedulePreview preview(const std::string& npc_id, int hour, const std::set<std::string>& available_maps) const;
    [[nodiscard]] nlohmann::json toJson() const;

    static NpcScheduleDocument fromJson(const nlohmann::json& json);
};

nlohmann::json npcSchedulePreviewToJson(const NpcSchedulePreview& preview);

} // namespace urpg::npc
