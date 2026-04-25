#include "engine/core/replay/replay_recorder.h"

#include <algorithm>
#include <iomanip>
#include <sstream>
#include <utility>

namespace urpg::replay {

namespace {

std::string fnv1a64(const std::string& value) {
    uint64_t hash = 14695981039346656037ull;
    for (const auto ch : value) {
        hash ^= static_cast<unsigned char>(ch);
        hash *= 1099511628211ull;
    }
    std::ostringstream stream;
    stream << std::hex << std::setw(16) << std::setfill('0') << hash;
    return stream.str();
}

nlohmann::json inputToJson(const ReplayInput& input) {
    return {{"tick", input.tick}, {"action", input.action}, {"payload", input.payload}};
}

ReplayInput inputFromJson(const nlohmann::json& json) {
    return ReplayInput{
        json.value("tick", int64_t{0}),
        json.value("action", ""),
        json.value("payload", nlohmann::json::object()),
    };
}

} // namespace

nlohmann::json ReplayArtifact::toJson() const {
    nlohmann::json json;
    json["schema_version"] = "urpg.replay.v1";
    json["id"] = id;
    json["seed"] = seed;
    json["project_version"] = project_version;
    json["labels"] = labels;
    json["input_log"] = nlohmann::json::array();
    for (const auto& input : input_log) {
        json["input_log"].push_back(inputToJson(input));
    }
    json["state_hashes"] = nlohmann::json::object();
    for (const auto& [tick, hash] : state_hashes) {
        json["state_hashes"][std::to_string(tick)] = hash;
    }
    return json;
}

ReplayArtifact ReplayArtifact::fromJson(const nlohmann::json& json) {
    ReplayArtifact artifact;
    artifact.id = json.value("id", "");
    artifact.seed = json.value("seed", uint64_t{0});
    artifact.project_version = json.value("project_version", "");
    for (const auto& label : json.value("labels", nlohmann::json::array())) {
        artifact.labels.insert(label.get<std::string>());
    }
    for (const auto& input : json.value("input_log", nlohmann::json::array())) {
        artifact.input_log.push_back(inputFromJson(input));
    }
    const auto state_hashes_json = json.value("state_hashes", nlohmann::json::object());
    for (const auto& [tick, hash] : state_hashes_json.items()) {
        artifact.state_hashes[std::stoll(tick)] = hash.get<std::string>();
    }
    return artifact;
}

ReplayRecorder::ReplayRecorder(uint64_t seed, std::string project_version)
    : seed_(seed), project_version_(std::move(project_version)) {}

void ReplayRecorder::addLabel(std::string label) {
    if (!label.empty()) {
        labels_.insert(std::move(label));
    }
}

void ReplayRecorder::recordInput(int64_t tick,
                                 std::string action,
                                 nlohmann::json payload,
                                 const nlohmann::json& deterministic_state) {
    input_log_.push_back(ReplayInput{tick, std::move(action), std::move(payload)});
    state_hashes_[tick] = hashState(deterministic_state);
}

ReplayArtifact ReplayRecorder::finish(std::string id) const {
    auto inputs = input_log_;
    std::stable_sort(inputs.begin(), inputs.end(), [](const auto& lhs, const auto& rhs) {
        return std::tie(lhs.tick, lhs.action) < std::tie(rhs.tick, rhs.action);
    });
    return ReplayArtifact{std::move(id), seed_, project_version_, labels_, inputs, state_hashes_};
}

std::string ReplayRecorder::hashState(const nlohmann::json& deterministic_state) {
    return fnv1a64(deterministic_state.dump());
}

} // namespace urpg::replay
