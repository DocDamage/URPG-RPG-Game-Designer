#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <utility>

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

/**
 * Waits until the runtime reports a matching input action, or until an optional timeout expires.
 */
class AbilityTask_WaitInput : public AbilityTask {
public:
    explicit AbilityTask_WaitInput(std::string inputName, float timeoutSeconds = -1.0f)
        : m_inputName(std::move(inputName)), m_timeoutSeconds(timeoutSeconds) {}

    void activate(ability::AbilitySystemComponent& asc) override {
        (void)asc;
        m_elapsed = 0.0f;
        m_finished = false;
        m_matchedInput.clear();
        m_completionReason.clear();
    }

    void tick(float deltaTime) override {
        if (m_finished || m_timeoutSeconds < 0.0f) {
            return;
        }

        m_elapsed += deltaTime;
        if (m_elapsed >= m_timeoutSeconds) {
            finish("timeout");
        }
    }

    bool isFinished() const override { return m_finished; }

    bool receiveInput(const std::string& inputName) {
        if (m_finished) {
            return false;
        }
        if (!m_inputName.empty() && inputName != m_inputName) {
            return false;
        }

        m_matchedInput = inputName;
        finish("input");
        return true;
    }

    const std::string& expectedInput() const { return m_inputName; }
    const std::string& matchedInput() const { return m_matchedInput; }
    const std::string& completionReason() const { return m_completionReason; }

private:
    void finish(std::string reason) {
        if (m_finished) {
            return;
        }
        m_finished = true;
        m_completionReason = std::move(reason);
        if (onFinished) {
            onFinished();
        }
    }

    std::string m_inputName;
    float m_timeoutSeconds = -1.0f;
    float m_elapsed = 0.0f;
    bool m_finished = false;
    std::string m_matchedInput;
    std::string m_completionReason;
};

/**
 * Waits until the runtime reports a matching named event, or until an optional timeout expires.
 */
class AbilityTask_WaitEvent : public AbilityTask {
public:
    explicit AbilityTask_WaitEvent(std::string eventName, float timeoutSeconds = -1.0f)
        : m_eventName(std::move(eventName)), m_timeoutSeconds(timeoutSeconds) {}

    void activate(ability::AbilitySystemComponent& asc) override {
        (void)asc;
        m_elapsed = 0.0f;
        m_finished = false;
        m_matchedEvent.clear();
        m_payload.clear();
        m_completionReason.clear();
    }

    void tick(float deltaTime) override {
        if (m_finished || m_timeoutSeconds < 0.0f) {
            return;
        }

        m_elapsed += deltaTime;
        if (m_elapsed >= m_timeoutSeconds) {
            finish("timeout");
        }
    }

    bool isFinished() const override { return m_finished; }

    bool dispatchEvent(const std::string& eventName, std::string payload = {}) {
        if (m_finished) {
            return false;
        }
        if (!m_eventName.empty() && eventName != m_eventName) {
            return false;
        }

        m_matchedEvent = eventName;
        m_payload = std::move(payload);
        finish("event");
        return true;
    }

    const std::string& expectedEvent() const { return m_eventName; }
    const std::string& matchedEvent() const { return m_matchedEvent; }
    const std::string& payload() const { return m_payload; }
    const std::string& completionReason() const { return m_completionReason; }

private:
    void finish(std::string reason) {
        if (m_finished) {
            return;
        }
        m_finished = true;
        m_completionReason = std::move(reason);
        if (onFinished) {
            onFinished();
        }
    }

    std::string m_eventName;
    float m_timeoutSeconds = -1.0f;
    float m_elapsed = 0.0f;
    bool m_finished = false;
    std::string m_matchedEvent;
    std::string m_payload;
    std::string m_completionReason;
};

struct AbilityProjectileCollision {
    std::string projectileId;
    std::string targetId;
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * Waits for a deterministic projectile-collision signal supplied by the runtime.
 */
class AbilityTask_WaitProjectileCollision : public AbilityTask {
public:
    AbilityTask_WaitProjectileCollision(std::string projectileId = {},
                                        std::string targetId = {},
                                        float timeoutSeconds = -1.0f)
        : m_projectileId(std::move(projectileId)),
          m_targetId(std::move(targetId)),
          m_timeoutSeconds(timeoutSeconds) {}

    void activate(ability::AbilitySystemComponent& asc) override {
        (void)asc;
        m_elapsed = 0.0f;
        m_finished = false;
        m_hasCollision = false;
        m_collision = {};
        m_completionReason.clear();
    }

    void tick(float deltaTime) override {
        if (m_finished || m_timeoutSeconds < 0.0f) {
            return;
        }

        m_elapsed += deltaTime;
        if (m_elapsed >= m_timeoutSeconds) {
            finish("timeout");
        }
    }

    bool isFinished() const override { return m_finished; }

    bool recordCollision(const AbilityProjectileCollision& collision) {
        if (m_finished) {
            return false;
        }
        if (!m_projectileId.empty() && collision.projectileId != m_projectileId) {
            return false;
        }
        if (!m_targetId.empty() && collision.targetId != m_targetId) {
            return false;
        }

        m_collision = collision;
        m_hasCollision = true;
        finish("projectile_collision");
        return true;
    }

    const std::string& expectedProjectileId() const { return m_projectileId; }
    const std::string& expectedTargetId() const { return m_targetId; }
    bool hasCollision() const { return m_hasCollision; }
    const AbilityProjectileCollision& collision() const { return m_collision; }
    const std::string& completionReason() const { return m_completionReason; }

private:
    void finish(std::string reason) {
        if (m_finished) {
            return;
        }
        m_finished = true;
        m_completionReason = std::move(reason);
        if (onFinished) {
            onFinished();
        }
    }

    std::string m_projectileId;
    std::string m_targetId;
    float m_timeoutSeconds = -1.0f;
    float m_elapsed = 0.0f;
    bool m_finished = false;
    bool m_hasCollision = false;
    AbilityProjectileCollision m_collision;
    std::string m_completionReason;
};

} // namespace urpg
