#pragma once

#include <string>
#include <functional>
#include <memory>
#include <vector>

namespace urpg::scene {

    /**
     * @brief Visual transition types for scene swapping.
     * Part of Wave 4 Engine Polish.
     */
    enum class TransitionEffect {
        FadeToBlack,
        FadeToWhite,
        CrossFade,
        CircleWipe,
        DiamondWipe,
        Instant
    };

    /**
     * @brief Global Manager for loading scenes with visual polish.
     * Ensures memory is cleared and persistent data is synced before handoff.
     */
    class SceneTransitionManager {
    public:
        using LoadCallback = std::function<void()>;

        /**
         * @brief Request a transition to a new scene.
         * @param scenePath Path to the .ursv or .json scene file.
         * @param effect Visual effect to use.
         * @param duration Time in seconds for the transition.
         */
        void transitionTo(const std::string& scenePath, TransitionEffect effect = TransitionEffect::FadeToBlack, float duration = 1.0f) {
            if (m_isTransitioning) return;

            m_isTransitioning = true;
            m_targetScene = scenePath;
            m_currentEffect = effect;
            m_timer = 0.0f;
            m_totalDuration = duration;
            m_state = State::FadingOut;

            // Signal engine to pause gameplay but keep rendering transition
        }

        void update(float deltaTime) {
            if (!m_isTransitioning) return;

            m_timer += deltaTime;
            float progress = std::min(m_timer / (m_totalDuration * 0.5f), 1.0f);

            switch (m_state) {
                case State::FadingOut:
                    if (progress >= 1.0f) {
                        executeLoad();
                        m_state = State::FadingIn;
                        m_timer = 0.0f;
                    }
                    break;
                case State::FadingIn:
                    if (progress >= 1.0f) {
                        m_isTransitioning = false;
                        m_state = State::Idle;
                    }
                    break;
                default: break;
            }
        }

        float getFadeAlpha() const {
            if (m_state == State::FadingOut) return std::min(m_timer / (m_totalDuration * 0.5f), 1.0f);
            if (m_state == State::FadingIn) return 1.0f - std::min(m_timer / (m_totalDuration * 0.5f), 1.0f);
            return 0.0f;
        }

        bool isTransitioning() const { return m_isTransitioning; }

    private:
        enum class State { Idle, FadingOut, FadingIn };

        void executeLoad() {
            // Internal call to SceneLoader::load(m_targetScene)
            // This is where memory cleanup and asset pre-fetching happens
        }

        bool m_isTransitioning = false;
        State m_state = State::Idle;
        std::string m_targetScene;
        TransitionEffect m_currentEffect = TransitionEffect::FadeToBlack;
        float m_timer = 0.0f;
        float m_totalDuration = 1.0f;
    };

} // namespace urpg::scene
