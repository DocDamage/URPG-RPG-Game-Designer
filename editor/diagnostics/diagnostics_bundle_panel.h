#pragma once

#include "engine/core/diagnostics/diagnostics_bundle_exporter.h"

#include <nlohmann/json.hpp>

#include <filesystem>

namespace urpg::editor {

class DiagnosticsBundlePanel {
public:
    void setInput(urpg::diagnostics::DiagnosticsBundleInput input);
    urpg::diagnostics::DiagnosticsBundleResult exportBundle(const std::filesystem::path& outputDirectory);
    void render();

    nlohmann::json lastRenderSnapshot() const;

private:
    urpg::diagnostics::DiagnosticsBundleInput input_;
    urpg::diagnostics::DiagnosticsBundleResult last_result_;
    nlohmann::json last_render_snapshot_ = nlohmann::json::object();
};

} // namespace urpg::editor
