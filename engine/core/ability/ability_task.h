#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>

namespace urpg {

namespace ability {
    class AbilitySystemComponent;
}

/**
 * Base class for asynchronous actions during an ability (e.g., waiting for time, an input, or an event).
 */
class AbilityTask {
public:
    virtual ~AbilityTask() = default;

    virtual void activate(ability::AbilitySystemComponent& asc) = 0;
    virtual void tick(float deltaTime) = 0;
    virtual bool isFinished() const = 0;

    std::function<void()> onFinished;
};

/**
 * A standard task that waits for a specific duration.
 */
class AbilityTask_WaitTime : public AbilityTask {
public:
    AbilityTask_WaitTime(float duration) : m_duration(duration) {}

    void activate(ability::AbilitySystemComponent& asc) override {
        (void)asc;
        m_elapsed = 0.0f;
    }

    void tick(float deltaTime) override {
        if (m_elapsed < m_duration) {
            m_elapsed += deltaTime;
            if (m_elapsed >= m_duration && onFinished) {
                onFinished();
            }
        }
    }

    bool isFinished() const override { return m_elapsed >= m_duration; }

private:
    float m_duration;
    float m_elapsed = 0.0f;
};

} // namespace urpg
