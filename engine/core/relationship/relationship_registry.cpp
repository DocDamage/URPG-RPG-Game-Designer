#include "engine/core/relationship/relationship_registry.h"

#include <algorithm>
#include <utility>

namespace urpg::relationship {

namespace {

int clampAffinity(int value) {
    return std::clamp(value, -100, 100);
}

} // namespace

void RelationshipRegistry::setAffinity(const std::string& subject_id, int value) {
    affinity_[subject_id] = clampAffinity(value);
}

int RelationshipRegistry::affinity(const std::string& subject_id) const {
    const auto it = affinity_.find(subject_id);
    return it == affinity_.end() ? 0 : it->second;
}

void RelationshipRegistry::addGate(ReputationGate gate) {
    gates_.push_back(std::move(gate));
}

std::vector<std::string> RelationshipRegistry::availableContent() const {
    std::vector<std::string> content;
    for (const auto& gate : gates_) {
        if (affinity(gate.subject_id) >= gate.minimum) {
            content.push_back(gate.content_id);
        }
    }
    std::sort(content.begin(), content.end());
    return content;
}

nlohmann::json RelationshipRegistry::serialize() const {
    nlohmann::json gates = nlohmann::json::array();
    for (const auto& gate : gates_) {
        gates.push_back({{"content_id", gate.content_id}, {"subject_id", gate.subject_id}, {"minimum", gate.minimum}});
    }
    return {{"schema_version", "urpg.relationship_registry.v1"}, {"affinity", affinity_}, {"gates", gates}};
}

RelationshipRegistry RelationshipRegistry::deserialize(const nlohmann::json& json) {
    RelationshipRegistry registry;
    if (json.contains("affinity") && json["affinity"].is_object()) {
        for (auto it = json["affinity"].begin(); it != json["affinity"].end(); ++it) {
            if (it.value().is_number_integer()) {
                registry.setAffinity(it.key(), it.value().get<int>());
            }
        }
    }
    if (json.contains("gates") && json["gates"].is_array()) {
        for (const auto& gate_json : json["gates"]) {
            registry.addGate({
                gate_json.value("content_id", ""),
                gate_json.value("subject_id", ""),
                gate_json.value("minimum", 0),
            });
        }
    }
    return registry;
}

} // namespace urpg::relationship
