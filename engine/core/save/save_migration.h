#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::save {

enum class SaveMigrationSeverity : uint8_t {
    Info = 0,
    Warning = 1,
    Error = 2,
};

struct SaveMigrationDiagnostic {
    SaveMigrationSeverity severity = SaveMigrationSeverity::Info;
    std::string code;
    std::string field_path;
    std::string message;
};

struct SaveMigrationResult {
    nlohmann::json migrated_metadata;
    std::vector<SaveMigrationDiagnostic> diagnostics;
    bool used_safe_fallback = false;
};

struct CompatSaveImportResult {
    nlohmann::json native_payload;
    nlohmann::json migrated_metadata;
    std::vector<SaveMigrationDiagnostic> diagnostics;
    bool used_safe_fallback = false;
};

SaveMigrationResult UpgradeCompatSaveMetadataDocument(const nlohmann::json& compat_document);
CompatSaveImportResult ImportCompatSaveDocument(const nlohmann::json& compat_document);
std::string ExportSaveMigrationDiagnosticsJsonl(const SaveMigrationResult& result);

} // namespace urpg::save
