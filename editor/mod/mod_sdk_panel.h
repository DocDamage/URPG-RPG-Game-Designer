#pragma once

#include "engine/core/mod/mod_sdk_sample_validator.h"

#include <nlohmann/json.hpp>

#include <filesystem>

namespace urpg::editor {

class ModSdkPanel {
public:
    urpg::mod::ModSdkSampleValidationResult validateSample(const std::filesystem::path& sampleRoot);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::mod::ModSdkSampleValidationResult last_result_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
