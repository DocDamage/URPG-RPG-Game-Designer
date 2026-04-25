#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::save {

struct SaveDebugSlot {
    int slot_id = -1;
    bool corrupted = false;
    std::string recovery_tier = "primary";
    nlohmann::json metadata = nlohmann::json::object();
    nlohmann::json subsystem_state = nlohmann::json::object();
    std::vector<std::string> migration_notes;
    std::vector<std::string> diagnostics;
};

class SaveDebugger {
public:
    SaveDebugSlot inspectSlot(const nlohmann::json& save_document) const;
    nlohmann::json exportDiagnostics(const SaveDebugSlot& slot) const;
};

} // namespace urpg::save
