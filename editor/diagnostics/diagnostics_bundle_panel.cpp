#include "editor/diagnostics/diagnostics_bundle_panel.h"

#include <utility>

namespace urpg::editor {

void DiagnosticsBundlePanel::setInput(urpg::diagnostics::DiagnosticsBundleInput input) {
    input_ = std::move(input);
}

urpg::diagnostics::DiagnosticsBundleResult DiagnosticsBundlePanel::exportBundle(
    const std::filesystem::path& outputDirectory) {
    urpg::diagnostics::DiagnosticsBundleExporter exporter;
    last_result_ = exporter.exportBundle(input_, outputDirectory);
    return last_result_;
}

void DiagnosticsBundlePanel::render() {
    last_render_snapshot_ = {
        {"ready", !input_.projectRoot.empty()},
        {"last_export_success", last_result_.success},
        {"excluded_count", last_result_.excludedPaths.size()},
        {"warning_count", last_result_.warnings.size()},
    };
}

nlohmann::json DiagnosticsBundlePanel::lastRenderSnapshot() const {
    return last_render_snapshot_;
}

} // namespace urpg::editor
