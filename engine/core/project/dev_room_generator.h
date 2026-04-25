#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::project {

struct DevRoomResult {
    nlohmann::json room;
    nlohmann::json audit_report;
};

class DevRoomGenerator {
public:
    DevRoomResult generate(const std::string& project_id) const;
    std::vector<std::string> validateScriptedRoute(const nlohmann::json& room) const;
};

} // namespace urpg::project
