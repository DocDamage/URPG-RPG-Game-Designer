#pragma once

#include "engine/core/save/save_compatibility_preview.h"

#include <nlohmann/json.hpp>

namespace urpg::editor {

class SaveMigrationPreviewPanel {
public:
    void setPreview(urpg::save::SaveCompatibilityPreview preview);
    void render();
    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::save::SaveCompatibilityPreview preview_;
    nlohmann::json snapshot_;
};

} // namespace urpg::editor
