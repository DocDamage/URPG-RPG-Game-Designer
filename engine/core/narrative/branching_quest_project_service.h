#pragma once

#include "engine/core/narrative/branching_quest_document.h"

#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::narrative {

struct BranchingQuestProjectResult {
    bool success = false;
    std::string code;
    std::string message;
    std::filesystem::path document_path;
    std::filesystem::path closure_path;

    BranchingQuestProjectResult() = default;
    BranchingQuestProjectResult(bool value, std::string result_code, std::string result_message,
                                std::filesystem::path result_document_path = {},
                                std::filesystem::path result_closure_path = {})
        : success(value), code(std::move(result_code)), message(std::move(result_message)),
          document_path(std::move(result_document_path)), closure_path(std::move(result_closure_path)) {}
};

class BranchingQuestProjectService {
public:
    BranchingQuestProjectResult save(const std::filesystem::path& project_root,
                                     const BranchingQuestDocument& document) const;
    std::optional<BranchingQuestDocument> load(const std::filesystem::path& project_root,
                                               std::string_view document_id,
                                               std::vector<BranchingQuestDiagnostic>* diagnostics = nullptr) const;
    BranchingQuestProjectResult publishPackage(const std::filesystem::path& package_root,
                                               const BranchingQuestDocument& document) const;
    std::optional<BranchingQuestDocument> loadInstalledPackage(
        const std::filesystem::path& package_root, std::string_view document_id,
        std::vector<BranchingQuestDiagnostic>* diagnostics = nullptr) const;
};

} // namespace urpg::narrative
