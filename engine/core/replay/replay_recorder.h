#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace urpg::replay {

struct ReplayInput {
    int64_t tick = 0;
    std::string action;
    nlohmann::json payload = nlohmann::json::object();

    bool operator==(const ReplayInput& other) const = default;
};

struct ReplayArtifact {
    std::string id;
    uint64_t seed = 0;
    std::string project_version;
    std::set<std::string> labels;
    std::vector<ReplayInput> input_log;
    std::map<int64_t, std::string> state_hashes;

    nlohmann::json toJson() const;
    static ReplayArtifact fromJson(const nlohmann::json& json);
};

class ReplayRecorder {
public:
    ReplayRecorder(uint64_t seed, std::string project_version);

    void addLabel(std::string label);
    void recordInput(int64_t tick,
                     std::string action,
                     nlohmann::json payload,
                     const nlohmann::json& deterministic_state);
    ReplayArtifact finish(std::string id) const;

    static std::string hashState(const nlohmann::json& deterministic_state);

private:
    uint64_t seed_ = 0;
    std::string project_version_;
    std::set<std::string> labels_;
    std::vector<ReplayInput> input_log_;
    std::map<int64_t, std::string> state_hashes_;
};

} // namespace urpg::replay
