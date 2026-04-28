#pragma once

#include "engine/core/relationship/relationship_registry.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::relationship {

struct AffinityRule {
    std::string id;
    std::string subject_id;
    std::string source;
    std::string tag;
    int delta = 0;
};

struct AffinityEvent {
    std::string source;
    std::string tag;
};

struct AffinityDiagnostic {
    std::string code;
    std::string message;
    std::string id;
};

struct AffinityPreview {
    std::vector<std::string> applied_rule_ids;
    std::vector<std::string> unlocked_content_ids;
    std::vector<AffinityDiagnostic> diagnostics;
    nlohmann::json projected_affinity;
};

class RelationshipAffinityDocument {
public:
    std::string document_id;
    std::vector<AffinityRule> rules;
    std::vector<ReputationGate> gates;

    static RelationshipAffinityDocument fromJson(const nlohmann::json& json);
    nlohmann::json toJson() const;
    std::vector<AffinityDiagnostic> validate() const;
    AffinityPreview preview(const RelationshipRegistry& registry, const AffinityEvent& event) const;
    AffinityPreview apply(RelationshipRegistry& registry, const AffinityEvent& event) const;
};

nlohmann::json affinityDiagnosticToJson(const AffinityDiagnostic& diagnostic);
nlohmann::json affinityPreviewToJson(const AffinityPreview& preview);

} // namespace urpg::relationship
