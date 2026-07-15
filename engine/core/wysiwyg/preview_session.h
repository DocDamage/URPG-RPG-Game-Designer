#pragma once

#include <string>
#include <vector>

namespace urpg::wysiwyg {

enum class PreviewMode {
    EditorOnly,
    RuntimeBacked,
};

enum class PreviewEvidenceKind {
    SavedData,
    LivePreview,
    RuntimeExecution,
    Diagnostics,
    ExportPackage,
    Tests,
};

struct PreviewTraceRow {
    std::string kind;
    std::string source_id;
    std::string detail;
};

struct PreviewDiagnostic {
    std::string code;
    std::string message;
    std::string source_id;
    bool blocker = true;
};

struct PreviewEvidence {
    PreviewEvidenceKind kind = PreviewEvidenceKind::SavedData;
    bool present = false;
    std::string detail;
};

struct WysiwygConfidenceReport {
    bool runtimeBacked = false;
    bool diagnosticsClean = true;
    bool savedDataPresent = false;
    bool exportAware = false;
    bool exactShipReady = false;
    bool dirtyData = false;
};

struct WysiwygPreviewSession {
    std::string route_id;
    std::string surface_id;
    std::string source_id;
    PreviewMode mode = PreviewMode::EditorOnly;
    std::vector<PreviewTraceRow> runtime_trace_rows;
    std::vector<PreviewTraceRow> summary_rows;
    std::vector<PreviewDiagnostic> diagnostics;
    std::vector<PreviewEvidence> evidence;
    WysiwygConfidenceReport confidence;

    void recomputeConfidence() {
        bool hasSavedEvidence = false;
        bool hasRuntimeEvidence = false;
        bool hasExportEvidence = false;
        bool hasBlockerDiagnostic = false;

        for (const auto& item : evidence) {
            if (item.kind == PreviewEvidenceKind::SavedData && item.present) {
                hasSavedEvidence = true;
            } else if (item.kind == PreviewEvidenceKind::RuntimeExecution && item.present) {
                hasRuntimeEvidence = true;
            } else if (item.kind == PreviewEvidenceKind::ExportPackage && item.present) {
                hasExportEvidence = true;
            }
        }
        for (const auto& diagnostic : diagnostics) {
            hasBlockerDiagnostic = hasBlockerDiagnostic || diagnostic.blocker;
        }

        confidence.savedDataPresent = !source_id.empty() && hasSavedEvidence;
        hasRuntimeEvidence = hasRuntimeEvidence || !runtime_trace_rows.empty();
        confidence.runtimeBacked = confidence.savedDataPresent && mode == PreviewMode::RuntimeBacked &&
                                   hasRuntimeEvidence && !runtime_trace_rows.empty();
        confidence.diagnosticsClean = !hasBlockerDiagnostic;
        confidence.exportAware = hasExportEvidence || surface_id == "export_preview";
        confidence.exactShipReady = confidence.exportAware && confidence.runtimeBacked &&
                                    confidence.diagnosticsClean && hasExportEvidence;
    }
};

} // namespace urpg::wysiwyg
