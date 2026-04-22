#pragma once

#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace urpg::testing {

struct HeadlessStep {
    std::string name;
    std::function<bool()> run;
    std::string description;
};

class HeadlessSession {
public:
    void addStep(HeadlessStep step) {
        steps_.push_back(std::move(step));
    }

    [[nodiscard]] bool finished() const {
        return current_step_ >= steps_.size();
    }

    bool update() {
        if (finished()) {
            return true;
        }

        if (steps_[current_step_].run()) {
            ++current_step_;
        }

        return finished();
    }

private:
    std::vector<HeadlessStep> steps_;
    size_t current_step_ = 0;
};

class HeadlessPlayMode {
public:
    void startSession(std::unique_ptr<HeadlessSession> session) {
        session_ = std::move(session);
    }

    void update(float /*delta_time*/) {
        if (session_) {
            session_->update();
        }
    }

    [[nodiscard]] bool isFinished() const {
        return !session_ || session_->finished();
    }

private:
    std::unique_ptr<HeadlessSession> session_;
};

} // namespace urpg::testing
