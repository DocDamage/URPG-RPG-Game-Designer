#include "engine/core/narrative/branching_quest_project_service.h"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace urpg::narrative {
namespace {

bool safeId(const std::string_view id) {
    return !id.empty() && id.size() <= 120 &&
           std::ranges::all_of(id, [](const unsigned char value) {
               return std::isalnum(value) != 0 || value == '.' || value == '_' || value == '-';
           });
}

bool writeJson(const std::filesystem::path& target, const nlohmann::json& value, std::string& error) {
    std::error_code filesystem_error;
    std::filesystem::create_directories(target.parent_path(), filesystem_error);
    if (filesystem_error) {
        error = "Unable to create the narrative directory: " + filesystem_error.message();
        return false;
    }
    auto temporary = target;
    temporary += ".tmp";
    std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
    if (!output) {
        error = "Unable to open the temporary narrative document.";
        return false;
    }
    output << value.dump(2) << '\n';
    output.close();
    if (!output) {
        std::filesystem::remove(temporary);
        error = "Unable to flush the temporary narrative document.";
        return false;
    }
    auto backup = target;
    backup += ".bak";
    const bool replacing = std::filesystem::exists(target);
    if (replacing) {
        std::filesystem::remove(backup, filesystem_error);
        filesystem_error.clear();
        std::filesystem::rename(target, backup, filesystem_error);
        if (filesystem_error) {
            std::filesystem::remove(temporary);
            error = "Unable to stage the prior narrative document: " + filesystem_error.message();
            return false;
        }
    }
    std::filesystem::rename(temporary, target, filesystem_error);
    if (filesystem_error) {
        error = "Unable to publish the narrative document: " + filesystem_error.message();
        std::filesystem::remove(temporary);
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup, target, restore_error);
            if (restore_error) error += " Prior document restoration failed: " + restore_error.message();
        }
        return false;
    }
    if (replacing) std::filesystem::remove(backup, filesystem_error);
    return true;
}

std::optional<BranchingQuestDocument> readDocument(const std::filesystem::path& path,
                                                   std::vector<BranchingQuestDiagnostic>* diagnostics) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        if (diagnostics) diagnostics->push_back({"narrative_document_missing", "Narrative document is missing.", ""});
        return std::nullopt;
    }
    const auto json = nlohmann::json::parse(input, nullptr, false);
    auto document = BranchingQuestDocument::fromJson(json);
    if (!document) {
        if (diagnostics) diagnostics->push_back({"narrative_document_invalid", "Narrative document is invalid.", ""});
        return std::nullopt;
    }
    const auto findings = document->validate();
    if (!findings.empty()) {
        if (diagnostics) *diagnostics = findings;
        return std::nullopt;
    }
    return document;
}

std::filesystem::path projectPath(const std::filesystem::path& root, const std::string_view id) {
    return root / "content" / "narrative" / (std::string(id) + ".json");
}

std::filesystem::path installedDirectory(const std::filesystem::path& root, const std::string_view id) {
    return root / "content" / "narrative" / "branching_quests" / std::string(id);
}

} // namespace

BranchingQuestProjectResult BranchingQuestProjectService::save(const std::filesystem::path& project_root,
                                                               const BranchingQuestDocument& document) const {
    if (project_root.empty() || !safeId(document.id)) {
        return {false, "narrative_save_target_invalid", "Project root and stable narrative ID are required."};
    }
    const auto diagnostics = document.validate();
    if (!diagnostics.empty()) {
        return {false, diagnostics.front().code, diagnostics.front().message};
    }
    const auto target = projectPath(project_root, document.id);
    std::string error;
    if (!writeJson(target, document.toJson(), error)) return {false, "narrative_save_failed", std::move(error)};
    return {true, "narrative_saved", "Saved the branching quest project document.", target, {}};
}

std::optional<BranchingQuestDocument> BranchingQuestProjectService::load(
    const std::filesystem::path& project_root, const std::string_view document_id,
    std::vector<BranchingQuestDiagnostic>* diagnostics) const {
    if (!safeId(document_id)) {
        if (diagnostics) diagnostics->push_back({"narrative_id_invalid", "Narrative ID is invalid.", ""});
        return std::nullopt;
    }
    return readDocument(projectPath(project_root, document_id), diagnostics);
}

BranchingQuestProjectResult BranchingQuestProjectService::publishPackage(
    const std::filesystem::path& package_root, const BranchingQuestDocument& document) const {
    if (package_root.empty() || !safeId(document.id)) {
        return {false, "narrative_package_target_invalid", "Package root and stable narrative ID are required."};
    }
    const auto diagnostics = document.validate();
    if (!diagnostics.empty()) return {false, diagnostics.front().code, diagnostics.front().message};

    const auto target_directory = installedDirectory(package_root, document.id);
    auto staging_directory = target_directory;
    staging_directory += ".staging";
    std::error_code filesystem_error;
    if (std::filesystem::exists(staging_directory)) std::filesystem::remove_all(staging_directory, filesystem_error);
    if (filesystem_error) return {false, "narrative_package_stage_failed", filesystem_error.message()};
    std::filesystem::create_directories(staging_directory, filesystem_error);
    if (filesystem_error) return {false, "narrative_package_stage_failed", filesystem_error.message()};

    std::string error;
    const auto document_path = staging_directory / "document.json";
    const auto closure_path = staging_directory / "package_closure.json";
    if (!writeJson(document_path, document.toJson(), error) ||
        !writeJson(closure_path, packageClosureToJson(document.packageClosure()), error)) {
        std::filesystem::remove_all(staging_directory, filesystem_error);
        return {false, "narrative_package_write_failed", std::move(error)};
    }

    auto backup_directory = target_directory;
    backup_directory += ".backup";
    std::filesystem::create_directories(target_directory.parent_path(), filesystem_error);
    if (filesystem_error) return {false, "narrative_package_publish_failed", filesystem_error.message()};
    const bool replacing = std::filesystem::exists(target_directory);
    if (replacing) {
        std::filesystem::remove_all(backup_directory, filesystem_error);
        filesystem_error.clear();
        std::filesystem::rename(target_directory, backup_directory, filesystem_error);
        if (filesystem_error) return {false, "narrative_package_publish_failed", filesystem_error.message()};
    }
    std::filesystem::rename(staging_directory, target_directory, filesystem_error);
    if (filesystem_error) {
        if (replacing) {
            std::error_code restore_error;
            std::filesystem::rename(backup_directory, target_directory, restore_error);
        }
        return {false, "narrative_package_publish_failed", filesystem_error.message()};
    }
    if (replacing) std::filesystem::remove_all(backup_directory, filesystem_error);
    return {true, "narrative_package_published", "Published the branching quest and semantic closure.",
            target_directory / "document.json", target_directory / "package_closure.json"};
}

std::optional<BranchingQuestDocument> BranchingQuestProjectService::loadInstalledPackage(
    const std::filesystem::path& package_root, const std::string_view document_id,
    std::vector<BranchingQuestDiagnostic>* diagnostics) const {
    if (!safeId(document_id)) {
        if (diagnostics) diagnostics->push_back({"narrative_id_invalid", "Narrative ID is invalid.", ""});
        return std::nullopt;
    }
    const auto directory = installedDirectory(package_root, document_id);
    auto document = readDocument(directory / "document.json", diagnostics);
    if (!document) return std::nullopt;
    std::ifstream closure_input(directory / "package_closure.json", std::ios::binary);
    const auto closure = nlohmann::json::parse(closure_input, nullptr, false);
    if (closure != packageClosureToJson(document->packageClosure())) {
        if (diagnostics) diagnostics->push_back({"narrative_package_closure_mismatch",
                                                  "Installed narrative closure does not match its document.",
                                                  document->id});
        return std::nullopt;
    }
    return document;
}

} // namespace urpg::narrative
