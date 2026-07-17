#pragma once

#include <filesystem>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
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
    uint64_t indexRevision() const { return index_revision_; }
    size_t lastQueryCandidateCount() const { return last_query_candidate_count_; }

private:
    using ObjectKey = std::pair<std::string, std::string>;
    void rebuildQueryIndexes();
    std::vector<ProjectReferenceEdge> queryPosting(const std::map<ObjectKey, std::vector<size_t>>& index,
                                                   std::string_view object_type,
                                                   std::string_view object_id) const;

    std::vector<ProjectReferenceEdge> edges_;
    std::map<ObjectKey, std::vector<size_t>> inbound_index_;
    std::map<ObjectKey, std::vector<size_t>> outbound_index_;
    std::map<ObjectKey, std::vector<size_t>> package_inclusion_index_;
    uint64_t index_revision_ = 0;
    mutable size_t last_query_candidate_count_ = 0;
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
