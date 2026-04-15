#include "clock.h"

namespace urpg {

Clock::Clock() {
    m_startTime = std::chrono::high_resolution_clock::now();
    m_lastTime = m_startTime;
}

void Clock::reset() {
    m_lastTime = std::chrono::high_resolution_clock::now();
}

float Clock::getDelta() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - m_lastTime;
    m_lastTime = currentTime;
    return elapsed.count();
}

float Clock::getTotalTime() const {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - m_startTime;
    return elapsed.count();
}

} // namespace urpg
