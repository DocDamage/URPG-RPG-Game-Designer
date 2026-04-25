#include "editor/save/save_migration_preview_panel.h"

#include <utility>

namespace urpg::editor {

void SaveMigrationPreviewPanel::setPreview(urpg::save::SaveCompatibilityPreview preview) {
    preview_ = std::move(preview);
}

void SaveMigrationPreviewPanel::render() {
    snapshot_ = {
        {"panel", "save_migration_preview"},
        {"ok", preview_.ok},
        {"native_payload", preview_.native_payload},
        {"migrated_metadata", preview_.migrated_metadata},
        {"retained_payload", preview_.retained_payload},
        {"diagnostics", preview_.diagnostics},
    };
}

nlohmann::json SaveMigrationPreviewPanel::lastRenderSnapshot() const {
    return snapshot_;
}

} // namespace urpg::editor
