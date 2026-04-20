#include "engine/core/save/save_migration.h"

#include <nlohmann/json.hpp>

#include <utility>

namespace urpg::save {

namespace {

using json = nlohmann::json;

std::string SeverityLabel(SaveMigrationSeverity severity) {
    switch (severity) {
    case SaveMigrationSeverity::Info:
        return "info";
    case SaveMigrationSeverity::Warning:
        return "warn";
    case SaveMigrationSeverity::Error:
        return "error";
    }
    return "info";
}

template <typename T>
void CopyIfPresent(const json& source, const char* source_key, json& target, const char* target_key) {
    const auto it = source.find(source_key);
    if (it == source.end() || it->is_null()) {
        return;
    }
    try {
        target[target_key] = it->get<T>();
    } catch (const json::exception&) {
    }
}

void AddMappingNote(json& target, std::string field_path, json value) {
    if (!target.contains("_compat_mapping_notes") || !target["_compat_mapping_notes"].is_array()) {
        target["_compat_mapping_notes"] = json::array();
    }
    target["_compat_mapping_notes"].push_back(
        {
            {"field_path", std::move(field_path)},
            {"value", std::move(value)},
        });
}

} // namespace

SaveMigrationResult UpgradeCompatSaveMetadataDocument(const nlohmann::json& compat_document) {
    SaveMigrationResult result;
    result.migrated_metadata = {{"_urpg_format_version", "1.0"}};

    const auto emit_diagnostic = [&](SaveMigrationSeverity severity, std::string code, std::string field_path,
                                     std::string message) {
        result.diagnostics.push_back(
            SaveMigrationDiagnostic{
                .severity = severity,
                .code = std::move(code),
                .field_path = std::move(field_path),
                .message = std::move(message),
            });
    };

    if (!compat_document.is_object()) {
        emit_diagnostic(SaveMigrationSeverity::Error, "invalid_compat_document", "/",
                        "Compat save metadata payload is not an object; emitting native fallback metadata.");
        result.used_safe_fallback = true;
        return result;
    }

    const auto* meta = &compat_document;
    if (const auto meta_it = compat_document.find("meta"); meta_it != compat_document.end() && meta_it->is_object()) {
        meta = &(*meta_it);
    }

    CopyIfPresent<int32_t>(*meta, "slotId", result.migrated_metadata, "_slot_id");
    CopyIfPresent<std::string>(*meta, "mapName", result.migrated_metadata, "_map_display_name");
    CopyIfPresent<uint64_t>(*meta, "playtimeSeconds", result.migrated_metadata, "_playtime_seconds");
    CopyIfPresent<std::string>(*meta, "saveVersion", result.migrated_metadata, "_save_version");
    CopyIfPresent<std::string>(*meta, "thumbnailHash", result.migrated_metadata, "_thumbnail_hash");
    CopyIfPresent<std::string>(*meta, "uiTab", result.migrated_metadata, "_ui_tab");

    if (const auto header_it = compat_document.find("pluginHeader");
        header_it != compat_document.end() && header_it->is_object()) {
        const auto& plugin_header = *header_it;
        CopyIfPresent<std::string>(plugin_header, "thumbnailHash", result.migrated_metadata, "_thumbnail_hash");
        CopyIfPresent<std::string>(plugin_header, "uiTab", result.migrated_metadata, "_ui_tab");

        for (auto it = plugin_header.begin(); it != plugin_header.end(); ++it) {
            if (it.key() == "thumbnailHash" || it.key() == "uiTab") {
                continue;
            }
            emit_diagnostic(SaveMigrationSeverity::Warning, "unmapped_plugin_header_field",
                            "/pluginHeader/" + it.key(),
                            "Plugin header field is not mapped into the native save schema; preserving a mapping note.");
            AddMappingNote(result.migrated_metadata, "/pluginHeader/" + it.key(), it.value());
            result.used_safe_fallback = true;
        }
    }

    for (auto it = meta->begin(); it != meta->end(); ++it) {
        if (it.key() == "_urpg_format_version" || it.key() == "slotId" || it.key() == "mapName" ||
            it.key() == "playtimeSeconds" || it.key() == "saveVersion" || it.key() == "thumbnailHash" ||
            it.key() == "uiTab") {
            continue;
        }
        emit_diagnostic(SaveMigrationSeverity::Warning, "unmapped_meta_field", "/meta/" + it.key(),
                        "Compat save metadata field is not mapped into the native save schema; preserving a mapping note.");
        AddMappingNote(result.migrated_metadata, "/meta/" + it.key(), it.value());
        result.used_safe_fallback = true;
    }

    return result;
}

std::string ExportSaveMigrationDiagnosticsJsonl(const SaveMigrationResult& result) {
    std::string jsonl;
    for (const auto& diagnostic : result.diagnostics) {
        json row = {
            {"level", SeverityLabel(diagnostic.severity)},
            {"subsystem", "save_migration"},
            {"code", diagnostic.code},
            {"field_path", diagnostic.field_path},
            {"message", diagnostic.message},
        };
        if (!jsonl.empty()) {
            jsonl.push_back('\n');
        }
        jsonl += row.dump();
    }
    return jsonl;
}

} // namespace urpg::save
