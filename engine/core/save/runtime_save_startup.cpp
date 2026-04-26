#include "engine/core/save/runtime_save_startup.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <map>
#include <regex>
#include <system_error>

#include <nlohmann/json.hpp>

namespace urpg {

namespace {

struct SlotDraft {
    RuntimeSaveStartupEntry entry;
    bool metadata_parse_failed = false;
};

bool fileExistsNonEmpty(const std::filesystem::path& path) {
    if (path.empty()) {
        return false;
    }

    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec) && !ec && std::filesystem::file_size(path, ec) > 0 && !ec;
}

std::string readAll(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return {};
    }
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

std::optional<int32_t> matchSlotId(const std::string& filename, const std::regex& pattern) {
    std::smatch match;
    if (!std::regex_match(filename, match, pattern) || match.size() < 2) {
        return std::nullopt;
    }

    try {
        return std::stoi(match[1].str());
    } catch (...) {
        return std::nullopt;
    }
}

SaveSlotCategory parseCategory(const std::string& value) {
    if (value == "autosave") {
        return SaveSlotCategory::Autosave;
    }
    if (value == "quicksave") {
        return SaveSlotCategory::Quicksave;
    }
    return SaveSlotCategory::Manual;
}

SaveRetentionClass parseRetentionClass(const std::string& value, SaveSlotCategory fallback_category) {
    if (value == "autosave") {
        return SaveRetentionClass::Autosave;
    }
    if (value == "quicksave") {
        return SaveRetentionClass::Quicksave;
    }
    if (value == "manual") {
        return SaveRetentionClass::Manual;
    }
    return RetentionClassForCategory(fallback_category);
}

bool hydrateMetaFromJson(const nlohmann::json& root, SaveSlotMeta& out_meta) {
    if (!root.is_object()) {
        return false;
    }

    if (root.contains("_slot_id") && root["_slot_id"].is_number_integer()) {
        out_meta.slot_id = root["_slot_id"].get<int32_t>();
    }
    if (root.contains("_slot_category") && root["_slot_category"].is_string()) {
        out_meta.category = parseCategory(root["_slot_category"].get<std::string>());
        out_meta.retention_class = RetentionClassForCategory(out_meta.category);
    }
    if (root.contains("_retention_class") && root["_retention_class"].is_string()) {
        out_meta.retention_class = parseRetentionClass(root["_retention_class"].get<std::string>(), out_meta.category);
    }
    if (root.contains("_save_version") && root["_save_version"].is_string()) {
        out_meta.save_version = root["_save_version"].get<std::string>();
    }
    if (root.contains("_timestamp") && root["_timestamp"].is_string()) {
        out_meta.timestamp = root["_timestamp"].get<std::string>();
    }
    if (root.contains("_playtime_seconds") && root["_playtime_seconds"].is_number_unsigned()) {
        out_meta.playtime_seconds = root["_playtime_seconds"].get<uint64_t>();
    }
    if (root.contains("_thumbnail_hash") && root["_thumbnail_hash"].is_string()) {
        out_meta.thumbnail_hash = root["_thumbnail_hash"].get<std::string>();
    }
    if (root.contains("_map_display_name") && root["_map_display_name"].is_string()) {
        out_meta.map_display_name = root["_map_display_name"].get<std::string>();
    }
    if (root.contains("_flags") && root["_flags"].is_object()) {
        const auto& flags = root["_flags"];
        if (flags.contains("autosave") && flags["autosave"].is_boolean()) {
            out_meta.flags.autosave = flags["autosave"].get<bool>();
            if (out_meta.flags.autosave) {
                out_meta.category = SaveSlotCategory::Autosave;
                out_meta.retention_class = SaveRetentionClass::Autosave;
            }
        }
        if (flags.contains("copilot_generated") && flags["copilot_generated"].is_boolean()) {
            out_meta.flags.copilot_generated = flags["copilot_generated"].get<bool>();
        }
        if (flags.contains("corrupted") && flags["corrupted"].is_boolean()) {
            out_meta.flags.corrupted = flags["corrupted"].get<bool>();
        }
    }

    return true;
}

bool tryHydrateMetadata(const std::filesystem::path& metadata_path, SaveSlotMeta& out_meta) {
    const auto payload = readAll(metadata_path);
    if (payload.empty()) {
        return false;
    }

    const auto parsed = nlohmann::json::parse(payload, nullptr, false);
    if (parsed.is_discarded()) {
        return false;
    }
    return hydrateMetaFromJson(parsed, out_meta);
}

std::string sortKeyFor(const RuntimeSaveStartupEntry& entry) {
    if (!entry.meta.timestamp.empty()) {
        return entry.meta.timestamp;
    }

    std::error_code ec;
    const auto path = !entry.paths.primary_save_path.empty() ? entry.paths.primary_save_path : entry.paths.metadata_path;
    const auto stamp = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return {};
    }
    return std::to_string(stamp.time_since_epoch().count());
}

void appendDiagnostic(RuntimeSaveStartupEntry& entry, const std::string& diagnostic) {
    if (entry.diagnostic.empty()) {
        entry.diagnostic = diagnostic;
        return;
    }
    entry.diagnostic += ";";
    entry.diagnostic += diagnostic;
}

RuntimeSaveStartupEntry finalizeDraft(SlotDraft draft, const std::filesystem::path& save_root) {
    auto& entry = draft.entry;
    entry.paths.slot_id = entry.slot_id;

    if (entry.paths.autosave_path.empty()) {
        entry.paths.autosave_path = save_root / "autosave.json";
    }
    if (entry.paths.primary_save_path.empty()) {
        entry.paths.primary_save_path = save_root / ("slot_" + std::to_string(entry.slot_id) + ".json");
    }
    if (entry.paths.metadata_path.empty()) {
        entry.paths.metadata_path = save_root / ("slot_" + std::to_string(entry.slot_id) + "_meta.json");
    }
    if (entry.paths.variables_path.empty()) {
        entry.paths.variables_path = save_root / ("slot_" + std::to_string(entry.slot_id) + "_vars.json");
    }

    entry.has_primary_payload = fileExistsNonEmpty(entry.paths.primary_save_path);
    entry.has_metadata_payload = fileExistsNonEmpty(entry.paths.metadata_path);
    entry.has_variables_payload = fileExistsNonEmpty(entry.paths.variables_path);

    entry.meta.slot_id = entry.slot_id;
    bool metadata_valid = false;
    if (entry.has_metadata_payload) {
        metadata_valid = tryHydrateMetadata(entry.paths.metadata_path, entry.meta);
        if (!metadata_valid) {
            draft.metadata_parse_failed = true;
            appendDiagnostic(entry, "metadata_parse_failed");
        }
        if (entry.meta.slot_id <= 0 && entry.slot_id > 0) {
            entry.meta.slot_id = entry.slot_id;
        }
    }

    if (entry.has_primary_payload) {
        entry.loadable = true;
        entry.expected_recovery_tier = SaveRecoveryTier::None;
        if (draft.metadata_parse_failed) {
            appendDiagnostic(entry, "primary_payload_loadable_without_metadata");
        }
    } else if (metadata_valid && entry.has_variables_payload) {
        entry.loadable = true;
        entry.requires_recovery = true;
        entry.expected_recovery_tier = SaveRecoveryTier::Level2MetadataVariables;
        appendDiagnostic(entry, "primary_missing_metadata_variables_recovery_available");
    } else if (metadata_valid) {
        entry.loadable = true;
        entry.requires_recovery = true;
        entry.expected_recovery_tier = SaveRecoveryTier::Level3SafeSkeleton;
        appendDiagnostic(entry, "primary_missing_safe_mode_recovery_available");
    } else if (!entry.diagnostic.empty()) {
        appendDiagnostic(entry, "slot_not_loadable");
    } else {
        entry.diagnostic = "slot_not_loadable";
    }

    return entry;
}

} // namespace

bool RuntimeSaveStartupState::hasLoadableSave() const {
    return newestLoadableSave() != nullptr;
}

const RuntimeSaveStartupEntry* RuntimeSaveStartupState::newestLoadableSave() const {
    for (const auto& entry : entries) {
        if (entry.loadable) {
            return &entry;
        }
    }
    return nullptr;
}

std::string RuntimeSaveStartupState::continueDisabledReason() const {
    if (hasLoadableSave()) {
        return {};
    }
    return "No loadable save data found";
}

std::filesystem::path defaultRuntimeSaveRoot(const std::filesystem::path& project_root) {
    const auto root = project_root.empty() ? std::filesystem::current_path() : project_root;
    return root / "saves";
}

RuntimeSaveStartupState discoverRuntimeSaves(const std::filesystem::path& project_root) {
    RuntimeSaveStartupState state;
    state.save_root = defaultRuntimeSaveRoot(project_root);

    std::error_code ec;
    if (!std::filesystem::is_directory(state.save_root, ec) || ec) {
        return state;
    }

    static const std::regex primary_pattern(R"(slot_(\d+)\.(json|ursv))", std::regex::icase);
    static const std::regex metadata_pattern(R"(slot_(\d+)(_meta|_metadata|\.meta)\.json)", std::regex::icase);
    static const std::regex variables_pattern(R"(slot_(\d+)(_vars|_variables|\.vars)\.json)", std::regex::icase);

    std::map<int32_t, SlotDraft> drafts;
    for (const auto& it : std::filesystem::directory_iterator(state.save_root, ec)) {
        if (ec || !it.is_regular_file()) {
            continue;
        }

        const auto path = it.path();
        const auto filename = path.filename().string();

        if (filename == "autosave.json") {
            auto& draft = drafts[0];
            draft.entry.slot_id = 0;
            draft.entry.paths.primary_save_path = path;
            draft.entry.paths.autosave_path = path;
            draft.entry.meta.category = SaveSlotCategory::Autosave;
            draft.entry.meta.retention_class = SaveRetentionClass::Autosave;
            draft.entry.meta.flags.autosave = true;
            continue;
        }

        if (const auto slot_id = matchSlotId(filename, primary_pattern)) {
            auto& draft = drafts[*slot_id];
            draft.entry.slot_id = *slot_id;
            draft.entry.paths.primary_save_path = path;
            continue;
        }

        if (const auto slot_id = matchSlotId(filename, metadata_pattern)) {
            auto& draft = drafts[*slot_id];
            draft.entry.slot_id = *slot_id;
            draft.entry.paths.metadata_path = path;
            continue;
        }

        if (const auto slot_id = matchSlotId(filename, variables_pattern)) {
            auto& draft = drafts[*slot_id];
            draft.entry.slot_id = *slot_id;
            draft.entry.paths.variables_path = path;
            continue;
        }
    }

    for (auto& [slot_id, draft] : drafts) {
        if (draft.entry.slot_id < 0) {
            draft.entry.slot_id = slot_id;
        }
        state.entries.push_back(finalizeDraft(std::move(draft), state.save_root));
    }

    std::sort(state.entries.begin(), state.entries.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.loadable != rhs.loadable) {
            return lhs.loadable && !rhs.loadable;
        }
        const auto lhs_key = sortKeyFor(lhs);
        const auto rhs_key = sortKeyFor(rhs);
        if (lhs_key != rhs_key) {
            return lhs_key > rhs_key;
        }
        return lhs.slot_id > rhs.slot_id;
    });

    return state;
}

RuntimeSaveLoadRequest makeRuntimeSaveLoadRequest(const RuntimeSaveSlotPaths& paths) {
    RuntimeSaveLoadRequest request;
    request.primary_save_path = paths.primary_save_path;
    request.autosave_path = paths.autosave_path;
    request.metadata_path = paths.metadata_path;
    request.variables_path = paths.variables_path;
    return request;
}

RuntimeSaveContinueResult continueNewestRuntimeSave(const RuntimeSaveStartupState& state) {
    RuntimeSaveContinueResult result;
    const auto* slot = state.newestLoadableSave();
    if (slot == nullptr) {
        result.error = "no_loadable_save";
        return result;
    }

    SaveCatalog catalog;
    SaveSessionCoordinator coordinator(catalog);
    const auto load_result = coordinator.load({slot->slot_id, makeRuntimeSaveLoadRequest(slot->paths)});

    result.ok = load_result.ok;
    result.slot_id = load_result.slot_id;
    result.boot_safe_mode = load_result.boot_safe_mode;
    result.loaded_from_recovery = load_result.loaded_from_recovery;
    result.recovery_tier = load_result.recovery_tier;
    result.error = load_result.error;
    result.active_meta = load_result.active_meta;

    if (result.ok && result.active_meta.slot_id <= 0 && slot->slot_id > 0) {
        result.active_meta.slot_id = slot->slot_id;
    }
    if (!result.ok && result.error.empty()) {
        result.error = "runtime_save_load_failed";
    }

    return result;
}

} // namespace urpg
