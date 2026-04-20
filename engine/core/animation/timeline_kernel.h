#pragma once

#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <algorithm>

namespace urpg::animation {

    /**
     * @brief Types of events that can occur on the timeline.
     */
    enum class TimelineEventType {
        Dialogue,
        MoveEntity,
        PlaySound,
        SetCamera,
        TriggerVFX,
        ScriptCallback
    };

    /**
     * @brief A single keyframe or event on a cutscene track.
     */
    struct TimelineEvent {
        float timestamp;
        TimelineEventType type;
        std::string entityId;
        std::string payload; // JSON or specific ID
        float duration = 0.0f;
    };

    /**
     * @brief A collection of events associated with a specific entity or system.
     */
    struct TimelineTrack {
        std::string trackName;
        std::vector<TimelineEvent> events;

        void sortEvents() {
            std::sort(events.begin(), events.end(), [](const TimelineEvent& a, const TimelineEvent& b) {
                return a.timestamp < b.timestamp;
            });
        }
    };

    /**
     * @brief Core Timeline Editor Kernel.
     * Manages playback and event triggering for cinematic cutscenes.
     */
    class TimelineKernel {
    public:
        void update(float deltaTime) {
            if (!m_isPlaying) return;

            float prevTime = m_currentTime;
            m_currentTime += deltaTime * m_playbackSpeed;

            // Simple event triggering logic
            for (auto& track : m_tracks) {
                for (auto& ev : track.events) {
                    if (ev.timestamp > prevTime && ev.timestamp <= m_currentTime) {
                        triggerEvent(ev);
                    }
                }
            }

            if (m_currentTime >= m_duration) {
                if (m_loop) m_currentTime = 0.0f;
                else m_isPlaying = false;
            }
        }

        void addTrack(const TimelineTrack& track) {
            TimelineTrack sortedTrack = track;
            sortedTrack.sortEvents();
            m_tracks.push_back(sortedTrack);
        }
        
        void play() { m_isPlaying = true; }
        void pause() { m_isPlaying = false; }
        void seek(float time) { m_currentTime = std::clamp(time, 0.0f, m_duration); }
        void setDuration(float duration) { m_duration = std::max(0.0f, duration); }
        const std::vector<TimelineEvent>& getTriggeredEvents() const { return m_triggeredEvents; }

    private:
        void triggerEvent(const TimelineEvent& ev) {
            m_triggeredEvents.push_back(ev);
        }

        std::vector<TimelineTrack> m_tracks;
        std::vector<TimelineEvent> m_triggeredEvents;
        float m_currentTime = 0.0f;
        float m_duration = 10.0f;
        float m_playbackSpeed = 1.0f;
        bool m_isPlaying = false;
        bool m_loop = false;
    };

} // namespace urpg::animation
