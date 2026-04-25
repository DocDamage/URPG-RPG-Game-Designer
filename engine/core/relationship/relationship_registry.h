#pragma once

#include <nlohmann/json.hpp>

#include <map>
#include <string>
#include <vector>

namespace urpg::relationship {

struct ReputationGate {
    std::string content_id;
    std::string subject_id;
    int minimum = 0;
};

class RelationshipRegistry {
public:
    void setAffinity(const std::string& subject_id, int value);
    int affinity(const std::string& subject_id) const;
    void addGate(ReputationGate gate);
    std::vector<std::string> availableContent() const;
    nlohmann::json serialize() const;
    static RelationshipRegistry deserialize(const nlohmann::json& json);

private:
    std::map<std::string, int> affinity_;
    std::vector<ReputationGate> gates_;
};

} // namespace urpg::relationship
