#include "engine/core/save/save_migration.h"

#include <algorithm>
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

void AppendDiagnostic(std::vector<SaveMigrationDiagnostic>& diagnostics,
                      SaveMigrationSeverity severity,
                      std::string code,
                      std::string field_path,
                      std::string message) {
    diagnostics.push_back(
        SaveMigrationDiagnostic{
            .severity = severity,
            .code = std::move(code),
            .field_path = std::move(field_path),
            .message = std::move(message),
        });
}

void AppendRetainedPayloadNote(json& target, std::string field_path, json value) {
    if (!target.contains("_compat_payload_retained") || !target["_compat_payload_retained"].is_object()) {
        target["_compat_payload_retained"] = json::object();
    }
    target["_compat_payload_retained"][std::move(field_path)] = std::move(value);
}

} // namespace

SaveMigrationResult UpgradeCompatSaveMetadataDocument(const nlohmann::json& compat_document) {
    SaveMigrationResult result;
    result.migrated_metadata = {{"_urpg_format_version", "1.0"}};

    const auto emit_diagnostic = [&](SaveMigrationSeverity severity, std::string code, std::string field_path,
                                     std::string message) {
        AppendDiagnostic(result.diagnostics, severity, std::move(code), std::move(field_path), std::move(message));
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

CompatSaveImportResult ImportCompatSaveDocument(const nlohmann::json& compat_document) {
    CompatSaveImportResult result;
    result.native_payload = {
        {"_urpg_format_version", "1.0"},
        {"player", json::object()},
    };

    const auto metadata_result = UpgradeCompatSaveMetadataDocument(compat_document);
    result.migrated_metadata = metadata_result.migrated_metadata;
    result.diagnostics = metadata_result.diagnostics;
    result.used_safe_fallback = metadata_result.used_safe_fallback;

    const auto emit_diagnostic = [&](SaveMigrationSeverity severity, std::string code, std::string field_path,
                                     std::string message) {
        AppendDiagnostic(result.diagnostics, severity, std::move(code), std::move(field_path), std::move(message));
    };

    if (!compat_document.is_object()) {
        emit_diagnostic(SaveMigrationSeverity::Error, "invalid_compat_payload", "/",
                        "Compat save payload is not an object; emitting safe native fallback payload.");
        result.used_safe_fallback = true;
        return result;
    }

    if (compat_document.contains("gold")) {
        if (compat_document["gold"].is_number_integer()) {
            result.native_payload["player"]["gold"] = compat_document["gold"].get<int32_t>();
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_gold_field", "/gold",
                            "Compat save gold field is not an integer; skipping native gold mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("mapId")) {
        if (compat_document["mapId"].is_number_integer()) {
            result.native_payload["map_id"] = compat_document["mapId"].get<int32_t>();
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_map_id_field", "/mapId",
                            "Compat save mapId field is not an integer; skipping native map mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("playerX")) {
        if (compat_document["playerX"].is_number_integer()) {
            result.native_payload["player"]["x"] = compat_document["playerX"].get<int32_t>();
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_player_x_field", "/playerX",
                            "Compat save playerX field is not an integer; skipping native player position mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("playerY")) {
        if (compat_document["playerY"].is_number_integer()) {
            result.native_payload["player"]["y"] = compat_document["playerY"].get<int32_t>();
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_player_y_field", "/playerY",
                            "Compat save playerY field is not an integer; skipping native player position mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("direction")) {
        if (compat_document["direction"].is_number_integer()) {
            result.native_payload["player"]["direction"] = compat_document["direction"].get<int32_t>();
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_direction_field", "/direction",
                            "Compat save direction field is not an integer; skipping native facing mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("switches")) {
        if (compat_document["switches"].is_object()) {
            result.native_payload["switches"] = compat_document["switches"];
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_switches_field", "/switches",
                            "Compat save switches field is not an object; skipping native switch mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("variables")) {
        if (compat_document["variables"].is_object()) {
            result.native_payload["variables"] = compat_document["variables"];
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_variables_field", "/variables",
                            "Compat save variables field is not an object; skipping native variable mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("party")) {
        if (compat_document["party"].is_array()) {
            result.native_payload["party"] = json::array();
            if (!result.migrated_metadata.contains("_party_snapshot") || !result.migrated_metadata["_party_snapshot"].is_array()) {
                result.migrated_metadata["_party_snapshot"] = json::array();
            }

            for (size_t index = 0; index < compat_document["party"].size(); ++index) {
                const auto& member = compat_document["party"][index];
                if (!member.is_object()) {
                    emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_party_member", "/party/" + std::to_string(index),
                                    "Compat party entry is not an object; preserving raw entry in retained payload.");
                    AppendRetainedPayloadNote(result.native_payload, "/party/" + std::to_string(index), member);
                    result.used_safe_fallback = true;
                    continue;
                }

                json native_member = json::object();
                bool has_actor_reference = false;
                if (member.contains("actorId") && member["actorId"].is_number_integer()) {
                    const int32_t actor_id = member["actorId"].get<int32_t>();
                    if (actor_id > 0) {
                        native_member["actor_id"] = actor_id;
                        has_actor_reference = true;
                    } else {
                        emit_diagnostic(SaveMigrationSeverity::Warning, "missing_actor_reference",
                                        "/party/" + std::to_string(index) + "/actorId",
                                        "Compat party entry references a non-positive actor id; preserving raw member payload.");
                        result.used_safe_fallback = true;
                    }
                } else {
                    emit_diagnostic(SaveMigrationSeverity::Warning, "missing_actor_reference",
                                    "/party/" + std::to_string(index) + "/actorId",
                                    "Compat party entry is missing actorId; preserving raw member payload.");
                    result.used_safe_fallback = true;
                }

                if (member.contains("name") && member["name"].is_string()) {
                    native_member["name"] = member["name"].get<std::string>();
                }
                if (member.contains("level") && member["level"].is_number_integer()) {
                    native_member["level"] = member["level"].get<int32_t>();
                }
                if (member.contains("hp") && member["hp"].is_number_integer()) {
                    native_member["hp"] = member["hp"].get<int32_t>();
                }
                if (member.contains("mhp") && member["mhp"].is_number_integer()) {
                    native_member["max_hp"] = member["mhp"].get<int32_t>();
                }

                if (has_actor_reference) {
                    result.native_payload["party"].push_back(native_member);
                    result.migrated_metadata["_party_snapshot"].push_back(
                        {
                            {"name", native_member.value("name", "Actor " + std::to_string(native_member["actor_id"].get<int32_t>()))},
                            {"level", native_member.value("level", 1)},
                            {"hp", native_member.value("hp", 0)},
                            {"max_hp", native_member.value("max_hp", 0)},
                        });
                } else {
                    AppendRetainedPayloadNote(result.native_payload, "/party/" + std::to_string(index), member);
                }
            }
        } else {
            emit_diagnostic(SaveMigrationSeverity::Warning, "invalid_party_field", "/party",
                            "Compat save party field is not an array; skipping native party mapping.");
            result.used_safe_fallback = true;
        }
    }

    if (compat_document.contains("pluginData")) {
        emit_diagnostic(SaveMigrationSeverity::Warning, "unsupported_plugin_blob", "/pluginData",
                        "Compat pluginData blob is retained for manual follow-up instead of being mapped into native runtime state.");
        AppendRetainedPayloadNote(result.native_payload, "/pluginData", compat_document["pluginData"]);
        result.used_safe_fallback = true;
    }

    static const std::vector<std::string> recognized_keys = {
        "_urpg_format_version", "meta", "pluginHeader", "gold", "mapId", "playerX", "playerY",
        "direction", "party", "switches", "variables", "pluginData"
    };

    for (auto it = compat_document.begin(); it != compat_document.end(); ++it) {
        if (std::find(recognized_keys.begin(), recognized_keys.end(), it.key()) != recognized_keys.end()) {
            continue;
        }

        emit_diagnostic(SaveMigrationSeverity::Warning, "retained_compat_payload_field", "/" + it.key(),
                        "Compat save field is not mapped into native runtime state; preserving retained payload copy.");
        AppendRetainedPayloadNote(result.native_payload, "/" + it.key(), it.value());
        result.used_safe_fallback = true;
    }

    if (result.native_payload["player"].empty()) {
        result.native_payload.erase("player");
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
