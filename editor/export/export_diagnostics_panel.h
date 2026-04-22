#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "engine/core/tools/export_packager.h"

namespace urpg::editor {

class ExportDiagnosticsPanel {
public:
    void setExportConfig(const urpg::tools::ExportConfig& config);
    void render();

    const nlohmann::json& lastRenderSnapshot() const { return last_render_snapshot_; }

private:
    std::optional<urpg::tools::ExportConfig> config_;
    nlohmann::json last_render_snapshot_;
};

} // namespace urpg::editor
