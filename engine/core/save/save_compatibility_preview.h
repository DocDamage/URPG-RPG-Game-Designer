#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::save {

struct SaveCompatibilityPreview {
    bool ok = true;
    nlohmann::json native_payload = nlohmann::json::object();
    nlohmann::json migrated_metadata = nlohmann::json::object();
    nlohmann::json retained_payload = nlohmann::json::object();
    std::vector<std::string> diagnostics;
};

class SaveCompatibilityPreviewer {
public:
    SaveCompatibilityPreview preview(const nlohmann::json& old_save_document) const;
};

} // namespace urpg::save
