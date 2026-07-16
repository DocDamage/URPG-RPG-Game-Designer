#include "editor/assets/asset_library_model.h"

#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/global_asset_library_store.h"
#include "engine/core/assets/global_asset_promotion_service.h"
#include "engine/core/assets/project_asset_attachment_service.h"
#include "engine/core/platform/process_runner.h"
#include "engine/core/security/sha256.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <system_error>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::editor {

namespace {

constexpr const char* kExternalExtractorEnv = "URPG_ASSET_ARCHIVE_EXTRACTOR";

std::string curationKeyForRecord(const urpg::assets::AssetRecord& asset) {
    if (!asset.asset_id.empty()) {
        return "promoted:" + asset.asset_id;
    }
    const auto identity = !asset.normalized_path.empty() ? asset.normalized_path : asset.path;
    return "catalog:" + urpg::security::Sha256::toHex(
                            urpg::security::Sha256::compute({identity.begin(), identity.end()}));
}

struct ParsedExternalExtractorCommand {
    std::vector<std::string> arguments;
    bool valid = true;
    std::string diagnostic;
};

ParsedExternalExtractorCommand splitConfiguredCommand(std::string_view command) {
    std::vector<std::string> parts;
    std::string current;
    char quote = '\0';
    bool escaping = false;
    for (const char ch : command) {
        if (escaping) {
            current.push_back(ch);
            escaping = false;
            continue;
        }
        if (ch == '\\' && quote == '"') {
            escaping = true;
            continue;
        }
        if ((ch == '"' || ch == '\'') && quote == '\0') {
            quote = ch;
            continue;
        }
        if (ch == quote) {
            quote = '\0';
            continue;
        }
        if (std::isspace(static_cast<unsigned char>(ch)) && quote == '\0') {
            if (!current.empty()) {
                parts.push_back(std::move(current));
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty()) {
        parts.push_back(std::move(current));
    }
    if (quote != '\0' || escaping) {
        return {{}, false, "external_extractor_command_parse_error"};
    }
    return {std::move(parts), true, ""};
}

std::vector<std::string> configuredExternalExtractorCommand(std::vector<std::string> explicitCommand) {
    if (!explicitCommand.empty()) {
        return explicitCommand;
    }
    const char* configured = std::getenv(kExternalExtractorEnv);
    if (configured == nullptr || std::string_view(configured).empty()) {
        return {};
    }
    const auto parsed = splitConfiguredCommand(configured);
    return parsed.valid ? parsed.arguments : std::vector<std::string>{};
}

std::filesystem::path catalogInterchangeToolPath() {
    std::error_code error;
    auto directory = std::filesystem::current_path(error);
    if (error) {
        return {};
    }
    while (!directory.empty()) {
        const auto candidate = directory / "tools" / "assets" / "catalog_interchange.py";
        if (std::filesystem::is_regular_file(candidate, error) && !error) {
            return candidate;
        }
        error.clear();
        const auto parent = directory.parent_path();
        if (parent == directory) {
            break;
        }
        directory = parent;
    }
    return {};
}

nlohmann::json externalExtractorConfigurationSnapshot() {
    const char* configured = std::getenv(kExternalExtractorEnv);
    if (configured == nullptr || std::string_view(configured).empty()) {
        return {
            {"configured", false},
            {"source", "none"},
            {"environment_variable", kExternalExtractorEnv},
            {"supports_rar_7z", false},
            {"supports_selected_entry_staging", false},
            {"command", nlohmann::json::array()},
            {"diagnostics", nlohmann::json::array()},
        };
    }
    const auto parsed = splitConfiguredCommand(configured);
    if (!parsed.valid) {
        return {
            {"configured", false},
            {"source", "environment"},
            {"environment_variable", kExternalExtractorEnv},
            {"supports_rar_7z", false},
            {"supports_selected_entry_staging", false},
            {"command", nlohmann::json::array()},
            {"diagnostics", nlohmann::json::array({parsed.diagnostic})},
        };
    }
    const auto hasArgument = [&](std::string_view argument) {
        return std::any_of(parsed.arguments.begin(), parsed.arguments.end(), [&](const auto& value) {
            return value.find(argument) != std::string::npos;
        });
    };
    const bool supportsSelectedEntryStaging = hasArgument("{source}") && hasArgument("{destination}") &&
                                             std::find(parsed.arguments.begin(), parsed.arguments.end(), "{selected_entries}") !=
                                                 parsed.arguments.end();
    return {
        {"configured", true},      {"source", "environment"},     {"environment_variable", kExternalExtractorEnv},
        {"supports_rar_7z", true}, {"supports_selected_entry_staging", supportsSelectedEntryStaging},
        {"command", parsed.arguments}, {"diagnostics", nlohmann::json::array()},
    };
}

bool atomicWriteJson(const std::filesystem::path& path, const nlohmann::json& value, std::string* error) {
    std::error_code filesystemError;
    std::filesystem::create_directories(path.parent_path(), filesystemError);
    if (filesystemError) {
        if (error) *error = filesystemError.message();
        return false;
    }
    const auto temporary = path.parent_path() / ("." + path.filename().string() + ".tmp");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output << value.dump(2) << '\n';
        if (!output) {
            if (error) *error = "Unable to write the temporary import-session manifest.";
            std::filesystem::remove(temporary, filesystemError);
            return false;
        }
    }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        if (error) *error = "Unable to atomically publish the import-session manifest.";
        std::filesystem::remove(temporary, filesystemError);
        return false;
    }
#else
    std::filesystem::rename(temporary, path, filesystemError);
    if (filesystemError) {
        if (error) *error = "Unable to atomically publish the import-session manifest: " + filesystemError.message();
        std::filesystem::remove(temporary, filesystemError);
        return false;
    }
#endif
    return true;
}

} // namespace

AssetLibraryModel::AssetLibraryModel() {
    refreshSnapshot();
}

void AssetLibraryModel::setImportToolCommand(std::vector<std::string> command_prefix) {
    if (command_prefix.size() >= 2) {
        import_tool_command_ = std::move(command_prefix);
    }
    refreshSnapshot();
}

void AssetLibraryModel::setDuplicateCsvDetailLimitBytes(std::uintmax_t limit_bytes) {
    duplicate_csv_detail_limit_bytes_ = limit_bytes;
}

void AssetLibraryModel::setPromotionCatalogDetailLimitBytes(std::uintmax_t limit_bytes) {
    promotion_catalog_detail_limit_bytes_ = limit_bytes;
}

bool AssetLibraryModel::loadExternalCatalog(const std::filesystem::path& catalog_directory, std::string* error_message) {
    const auto result = external_catalog_.load(catalog_directory);
    external_catalog_diagnostics_ = result.diagnostics;
    if (!result.success) {
        if (error_message) {
            std::ostringstream message;
            for (size_t index = 0; index < result.diagnostics.size(); ++index) {
                if (index > 0) {
                    message << '\n';
                }
                message << result.diagnostics[index];
            }
            *error_message = message.str();
        }
        refreshSnapshot();
        return false;
    }
    external_catalog_directory_ = catalog_directory;
    if (error_message) {
        error_message->clear();
    }
    refreshSnapshot();
    return true;
}

void AssetLibraryModel::setExternalCatalogQuery(urpg::assets::LocalAssetCatalogQuery query) {
    external_catalog_query_ = std::move(query);
    refreshSnapshot();
}

void AssetLibraryModel::selectExternalCatalogAsset(std::string asset_id) {
    selected_external_catalog_asset_id_ = std::move(asset_id);
    refreshSnapshot();
}

nlohmann::json AssetLibraryModel::refreshExternalCatalog(ConversionCommandExecutor executor) {
    const auto database = external_catalog_directory_ / "asset_catalog.db";
    const auto tool = catalogInterchangeToolPath();
    if (external_catalog_directory_.empty() || !std::filesystem::is_regular_file(database) ||
        !std::filesystem::is_regular_file(tool)) {
        nlohmann::json action = {
            {"action", "refresh_external_catalog"},
            {"success", false},
            {"code", "external_catalog_refresh_unavailable"},
            {"message", "Refresh requires the local index database and catalog interchange tool."},
            {"database", database.generic_string()},
            {"tool", tool.generic_string()},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    ConversionCommand command;
    command.working_directory = std::filesystem::current_path();
    command.arguments = {"python", tool.generic_string(), "--db", database.generic_string(), "--output",
                         external_catalog_directory_.generic_string()};
    const auto result = executor ? executor(command) : runConversionCommand(command);
    if (result.exit_code != 0) {
        nlohmann::json action = {
            {"action", "refresh_external_catalog"},
            {"success", false},
            {"code", "external_catalog_refresh_failed"},
            {"message", "The local asset catalog could not be refreshed."},
            {"exit_code", result.exit_code},
            {"stdout", result.stdout_text},
            {"stderr", result.stderr_text},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    std::string error;
    if (!loadExternalCatalog(external_catalog_directory_, &error)) {
        nlohmann::json action = {
            {"action", "refresh_external_catalog"},
            {"success", false},
            {"code", "external_catalog_refresh_reload_failed"},
            {"message", "The catalog export completed but the refreshed interchange could not be loaded."},
            {"error", error},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    nlohmann::json action = {
        {"action", "refresh_external_catalog"},
        {"success", true},
        {"code", "external_catalog_refreshed"},
        {"message", "The local asset catalog was refreshed."},
        {"stdout", result.stdout_text},
        {"stderr", result.stderr_text},
    };
    action_history_.push_back(action);
    refreshSnapshot();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::openSelectedExternalCatalogSource(ConversionCommandExecutor executor) {
    const auto page = external_catalog_.query(external_catalog_query_);
    const auto selected = std::find_if(page.records.begin(), page.records.end(), [&](const auto& record) {
        return record.assetId == selected_external_catalog_asset_id_;
    });
    if (selected == page.records.end()) {
        nlohmann::json action = {{"action", "open_external_catalog_source"},
                                 {"success", false},
                                 {"code", "external_catalog_source_not_selected"},
                                 {"message", "Select a visible external catalog record before opening its source location."}};
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    std::error_code error;
    const auto root = std::filesystem::weakly_canonical(selected->sourceRoot, error);
    if (error || root.empty() || !root.is_absolute()) {
        nlohmann::json action = {{"action", "open_external_catalog_source"},
                                 {"success", false},
                                 {"code", "external_catalog_source_root_unavailable"},
                                 {"message", "The selected record does not provide an accessible absolute source root."}};
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }
    const auto candidate = std::filesystem::weakly_canonical(root / selected->virtualPath, error);
    bool insideRoot = !error;
    auto rootIt = root.begin();
    auto candidateIt = candidate.begin();
    while (insideRoot && rootIt != root.end()) {
        if (candidateIt == candidate.end() || *rootIt != *candidateIt) {
            insideRoot = false;
            break;
        }
        ++rootIt;
        ++candidateIt;
    }
    if (!insideRoot || !std::filesystem::exists(candidate, error) || error) {
        nlohmann::json action = {{"action", "open_external_catalog_source"},
                                 {"success", false},
                                 {"code", "external_catalog_source_path_unavailable"},
                                 {"message", "The selected external source path is missing or escaped its configured root."}};
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    ConversionCommand command;
#ifdef _WIN32
    command.arguments = {"explorer.exe", "/select," + candidate.string()};
#elif defined(__APPLE__)
    command.arguments = {"open", candidate.string()};
#elif defined(__linux__)
    command.arguments = {"xdg-open", candidate.parent_path().string()};
#else
    command.arguments = {};
#endif
    if (command.arguments.empty()) {
        nlohmann::json action = {{"action", "open_external_catalog_source"},
                                 {"success", false},
                                 {"code", "external_catalog_source_open_unsupported"},
                                 {"message", "Opening source locations is not supported on this platform."}};
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }
    const auto result = executor ? executor(command) : runConversionCommand(command);
    nlohmann::json action = {{"action", "open_external_catalog_source"},
                             {"success", result.exit_code == 0},
                             {"code", result.exit_code == 0 ? "external_catalog_source_opened" : "external_catalog_source_open_failed"},
                             {"message", result.exit_code == 0 ? "Opened the selected external source location."
                                                                 : "The selected external source location could not be opened."},
                             {"source_path", candidate.generic_string()},
                             {"stderr", result.stderr_text}};
    action_history_.push_back(action);
    refreshSnapshot();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::browseArchive(const std::filesystem::path& archive_path) {
    std::error_code filesystemError;
    const auto normalizedPath = std::filesystem::weakly_canonical(archive_path, filesystemError);
    const auto cachePath = filesystemError ? archive_path.lexically_normal() : normalizedPath;
    filesystemError.clear();
    const auto discoveredSize = std::filesystem::file_size(cachePath, filesystemError);
    const bool hasSize = !filesystemError;
    const auto archiveSize = hasSize ? discoveredSize : uintmax_t{0};
    filesystemError.clear();
    const auto archiveWriteTime = std::filesystem::last_write_time(cachePath, filesystemError);
    const bool hasWriteTime = !filesystemError;
    const bool cacheable = hasSize && hasWriteTime;
    if (cacheable && cachePath == cached_archive_path_ && archiveSize == cached_archive_size_ &&
        archiveWriteTime == cached_archive_write_time_ && !archive_browser_.empty()) {
        archive_browser_["cached"] = true;
        refreshSnapshot();
        return archive_browser_;
    }
    const auto result = urpg::assets::ArchiveCatalog{}.list(archive_path, {}, configuredExternalExtractorCommand({}));
    nlohmann::json entries = nlohmann::json::array();
    constexpr size_t maxRows = 200;
    for (size_t index = 0; index < result.entries.size() && index < maxRows; ++index) {
        const auto& entry = result.entries[index];
        entries.push_back({{"path", entry.path},
                           {"compressed_bytes", entry.compressedBytes},
                           {"expanded_bytes", entry.expandedBytes},
                           {"directory", entry.directory}});
    }
    archive_browser_ = {
        {"archive_path", archive_path.generic_string()},
        {"status", result.success ? "listed" : "blocked"},
        {"code", result.code},
        {"message", result.message},
        {"entry_count", result.entries.size()},
        {"entries", std::move(entries)},
        {"entries_truncated", result.entries.size() > maxRows},
        {"diagnostics", result.diagnostics},
        {"cached", false},
        {"cache_key", {{"size_bytes", archiveSize}, {"modified_time", archiveWriteTime.time_since_epoch().count()}}},
        {"promotion_eligible", false},
        {"release_eligible", false},
        {"next_action", result.success ? "Select entries for isolated staging before import." : "Resolve archive safety diagnostics."},
    };
    if (cacheable) {
        cached_archive_path_ = cachePath;
        cached_archive_size_ = archiveSize;
        cached_archive_write_time_ = archiveWriteTime;
    }
    refreshSnapshot();
    return archive_browser_;
}

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

std::string shellQuoteArgument(const std::string& value) {
    if (value.find_first_of(" \t\"'") == std::string::npos) {
        return value;
    }
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

std::string joinExternalExtractorCommand(const std::vector<std::string>& command) {
    std::ostringstream joined;
    bool first = true;
    for (const auto& part : command) {
        if (!first) {
            joined << ' ';
        }
        joined << shellQuoteArgument(part);
        first = false;
    }
    return joined.str();
}

nlohmann::json AssetLibraryModel::requestImportSource(const std::filesystem::path& source,
                                                      const std::filesystem::path& library_root, std::string session_id,
                                                      std::string license_note,
                                                      std::vector<std::string> external_extractor_command,
                                                      std::vector<std::string> selected_archive_entries) {
    external_extractor_command = configuredExternalExtractorCommand(std::move(external_extractor_command));
    const auto sourcePath = source.generic_string();
    const auto libraryRoot = library_root.generic_string();
    const auto expectedManifest =
        (library_root / "catalog" / "import_sessions" / (session_id + ".json")).generic_string();
    nlohmann::json command = nlohmann::json::array();
    for (const auto& part : import_tool_command_) {
        command.push_back(part);
    }
    command.push_back("--source");
    command.push_back(sourcePath);
    command.push_back("--library-root");
    command.push_back(libraryRoot);
    command.push_back("--session-id");
    command.push_back(session_id);
    if (!license_note.empty()) {
        command.push_back("--license-note");
        command.push_back(license_note);
    }
    if (!external_extractor_command.empty()) {
        command.push_back("--external-extractor-command");
        command.push_back(joinExternalExtractorCommand(external_extractor_command));
    }
    for (const auto& entry : selected_archive_entries) {
        if (!entry.empty()) {
            command.push_back("--selected-archive-entry");
            command.push_back(entry);
        }
    }

    pending_import_request_ = {
        {"source_path", sourcePath},
        {"library_root", libraryRoot},
        {"session_id", session_id},
        {"license_note", license_note},
        {"external_extractor_command", external_extractor_command},
        {"selected_archive_entries", selected_archive_entries},
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
        {"external_extractor_command", external_extractor_command},
        {"selected_archive_entries", selected_archive_entries},
        {"expected_manifest_path", expectedManifest},
        {"command", command},
    };
    action_history_.push_back(action);
    refreshSnapshot();
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::executePendingImportRequest(ConversionCommandExecutor executor) {
    const auto request = pending_import_request_;
    const auto commandIt = request.find("command");
    if (!request.is_object() || request.empty() || commandIt == request.end() || !commandIt->is_array() ||
        commandIt->empty() || !(*commandIt)[0].is_string()) {
        nlohmann::json action = {
            {"action", "execute_import_source"},
            {"success", false},
            {"code", "import_source_request_missing"},
            {"message", "Choose an asset source before running the importer."},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    ConversionCommand importCommand;
    for (const auto& part : *commandIt) {
        if (!part.is_string()) {
            nlohmann::json action = {
                {"action", "execute_import_source"},
                {"success", false},
                {"code", "import_source_request_invalid"},
                {"message", "The requested importer command is invalid."},
            };
            action_history_.push_back(action);
            refreshSnapshot();
            snapshot_.last_action = action;
            snapshot_.action_history = action_history_;
            return action;
        }
        importCommand.arguments.push_back(part.get<std::string>());
    }

    const auto processResult = executor ? executor(importCommand) : runConversionCommand(importCommand);
    const auto expectedManifest = std::filesystem::path(request.value("expected_manifest_path", ""));
    if (processResult.exit_code != 0) {
        nlohmann::json action = {
            {"action", "execute_import_source"},
            {"success", false},
            {"code", "import_source_command_failed"},
            {"message", "The asset importer did not complete."},
            {"exit_code", processResult.exit_code},
            {"stdout", processResult.stdout_text},
            {"stderr", processResult.stderr_text},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    std::string loadError;
    if (expectedManifest.empty() || !loadImportSessionManifest(expectedManifest, &loadError)) {
        nlohmann::json action = {
            {"action", "execute_import_source"},
            {"success", false},
            {"code", "import_source_manifest_missing"},
            {"message", "The importer completed but did not produce a readable review manifest."},
            {"expected_manifest_path", expectedManifest.generic_string()},
            {"stdout", processResult.stdout_text},
            {"stderr", processResult.stderr_text},
            {"error", loadError},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    }

    nlohmann::json action = {
        {"action", "execute_import_source"},
        {"success", true},
        {"code", "import_source_loaded_for_review"},
        {"message", "Asset source was scanned and is ready for review."},
        {"session_id", request.value("session_id", "")},
        {"expected_manifest_path", expectedManifest.generic_string()},
        {"stdout", processResult.stdout_text},
        {"stderr", processResult.stderr_text},
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
    auto found = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                              [&](const auto& existing) { return existing.sessionId == session.sessionId; });
    if (found == import_sessions_.end()) {
        import_sessions_.push_back(std::move(session));
    } else {
        *found = std::move(session);
    }
    refreshSnapshot();
}

void AssetLibraryModel::clearImportSessions() {
    import_sessions_.clear();
    import_session_manifest_paths_.clear();
    pending_import_request_ = nlohmann::json::object();
    refreshSnapshot();
}

bool AssetLibraryModel::persistImportSession(const urpg::assets::AssetImportSession& session, std::string* error_message) {
    const auto path = import_session_manifest_paths_.find(session.sessionId);
    if (path == import_session_manifest_paths_.end() || path->second.empty()) {
        if (error_message) *error_message = "No governed import-session manifest is available for this session.";
        return false;
    }
    return atomicWriteJson(path->second, urpg::assets::serializeAssetImportSession(session), error_message);
}

namespace {

std::filesystem::path companionPromotionSummaryPath(const std::filesystem::path& catalog_path) {
    const std::string suffix = "_promotion_catalog.json";
    const std::string filename = catalog_path.filename().string();
    if (filename.size() < suffix.size() || !filename.ends_with(suffix)) {
        return {};
    }
    return catalog_path.parent_path() /
           (filename.substr(0, filename.size() - suffix.size()) + "_promotion_summary.json");
}

nlohmann::json summaryCatalogFromSummaryFile(const std::filesystem::path& summary_path) {
    std::ifstream summary_stream(summary_path);
    const auto summary = nlohmann::json::parse(summary_stream);
    return {
        {"source_id", summary.value("source_id", "")},
        {"source_root", summary.value("source_root", "")},
        {"promotion_status", summary.value("promotion_status", "")},
        {"export_eligible", summary.value("export_eligible", false)},
        {"summary", summary},
    };
}

size_t ingestCatalogWithShards(urpg::assets::AssetLibrary& library,
                               const std::filesystem::path& reports_root,
                               const nlohmann::json& promotion_catalog,
                               std::uintmax_t shard_detail_limit_bytes) {
    library.ingestPromotionCatalog(promotion_catalog);
    const auto shards = promotion_catalog.find("shards");
    if (shards == promotion_catalog.end() || !shards->is_array()) {
        return 0;
    }
    const auto repo_root = reports_root.parent_path();
    size_t skipped_shards = 0;
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
        std::error_code shard_size_error;
        const auto shard_size = std::filesystem::file_size(shard_path, shard_size_error);
        if (!shard_size_error && shard_size > shard_detail_limit_bytes) {
            ++skipped_shards;
            continue;
        }
        std::ifstream shard_stream(shard_path);
        library.ingestPromotionCatalog(nlohmann::json::parse(shard_stream));
    }
    return skipped_shards;
}

nlohmann::json filterControls(const urpg::assets::AssetLibraryFilter& filter,
                              const urpg::assets::AssetLibrarySnapshot& snapshot, size_t filteredCount,
                              size_t projectAttachedCount, size_t projectAttachableCount) {
    return {
        {"active_filter",
         {
             {"media_kind", filter.media_kind},
             {"category", filter.category},
             {"game_use_category", filter.game_use_category},
             {"required_tag", filter.required_tag},
             {"required_game_use_tag", filter.required_game_use_tag},
             {"source_bundle_id", filter.source_bundle_id},
             {"required_status", filter.required_status.has_value()
                                     ? nlohmann::json(urpg::assets::toString(*filter.required_status))
                                     : nlohmann::json(nullptr)},
             {"referenced_only", filter.referenced_only},
             {"runtime_ready_only", filter.runtime_ready_only},
             {"previewable_only", filter.previewable_only},
             {"project_attached_only", filter.project_attached_only},
             {"attachable_only", filter.attachable_only},
             {"release_eligible_only", filter.release_eligible_only},
             {"result_count", filteredCount},
         }},
        {"facets",
         {
             {"game_use_categories", snapshot.game_use_category_counts},
             {"game_use_tags", snapshot.game_use_tag_counts},
             {"source_bundles", snapshot.source_bundle_counts},
             {"source_categories", snapshot.category_counts},
             {"media_kinds", snapshot.kind_counts},
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
             {"release_eligible",
              {
                  {"visible", true},
                  {"enabled", snapshot.game_use_tag_counts.contains("release:eligible")},
                  {"action", "filter_release_eligible_assets"},
                  {"count", snapshot.game_use_tag_counts.contains("release:eligible")
                                ? snapshot.game_use_tag_counts.at("release:eligible")
                                : 0},
              }},
             {"characters",
              {
                  {"visible", true},
                  {"enabled", snapshot.game_use_category_counts.contains("characters/sprites")},
                  {"action", "filter_character_assets"},
                  {"game_use_category", "characters/sprites"},
                  {"count", snapshot.game_use_category_counts.contains("characters/sprites")
                                ? snapshot.game_use_category_counts.at("characters/sprites")
                                : 0},
              }},
             {"tilesets",
              {
                  {"visible", true},
                  {"enabled", snapshot.game_use_category_counts.contains("environment/tiles/top_down") ||
                                  snapshot.game_use_category_counts.contains("environment/tiles/isometric")},
                  {"action", "filter_tileset_assets"},
                  {"required_game_use_tag", "asset_type:tileset"},
                  {"count", (snapshot.game_use_category_counts.contains("environment/tiles/top_down")
                                 ? snapshot.game_use_category_counts.at("environment/tiles/top_down")
                                 : 0) +
                                (snapshot.game_use_category_counts.contains("environment/tiles/isometric")
                                     ? snapshot.game_use_category_counts.at("environment/tiles/isometric")
                                     : 0)},
              }},
             {"ui_assets",
              {
                  {"visible", true},
                  {"enabled", snapshot.game_use_category_counts.contains("ui/widgets")},
                  {"action", "filter_ui_assets"},
                  {"game_use_category", "ui/widgets"},
                  {"count", snapshot.game_use_category_counts.contains("ui/widgets")
                                ? snapshot.game_use_category_counts.at("ui/widgets")
                                : 0},
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

    const auto reviewState = currentStep == "review"
                                 ? "active"
                                 : (hasReviewRows || hasPromotedAssets || hasAttachedAssets ? "complete" : "pending");
    const auto promoteState = currentStep == "review" && hasPromotableRows
                                  ? "available"
                                  : (hasPromotedAssets || hasAttachedAssets ? "complete" : "pending");
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
                  {"disabled_reason",
                   hasPromotableRows ? nlohmann::json(nullptr) : nlohmann::json("no_promotable_import_records")},
              }},
             {"convert_selected",
              {
                  {"enabled", hasConvertibleRows},
                  {"action", "asset_library_convert_selected"},
                  {"eligible_count", snapshot.import_needs_conversion_count},
                  {"disabled_reason",
                   hasConvertibleRows ? nlohmann::json(nullptr) : nlohmann::json("no_convertible_import_records")},
              }},
             {"attach_selected",
              {
                  {"enabled", hasPromotedAssets},
                  {"action", "asset_library_attach_selected"},
                  {"eligible_count", snapshot.project_attachable_count},
                  {"disabled_reason",
                   hasPromotedAssets ? nlohmann::json(nullptr) : nlohmann::json("no_promoted_project_ready_assets")},
              }},
             {"package_validate",
              {
                  {"enabled", hasAttachedAssets},
                  {"action", "asset_library_package_validate"},
                  {"eligible_count", snapshot.project_attached_count},
                  {"disabled_reason",
                   hasAttachedAssets ? nlohmann::json(nullptr) : nlohmann::json("no_attached_project_assets")},
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
        {"extractor_configuration", externalExtractorConfigurationSnapshot()},
        {"pending_request", hasPendingRequest ? pendingImportRequest : nlohmann::json(nullptr)},
    };
}

urpg::assets::AssetPromotionManifest manifestFromAssetRecord(const urpg::assets::AssetRecord& record) {
    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = record.asset_id;
    manifest.sourcePath = record.source_path.empty() ? record.path : record.source_path;
    manifest.sourceSha256 = record.sha256;
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

std::string projectAttachmentManifestPath(const urpg::assets::AssetRecord& asset) {
    constexpr std::string_view prefix = "project_asset_attachment:";
    for (const auto& owner : asset.used_by) {
        if (owner.rfind(prefix, 0) == 0) {
            return owner.substr(prefix.size());
        }
    }
    return {};
}

std::string attachmentConflictPolicyName(const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    switch (policy) {
    case urpg::assets::ProjectAssetAttachmentConflictPolicy::Cancel:
        return "cancel";
    case urpg::assets::ProjectAssetAttachmentConflictPolicy::Replace:
        return "replace";
    case urpg::assets::ProjectAssetAttachmentConflictPolicy::KeepBoth:
        return "keep_both";
    case urpg::assets::ProjectAssetAttachmentConflictPolicy::RelinkExisting:
        return "relink_existing";
    }
    return "unknown";
}

std::string attachmentOperationId(std::string asset_id, const std::string& source_revision,
                                  const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    for (auto& ch : asset_id) {
        if (!std::isalnum(static_cast<unsigned char>(ch)) && ch != '-' && ch != '_') {
            ch = '-';
        }
    }
    const auto revision_prefix = source_revision.substr(0, std::min<size_t>(16, source_revision.size()));
    return "asset-attach-" + asset_id + "-" + attachmentConflictPolicyName(policy) + "-" + revision_prefix;
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
        targets.push_back("spatial_authoring");
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
        const auto manifest = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(manifestStream));
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
    std::sort(rows.begin(), rows.end(),
              [](const auto& lhs, const auto& rhs) { return lhs.value("asset_id", "") < rhs.value("asset_id", ""); });
    return rows;
}

nlohmann::json buildVirtualCatalogSnapshot(const urpg::assets::AssetLibrarySnapshot& snapshot) {
    return {
        {"schema", "urpg.asset_virtual_catalog.v1"},
        {"description", "Production-facing virtual browse facets derived from governed asset metadata."},
        {"preserves_physical_layout", true},
        {"source_manifest_layout", "imports/manifests/asset_bundles"},
        {"normalized_payload_layout", "imports/normalized"},
        {"game_use_categories", snapshot.game_use_category_counts},
        {"game_use_tags", snapshot.game_use_tag_counts},
        {"source_bundles", snapshot.source_bundle_counts},
        {"source_categories", snapshot.category_counts},
        {"media_kinds", snapshot.kind_counts},
    };
}

} // namespace

AssetLibraryModel::ConversionCommandResult AssetLibraryModel::runConversionCommand(const ConversionCommand& command) {
    if (command.arguments.empty()) {
        return {1, "", "conversion command is empty"};
    }

    urpg::platform::ProcessCommand processCommand;
    processCommand.executable = command.arguments.front();
    processCommand.arguments.assign(command.arguments.begin() + 1, command.arguments.end());
    processCommand.workingDirectory = command.working_directory;
    const auto result = urpg::platform::runProcess(processCommand);
    return {result.exitCode, result.stdoutText,
            result.error.empty() ? result.stderrText : result.stderrText + result.error};
}

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
        auto session = urpg::assets::deserializeAssetImportSession(nlohmann::json::parse(manifest_stream));
        const auto sessionId = session.sessionId;
        ingestImportSession(std::move(session));
        if (!sessionId.empty()) import_session_manifest_paths_[sessionId] = manifest_path;
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
        import_session_manifest_paths_.clear();
        for (const auto& session : import_sessions_) {
            if (!session.sessionId.empty()) {
                import_session_manifest_paths_[session.sessionId] =
                    library_root / "catalog" / "import_sessions" / (session.sessionId + ".json");
            }
        }
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

urpg::assets::AssetLibraryActionResult
AssetLibraryModel::promoteImportRecord(std::string session_id, std::string asset_id, std::string license_id,
                                       std::string promoted_root, bool include_in_runtime) {
    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                                [&](const auto& candidate) { return candidate.sessionId == session_id; });
    if (session == import_sessions_.end()) {
        urpg::assets::AssetLibraryActionResult result{"promote_import_record", asset_id, false,
                                                      "import_session_not_found", "Import session was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    auto record = std::find_if(session->records.begin(), session->records.end(),
                               [&](const auto& candidate) { return candidate.assetId == asset_id; });
    if (record == session->records.end()) {
        urpg::assets::AssetLibraryActionResult result{"promote_import_record", asset_id, false,
                                                      "import_record_not_found", "Import record was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    auto manifest = urpg::assets::planAssetPromotionManifest(*session, *record, std::move(license_id),
                                                             std::move(promoted_root), include_in_runtime);
    library_.ingestPromotionManifest(manifest);
    rebuildCleanupPreview();

    const bool success =
        manifest.status == urpg::assets::AssetPromotionStatus::RuntimeReady && manifest.diagnostics.empty();
    urpg::assets::AssetLibraryActionResult result{"promote_import_record", asset_id, success,
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
    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                                [&](const auto& candidate) { return candidate.sessionId == session_id; });
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

    auto record = std::find_if(session->records.begin(), session->records.end(),
                               [&](const auto& candidate) { return candidate.assetId == asset_id; });
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
    const auto result = executor ? executor(command) : AssetLibraryModel::runConversionCommand(command);

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
    const auto extension = record->extension;
    const bool imageOutput = extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
                             extension == ".bmp" || extension == ".gif";
    record->mediaKind = imageOutput ? "image" : "audio";
    record->sourceOnly = false;
    record->previewAvailable = true;
    record->previewKind = imageOutput ? "image" : "audio";
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

nlohmann::json AssetLibraryModel::setImportRecordSpriteSheetSlice(std::string session_id, std::string asset_id,
                                                                   const int32_t frame_width, const int32_t frame_height,
                                                                   const int32_t rows, const int32_t columns,
                                                                   std::string direction, const bool loop,
                                                                   const float frame_duration) {
    const auto recordAction = [&](const bool success, std::string code, std::string message) {
        nlohmann::json action = {
            {"action", "set_import_record_sprite_sheet_slice"},
            {"success", success},
            {"code", std::move(code)},
            {"message", std::move(message)},
            {"session_id", session_id},
            {"asset_id", asset_id},
        };
        action_history_.push_back(action);
        refreshSnapshot();
        snapshot_.last_action = action;
        snapshot_.action_history = action_history_;
        return action;
    };

    const auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                                      [&](const auto& candidate) { return candidate.sessionId == session_id; });
    if (session == import_sessions_.end()) {
        return recordAction(false, "import_session_not_found", "Import session was not found.");
    }
    const auto record = std::find_if(session->records.begin(), session->records.end(),
                                     [&](const auto& candidate) { return candidate.assetId == asset_id; });
    if (record == session->records.end()) {
        return recordAction(false, "import_record_not_found", "Import record was not found.");
    }
    if (record->mediaKind != "image") {
        return recordAction(false, "sprite_slice_requires_image", "Grid slicing is available only for image import records.");
    }
    if (frame_width <= 0 || frame_height <= 0 || rows <= 0 || columns <= 0 || frame_duration <= 0.0f ||
        (direction != "down" && direction != "left" && direction != "right" && direction != "up")) {
        return recordAction(false, "sprite_slice_invalid", "Set positive grid dimensions, a supported direction, and frame duration.");
    }
    const auto grid_width = static_cast<int64_t>(frame_width) * columns;
    const auto grid_height = static_cast<int64_t>(frame_height) * rows;
    if ((record->width > 0 && grid_width > record->width) || (record->height > 0 && grid_height > record->height)) {
        return recordAction(false, "sprite_slice_out_of_bounds", "The requested sprite grid exceeds the imported image dimensions.");
    }

    const auto previousMetadata = record->authoredMetadata;
    record->authoredMetadata["sprite_sheet_slice"] = {
        {"schema", "urpg.sprite_sheet_slice.v1"},
        {"frame_width", frame_width},
        {"frame_height", frame_height},
        {"rows", rows},
        {"columns", columns},
        {"direction", std::move(direction)},
        {"loop", loop},
        {"frame_duration", frame_duration},
    };
    std::string persistenceError;
    if (!persistImportSession(*session, &persistenceError)) {
        record->authoredMetadata = previousMetadata;
        return recordAction(false, "sprite_slice_manifest_write_failed",
                            "Sprite slicing was not saved to the governed import-session manifest: " + persistenceError);
    }
    return recordAction(true, "sprite_slice_saved",
                        "Sprite slicing metadata was saved to the governed import-session manifest.");
}

nlohmann::json AssetLibraryModel::promoteImportRecords(std::string session_id, std::vector<std::string> asset_ids,
                                                       std::string license_id, std::string promoted_root,
                                                       bool include_in_runtime) {
    nlohmann::json rows = nlohmann::json::array();
    size_t promotedCount = 0;
    size_t blockedCount = 0;
    size_t missingCount = 0;

    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                                [&](const auto& candidate) { return candidate.sessionId == session_id; });
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
            auto record = std::find_if(session->records.begin(), session->records.end(),
                                       [&](const auto& candidate) { return candidate.assetId == asset_id; });
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

            auto manifest = urpg::assets::planAssetPromotionManifest(*session, *record, license_id, promoted_root,
                                                                     include_in_runtime);
            library_.ingestPromotionManifest(manifest);
            const bool success =
                manifest.status == urpg::assets::AssetPromotionStatus::RuntimeReady && manifest.diagnostics.empty();
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
    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                                [&](const auto& candidate) { return candidate.sessionId == session_id; });
    if (session == import_sessions_.end()) {
        urpg::assets::AssetLibraryActionResult result{"promote_import_record_global", asset_id, false,
                                                      "import_session_not_found", "Import session was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    auto record = std::find_if(session->records.begin(), session->records.end(),
                               [&](const auto& candidate) { return candidate.assetId == asset_id; });
    if (record == session->records.end()) {
        urpg::assets::AssetLibraryActionResult result{"promote_import_record_global", asset_id, false,
                                                      "import_record_not_found", "Import record was not found."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    urpg::assets::GlobalAssetPromotionService service;
    const auto promotion = service.promoteImportRecord(*session, *record, std::move(license_id), promoted_root);
    library_.ingestPromotionManifest(promotion.manifest);
    urpg::assets::AssetLibraryActionResult result{"promote_import_record_global", asset_id, promotion.success,
                                                  promotion.code, promotion.message};
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

    auto session = std::find_if(import_sessions_.begin(), import_sessions_.end(),
                                [&](const auto& candidate) { return candidate.sessionId == session_id; });
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
            auto record = std::find_if(session->records.begin(), session->records.end(),
                                       [&](const auto& candidate) { return candidate.assetId == asset_id; });
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
        {"message", success
                        ? "All selected import records were copied into the promoted global asset library."
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
        std::error_code duplicate_size_error;
        const auto duplicate_csv_size = std::filesystem::file_size(duplicates_path, duplicate_size_error);
        const bool skip_duplicate_details =
            !duplicate_size_error && duplicate_csv_size > duplicate_csv_detail_limit_bytes_;
        std::string duplicate_csv;
        if (!skip_duplicate_details) {
            std::ifstream duplicate_stream(duplicates_path);
            std::stringstream duplicate_buffer;
            duplicate_buffer << duplicate_stream.rdbuf();
            duplicate_csv = duplicate_buffer.str();
        }

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
        size_t skipped_promotion_catalog_details = 0;
        size_t skipped_promotion_shards = 0;
        for (const auto& path : promotion_catalog_paths) {
            std::error_code catalog_size_error;
            const auto catalog_size = std::filesystem::file_size(path, catalog_size_error);
            if (!catalog_size_error && catalog_size > promotion_catalog_detail_limit_bytes_) {
                const auto summary_path = companionPromotionSummaryPath(path);
                if (!summary_path.empty() && std::filesystem::is_regular_file(summary_path)) {
                    library_.ingestPromotionCatalog(summaryCatalogFromSummaryFile(summary_path));
                    ++skipped_promotion_catalog_details;
                    continue;
                }
            }
            std::ifstream promotion_stream(path);
            skipped_promotion_shards +=
                ingestCatalogWithShards(library_, reports_root, nlohmann::json::parse(promotion_stream),
                                        promotion_catalog_detail_limit_bytes_);
        }
        if (!skip_duplicate_details) {
            library_.ingestDuplicateCsv(duplicate_csv);
        }
        library_.detectCaseCollisions();
        rebuildCleanupPreview();
        snapshot_.reports_loaded = true;
        snapshot_.status = "ready";
        std::vector<std::string> detail_skip_messages;
        if (skip_duplicate_details) {
            detail_skip_messages.push_back("detailed duplicate rows");
        }
        if (skipped_promotion_catalog_details > 0 || skipped_promotion_shards > 0) {
            detail_skip_messages.push_back("detailed promotion catalog records");
        }
        if (!detail_skip_messages.empty()) {
            snapshot_.status_message =
                "Skipped oversized " + detail_skip_messages.front() +
                (detail_skip_messages.size() > 1 ? " and " + detail_skip_messages.back() : "") +
                "; summary counts are loaded.";
        } else {
            snapshot_.status_message = "";
        }
        snapshot_.error_message = "";
        if (skip_duplicate_details || skipped_promotion_catalog_details > 0 || skipped_promotion_shards > 0) {
            std::string code = "asset_report_details_skipped";
            if (skip_duplicate_details && skipped_promotion_catalog_details == 0 && skipped_promotion_shards == 0) {
                code = "duplicate_details_skipped";
            } else if (!skip_duplicate_details &&
                       (skipped_promotion_catalog_details > 0 || skipped_promotion_shards > 0)) {
                code = "promotion_catalog_details_skipped";
            }
            nlohmann::json action = {
                {"action", "load_asset_reports"},
                {"success", true},
                {"code", code},
                {"message", "Loaded asset report summaries without expanding oversized report details."},
                {"duplicate_csv_bytes", duplicate_csv_size},
                {"duplicate_detail_limit_bytes", duplicate_csv_detail_limit_bytes_},
                {"skipped_promotion_catalogs", skipped_promotion_catalog_details},
                {"skipped_promotion_shards", skipped_promotion_shards},
                {"promotion_catalog_detail_limit_bytes", promotion_catalog_detail_limit_bytes_},
            };
            action_history_.push_back(action);
            snapshot_.last_action = action;
            snapshot_.action_history = action_history_;
        }
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

bool AssetLibraryModel::loadAssetBundleManifestsFromDirectory(const std::filesystem::path& bundle_root,
                                                              std::string* error_message) {
    if (!std::filesystem::is_directory(bundle_root)) {
        if (error_message != nullptr) {
            *error_message = "asset bundle manifest directory is missing";
        }
        refreshSnapshot();
        snapshot_.status_message = snapshot_.reports_loaded ? "" : "Asset bundle manifests are missing.";
        snapshot_.error_message = "Missing asset bundle manifest directory: " + bundle_root.string();
        return false;
    }

    try {
        size_t loaded = 0;
        for (const auto& entry : std::filesystem::directory_iterator(bundle_root)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".json" ||
                entry.path().filename().string() == "asset_bundle.schema.json") {
                continue;
            }
            std::ifstream manifest_stream(entry.path());
            library_.ingestAssetBundleManifest(nlohmann::json::parse(manifest_stream));
            ++loaded;
        }
        rebuildCleanupPreview();
        if (loaded == 0) {
            if (error_message != nullptr) {
                *error_message = "asset bundle manifests are missing";
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
        snapshot_.status_message = "Asset bundle manifests could not be loaded.";
        snapshot_.error_message = ex.what();
        return false;
    }
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

nlohmann::json AssetLibraryModel::planPromotedAssetAttachmentToProject(
    std::string path, const std::filesystem::path& project_root,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    std::replace(path.begin(), path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "plan_project_asset_attachment"},
        {"path", path},
        {"project_root", project_root.generic_string()},
        {"conflict_policy", attachmentConflictPolicyName(policy)},
    };
    const auto found = library_.findAsset(path);
    if (!found.has_value()) {
        action["success"] = false;
        action["code"] = "asset_not_found";
        action["message"] = "Asset was not found in the library.";
    } else {
        urpg::assets::ProjectAssetAttachmentService service;
        const auto plan = service.planPromotedAssetAttachment(manifestFromAssetRecord(*found), project_root, policy);
        action["asset_id"] = found->asset_id;
        action["success"] = plan.valid;
        action["code"] = plan.valid ? "project_asset_attachment_planned" : "project_asset_attachment_plan_invalid";
        action["message"] = plan.valid ? "Review the attachment paths and revision before confirming."
                                      : "The project asset attachment cannot be planned.";
        action["expected_source_revision"] = plan.sourceRevision;
        action["operation_id"] = plan.valid ? attachmentOperationId(found->asset_id, plan.sourceRevision, policy) : "";
        action["payload_path"] = plan.payloadPath.empty() ? "" : plan.payloadPath.generic_string();
        action["manifest_path"] = plan.manifestPath.empty() ? "" : plan.manifestPath.generic_string();
        action["diagnostics"] = plan.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::confirmPromotedAssetAttachmentToProject(
    std::string path, const std::filesystem::path& project_root, std::string expected_source_revision,
    std::string operation_id, const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    std::replace(path.begin(), path.end(), '\\', '/');
    const auto found = library_.findAsset(path);
    if (!found.has_value()) {
        urpg::assets::AssetLibraryActionResult result{"confirm_project_asset_attachment", path, false, "asset_not_found",
                                                      "Asset was not found in the library."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }
    if (expected_source_revision.empty() || operation_id.empty()) {
        urpg::assets::AssetLibraryActionResult result{
            "confirm_project_asset_attachment", path, false, "attachment_confirmation_missing",
            "Attachment confirmation requires the source revision and operation ID from a current plan."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    urpg::assets::ProjectAssetAttachmentService service;
    urpg::assets::ProjectAssetAttachmentRequest request;
    request.manifest = manifestFromAssetRecord(*found);
    request.projectRoot = project_root;
    request.conflictPolicy = policy;
    request.operationId = std::move(operation_id);
    request.expectedSourceRevision = std::move(expected_source_revision);
    const auto attach_result = service.attachPromotedAsset(request);
    urpg::assets::AssetLibraryActionResult result{"confirm_project_asset_attachment", path, attach_result.success,
                                                  attach_result.code, attach_result.message};
    if (attach_result.success) {
        library_.addUsageReference(path, "project_asset_attachment:" + attach_result.manifestPath.generic_string());
    }
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

nlohmann::json AssetLibraryModel::planDerivedRevisionAttachmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "plan_derived_project_asset_attachment"},
        {"path", source_path},
        {"derived_manifest_path", derived_manifest_path.generic_string()},
        {"project_root", project_root.generic_string()},
        {"conflict_policy", attachmentConflictPolicyName(policy)},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"expected_source_revision", ""},
        {"operation_id", ""},
        {"payload_path", ""},
        {"manifest_path", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::ProjectAssetAttachmentService service;
        const auto plan = service.planDerivedRevisionAttachment(manifestFromAssetRecord(*found), derived_manifest_path,
                                                                 project_root, policy);
        action["asset_id"] = plan.assetId;
        action["success"] = plan.valid;
        action["code"] = plan.valid ? "project_derived_asset_attachment_planned"
                                      : "project_derived_asset_attachment_plan_invalid";
        action["message"] = plan.valid ? "Review the derived revision and attachment paths before confirming."
                                         : "The derived revision attachment cannot be planned.";
        action["expected_source_revision"] = plan.sourceRevision;
        action["operation_id"] = plan.valid ? attachmentOperationId(plan.assetId, plan.sourceRevision, policy) : "";
        action["payload_path"] = plan.payloadPath.empty() ? "" : plan.payloadPath.generic_string();
        action["manifest_path"] = plan.manifestPath.empty() ? "" : plan.manifestPath.generic_string();
        action["diagnostics"] = plan.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::confirmDerivedRevisionAttachmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    const auto found = library_.findAsset(source_path);
    if (!found.has_value()) {
        urpg::assets::AssetLibraryActionResult result{"confirm_derived_project_asset_attachment", source_path, false,
                                                      "asset_not_found", "Asset was not found in the library."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }
    if (expected_source_revision.empty() || operation_id.empty()) {
        urpg::assets::AssetLibraryActionResult result{
            "confirm_derived_project_asset_attachment", source_path, false, "attachment_confirmation_missing",
            "Derived revision attachment confirmation requires the source revision and operation ID from a current plan."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }

    urpg::assets::ProjectDerivedAssetAttachmentRequest request;
    request.source = manifestFromAssetRecord(*found);
    request.derivedManifestPath = derived_manifest_path;
    request.projectRoot = project_root;
    request.conflictPolicy = policy;
    request.operationId = std::move(operation_id);
    request.expectedSourceRevision = std::move(expected_source_revision);
    urpg::assets::ProjectAssetAttachmentService service;
    const auto attachResult = service.attachDerivedRevision(request);
    urpg::assets::AssetLibraryActionResult result{"confirm_derived_project_asset_attachment", source_path,
                                                  attachResult.success, attachResult.code, attachResult.message};
    if (attachResult.success && std::filesystem::is_regular_file(attachResult.manifestPath)) {
        std::ifstream manifestStream(attachResult.manifestPath);
        const auto attachedManifest = urpg::assets::deserializeAssetPromotionManifest(nlohmann::json::parse(manifestStream));
        library_.ingestPromotionManifest(attachedManifest);
        const auto attachedPath = findAssetPathById(library_, attachedManifest.assetId);
        if (!attachedPath.empty()) {
            library_.addUsageReference(attachedPath,
                                       "project_asset_attachment:" + attachResult.manifestPath.generic_string());
        }
    }
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::recoverDerivedAttachmentReference(
    const std::filesystem::path& derived_manifest_path, const std::filesystem::path& project_root) {
    urpg::assets::ProjectAssetAttachmentService service;
    const auto recovery = service.recoverDerivedAttachmentReference(derived_manifest_path, project_root);
    urpg::assets::AssetLibraryActionResult result{
        "recover_derived_attachment_reference", derived_manifest_path.generic_string(), recovery.success,
        recovery.code, recovery.message};
    if (recovery.success) {
        std::string loadError;
        (void)loadProjectAssetAttachments(project_root, &loadError);
    }
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

nlohmann::json AssetLibraryModel::planDerivedTilesetAssignmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "plan_derived_tileset_assignment"},
        {"path", source_path},
        {"derived_manifest_path", derived_manifest_path.generic_string()},
        {"project_root", project_root.generic_string()},
        {"conflict_policy", attachmentConflictPolicyName(policy)},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"tileset_id", ""},
        {"expected_source_revision", ""},
        {"operation_id", ""},
        {"tile_directory", ""},
        {"manifest_path", ""},
        {"columns", 0},
        {"rows", 0},
        {"tile_width", 0},
        {"tile_height", 0},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::ProjectAssetAttachmentService service;
        const auto plan = service.planDerivedTilesetAssignment(manifestFromAssetRecord(*found), derived_manifest_path,
                                                                 project_root, policy);
        action["tileset_id"] = plan.tilesetId;
        action["success"] = plan.valid;
        action["code"] = plan.valid ? "project_derived_tileset_assignment_planned"
                                      : "project_derived_tileset_assignment_plan_invalid";
        action["message"] = plan.valid ? "Review the derived tileset bundle and project paths before confirming."
                                        : "The derived tileset assignment cannot be planned.";
        action["expected_source_revision"] = plan.sourceRevision;
        action["operation_id"] = plan.valid ? "tileset-" + attachmentOperationId(plan.tilesetId, plan.sourceRevision, policy)
                                              : "";
        action["tile_directory"] = plan.tileDirectory.generic_string();
        action["manifest_path"] = plan.manifestPath.generic_string();
        action["columns"] = plan.columns;
        action["rows"] = plan.rows;
        action["tile_width"] = plan.tileWidth;
        action["tile_height"] = plan.tileHeight;
        action["diagnostics"] = plan.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

urpg::assets::AssetLibraryActionResult AssetLibraryModel::confirmDerivedTilesetAssignmentToProject(
    std::string source_path, const std::filesystem::path& derived_manifest_path,
    const std::filesystem::path& project_root, std::string expected_source_revision, std::string operation_id,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    const auto found = library_.findAsset(source_path);
    if (!found.has_value()) {
        urpg::assets::AssetLibraryActionResult result{"confirm_derived_tileset_assignment", source_path, false,
                                                      "asset_not_found", "Asset was not found in the library."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }
    if (expected_source_revision.empty() || operation_id.empty()) {
        urpg::assets::AssetLibraryActionResult result{
            "confirm_derived_tileset_assignment", source_path, false, "tileset_assignment_confirmation_missing",
            "Tileset assignment confirmation requires the review revision and operation ID from a current plan."};
        action_history_.push_back(result.toJson());
        refreshSnapshot();
        snapshot_.last_action = result.toJson();
        snapshot_.action_history = action_history_;
        return result;
    }
    urpg::assets::ProjectDerivedTilesetAssignmentRequest request;
    request.source = manifestFromAssetRecord(*found);
    request.derivedManifestPath = derived_manifest_path;
    request.projectRoot = project_root;
    request.conflictPolicy = policy;
    request.operationId = std::move(operation_id);
    request.expectedSourceRevision = std::move(expected_source_revision);
    urpg::assets::ProjectAssetAttachmentService service;
    const auto assignment = service.assignDerivedTileset(request);
    urpg::assets::AssetLibraryActionResult result{"confirm_derived_tileset_assignment", source_path, assignment.success,
                                                  assignment.code, assignment.message};
    if (assignment.success) {
        library_.addUsageReference(source_path, "project_tileset_assignment:" + assignment.manifestPath.generic_string());
    }
    action_history_.push_back(result.toJson());
    rebuildCleanupPreview();
    snapshot_.last_action = result.toJson();
    snapshot_.action_history = action_history_;
    return result;
}

nlohmann::json AssetLibraryModel::createImageCropScaleRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const int32_t crop_x, const int32_t crop_y, const int32_t crop_width, const int32_t crop_height,
    const int32_t output_width, const int32_t output_height) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "create_image_crop_scale_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"operation_id", operation_id},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"manifest_path", ""},
        {"output_path", ""},
        {"source_revision", ""},
        {"derived_revision", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetImageCropScalePlan plan;
        plan.operationId = std::move(operation_id);
        plan.source = manifestFromAssetRecord(*found);
        plan.derivedRoot = derived_root;
        plan.cropX = crop_x;
        plan.cropY = crop_y;
        plan.cropWidth = crop_width;
        plan.cropHeight = crop_height;
        plan.outputWidth = output_width;
        plan.outputHeight = output_height;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.createImageCropScaleRevision(plan);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["manifest_path"] = result.manifestPath.generic_string();
        action["output_path"] = result.outputPath.generic_string();
        action["source_revision"] = result.sourceRevision;
        action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::createImagePaletteRevision(std::string source_path,
                                                              const std::filesystem::path& derived_root,
                                                              std::string operation_id,
                                                              std::vector<uint32_t> colors_rgba, const bool dither) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "create_image_palette_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"operation_id", operation_id},
        {"dither", dither},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"manifest_path", ""},
        {"output_path", ""},
        {"source_revision", ""},
        {"derived_revision", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetImagePalettePlan plan;
        plan.operationId = std::move(operation_id);
        plan.source = manifestFromAssetRecord(*found);
        plan.derivedRoot = derived_root;
        plan.colorsRgba = std::move(colors_rgba);
        plan.dither = dither;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.createImagePaletteRevision(plan);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["manifest_path"] = result.manifestPath.generic_string();
        action["output_path"] = result.outputPath.generic_string();
        action["source_revision"] = result.sourceRevision;
        action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::createImagePaletteExtractRevision(std::string source_path,
                                                                     const std::filesystem::path& derived_root,
                                                                     std::string operation_id,
                                                                     const int32_t max_colors, const bool dither) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "create_image_palette_extract_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"operation_id", operation_id},
        {"max_colors", max_colors},
        {"dither", dither},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"manifest_path", ""},
        {"output_path", ""},
        {"source_revision", ""},
        {"derived_revision", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetImagePaletteExtractPlan plan;
        plan.operationId = std::move(operation_id);
        plan.source = manifestFromAssetRecord(*found);
        plan.derivedRoot = derived_root;
        plan.maxColors = max_colors;
        plan.dither = dither;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.createImagePaletteExtractRevision(plan);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["manifest_path"] = result.manifestPath.generic_string();
        action["output_path"] = result.outputPath.generic_string();
        action["source_revision"] = result.sourceRevision;
        action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::createAudioTrimFadeGainRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const uint64_t start_frame, const uint64_t end_frame, const uint64_t fade_in_frames,
    const uint64_t fade_out_frames, const int32_t gain_milli_db, const int64_t loop_start_frame,
    const int64_t loop_end_frame) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "create_audio_trim_fade_gain_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"operation_id", operation_id},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"manifest_path", ""},
        {"output_path", ""},
        {"source_revision", ""},
        {"derived_revision", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetAudioTrimFadeGainPlan plan;
        plan.operationId = std::move(operation_id);
        plan.source = manifestFromAssetRecord(*found);
        plan.derivedRoot = derived_root;
        plan.startFrame = start_frame;
        plan.endFrame = end_frame;
        plan.fadeInFrames = fade_in_frames;
        plan.fadeOutFrames = fade_out_frames;
        plan.gainMilliDb = gain_milli_db;
        plan.loopStartFrame = loop_start_frame;
        plan.loopEndFrame = loop_end_frame;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.createAudioTrimFadeGainRevision(plan);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["manifest_path"] = result.manifestPath.generic_string();
        action["output_path"] = result.outputPath.generic_string();
        action["source_revision"] = result.sourceRevision;
        action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::inspectAudioTrimFadeGainSource(std::string source_path) const {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "inspect_audio_trim_fade_gain_source"},
        {"path", source_path},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"source_revision", ""},
        {"channels", 0},
        {"sample_rate", 0},
        {"frame_count", 0},
        {"duration_ms", 0},
        {"waveform_peaks", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.inspectAudioTrimFadeGainSource(manifestFromAssetRecord(*found));
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["source_revision"] = result.sourceRevision;
        action["channels"] = result.channels;
        action["sample_rate"] = result.sampleRate;
        action["frame_count"] = result.frameCount;
        action["duration_ms"] = result.durationMs;
        action["waveform_peaks"] = result.waveformPeaks;
    }
    return action;
}

nlohmann::json AssetLibraryModel::recoverStagedDerivedRevisions(
    std::string source_path, const std::filesystem::path& derived_root) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "recover_staged_derived_revisions"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetTransformStagingRecoveryRequest request;
        request.derivedRoot = derived_root;
        request.assetId = found->asset_id;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.recoverStagedRevisions(request);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::removeDerivedRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string derived_revision) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "remove_derived_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"derived_revision", derived_revision},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        const urpg::assets::AssetTransformRevisionRemovalRequest request{
            derived_root, found->asset_id, std::move(derived_revision)};
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.removeDerivedRevision(request);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["source_revision"] = result.sourceRevision;
        if (!result.derivedRevision.empty()) action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::createTilesetSliceRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const int32_t tile_width, const int32_t tile_height, const int32_t margin, const int32_t spacing) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "create_tileset_slice_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"operation_id", operation_id},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"manifest_path", ""},
        {"output_path", ""},
        {"source_revision", ""},
        {"derived_revision", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetTilesetSlicePlan plan;
        plan.operationId = std::move(operation_id);
        plan.source = manifestFromAssetRecord(*found);
        plan.derivedRoot = derived_root;
        plan.tileWidth = tile_width;
        plan.tileHeight = tile_height;
        plan.margin = margin;
        plan.spacing = spacing;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.createTilesetSliceRevision(plan);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["manifest_path"] = result.manifestPath.generic_string();
        action["output_path"] = result.outputPath.generic_string();
        action["source_revision"] = result.sourceRevision;
        action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

nlohmann::json AssetLibraryModel::createAtlasMetadataRevision(
    std::string source_path, const std::filesystem::path& derived_root, std::string operation_id,
    const int32_t atlas_width, const int32_t atlas_height, const int32_t frame_width, const int32_t frame_height) {
    std::replace(source_path.begin(), source_path.end(), '\\', '/');
    nlohmann::json action = {
        {"action", "create_atlas_metadata_revision"},
        {"path", source_path},
        {"derived_root", derived_root.generic_string()},
        {"operation_id", operation_id},
        {"success", false},
        {"code", "asset_not_found"},
        {"message", "Asset was not found in the library."},
        {"manifest_path", ""},
        {"source_revision", ""},
        {"derived_revision", ""},
        {"diagnostics", nlohmann::json::array()},
    };
    const auto found = library_.findAsset(source_path);
    if (found.has_value()) {
        urpg::assets::AssetAtlasMetadataPlan plan;
        plan.operationId = std::move(operation_id);
        plan.source = manifestFromAssetRecord(*found);
        plan.derivedRoot = derived_root;
        plan.atlasWidth = atlas_width;
        plan.atlasHeight = atlas_height;
        plan.frameWidth = frame_width;
        plan.frameHeight = frame_height;
        urpg::assets::AssetTransformRevisionService service;
        const auto result = service.createAtlasMetadataRevision(plan);
        action["asset_id"] = found->asset_id;
        action["success"] = result.success;
        action["code"] = result.code;
        action["message"] = result.message;
        action["manifest_path"] = result.manifestPath.generic_string();
        action["source_revision"] = result.sourceRevision;
        action["derived_revision"] = result.derivedRevision;
        action["diagnostics"] = result.diagnostics;
    }
    action_history_.push_back(action);
    snapshot_.last_action = action;
    snapshot_.action_history = action_history_;
    return action;
}

urpg::assets::AssetLibraryActionResult
AssetLibraryModel::attachPromotedAssetToProject(
    std::string path, const std::filesystem::path& project_root,
    const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    const auto plan = planPromotedAssetAttachmentToProject(path, project_root, policy);
    if (!plan.value("success", false)) {
        return {"attach_project_asset", path, false, plan.value("code", "project_asset_attachment_plan_invalid"),
                plan.value("message", "The project asset attachment cannot be planned.")};
    }
    const auto confirmed = confirmPromotedAssetAttachmentToProject(path, project_root,
                                                                    plan.value("expected_source_revision", ""),
                                                                    plan.value("operation_id", ""), policy);
    auto compatibility_action = confirmed.toJson();
    compatibility_action["action"] = "attach_project_asset";
    if (!action_history_.empty()) {
        action_history_.back() = compatibility_action;
    }
    snapshot_.last_action = compatibility_action;
    snapshot_.action_history = action_history_;
    return {"attach_project_asset", path, confirmed.success, confirmed.code, confirmed.message};
}

nlohmann::json AssetLibraryModel::attachPromotedAssetsToProject(std::vector<std::string> paths,
                                                                const std::filesystem::path& project_root,
                                                                const urpg::assets::ProjectAssetAttachmentConflictPolicy policy) {
    nlohmann::json rows = nlohmann::json::array();
    size_t attachedCount = 0;
    size_t blockedCount = 0;
    size_t missingCount = 0;

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

        const auto plan = planPromotedAssetAttachmentToProject(path, project_root, policy);
        const auto attachResult = plan.value("success", false)
                                      ? [&] {
                                            return confirmPromotedAssetAttachmentToProject(
                                                path, project_root, plan.value("expected_source_revision", ""),
                                                plan.value("operation_id", ""), policy);
                                        }()
                                      : urpg::assets::AssetLibraryActionResult{
                                            "confirm_project_asset_attachment", path, false,
                                            plan.value("code", "project_asset_attachment_plan_invalid"),
                                            plan.value("message", "The project asset attachment cannot be planned.")};
        if (attachResult.success) {
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
            {"expected_source_revision", plan.value("expected_source_revision", "")},
            {"operation_id", plan.value("operation_id", "")},
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
    if (filter_id == "release_eligible") {
        filter.release_eligible_only = true;
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Release-eligible asset filter applied.");
        return true;
    }
    if (filter_id == "characters") {
        filter.game_use_category = "characters/sprites";
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Character asset filter applied.");
        return true;
    }
    if (filter_id == "tilesets") {
        filter.required_game_use_tag = "asset_type:tileset";
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "Tileset asset filter applied.");
        return true;
    }
    if (filter_id == "ui_assets") {
        filter.game_use_category = "ui/widgets";
        setFilter(filter);
        recordResult(true, "quick_filter_applied", "UI asset filter applied.");
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

void AssetLibraryModel::applyUserAssetCuration(const urpg::settings::EditorSettings& settings) {
    favorite_asset_keys_ = settings.asset_favorite_keys;
    asset_collections_ = settings.asset_collections;
    refreshSnapshot();
}

void AssetLibraryModel::writeUserAssetCuration(urpg::settings::EditorSettings* settings) const {
    if (settings == nullptr) {
        return;
    }
    settings->asset_favorite_keys = favorite_asset_keys_;
    settings->asset_collections = asset_collections_;
}

std::string AssetLibraryModel::curationKeyForPath(std::string_view path) const {
    const auto asset = library_.findAsset(path);
    return asset.has_value() ? curationKeyForRecord(*asset) : std::string{};
}

bool AssetLibraryModel::setAssetFavorite(std::string_view path, bool favorite) {
    const auto key = curationKeyForPath(path);
    if (key.empty()) {
        return false;
    }
    const auto found = std::find(favorite_asset_keys_.begin(), favorite_asset_keys_.end(), key);
    if (favorite && found == favorite_asset_keys_.end()) {
        favorite_asset_keys_.push_back(key);
    } else if (!favorite && found != favorite_asset_keys_.end()) {
        favorite_asset_keys_.erase(found);
    }
    refreshSnapshot();
    return true;
}

bool AssetLibraryModel::isAssetFavorite(std::string_view path) const {
    const auto key = curationKeyForPath(path);
    return !key.empty() && std::find(favorite_asset_keys_.begin(), favorite_asset_keys_.end(), key) !=
                              favorite_asset_keys_.end();
}

bool AssetLibraryModel::isAssetInCollection(std::string_view collectionId, std::string_view path) const {
    const auto key = curationKeyForPath(path);
    const auto collection = std::find_if(asset_collections_.begin(), asset_collections_.end(),
                                         [&](const auto& item) { return item.id == collectionId; });
    return !key.empty() && collection != asset_collections_.end() &&
           std::find(collection->asset_keys.begin(), collection->asset_keys.end(), key) != collection->asset_keys.end();
}

bool AssetLibraryModel::createAssetCollection(std::string id, std::string label) {
    if (id.empty() || label.empty() || std::any_of(asset_collections_.begin(), asset_collections_.end(),
                                                   [&](const auto& collection) { return collection.id == id; })) {
        return false;
    }
    asset_collections_.push_back({std::move(id), std::move(label), {}});
    refreshSnapshot();
    return true;
}

bool AssetLibraryModel::setAssetCollectionMembership(std::string_view collection_id, std::string_view path,
                                                      bool included) {
    const auto key = curationKeyForPath(path);
    if (key.empty()) {
        return false;
    }
    const auto collection = std::find_if(asset_collections_.begin(), asset_collections_.end(),
                                         [&](const auto& item) { return item.id == collection_id; });
    if (collection == asset_collections_.end()) {
        return false;
    }
    const auto found = std::find(collection->asset_keys.begin(), collection->asset_keys.end(), key);
    if (included && found == collection->asset_keys.end()) {
        collection->asset_keys.push_back(key);
    } else if (!included && found != collection->asset_keys.end()) {
        collection->asset_keys.erase(found);
    }
    refreshSnapshot();
    return true;
}

void AssetLibraryModel::rebuildCleanupPreview() {
    cleanup_plan_ = cleanup_planner_.buildDuplicateCleanupPlan(library_);
    refreshSnapshot();
}

void AssetLibraryModel::clear() {
    library_.clear();
    cleanup_plan_ = {};
    import_sessions_.clear();
    import_session_manifest_paths_.clear();
    pending_import_request_ = nlohmann::json::object();
    action_history_ = nlohmann::json::array();
    external_catalog_.clear();
    external_catalog_query_ = {};
    external_catalog_directory_.clear();
    selected_external_catalog_asset_id_.clear();
    external_catalog_diagnostics_.clear();
    archive_browser_ = nlohmann::json::object();
    cached_archive_path_.clear();
    cached_archive_size_ = 0;
    cached_archive_write_time_ = {};
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
    snapshot_.filter_controls = filterControls(filter_, asset_snapshot, snapshot_.filtered_asset_count,
                                               snapshot_.project_attached_count, snapshot_.project_attachable_count);
    snapshot_.favorite_asset_count = favorite_asset_keys_.size();
    snapshot_.asset_collection_count = asset_collections_.size();
    snapshot_.user_curation = {{"favorite_count", snapshot_.favorite_asset_count},
                               {"favorites", favorite_asset_keys_},
                               {"collections", nlohmann::json::array()}};
    for (const auto& collection : asset_collections_) {
        snapshot_.user_curation["collections"].push_back(
            {{"id", collection.id}, {"label", collection.label}, {"asset_count", collection.asset_keys.size()},
             {"asset_keys", collection.asset_keys}});
    }
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
    snapshot_.virtual_catalog = buildVirtualCatalogSnapshot(asset_snapshot);
    refreshExternalCatalogSnapshot();
    snapshot_.archive_browser = archive_browser_;
    snapshot_.action_history = action_history_;
    if (!action_history_.empty()) {
        snapshot_.last_action = action_history_.back();
    }
    snapshot_.category_counts = asset_snapshot.category_counts;
    snapshot_.game_use_category_counts = asset_snapshot.game_use_category_counts;
    snapshot_.game_use_tag_counts = asset_snapshot.game_use_tag_counts;
    snapshot_.kind_counts = asset_snapshot.kind_counts;
    snapshot_.source_bundle_counts = asset_snapshot.source_bundle_counts;
    snapshot_.reports_loaded = asset_snapshot.assets.size() > 0 || asset_snapshot.duplicate_groups.size() > 0 ||
                               asset_snapshot.file_count > 0 || asset_snapshot.duplicate_group_count > 0 ||
                               asset_snapshot.catalog_asset_count > 0 || asset_snapshot.catalog_shard_count > 0 ||
                               cleanup_plan_.allowed_count > 0 || cleanup_plan_.refused_count > 0 ||
                               !import_sessions_.empty() || external_catalog_.isLoaded();
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

void AssetLibraryModel::refreshExternalCatalogSnapshot() {
    nlohmann::json diagnosticRows = nlohmann::json::array();
    for (const auto& diagnostic : external_catalog_diagnostics_) {
        diagnosticRows.push_back(diagnostic);
    }
    if (!external_catalog_.isLoaded()) {
        snapshot_.external_catalog_asset_count = 0;
        snapshot_.external_catalog_hash_pending_count = 0;
        snapshot_.external_catalog_archive_count = 0;
        snapshot_.external_catalog = {
            {"loaded", false},
            {"diagnostics", std::move(diagnosticRows)},
            {"actions",
             {{"refresh_index", {{"enabled", false}, {"reason", "No compatible catalog interchange is loaded."}}},
              {"open_source_location", {{"enabled", false}, {"reason", "Select an external catalog record first."}}}}},
        };
        return;
    }

    const auto& metadata = external_catalog_.metadata();
    const auto page = external_catalog_.query(external_catalog_query_);
    for (const auto& diagnostic : page.diagnostics) {
        diagnosticRows.push_back(diagnostic);
    }
    nlohmann::json records = nlohmann::json::array();
    for (const auto& record : page.records) {
        records.push_back({
            {"asset_id", record.assetId},
            {"selected", record.assetId == selected_external_catalog_asset_id_},
            {"virtual_path", record.virtualPath},
            {"source_root", record.sourceRoot},
            {"filename", record.filename},
            {"extension", record.extension},
            {"media_kind", record.mediaKind},
            {"archive_kind", record.archiveKind},
            {"size_bytes", record.sizeBytes},
            {"modified_time_ns", record.modifiedTimeNs},
            {"hash_pending", record.sha256.empty()},
            {"pack", record.pack},
            {"category", record.category},
            {"tags", record.tags},
        });
    }
    nlohmann::json roots = nlohmann::json::array();
    for (const auto& root : metadata.roots) {
        roots.push_back({{"id", root.id},
                         {"state", root.state},
                         {"asset_count", root.assetCount},
                         {"hash_pending_count", root.hashPendingCount}});
    }
    snapshot_.external_catalog_asset_count = metadata.assetCount;
    snapshot_.external_catalog_hash_pending_count = metadata.hashPendingCount;
    snapshot_.external_catalog_archive_count = metadata.archiveCount;
    snapshot_.external_catalog = {
        {"loaded", true},
        {"schema_version", metadata.schemaVersion},
        {"generated_at", metadata.generatedAt},
        {"scan_complete", metadata.scanComplete},
        {"roots", std::move(roots)},
        {"query",
         {{"text", external_catalog_query_.text},
          {"media_kind", external_catalog_query_.mediaKind},
          {"extension", external_catalog_query_.extension},
          {"pack", external_catalog_query_.pack},
          {"category", external_catalog_query_.category},
          {"archive_only", external_catalog_query_.archiveOnly},
          {"offset", external_catalog_query_.offset},
          {"page_size", external_catalog_query_.pageSize}}},
        {"page",
         {{"total_matches", page.totalMatches}, {"has_more", page.hasMore}, {"records", std::move(records)}}},
        {"diagnostics", std::move(diagnosticRows)},
        {"actions",
         {{"refresh_index",
           {{"enabled", std::filesystem::is_regular_file(external_catalog_directory_ / "asset_catalog.db") &&
                         !catalogInterchangeToolPath().empty()},
            {"command", urpg::assets::kLocalAssetCatalogRegenerateCommand},
            {"reason", "Refresh exports metadata only from the configured local index database."}}},
          {"open_source_location",
           {{"enabled", std::any_of(page.records.begin(), page.records.end(), [&](const auto& record) {
                 return record.assetId == selected_external_catalog_asset_id_;
             })},
            {"reason", "Select a visible catalog record to open its containing source location."}}}}},
    };
}

} // namespace urpg::editor
