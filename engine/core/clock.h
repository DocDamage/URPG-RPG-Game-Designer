#pragma once

#include <chrono>

namespace urpg {

/**
 * @brief High-precision clock for engine timing and delta calculation.
 */
class Clock {
public:
    Clock();

    /**
     * @brief Resets the delta timer.
     */
    void reset();

    /**
     * @brief Calculates delta time (in seconds) since the last call to getDelta().
     */
    float getDelta();

    /**
     * @brief Total time elapsed since clock creation.
     */
    float getTotalTime() const;

private:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_lastTime;
};

} // namespace urpg
