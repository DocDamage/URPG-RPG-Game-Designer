#pragma once

#include <nlohmann/json.hpp>

#include <set>
#include <string>
#include <vector>

namespace urpg::tutorial {

class TutorialLesson {
public:
    TutorialLesson() = default;
    TutorialLesson(std::string id, std::vector<std::string> steps);

    const std::string& id() const;
    bool completeStep(const std::string& step_id);
    bool isComplete() const;
    void reset();
    nlohmann::json serialize() const;
    static TutorialLesson deserialize(const nlohmann::json& json);

private:
    std::string id_;
    std::vector<std::string> steps_;
    std::set<std::string> completed_steps_;
};

} // namespace urpg::tutorial
