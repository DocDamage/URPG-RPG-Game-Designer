#include "engine/core/save/save_compatibility_preview.h"

#include "engine/core/save/save_migration.h"

namespace urpg::save {

namespace {

std::string severityLabel(SaveMigrationSeverity severity) {
    switch (severity) {
    case SaveMigrationSeverity::Info:
        return "info";
    case SaveMigrationSeverity::Warning:
        return "warning";
    case SaveMigrationSeverity::Error:
        return "error";
    }
    return "info";
}

} // namespace

SaveCompatibilityPreview SaveCompatibilityPreviewer::preview(const nlohmann::json& old_save_document) const {
    SaveCompatibilityPreview preview;
    const auto imported = ImportCompatSaveDocument(old_save_document);
    preview.native_payload = imported.native_payload;
    preview.migrated_metadata = imported.migrated_metadata;
    preview.ok = !imported.used_safe_fallback;

    if (preview.native_payload.contains("_compat_payload_retained")) {
        preview.retained_payload = preview.native_payload["_compat_payload_retained"];
    }

    for (const auto& diagnostic : imported.diagnostics) {
        preview.diagnostics.push_back(
            severityLabel(diagnostic.severity) + ":" + diagnostic.code + ":" + diagnostic.field_path);
    }
    return preview;
}

} // namespace urpg::save
