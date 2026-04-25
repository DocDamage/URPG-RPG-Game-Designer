#include "engine/core/save/save_debugger.h"

namespace urpg::save {

SaveDebugSlot SaveDebugger::inspectSlot(const nlohmann::json& save_document) const {
    SaveDebugSlot slot;
    if (!save_document.is_object()) {
        slot.corrupted = true;
        slot.diagnostics.push_back("corrupted_json_document");
        return slot;
    }

    slot.slot_id = save_document.value("slot_id", save_document.value("_slot_id", -1));
    slot.recovery_tier = save_document.value("recovery_tier", "primary");

    if (save_document.contains("metadata") && save_document["metadata"].is_object()) {
        slot.metadata = save_document["metadata"];
    } else {
        for (const auto& key : {"_save_version", "_timestamp", "_map_display_name", "_playtime_seconds"}) {
            if (save_document.contains(key)) {
                slot.metadata[key] = save_document[key];
            }
        }
    }

    if (save_document.contains("subsystems") && save_document["subsystems"].is_object()) {
        slot.subsystem_state = save_document["subsystems"];
    }

    if (save_document.contains("migration_notes") && save_document["migration_notes"].is_array()) {
        for (const auto& note : save_document["migration_notes"]) {
            if (note.is_string()) {
                slot.migration_notes.push_back(note.get<std::string>());
            }
        }
    }

    if (save_document.value("schema_version", "urpg.save.v1") != "urpg.save.v1") {
        slot.diagnostics.push_back("unexpected_schema_version:" + save_document.value("schema_version", ""));
    }
    if (save_document.value("corrupted", false)) {
        slot.corrupted = true;
        slot.diagnostics.push_back("slot_marked_corrupted");
    }
    if (slot.slot_id < 0) {
        slot.diagnostics.push_back("missing_slot_id");
    }
    return slot;
}

nlohmann::json SaveDebugger::exportDiagnostics(const SaveDebugSlot& slot) const {
    return {
        {"slot_id", slot.slot_id},
        {"corrupted", slot.corrupted},
        {"recovery_tier", slot.recovery_tier},
        {"metadata", slot.metadata},
        {"subsystem_state", slot.subsystem_state},
        {"migration_notes", slot.migration_notes},
        {"diagnostics", slot.diagnostics},
    };
}

} // namespace urpg::save
