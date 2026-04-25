#include "engine/core/tutorial/tutorial_lesson.h"

#include <algorithm>
#include <utility>

namespace urpg::tutorial {

TutorialLesson::TutorialLesson(std::string id, std::vector<std::string> steps)
    : id_(std::move(id)), steps_(std::move(steps)) {}

const std::string& TutorialLesson::id() const {
    return id_;
}

bool TutorialLesson::completeStep(const std::string& step_id) {
    if (std::find(steps_.begin(), steps_.end(), step_id) == steps_.end()) {
        return false;
    }
    completed_steps_.insert(step_id);
    return true;
}

bool TutorialLesson::isComplete() const {
    return !steps_.empty() && completed_steps_.size() == steps_.size();
}

void TutorialLesson::reset() {
    completed_steps_.clear();
}

nlohmann::json TutorialLesson::serialize() const {
    return {
        {"schema_version", "urpg.tutorial_lesson.v1"},
        {"id", id_},
        {"steps", steps_},
        {"completed_steps", completed_steps_},
        {"complete", isComplete()},
    };
}

TutorialLesson TutorialLesson::deserialize(const nlohmann::json& json) {
    TutorialLesson lesson;
    if (!json.is_object()) {
        return lesson;
    }
    lesson.id_ = json.value("id", "");
    if (json.contains("steps") && json["steps"].is_array()) {
        lesson.steps_ = json["steps"].get<std::vector<std::string>>();
    }
    if (json.contains("completed_steps") && json["completed_steps"].is_array()) {
        for (const auto& step : json["completed_steps"]) {
            if (step.is_string()) {
                lesson.completed_steps_.insert(step.get<std::string>());
            }
        }
    }
    return lesson;
}

} // namespace urpg::tutorial
