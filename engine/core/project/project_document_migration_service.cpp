#include "engine/core/project/project_document_migration_service.h"

#include "engine/core/project/project_schema_migration_registry.h"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iterator>
#include <system_error>

#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace urpg::project {
namespace {

struct MigrationCandidate {
    std::string document_type;
    std::string target_version;
    std::filesystem::path path;
    std::string original_text;
    std::string migrated_text;
    std::filesystem::path backup_path;
};

bool atomicWrite(const std::filesystem::path& target, const std::string& contents, std::string& diagnostic) {
    std::error_code error;
    std::filesystem::create_directories(target.parent_path(), error);
    if (error) {
        diagnostic = "project_document_migration_directory_failed:" + target.parent_path().generic_string();
        return false;
    }
    const auto temporary = target.parent_path() / ("." + target.filename().string() + ".migration.tmp");
    {
        std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
        output << contents;
        output.close();
        if (!output) {
            diagnostic = "project_document_migration_write_failed:" + target.generic_string();
            std::filesystem::remove(temporary, error);
            return false;
        }
    }
#ifdef _WIN32
    if (!MoveFileExW(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        diagnostic = "project_document_migration_replace_failed:" + target.generic_string();
        std::filesystem::remove(temporary, error);
        return false;
    }
#else
    std::filesystem::rename(temporary, target, error);
    if (error) {
        diagnostic = "project_document_migration_replace_failed:" + target.generic_string();
        std::filesystem::remove(temporary, error);
        return false;
    }
#endif
    return true;
}

bool readText(const std::filesystem::path& path, std::string& text) {
    std::ifstream input(path, std::ios::binary);
    if (!input.good()) return false;
    text.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return input.good() || input.eof();
}

void appendJsonFiles(const std::filesystem::path& directory, const std::string& type,
                     const std::string& target, std::vector<MigrationCandidate>& candidates) {
    std::error_code error;
    if (std::filesystem::is_symlink(directory, error) || error ||
        !std::filesystem::is_directory(directory, error) || error) return;
    for (const auto& entry : std::filesystem::directory_iterator(directory, error)) {
        if (error) break;
        if (entry.is_symlink(error) || error || !entry.is_regular_file(error) || error ||
            entry.path().extension() != ".json") {
            error.clear();
            continue;
        }
        candidates.push_back({type, target, entry.path(), {}, {}, {}});
    }
}

std::filesystem::path uniqueBackupRoot(const std::filesystem::path& project_root) {
    const auto base = project_root / ".urpg" / "migration_backups";
    const auto stamp = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    for (size_t suffix = 0;; ++suffix) {
        const auto candidate = base / (suffix == 0 ? stamp : stamp + "-" + std::to_string(suffix));
        if (!std::filesystem::exists(candidate)) return candidate;
    }
}

} // namespace

ProjectDocumentMigrationServiceResult migrateProjectDocumentsOnDisk(
    const std::filesystem::path& project_root, const bool dry_run,
    std::function<bool(const std::filesystem::path&)> before_publish) {
    ProjectDocumentMigrationServiceResult result;
    result.dry_run = dry_run;
    std::error_code error;
    const auto root = std::filesystem::weakly_canonical(project_root, error);
    if (error || !std::filesystem::is_directory(root, error) || error) {
        result.code = "project_document_migration_root_invalid";
        return result;
    }

    ProjectSchemaMigrationRegistry registry;
    std::string diagnostic;
    if (!registerBuiltInProjectSchemaMigrations(registry, &diagnostic)) {
        result.code = "project_document_migration_registry_invalid";
        result.diagnostics.push_back(std::move(diagnostic));
        return result;
    }

    std::vector<MigrationCandidate> candidates;
    appendJsonFiles(root / "content" / "abilities", "ability", "urpg.ability.v1", candidates);
    appendJsonFiles(root / "content" / "characters", "character_creator", "1.0.0", candidates);
    appendJsonFiles(root / "content" / "vendors", "vendor_catalog", "urpg.vendor_catalog.v1", candidates);
    const auto databasePath = root / "content" / "database.json";
    if (!std::filesystem::is_symlink(databasePath, error) && !error &&
        std::filesystem::is_regular_file(databasePath, error) && !error) {
        candidates.push_back({"database", "urpg.database.v1", databasePath, {}, {}, {}});
    }
    error.clear();
    const auto menuPath = root / "content" / "ui" / "menus.json";
    if (!std::filesystem::is_symlink(menuPath, error) && !error &&
        std::filesystem::is_regular_file(menuPath, error) && !error) {
        candidates.push_back({"menu_studio", "urpg.menu_graph.v1", menuPath, {}, {}, {}});
    }
    std::sort(candidates.begin(), candidates.end(), [](const auto& left, const auto& right) {
        return left.path.generic_string() < right.path.generic_string();
    });

    std::vector<MigrationCandidate> changed;
    for (auto& candidate : candidates) {
        if (!readText(candidate.path, candidate.original_text)) {
            result.code = "project_document_migration_read_failed";
            result.diagnostics.push_back(candidate.path.generic_string());
            return result;
        }
        auto document = nlohmann::json::parse(candidate.original_text, nullptr, false);
        if (document.is_discarded() || !document.is_object()) {
            result.code = "project_document_migration_json_invalid";
            result.diagnostics.push_back(candidate.path.generic_string());
            return result;
        }
        auto migration = registry.migrate(candidate.document_type, candidate.target_version, document, dry_run);
        if (!migration.success) {
            result.code = migration.code;
            result.diagnostics.insert(result.diagnostics.end(), migration.diagnostics.begin(), migration.diagnostics.end());
            result.diagnostics.push_back(candidate.path.generic_string());
            return result;
        }
        result.records.push_back({candidate.document_type, candidate.path, {}, migration.changed});
        if (migration.changed) {
            candidate.migrated_text = migration.migrated.dump(2) + '\n';
            changed.push_back(std::move(candidate));
        }
    }
    if (changed.empty()) {
        result.success = true;
        result.code = "project_document_migration_already_current";
        return result;
    }
    if (dry_run) {
        result.success = true;
        result.code = "project_document_migration_dry_run_ready";
        return result;
    }

    const auto backupRoot = uniqueBackupRoot(root);
    for (auto& candidate : changed) {
        candidate.backup_path = backupRoot / std::filesystem::relative(candidate.path, root, error);
        if (error || !atomicWrite(candidate.backup_path, candidate.original_text, diagnostic)) {
            result.code = "project_document_migration_backup_failed";
            if (!diagnostic.empty()) result.diagnostics.push_back(std::move(diagnostic));
            return result;
        }
        const auto record = std::find_if(result.records.begin(), result.records.end(), [&](const auto& value) {
            return value.document_path == candidate.path;
        });
        if (record != result.records.end()) record->backup_path = candidate.backup_path;
    }

    size_t published = 0;
    for (; published < changed.size(); ++published) {
        auto& candidate = changed[published];
        if ((before_publish && !before_publish(candidate.path)) ||
            !atomicWrite(candidate.path, candidate.migrated_text, diagnostic)) {
            bool restored = true;
            for (size_t rollback = published; rollback > 0; --rollback) {
                std::string restoreDiagnostic;
                if (!atomicWrite(changed[rollback - 1].path, changed[rollback - 1].original_text,
                                 restoreDiagnostic)) {
                    restored = false;
                    result.diagnostics.push_back(std::move(restoreDiagnostic));
                }
            }
            result.code = restored ? "project_document_migration_publish_failed_rolled_back"
                                   : "project_document_migration_rollback_failed";
            if (!diagnostic.empty()) result.diagnostics.push_back(std::move(diagnostic));
            return result;
        }
    }
    result.success = true;
    result.code = "project_document_migration_applied";
    return result;
}

} // namespace urpg::project
