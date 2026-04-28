#include "engine/core/relationship/relationship_affinity.h"

#include <set>

namespace urpg::relationship {
namespace {

AffinityRule ruleFromJson(const nlohmann::json& json) {
    return {json.value("id", ""),
            json.value("subject_id", ""),
            json.value("source", ""),
            json.value("tag", ""),
            json.value("delta", 0)};
}

nlohmann::json ruleToJson(const AffinityRule& rule) {
    return {{"id", rule.id},
            {"subject_id", rule.subject_id},
            {"source", rule.source},
            {"tag", rule.tag},
            {"delta", rule.delta}};
}

ReputationGate gateFromJson(const nlohmann::json& json) {
    return {json.value("content_id", ""), json.value("subject_id", ""), json.value("minimum", 0)};
}

nlohmann::json gateToJson(const ReputationGate& gate) {
    return {{"content_id", gate.content_id}, {"subject_id", gate.subject_id}, {"minimum", gate.minimum}};
}

} // namespace

RelationshipAffinityDocument RelationshipAffinityDocument::fromJson(const nlohmann::json& json) {
    RelationshipAffinityDocument document;
    document.document_id = json.value("document_id", "");
    if (json.contains("rules") && json["rules"].is_array()) {
        for (const auto& rule_json : json["rules"]) {
            document.rules.push_back(ruleFromJson(rule_json));
        }
    }
    if (json.contains("gates") && json["gates"].is_array()) {
        for (const auto& gate_json : json["gates"]) {
            document.gates.push_back(gateFromJson(gate_json));
        }
    }
    return document;
}

nlohmann::json RelationshipAffinityDocument::toJson() const {
    nlohmann::json rule_array = nlohmann::json::array();
    for (const auto& rule : rules) {
        rule_array.push_back(ruleToJson(rule));
    }
    nlohmann::json gate_array = nlohmann::json::array();
    for (const auto& gate : gates) {
        gate_array.push_back(gateToJson(gate));
    }
    return {{"schema_version", "urpg.relationship_affinity.v1"},
            {"document_id", document_id},
            {"rules", std::move(rule_array)},
            {"gates", std::move(gate_array)}};
}

std::vector<AffinityDiagnostic> RelationshipAffinityDocument::validate() const {
    std::vector<AffinityDiagnostic> diagnostics;
    if (document_id.empty()) {
        diagnostics.push_back({"missing_document_id", "Relationship affinity document requires an id.", ""});
    }
    std::set<std::string> rule_ids;
    for (const auto& rule : rules) {
        if (rule.id.empty() || rule.subject_id.empty() || rule.source.empty() || rule.tag.empty()) {
            diagnostics.push_back({"invalid_affinity_rule", "Affinity rule requires id, subject, source, and tag.",
                                   rule.id});
        }
        if (!rule.id.empty() && !rule_ids.insert(rule.id).second) {
            diagnostics.push_back({"duplicate_affinity_rule", "Affinity rule id is duplicated.", rule.id});
        }
    }
    for (const auto& gate : gates) {
        if (gate.content_id.empty() || gate.subject_id.empty()) {
            diagnostics.push_back({"invalid_affinity_gate", "Affinity gate requires content and subject ids.",
                                   gate.content_id});
        }
    }
    return diagnostics;
}

AffinityPreview RelationshipAffinityDocument::preview(const RelationshipRegistry& registry,
                                                      const AffinityEvent& event) const {
    RelationshipRegistry copy = RelationshipRegistry::deserialize(registry.serialize());
    return apply(copy, event);
}

AffinityPreview RelationshipAffinityDocument::apply(RelationshipRegistry& registry, const AffinityEvent& event) const {
    AffinityPreview preview;
    preview.diagnostics = validate();
    if (!preview.diagnostics.empty()) {
        return preview;
    }
    for (const auto& gate : gates) {
        registry.addGate(gate);
    }
    for (const auto& rule : rules) {
        if (rule.source != event.source || rule.tag != event.tag) {
            continue;
        }
        registry.setAffinity(rule.subject_id, registry.affinity(rule.subject_id) + rule.delta);
        preview.applied_rule_ids.push_back(rule.id);
    }
    preview.unlocked_content_ids = registry.availableContent();
    preview.projected_affinity = registry.serialize()["affinity"];
    return preview;
}

nlohmann::json affinityDiagnosticToJson(const AffinityDiagnostic& diagnostic) {
    return {{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}};
}

nlohmann::json affinityPreviewToJson(const AffinityPreview& preview) {
    nlohmann::json diagnostics = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        diagnostics.push_back(affinityDiagnosticToJson(diagnostic));
    }
    return {{"applied_rule_ids", preview.applied_rule_ids},
            {"unlocked_content_ids", preview.unlocked_content_ids},
            {"projected_affinity", preview.projected_affinity},
            {"diagnostics", std::move(diagnostics)}};
}

} // namespace urpg::relationship
