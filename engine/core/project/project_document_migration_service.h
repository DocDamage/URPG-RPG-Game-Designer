#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace urpg::project {

struct ProjectDocumentMigrationRecord {
    std::string document_type;
    std::filesystem::path document_path;
    std::filesystem::path backup_path;
    bool changed = false;
};

struct ProjectDocumentMigrationServiceResult {
    bool success = false;
    bool dry_run = false;
    std::string code;
    std::vector<ProjectDocumentMigrationRecord> records;
    std::vector<std::string> diagnostics;
};

// Applies all registered forward migrations as one project-open transaction.
// Every original is durably backed up before the first owner is published;
// a publication failure restores all earlier owners before returning.
ProjectDocumentMigrationServiceResult migrateProjectDocumentsOnDisk(
    const std::filesystem::path& project_root, bool dry_run = false,
    std::function<bool(const std::filesystem::path&)> before_publish = {});

} // namespace urpg::project
