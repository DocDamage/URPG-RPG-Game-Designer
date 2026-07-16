#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json_fwd.hpp>

namespace urpg::project {

struct ProjectReferenceEdge {
    std::string source_type;
    std::string source_id;
    std::string target_type;
    std::string target_id;
    std::string reference_type;
    std::filesystem::path document_path;
    std::string local_id;
    bool package_inclusion = false;

    bool operator==(const ProjectReferenceEdge&) const = default;
};

struct ProjectReferenceDocument {
    std::filesystem::path document_path;
    std::vector<ProjectReferenceEdge> edges;
};

struct ProjectReferenceUpdateResult {
    bool success = false;
    std::string code;
    std::string message;
};

struct ProjectReferenceExtractionResult {
    bool success = false;
    std::string code;
    ProjectReferenceDocument document;
    std::vector<std::string> diagnostics;
};

struct ProjectReferenceQueryResult {
    bool success = false;
    std::string code;
    std::string object_type;
    std::string object_id;
    std::vector<ProjectReferenceEdge> matches;
};

// Deterministic, read-only graph materialized from authoritative domain
// documents. Domain owners produce edges; this index never mutates documents.
class ProjectReferenceIndex {
public:
    ProjectReferenceUpdateResult rebuild(const std::vector<ProjectReferenceDocument>& documents);
    ProjectReferenceUpdateResult replaceDocument(const ProjectReferenceDocument& document);
    void removeDocument(const std::filesystem::path& document_path);

    const std::vector<ProjectReferenceEdge>& edges() const { return edges_; }
    std::vector<ProjectReferenceEdge> inbound(std::string_view target_type, std::string_view target_id) const;
    std::vector<ProjectReferenceEdge> outbound(std::string_view source_type, std::string_view source_id) const;
    std::vector<ProjectReferenceEdge> whyIncluded(std::string_view target_type, std::string_view target_id) const;
    ProjectReferenceQueryResult findUses(std::string_view target_type, std::string_view target_id) const;
    ProjectReferenceQueryResult findReferences(std::string_view source_type, std::string_view source_id) const;
    ProjectReferenceQueryResult explainInclusion(std::string_view target_type, std::string_view target_id) const;

private:
    std::vector<ProjectReferenceEdge> edges_;
};

struct ProjectReferenceBuildResult {
    bool success = false;
    ProjectReferenceIndex index;
    std::vector<ProjectReferenceDocument> documents;
    std::vector<std::string> diagnostics;
};

ProjectReferenceBuildResult buildProjectReferenceIndex(const std::filesystem::path& project_root);

ProjectReferenceExtractionResult extractPerspective2DReferences(const std::filesystem::path& document_path,
                                                                const nlohmann::json& document);
ProjectReferenceExtractionResult extractDialogueReferences(const std::filesystem::path& document_path,
                                                            const nlohmann::json& document);
ProjectReferenceExtractionResult extractQuestReferences(const std::filesystem::path& document_path,
                                                         const nlohmann::json& document);
ProjectReferenceExtractionResult extractMenuReferences(const std::filesystem::path& document_path,
                                                        const nlohmann::json& document);
ProjectReferenceExtractionResult extractDatabaseReferences(const std::filesystem::path& document_path,
                                                            const nlohmann::json& document);
ProjectReferenceExtractionResult extractGameplayRecipeReferences(const std::filesystem::path& document_path,
                                                                  const nlohmann::json& document);
ProjectReferenceExtractionResult extractMzPluginLockReferences(const std::filesystem::path& document_path,
                                                               const nlohmann::json& document);
ProjectReferenceExtractionResult extractModManifestReferences(const std::filesystem::path& document_path,
                                                              const nlohmann::json& document);

} // namespace urpg::project
