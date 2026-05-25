#include "editor/diagnostics/diagnostics_facade.h"

namespace urpg::editor {

DiagnosticsFacade::DiagnosticsFacade(DiagnosticsWorkspace& workspace) : workspace_(workspace) {}

std::string DiagnosticsFacade::emitSnapshot() const {
    // Simply proxy to the existing exportAsJson logic on the workspace.
    // In the future, this can be expanded to include metadata like timestamps,
    // workspace session IDs, or context from other systems.
    return workspace_.exportAsJson();
}

nlohmann::json DiagnosticsFacade::emitTelemetryEnvelope() const {
    urpg::telemetry::TelemetryEvent event;
    event.subsystem = "editor.diagnostics";
    event.name = "diagnostics_snapshot";
    event.severity = urpg::telemetry::TelemetrySeverity::Info;
    event.code = "diagnostics.snapshot";
    event.message = "Diagnostics snapshot emitted.";
    event.fields["snapshot"] = emitSnapshot();
    return event.toJson();
}

void DiagnosticsFacade::refreshAndEmit(std::function<void(std::string_view)> callback) const {
    // Ensure data is fresh before capture.
    workspace_.refresh();
    workspace_.update();

    // In many UI contexts (like Electron or Flutter shells), this would
    // be dispatched over a transport mechanism (Protobuf, ZeroMQ, IPC).
    // For now, we provide the string result to the caller via callback.
    callback(emitSnapshot());
}

} // namespace urpg::editor
