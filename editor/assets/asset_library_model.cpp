#include "editor/assets/asset_library_model.h"

#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/global_asset_library_store.h"
#include "engine/core/assets/global_asset_promotion_service.h"
#include "engine/core/assets/project_asset_attachment_service.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace urpg::editor {

void AssetLibraryModel::ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                                      std::string_view duplicate_csv) {
    ingestReports(hygiene_summary, intake_report, nlohmann::json::object(), duplicate_csv);
}

void AssetLibraryModel::ingestReports(const nlohmann::json& hygiene_summary, const nlohmann::json& intake_report,
                                      const nlohmann::json& promotion_catalog, std::string_view duplicate_csv) {
    library_.clear();
    action_history_ = nlohmann::json::array();
    library_.ingestHygieneSummary(hygiene_summary);
    library_.ingestIntakeReport(intake_report);
    library_.ingestPromotionCatalog(promotion_catalog);
    library_.ingestDuplicateCsv(duplicate_csv);
    library_.detectCaseCollisions();
    rebuildCleanupPreview();
    snapshot_.reports_loaded = true;
    snapshot_.status = "ready";
    snapshot_.status_message = "";
    snapshot_.error_message = "";
}

void AssetLibraryModel::ingestPromotionManifest(const urpg::assets::AssetPromotionManifest& manifest) {
    library_.ingestPromotionManifest(manifest);
    rebuildCleanupPreview();
    snapshot_.reports_loaded = true;
    snapshot_.status = "ready";
    snapshot_.status_message = "";
    snapshot_.error_message = "";
}

nlohmann::json AssetLibraryModel::requestImportSource(const std::filesystem::path& source,
                                                      const std::filesystem::path& library_root,
                                                      std::string session_id,
                                                      std::string license_note) {
    const auto sourcePath = source.generic_string();
    const auto libraryRoot = library_root.generic_string();
    const auto expectedManifest =
        (library_root / "catalog" / "import_sessions" / (session_id + ".json")).generic_string();
    nlohmann::json command = nlohmann::json::array({
        "python",
        "tools/assets/global_asset_import.py",
        "--source",
        sourcePath,
        "--library-root",
        libraryRoot,
        "--session-id",
        session_id,
    });
    if (!license_note.empty()) {
        command.push_back("--license-note");
        command.push_back(license_note);
    }

    pending_import_request_ = {
        {"source_path", sourcePath},
        {"library_root", libraryRoot},
        {"session_id", session_id},
        {"license_note", license_note},
        {"expected_manifest_path", expectedManifest},
        {"command", command},
    };
    nlohmann::json action = {
        {"action", "request_import_source"},
        {"success", !sourcePath.empty() && !libraryRoot.empty() && !session_id.empty()},
        {"code", (!sourcePath.empty() && !libraryRoot.empty() && !session_id.empty())
                     ? "import_source_requested"
                     : "import_source_request_invalid"},
        {"message", (!sourcePath.empty() && !libraryRoot.empty() && !session_id.empty())
                        ? "Import source request is ready for the external importer."
                        : "Import source, library root, and session id are required."},
        {"source_path", sourcePath},
        {"library_root", libraryRoot},
        {"session_id", session_id},
        {"expected_manifest_path", expectedManifest},
        {"command", command},
    };
    action_history_.push_back(action);
    refreshSnapshot();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

void AssetLibraryModel::ingestImportSession(urpg::assets::AssetImportSession session) {
    pending_import_request_ = nlohmann::json::object();
    session.summary = urpg::assets::summarizeAssetImportSession(session);
    auto found = std::find_if(import_sessions_.begin(), import_sessions_.end(), [&](const auto& existing) {
        return existing.sessionId == session.sessionId;
    });
    if (found == import_sessions_.end()) {
        import_sessions_.push_back(std::move(session));
    } else {
        *found = std::move(session);
    }
    refreshSnapshot();
}

void AssetLibraryModel::clearImportSessions() {
    import_sessions_.clear();
    pending_import_request_ = nlohmann::json::object();
    refreshSnapshot();
}

namespace {

void ingestCatalogWithShards(urpg::assets::AssetLibrary& library, const std::filesystem::path& reports_root,
                             const nlohmann::json& promotion_catalog) {
    library.ingestPromotionCatalog(promotion_catalog);
    const auto shards = promotion_catalog.find("shards");
    if (shards == promotion_catalog.end() || !shards->is_array()) {
        return;
    }
    const auto repo_root = reports_root.parent_path();
    for (const auto& shard : *shards) {
        if (!shard.is_object()) {
            continue;
        }
        const auto path_it = shard.find("path");
        if (path_it == shard.end() || !path_it->is_string()) {
            continue;
        }
        auto shard_path = std::filesystem::path(path_it->get<std::string>());
        if (shard_path.is_relative()) {
            const auto relative = shard_path;
            const auto repo_relative = repo_root.parent_path() / relative;
            const auto imports_relative = repo_root / relative;
            const auto reports_relative = reports_root / relative;
            if (std::filesystem::is_regular_file(reports_relative)) {
                shard_path = reports_relative;
            } else if (std::filesystem::is_regular_file(repo_relative)) {
                shard_path = repo_relative;
            } else {
                shard_path = imports_relative;
            }
        }
        if (!std::filesystem::is_regular_file(shard_path)) {
            continue;
        }
        std::ifstream shard_stream(shard_path);
        library.ingestPromotionCatalog(nlohmann::json::parse(shard_stream));
    }
}

nlohmann::json filterControls(const urpg::assets::AssetLibraryFilter& filter,
                              const urpg::assets::AssetLibrarySnapshot& snapshot, size_t filteredCount,
                              size_t projectAttachedCount, size_t projectAttachableCount) {
    return {
        {"active_filter",
         {
             {"media_kind", filter.media_kind},
             {"category", filter.category},
             {"required_tag", filter.required_tag},
             {"required_status", filter.required_status.has_value()
                                     ? nlohmann::json(urpg::assets::toString(*filter.required_status))
                                     : nlohmann::json(nullptr)},
             {"referenced_only", filter.referenced_only},
             {"runtime_ready_only", filter.runtime_ready_only},
             {"previewable_only", filter.previewable_only},
             {"project_attached_only", filter.project_attached_only},
             {"attachable_only", filter.attachable_only},
             {"result_count", filteredCount},
         }},
        {"quick_filters",
         {
             {"sequence_packs",
              {
                  {"visible", true},
                  {"enabled", snapshot.sequence_asset_count > 0},
                  {"label", "Sequence Packs"},
                  {"action", "filter_asset_sequence_packs"},
                  {"media_kind", "image_sequence_collection"},
                  {"count", snapshot.sequence_asset_count},
                  {"frame_count", snapshot.sequence_frame_count},
                  {"clip_count", snapshot.sequence_clip_count},
              }},
             {"runtime_ready",
              {
                  {"visible", true},
                  {"enabled", snapshot.runtime_ready_count > 0},
                  {"action", "filter_runtime_ready_assets"},
                  {"count", snapshot.runtime_ready_count},
              }},
             {"previewable",
              {
                  {"visible", true},
                  {"enabled", snapshot.previewable_count > 0},
                  {"action", "filter_previewable_assets"},
                  {"count", snapshot.previewable_count},
              }},
             {"attachable",
              {
                  {"visible", true},
                  {"enabled", projectAttachableCount > 0},
                  {"action", "filter_attachable_assets"},
                  {"count", projectAttachableCount},
              }},
             {"project_attached",
              {
                  {"visible", true},
                  {"enabled", projectAttachedCount > 0},
                  {"action", "filter_project_attached_assets"},
                  {"count", projectAttachedCount},
              }},
         }},
    };
}

nlohmann::json wizardStep(std::string id, std::string label, std::string state, size_t count = 0) {
    return {
        {"id", std::move(id)},
        {"label", std::move(label)},
        {"state", std::move(state)},
        {"count", count},
    };
}

nlohmann::json buildImportWizardSnapshot(const AssetLibraryModelSnapshot& snapshot,
                                         const nlohmann::json& pendingImportRequest) {
    const bool hasSource = snapshot.import_session_count > 0;
    const bool hasReviewRows = snapshot.import_review_row_count > 0;
    const bool hasPromotableRows = snapshot.import_ready_count > 0;
    const bool hasConvertibleRows = snapshot.import_needs_conversion_count > 0;
    const bool hasPromotedAssets = snapshot.project_attachable_count > 0;
    const bool hasAttachedAssets = snapshot.project_attached_count > 0;
    const bool hasPendingRequest = pendingImportRequest.is_object() && !pendingImportRequest.empty();

    std::string currentStep = "add_source";
    std::string status = "empty";
    if (hasAttachedAssets) {
        currentStep = "package";
        status = "package_ready";
    } else if (hasPromotedAssets) {
        currentStep = "attach";
        status = "ready_to_attach";
    } else if (hasReviewRows) {
        currentStep = "review";
        status = "review_required";
    } else if (hasSource) {
        currentStep = "review";
        status = "source_loaded";
    } else if (hasPendingRequest) {
        currentStep = "add_source";
        status = "source_requested";
    }

    const auto reviewState = currentStep == "review" ? "active" : (hasReviewRows || hasPromotedAssets || hasAttachedAssets ? "complete" : "pending");
    const auto promoteState = currentStep == "review" && hasPromotableRows ? "available" : (hasPromotedAssets || hasAttachedAssets ? "complete" : "pending");
    const auto attachState = currentStep == "attach" ? "active" : (hasAttachedAssets ? "complete" : "pending");
    const auto packageState = currentStep == "package" ? "active" : "pending";

    return {
        {"status", status},
        {"current_step", currentStep},
        {"steps",
         nlohmann::json::array({
             wizardStep("add_source", "Add Source", hasSource ? "complete" : "active", snapshot.import_session_count),
             wizardStep("review", "Review", reviewState, snapshot.import_review_row_count),
             wizardStep("promote", "Promote", promoteState, snapshot.import_ready_count),
             wizardStep("attach", "Attach", attachState, snapshot.project_attachable_count),
             wizardStep("package", "Package", packageState, snapshot.project_attached_count),
         })},
        {"actions",
         {
             {"add_source",
              {
                  {"enabled", true},
                  {"action", "asset_library_add_source"},
                  {"requires_native_picker", true},
                  {"pending_request", hasPendingRequest},
              }},
             {"promote_selected",
              {
                  {"enabled", hasPromotableRows},
                  {"action", "asset_library_promote_selected"},
                  {"eligible_count", snapshot.import_ready_count},
                  {"disabled_reason", hasPromotableRows ? nlohmann::json(nullptr) : nlohmann::json("no_promotable_import_records")},
              }},
             {"convert_selected",
              {
                  {"enabled", hasConvertibleRows},
                  {"action", "asset_library_convert_selected"},
                  {"eligible_count", snapshot.import_needs_conversion_count},
                  {"disabled_reason", hasConvertibleRows ? nlohmann::json(nullptr) : nlohmann::json("no_convertible_import_records")},
              }},
             {"attach_selected",
              {
                  {"enabled", hasPromotedAssets},
                  {"action", "asset_library_attach_selected"},
                  {"eligible_count", snapshot.project_attachable_count},
                  {"disabled_reason", hasPromotedAssets ? nlohmann::json(nullptr) : nlohmann::json("no_promoted_project_ready_assets")},
              }},
             {"package_validate",
              {
                  {"enabled", hasAttachedAssets},
                  {"action", "asset_library_package_validate"},
                  {"eligible_count", snapshot.project_attached_count},
                  {"disabled_reason", hasAttachedAssets ? nlohmann::json(nullptr) : nlohmann::json("no_attached_project_assets")},
              }},
         }},
        {"counts",
         {
             {"sessions", snapshot.import_session_count},
             {"review_rows", snapshot.import_review_row_count},
             {"ready_to_promote", snapshot.import_ready_count},
             {"needs_conversion", snapshot.import_needs_conversion_count},
             {"missing_license", snapshot.import_missing_license_count},
             {"promoted_attachable", snapshot.project_attachable_count},
             {"project_attached", snapshot.project_attached_count},
         }},
        {"pending_request", hasPendingRequest ? pendingImportRequest : nlohmann::json(nullptr)},
    };
}

urpg::assets::AssetPromotionManifest manifestFromAssetRecord(const urpg::assets::AssetRecord& record) {
    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = record.asset_id;
    manifest.sourcePath = record.source_path.empty() ? record.path : record.source_path;
    manifest.promotedPath = record.promoted_path;
    manifest.licenseId = record.license_id;
    manifest.status = urpg::assets::assetPromotionStatusFromString(record.promotion_status);
    manifest.preview.kind = record.preview_kind.empty() ? "none" : record.preview_kind;
    manifest.preview.thumbnailPath = record.preview_path;
    manifest.preview.width = record.preview_width;
    manifest.preview.height = record.preview_height;
    manifest.package.includeInRuntime = record.include_in_runtime;
    manifest.package.requiredForRelease = record.required_for_release;
    manifest.diagnostics = record.promotion_diagnostics;
    return manifest;
}

std::string findAssetPathById(const urpg::assets::AssetLibrary& library, const std::string& asset_id) {
    for (const auto& asset : library.snapshot().assets) {
        if (asset.asset_id == asset_id) {
            return asset.path;
        }
    }
    return {};
}

void eraseDiagnostic(std::vector<std::string>& diagnostics, std::string_view code) {
    diagnostics.erase(std::remove(diagnostics.begin(), diagnostics.end(), code), diagnostics.end());
}

std::string quoteShellArg(const std::string& value) {
    std::string out = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            out += "\\\"";
        } else {
            out += ch;
        }
    }
    out += "\"";
    return out;
}

AssetLibraryModel::ConversionCommandResult runConversionCommandWithSystem(
    const AssetLibraryModel::ConversionCommand& command) {
    if (command.arguments.empty()) {
        return {1, "", "conversion command is empty"};
    }

    std::ostringstream shell;
    bool first = true;
    for (const auto& arg : command.arguments) {
        if (!first) {
            shell << ' ';
        }
        shell << quoteShellArg(arg);
        first = false;
    }

    const auto previous = std::filesystem::current_path();
    std::error_code cwdError;
    std::filesystem::current_path(command.working_directory, cwdError);
    if (cwdError) {
        return {1, "", cwdError.message()};
    }
    const int exitCode = std::system(shell.str().c_str());
    std::filesystem::current_path(previous, cwdError);
    return {exitCode, "", ""};
}

std::string projectAttachmentManifestPath(const urpg::assets::AssetRecord& asset) {
    constexpr std::string_view prefix = "project_asset_attachment:";
    for (const auto& owner : asset.used_by) {
        if (owner.rfind(prefix, 0) == 0) {
            return owner.substr(prefix.size());
        }
    }
    return {};
}

std::string pickerKindForAsset(const urpg::assets::AssetRecord& asset,
                               const urpg::assets::AssetPromotionManifest& manifest) {
    const auto mediaKind = asset.media_kind.empty() ? manifest.preview.kind : asset.media_kind;
    if (mediaKind == "audio") {
        return "audio";
    }
    if (mediaKind == "image") {
        const auto category = asset.category.empty() ? asset.path : asset.category;
        if (category.find("tileset") != std::string::npos || category.find("tile") != std::string::npos) {
            return "tileset";
        }
        if (category.find("ui") != std::string::npos) {
            return "ui";
        }
        if (category.find("background") != std::string::npos) {
            return "background";
        }
        if (category.find("portrait") != std::string::npos || category.find("face") != std::string::npos) {
            return "portrait";
        }
        if (category.find("vfx") != std::string::npos || category.find("effect") != std::string::npos) {
            return "vfx";
        }
        return "sprite";
    }
    return mediaKind.empty() ? "asset" : mediaKind;
}

nlohmann::json pickerTargetsForKind(const std::string& pickerKind) {
    nlohmann::json targets = nlohmann::json::array();
    if (pickerKind == "audio") {
        targets.push_back("audio_selector");
    } else if (pickerKind == "ui") {
        targets.push_back("ui_theme_selector");
    } else if (pickerKind == "tileset" || pickerKind == "background" || pickerKind == "sprite") {
        targets.push_back("level_builder");
        targets.push_back("sprite_selector");
    } else if (pickerKind == "portrait" || pickerKind == "vfx") {
        targets.push_back("sprite_selector");
    }
    return targets;
}

nlohmann::json buildProjectAssetPickerRows(const urpg::assets::AssetLibrarySnapshot& snapshot) {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& asset : snapshot.assets) {
        const auto manifestPath = projectAttachmentManifestPath(asset);
        if (manifestPath.empty()) {
            continue;
        }
        std::ifstream manifestStream(manifestPath);
        if (!manifestStream) {
            continue;
        }
        const auto manifest =
            urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(manifestStream));
        const auto pickerKind = pickerKindForAsset(asset, manifest);
        rows.push_back({
            {"asset_id", asset.asset_id},
            {"source_path", asset.path},
            {"project_path", manifest.promotedPath},
            {"manifest_path", std::filesystem::path(manifestPath).generic_string()},
            {"media_kind", asset.media_kind.empty() ? manifest.preview.kind : asset.media_kind},
            {"category", asset.category},
            {"preview_kind", manifest.preview.kind},
            {"preview_path", manifest.preview.thumbnailPath},
            {"width", manifest.preview.width},
            {"height", manifest.preview.height},
            {"picker_kind", pickerKind},
            {"picker_targets", pickerTargetsForKind(pickerKind)},
        });
    }
    std::sort(rows.begin(), rows.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.value("asset_id", "") < rhs.value("asset_id", "");
    });
    return rows;
}

} // namespace

bool AssetLibraryModel::loadImportSessionManifest(const std::filesystem::path& manifest_path,
                                                  std::string* error_message) {
    if (!std::filesystem::is_regular_file(manifest_path)) {
        if (error_message != nullptr) {
            *error_message = "asset import session manifest is missing";
        }
        snapshot_.status = "error";
        snapshot_.status_message = "Asset import session manifest is missing.";
        snapshot_.error_message = manifest_path.string();
        return false;
    }
    try {
        std::ifstream manifest_stream(manifest_path);
        ingestImportSession(urpg::assets::deserializeAssetImportSession(nlohmann::json::parse(manifest_stream)));
        snapshot_.status = "ready";
        snapshot_.status_message = "";
        snapshot_.error_message = "";
        return true;
    } catch (const std::exception& ex) {
        if (error_message != nullptr) {
            *error_message = ex.what();
        }
        snapshot_.status = "error";
        snapshot_.status_message = "Asset import session manifest could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }
}

bool AssetLibraryModel::loadImportSessionsFromLibraryRoot(const std::filesystem::path& library_root,
                                                          std::string* error_message) {
    try {
        urpg::assets::GlobalAssetLibraryStore store(library_root);
        auto sessions = store.loadImportSessions();
        if (sessions.empty()) {
            if (error_message != nullptr) {
                *error_message = "asset import session manifests are missing";
            }
            refreshSnapshot();
            snapshot_.status_message = snapshot_.reports_loaded ? "" : "No asset import sessions are loaded.";
            snapshot_.error_message = "Missing import session manifests under " + library_root.string();
            return false;
        }

        import_sessions_ = std::move(sessions);
        refreshSnapshot();
        if (error_message != nullptr) {
            error_message->clear();
        }
        return true;
    } catch (const std::exception& ex) {
        if (error_message != nullptr) {
            *error_message = ex.what();
        }
        snapshot_.status = "error";
        snapshot_.status_message = "Asset import session manifests could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }
}

bool AssetLibraryModel::loadPromotedAssetsFromLibraryRoot(const std::filesystem::path& library_root,
                                                          std::string* error_message) {
    try {
        urpg::assets::GlobalAssetLibraryStore store(library_root);
        const auto manifests = store.loadPromotedAssetManifests();
        if (manifests.empty()) {
            if (error_message != nullptr) {
                *error_message = "promoted asset manifests are missing";
            }
            refreshSnapshot();
            return false;
        }
        for (const auto& manifest : manifests) {
            library_.ingestPromotionManifest(manifest);
        }
        rebuildCleanupPreview();
        if (error_message != nullptr) {
            error_message->clear();
        }
        return true;
    } catch (const std::exception& ex) {
        if (error_message != nullptr) {
            *error_message = ex.what();
        }
        snapshot_.status = "error";
        snapshot_.status_message = "Promoted asset manifests could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::promoteImportRecord(std::string session_id,
                                                                              std::string asset_id,
                                                                              std::string license_id,
                                                                              std::string promoted_root,
                                                                              bool include_in_runtime) {
    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(), [&](const auto& candidate) {
        return candidate.sessionId == session_id;
    });
    if (session == import_sessions_.end()) {
        urpg::assets::AssetLibraryActionResult result{
            "promote_import_record", asset_id, false, "import_session_not_found", "Import session was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    auto record = std::find_if(session->records.begin(), session->records.end(), [&](const auto& candidate) {
        return candidate.assetId == asset_id;
    });
    if (record == session->records.end()) {
        urpg::assets::AssetLibraryActionResult result{
            "promote_import_record", asset_id, false, "import_record_not_found", "Import record was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    auto manifest = urpg::assets::planAssetPromotionManifest(
        *session, *record, std::move(license_id), std::move(promoted_root), include_in_runtime);
    library_.ingestPromotionManifest(manifest);
    rebuildCleanupPreview();

    const bool success = manifest.status == urpg::assets::AssetPromotionStatus::RuntimeReady &&
                         manifest.diagnostics.empty();
    urpg::assets::AssetLibraryActionResult result{
        "promote_import_record",
        asset_id,
        success,
        success ? "import_record_promoted" : "import_record_blocked",
        success ? "Import record was promoted into the global asset library."
                : "Import record requires review before promotion."};
    action_history_.push_back(result.toJson());
    refreshSnapshot();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

nlohmann::json AssetLibraryModel::runImportRecordConversion(std::string session_id, std::string asset_id,
                                                            ConversionCommandExecutor executor) {
    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(), [&](const auto& candidate) {
        return candidate.sessionId == session_id;
    });
    if (session == import_sessions_.end()) {
        nlohmann::json action = {
            {"action", "run_import_record_conversion"},
            {"success", false},
            {"code", "import_session_not_found"},
            {"message", "Import session was not found."},
            {"session_id", session_id},
            {"asset_id", asset_id},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    auto record = std::find_if(session->records.begin(), session->records.end(), [&](const auto& candidate) {
        return candidate.assetId == asset_id;
    });
    if (record == session->records.end()) {
        nlohmann::json action = {
            {"action", "run_import_record_conversion"},
            {"success", false},
            {"code", "import_record_not_found"},
            {"message", "Import record was not found."},
            {"session_id", session_id},
            {"asset_id", asset_id},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    if (!record->conversionRequired || record->conversionTargetPath.empty() || record->conversionCommand.empty()) {
        nlohmann::json action = {
            {"action", "run_import_record_conversion"},
            {"success", false},
            {"code", "import_record_conversion_not_required"},
            {"message", "Import record does not have a conversion handoff."},
            {"session_id", session_id},
            {"asset_id", asset_id},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    ConversionCommand command;
    command.working_directory = std::filesystem::path(session->managedSourceRoot);
    command.output_path = command.working_directory / std::filesystem::path(record->conversionTargetPath);
    command.arguments = record->conversionCommand;
    const auto result = executor ? executor(command) : runConversionCommandWithSystem(command);

    std::error_code existsError;
    const bool outputExists = std::filesystem::is_regular_file(command.output_path, existsError);
    if (result.exit_code != 0 || !outputExists) {
        nlohmann::json action = {
            {"action", "run_import_record_conversion"},
            {"success", false},
            {"code", result.exit_code == 0 ? "conversion_output_missing" : "conversion_command_failed"},
            {"message", result.exit_code == 0 ? "Conversion command did not produce the expected output."
                                               : "Conversion command failed."},
            {"session_id", session_id},
            {"asset_id", asset_id},
            {"exit_code", result.exit_code},
            {"stderr", result.stderr_text},
            {"expected_output", command.output_path.generic_string()},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    record->relativePath = std::filesystem::path(record->conversionTargetPath).generic_string();
    record->extension = std::filesystem::path(record->relativePath).extension().generic_string();
    record->sizeBytes = static_cast<uint64_t>(std::filesystem::file_size(command.output_path, existsError));
    record->runtimeReady = true;
    record->conversionRequired = false;
    record->conversionTargetPath.clear();
    record->conversionCommand.clear();
    record->previewAvailable = true;
    record->previewKind = "audio";
    record->noPreviewDiagnostic.clear();
    eraseDiagnostic(record->diagnostics, "conversion_required");
    eraseDiagnostic(record->diagnostics, "source_record_requires_conversion");
    session->summary = urpg::assets::summarizeAssetImportSession(*session);

    nlohmann::json action = {
        {"action", "run_import_record_conversion"},
        {"success", true},
        {"code", "import_record_converted"},
        {"message", "Import record conversion completed."},
        {"session_id", session_id},
        {"asset_id", asset_id},
        {"converted_path", record->relativePath},
        {"output_path", command.output_path.generic_string()},
    };
    action_history_.push_back(action);
    refreshSnapshot();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::runImportRecordConversions(std::string session_id, std::vector<std::string> asset_ids,
                                                             ConversionCommandExecutor executor) {
    nlohmann::json rows = nlohmann::json::array();
    size_t convertedCount = 0;
    size_t failedCount = 0;

    for (const auto& asset_id : asset_ids) {
        const auto row = runImportRecordConversion(session_id, asset_id, executor);
        if (row.value("success", false)) {
            ++convertedCount;
        } else {
            ++failedCount;
        }
        rows.push_back(row);
    }

    const bool success = convertedCount == asset_ids.size() && failedCount == 0;
    nlohmann::json action = {
        {"action", "convert_import_records"},
        {"success", success},
        {"code", success ? "import_records_converted" : "import_records_conversion_partial"},
        {"message", success ? "All selected import records were converted."
                            : "Some selected import records could not be converted."},
        {"session_id", session_id},
        {"selected_count", asset_ids.size()},
        {"converted_count", convertedCount},
        {"failed_count", failedCount},
        {"rows", rows},
    };
    action_history_.push_back(action);
    refreshSnapshot();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::promoteImportRecords(std::string session_id, std::vector<std::string> asset_ids,
                                                       std::string license_id, std::string promoted_root,
                                                       bool include_in_runtime) {
    nlohmann::json rows = nlohmann::json::array();
    size_t promotedCount = 0;
    size_t blockedCount = 0;
    size_t missingCount = 0;

    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(), [&](const auto& candidate) {
        return candidate.sessionId == session_id;
    });
    if (session == import_sessions_.end()) {
        for (const auto& asset_id : asset_ids) {
            rows.push_back({
                {"asset_id", asset_id},
                {"success", false},
                {"code", "import_session_not_found"},
                {"message", "Import session was not found."},
                {"diagnostics", nlohmann::json::array()},
            });
            ++missingCount;
        }
    } else {
        for (const auto& asset_id : asset_ids) {
            auto record = std::find_if(session->records.begin(), session->records.end(), [&](const auto& candidate) {
                return candidate.assetId == asset_id;
            });
            if (record == session->records.end()) {
                rows.push_back({
                    {"asset_id", asset_id},
                    {"success", false},
                    {"code", "import_record_not_found"},
                    {"message", "Import record was not found."},
                    {"diagnostics", nlohmann::json::array()},
                });
                ++missingCount;
                continue;
            }

            auto manifest = urpg::assets::planAssetPromotionManifest(
                *session, *record, license_id, promoted_root, include_in_runtime);
            library_.ingestPromotionManifest(manifest);
            const bool success = manifest.status == urpg::assets::AssetPromotionStatus::RuntimeReady &&
                                 manifest.diagnostics.empty();
            if (success) {
                ++promotedCount;
            } else {
                ++blockedCount;
            }
            rows.push_back({
                {"asset_id", asset_id},
                {"path", manifest.sourcePath},
                {"success", success},
                {"code", success ? "import_record_promoted" : "import_record_blocked"},
                {"message", success ? "Import record was promoted into the global asset library."
                                    : "Import record requires review before promotion."},
                {"promotion_status", urpg::assets::toString(manifest.status)},
                {"promoted_path", manifest.promotedPath},
                {"diagnostics", manifest.diagnostics},
            });
        }
    }

    const bool success = promotedCount == asset_ids.size() && missingCount == 0 && blockedCount == 0;
    nlohmann::json action = {
        {"action", "promote_import_records"},
        {"success", success},
        {"code", success ? "import_records_promoted" : "import_records_partial"},
        {"message", success ? "All selected import records were promoted."
                            : "Some selected import records require review before promotion."},
        {"session_id", session_id},
        {"selected_count", asset_ids.size()},
        {"promoted_count", promotedCount},
        {"blocked_count", blockedCount},
        {"missing_count", missingCount},
        {"rows", rows},
    };
    action_history_.push_back(action);
    rebuildCleanupPreview();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::promoteImportRecordToGlobalLibrary(
    std::string session_id, std::string asset_id, std::string license_id, const std::filesystem::path& promoted_root) {
    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(), [&](const auto& candidate) {
        return candidate.sessionId == session_id;
    });
    if (session == import_sessions_.end()) {
        urpg::assets::AssetLibraryActionResult result{
            "promote_import_record_global", asset_id, false, "import_session_not_found", "Import session was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    auto record = std::find_if(session->records.begin(), session->records.end(), [&](const auto& candidate) {
        return candidate.assetId == asset_id;
    });
    if (record == session->records.end()) {
        urpg::assets::AssetLibraryActionResult result{
            "promote_import_record_global", asset_id, false, "import_record_not_found", "Import record was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    urpg::assets::GlobalAssetPromotionService service;
    const auto promotion = service.promoteImportRecord(*session, *record, std::move(license_id), promoted_root);
    library_.ingestPromotionManifest(promotion.manifest);
    urpg::assets::AssetLibraryActionResult result{
        "promote_import_record_global", asset_id, promotion.success, promotion.code, promotion.message};
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

nlohmann::json AssetLibraryModel::promoteImportRecordsToGlobalLibrary(std::string session_id,
                                                                      std::vector<std::string> asset_ids,
                                                                      std::string license_id,
                                                                      const std::filesystem::path& promoted_root) {
    nlohmann::json rows = nlohmann::json::array();
    size_t promotedCount = 0;
    size_t blockedCount = 0;
    size_t missingCount = 0;

    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(), [&](const auto& candidate) {
        return candidate.sessionId == session_id;
    });
    if (session == import_sessions_.end()) {
        for (const auto& asset_id : asset_ids) {
            rows.push_back({
                {"asset_id", asset_id},
                {"success", false},
                {"code", "import_session_not_found"},
                {"message", "Import session was not found."},
                {"diagnostics", nlohmann::json::array()},
            });
            ++missingCount;
        }
    } else {
        urpg::assets::GlobalAssetPromotionService service;
        for (const auto& asset_id : asset_ids) {
            auto record = std::find_if(session->records.begin(), session->records.end(), [&](const auto& candidate) {
                return candidate.assetId == asset_id;
            });
            if (record == session->records.end()) {
                rows.push_back({
                    {"asset_id", asset_id},
                    {"success", false},
                    {"code", "import_record_not_found"},
                    {"message", "Import record was not found."},
                    {"diagnostics", nlohmann::json::array()},
                });
                ++missingCount;
                continue;
            }

            const auto promotion = service.promoteImportRecord(*session, *record, license_id, promoted_root);
            library_.ingestPromotionManifest(promotion.manifest);
            if (promotion.success) {
                ++promotedCount;
            } else {
                ++blockedCount;
            }
            rows.push_back({
                {"asset_id", asset_id},
                {"path", promotion.manifest.sourcePath},
                {"success", promotion.success},
                {"code", promotion.code},
                {"message", promotion.message},
                {"promotion_status", urpg::assets::toString(promotion.manifest.status)},
                {"promoted_path", promotion.manifest.promotedPath},
                {"payload_path", promotion.payloadPath.empty() ? "" : promotion.payloadPath.generic_string()},
                {"manifest_path", promotion.manifestPath.empty() ? "" : promotion.manifestPath.generic_string()},
                {"diagnostics", promotion.diagnostics},
            });
        }
    }

    const bool success = promotedCount == asset_ids.size() && missingCount == 0 && blockedCount == 0;
    nlohmann::json action = {
        {"action", "promote_import_records_global"},
        {"success", success},
        {"code", success ? "global_import_records_promoted" : "global_import_records_partial"},
        {"message", success ? "All selected import records were copied into the promoted global asset library."
                            : "Some selected import records could not be copied into the promoted global asset library."},
        {"session_id", session_id},
        {"selected_count", asset_ids.size()},
        {"promoted_count", promotedCount},
        {"blocked_count", blockedCount},
        {"missing_count", missingCount},
        {"rows", rows},
    };
    action_history_.push_back(action);
    rebuildCleanupPreview();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

bool AssetLibraryModel::loadReportsFromDirectory(const std::filesystem::path& reports_root,
                                                 std::string* error_message) {
    const auto hygiene_path = reports_root / "asset_hygiene_summary.json";
    const auto duplicates_path = reports_root / "asset_hygiene_duplicates.csv";
    const auto intake_path = reports_root / "asset_intake" / "source_capture_status.json";
    const auto promotion_catalog_path = reports_root / "asset_intake" / "urpg_stuff_promotion_catalog.json";

    if (!std::filesystem::is_regular_file(hygiene_path) || !std::filesystem::is_regular_file(duplicates_path) ||
        !std::filesystem::is_regular_file(intake_path)) {
        clear();
        snapshot_.status = "empty";
        snapshot_.status_message = "Asset library reports are missing.";
        snapshot_.error_message = "Missing required reports under " + reports_root.string();
        if (error_message != nullptr) {
            *error_message = "asset library reports are missing";
        }
        return false;
    }

    try {
        std::ifstream hygiene_stream(hygiene_path);
        std::ifstream intake_stream(intake_path);
        std::ifstream duplicate_stream(duplicates_path);
        std::stringstream duplicate_buffer;
        duplicate_buffer << duplicate_stream.rdbuf();

        const auto hygiene_summary = nlohmann::json::parse(hygiene_stream);
        const auto intake_report = nlohmann::json::parse(intake_stream);
        std::vector<std::filesystem::path> promotion_catalog_paths;
        if (std::filesystem::is_regular_file(promotion_catalog_path)) {
            promotion_catalog_paths.push_back(promotion_catalog_path);
        }
        const auto asset_intake_root = reports_root / "asset_intake";
        if (std::filesystem::is_directory(asset_intake_root)) {
            for (const auto& entry : std::filesystem::directory_iterator(asset_intake_root)) {
                if (!entry.is_regular_file()) {
                    continue;
                }
                const auto path = entry.path();
                const auto filename = path.filename().string();
                if (filename.size() >= std::string("_promotion_catalog.json").size() &&
                    filename.ends_with("_promotion_catalog.json") && path != promotion_catalog_path) {
                    promotion_catalog_paths.push_back(path);
                }
            }
        }
        library_.clear();
        action_history_ = nlohmann::json::array();
        library_.ingestHygieneSummary(hygiene_summary);
        library_.ingestIntakeReport(intake_report);
        for (const auto& path : promotion_catalog_paths) {
            std::ifstream promotion_stream(path);
            ingestCatalogWithShards(library_, reports_root, nlohmann::json::parse(promotion_stream));
        }
        library_.ingestDuplicateCsv(duplicate_buffer.str());
        library_.detectCaseCollisions();
        rebuildCleanupPreview();
        snapshot_.reports_loaded = true;
        snapshot_.status = "ready";
        snapshot_.status_message = "";
        snapshot_.error_message = "";
    } catch (const std::exception& ex) {
        if (error_message != nullptr) {
            *error_message = ex.what();
        }
        clear();
        snapshot_.status = "error";
        snapshot_.status_message = "Asset library reports could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }

    return true;
}

void AssetLibraryModel::addReferencedAsset(std::string path) {
    library_.addReferencedAsset(std::move(path));
    rebuildCleanupPreview();
}

void AssetLibraryModel::addUsageReference(std::string path, std::string owner_id) {
    library_.addUsageReference(std::move(path), std::move(owner_id));
    rebuildCleanupPreview();
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::promoteAsset(std::string path) {
    auto result = library_.promoteAsset(std::move(path));
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::archiveAsset(std::string path, std::string reason) {
    auto result = library_.archiveAsset(std::move(path), std::move(reason));
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::attachPromotedAssetToProject(
    std::string path, const std::filesystem::path& project_root) {
    std::replace(path.begin(), path.end(), '\\', '/');
    const auto found = library_.findAsset(path);
    if (!found.has_value()) {
        urpg::assets::AssetLibraryActionResult result{
            "attach_project_asset", path, false, "asset_not_found", "Asset was not found in the library."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    urpg::assets::ProjectAssetAttachmentService service;
    const auto attachResult = service.attachPromotedAsset(manifestFromAssetRecord(*found), project_root);
    urpg::assets::AssetLibraryActionResult result{
        "attach_project_asset", path, attachResult.success, attachResult.code, attachResult.message};
    if (attachResult.success) {
        library_.addUsageReference(path, "project_asset_attachment:" + attachResult.manifestPath.generic_string());
    }
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

nlohmann::json AssetLibraryModel::attachPromotedAssetsToProject(std::vector<std::string> paths,
                                                                const std::filesystem::path& project_root) {
    nlohmann::json rows = nlohmann::json::array();
    size_t attachedCount = 0;
    size_t blockedCount = 0;
    size_t missingCount = 0;

    urpg::assets::ProjectAssetAttachmentService service;
    for (auto path : paths) {
        std::replace(path.begin(), path.end(), '\\', '/');
        const auto found = library_.findAsset(path);
        if (!found.has_value()) {
            rows.push_back({
                {"path", path},
                {"success", false},
                {"code", "asset_not_found"},
                {"message", "Asset was not found in the library."},
                {"payload_path", ""},
                {"manifest_path", ""},
                {"diagnostics", nlohmann::json::array()},
            });
            ++missingCount;
            continue;
        }

        const auto attachResult = service.attachPromotedAsset(manifestFromAssetRecord(*found), project_root);
        if (attachResult.success) {
            library_.addUsageReference(path, "project_asset_attachment:" + attachResult.manifestPath.generic_string());
            ++attachedCount;
        } else {
            ++blockedCount;
        }
        rows.push_back({
            {"path", path},
            {"asset_id", found->asset_id},
            {"success", attachResult.success},
            {"code", attachResult.code},
            {"message", attachResult.message},
            {"payload_path", attachResult.payloadPath.empty() ? "" : attachResult.payloadPath.generic_string()},
            {"manifest_path", attachResult.manifestPath.empty() ? "" : attachResult.manifestPath.generic_string()},
            {"diagnostics", attachResult.diagnostics},
        });
    }

    const bool success = attachedCount == paths.size() && blockedCount == 0 && missingCount == 0;
    nlohmann::json action = {
        {"action", "attach_project_assets"},
        {"success", success},
        {"code", success ? "project_assets_attached" : "project_assets_partial"},
        {"message", success ? "All selected promoted assets were attached to the project."
                            : "Some selected promoted assets could not be attached to the project."},
        {"selected_count", paths.size()},
        {"attached_count", attachedCount},
        {"blocked_count", blockedCount},
        {"missing_count", missingCount},
        {"rows", rows},
    };
    action_history_.push_back(action);
    rebuildCleanupPreview();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

bool AssetLibraryModel::loadProjectAssetAttachments(const std::filesystem::path& project_root,
                                                    std::string* error_message) {
    const auto manifest_root = project_root / "content" / "assets" / "manifests";
    if (!std::filesystem::is_directory(manifest_root)) {
        if (error_message != nullptr) {
            *error_message = "project asset attachment manifests are missing";
        }
        refreshSnapshot();
        return false;
    }

    try {
        size_t loaded = 0;
        for (const auto& entry : std::filesystem::directory_iterator(manifest_root)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json") {
                continue;
            }
            std::ifstream manifest_stream(entry.path());
            const auto manifest =
                urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(manifest_stream));
            library_.ingestPromotionManifest(manifest);
            const auto attachedPath = findAssetPathById(library_, manifest.assetId);
            if (!attachedPath.empty()) {
                library_.addUsageReference(attachedPath, "project_asset_attachment:" + entry.path().generic_string());
            }
            ++loaded;
        }
        rebuildCleanupPreview();
        if (loaded == 0) {
            if (error_message != nullptr) {
                *error_message = "project asset attachment manifests are missing";
            }
            return false;
        }
        if (error_message != nullptr) {
            error_message->clear();
        }
        return true;
    } catch (const std::exception& ex) {
        if (error_message != nullptr) {
            *error_message = ex.what();
        }
        snapshot_.status = "error";
        snapshot_.status_message = "Project asset attachment manifests could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }
}

void AssetLibraryModel::setFilter(urpg::assets::AssetLibraryFilter filter) {
    filter_ = std::move(filter);
    refreshSnapshot();
}

bool AssetLibraryModel::applyQuickFilter(std::string_view filter_id) {
    urpg::assets::AssetLibraryFilter filter;
    const auto recordResult = [&](bool success, std::string code, std::string message) {
        snapshot_.last_action = {
            {"action", "filter_assets"},
            {"success", success},
            {"code", std::move(code)},
            {"message", std::move(message)},
            {"filter_id", std::string(filter_id)},
        };
        action_history_.push_back(snapshot_.last_action);
        snapshot_.action_history = action_history_;
    };
    if (filter_id == "all_assets") {
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "All asset filters cleared.");
        return true;
    }
    if (filter_id == "sequence_packs") {
        filter.media_kind = "image_sequence_collection";
        filter.runtime_ready_only = true;
        filter.previewable_only = true;
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Sequence pack filter applied.");
        return true;
    }
    if (filter_id == "runtime_ready") {
        filter.runtime_ready_only = true;
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Runtime-ready asset filter applied.");
        return true;
    }
    if (filter_id == "previewable") {
        filter.previewable_only = true;
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Previewable asset filter applied.");
        return true;
    }
    if (filter_id == "attachable") {
        filter.attachable_only = true;
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Attachable asset filter applied.");
        return true;
    }
    if (filter_id == "project_attached") {
        filter.project_attached_only = true;
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Project-attached asset filter applied.");
        return true;
    }
    recordResult(false, "unknown_quick_filter", "Asset quick filter is not registered.");
    return false;
}

void AssetLibraryModel::rebuildCleanupPreview() {
    cleanup_plan_ = cleanup_planner_.buildDuplicateCleanupPlan(library_);
    refreshSnapshot();
}

void AssetLibraryModel::clear() {
    library_.clear();
    cleanup_plan_ = {};
    import_sessions_.clear();
    pending_import_request_ = nlohmann::json::object();
    action_history_ = nlohmann::json::array();
    snapshot_ = {};
    snapshot_.status = "empty";
    snapshot_.reports_loaded = false;
    snapshot_.status_message = "No asset library reports are loaded.";
    snapshot_.remediation = "Run tools/assets/asset_hygiene.py --write-reports to generate asset library reports.";
}

void AssetLibraryModel::refreshSnapshot() {
    const auto& asset_snapshot = library_.snapshot();
    snapshot_.asset_count = asset_snapshot.assets.size();
    snapshot_.catalog_asset_count = asset_snapshot.catalog_asset_count;
    snapshot_.canonical_asset_count = asset_snapshot.canonical_asset_count;
    snapshot_.duplicate_group_count = asset_snapshot.duplicate_groups.size();
    if (snapshot_.duplicate_group_count == 0) {
        snapshot_.duplicate_group_count = asset_snapshot.duplicate_group_count;
    }
    snapshot_.duplicate_asset_count = asset_snapshot.duplicate_asset_count;
    snapshot_.unsupported_count = asset_snapshot.unsupported_count;
    snapshot_.catalog_shard_count = asset_snapshot.catalog_shard_count;
    snapshot_.referenced_asset_count = asset_snapshot.referenced_asset_count;
    snapshot_.runtime_ready_count = asset_snapshot.runtime_ready_count;
    snapshot_.previewable_count = asset_snapshot.previewable_count;
    snapshot_.sequence_asset_count = asset_snapshot.sequence_asset_count;
    snapshot_.sequence_frame_count = asset_snapshot.sequence_frame_count;
    snapshot_.sequence_clip_count = asset_snapshot.sequence_clip_count;
    snapshot_.promoted_count = asset_snapshot.promoted_count;
    snapshot_.archived_count = asset_snapshot.archived_count;
    const auto all_action_rows = urpg::assets::buildAssetActionRows(asset_snapshot);
    snapshot_.project_attached_count = 0;
    snapshot_.project_attachable_count = 0;
    for (const auto& row : all_action_rows) {
        if (row.value("project_attached", false)) {
            ++snapshot_.project_attached_count;
        }
        const auto attach_button = row.find("attach_button");
        if (attach_button != row.end() && attach_button->value("enabled", false)) {
            ++snapshot_.project_attachable_count;
        }
    }
    auto filteredAssets = library_.filterAssets(filter_);
    snapshot_.filtered_asset_count = filteredAssets.size();
    snapshot_.filter_controls = filterControls(filter_,
                                               asset_snapshot,
                                               snapshot_.filtered_asset_count,
                                               snapshot_.project_attached_count,
                                               snapshot_.project_attachable_count);
    snapshot_.cleanup_allowed_count = cleanup_plan_.allowed_count;
    snapshot_.cleanup_refused_count = cleanup_plan_.refused_count;
    snapshot_.export_eligible = asset_snapshot.export_eligible;
    snapshot_.promotion_status = asset_snapshot.promotion_status;
    auto visible_snapshot = asset_snapshot;
    visible_snapshot.assets = std::move(filteredAssets);
    snapshot_.asset_action_rows = urpg::assets::buildAssetActionRows(visible_snapshot);
    snapshot_.asset_preview_rows = urpg::assets::buildAssetPreviewRows(visible_snapshot);
    snapshot_.project_asset_picker_rows = buildProjectAssetPickerRows(asset_snapshot);
    snapshot_.import_session_rows = urpg::assets::buildAssetImportSessionRows(import_sessions_);
    snapshot_.import_review_rows = urpg::assets::buildAssetImportReviewRows(import_sessions_);
    snapshot_.import_session_count = import_sessions_.size();
    snapshot_.import_review_row_count = snapshot_.import_review_rows.size();
    snapshot_.import_ready_count = 0;
    snapshot_.import_needs_conversion_count = 0;
    snapshot_.import_duplicate_count = 0;
    snapshot_.import_missing_license_count = 0;
    snapshot_.import_unsupported_count = 0;
    snapshot_.import_source_only_count = 0;
    snapshot_.import_error_count = 0;
    for (const auto& row : snapshot_.import_review_rows) {
        const auto state = row.value("review_state", "");
        if (state == "ready_to_promote") {
            ++snapshot_.import_ready_count;
        } else if (state == "needs_conversion") {
            ++snapshot_.import_needs_conversion_count;
        } else if (state == "duplicate") {
            ++snapshot_.import_duplicate_count;
        } else if (state == "missing_license") {
            ++snapshot_.import_missing_license_count;
        } else if (state == "unsupported") {
            ++snapshot_.import_unsupported_count;
        } else if (state == "source_only") {
            ++snapshot_.import_source_only_count;
        } else if (state == "error") {
            ++snapshot_.import_error_count;
        }
    }
    snapshot_.import_wizard = buildImportWizardSnapshot(snapshot_, pending_import_request_);
    snapshot_.action_history = action_history_;
    if (!action_history_.empty()) {
        snapshot_.last_action = action_history_.back();
    }
    snapshot_.category_counts = asset_snapshot.category_counts;
    snapshot_.kind_counts = asset_snapshot.kind_counts;
    snapshot_.reports_loaded = asset_snapshot.assets.size() > 0 || asset_snapshot.duplicate_groups.size() > 0 ||
                               asset_snapshot.catalog_asset_count > 0 || asset_snapshot.catalog_shard_count > 0 ||
                               cleanup_plan_.allowed_count > 0 || cleanup_plan_.refused_count > 0 ||
                               !import_sessions_.empty();
    snapshot_.status = snapshot_.reports_loaded ? "ready" : "empty";
    snapshot_.status_message = snapshot_.reports_loaded ? "" : "No asset library reports are loaded.";
    if (snapshot_.reports_loaded) {
        snapshot_.error_message = "";
    }
    snapshot_.issue_count = 0;
    for (const auto& asset : asset_snapshot.assets) {
        if (!(asset.statuses.size() == 1 && asset.statuses.contains(urpg::assets::AssetStatus::Usable))) {
            ++snapshot_.issue_count;
        }
    }
}

} // namespace urpg::editor
