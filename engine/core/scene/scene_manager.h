#pragma once

#include "engine/core/input/input_core.h"
#include <string>
#include <vector>
#include <memory>

namespace urpg::scene {

/**
 * @brief Native representation of possible engine scenes.
 */
enum class SceneType : uint8_t {
    BOOT,
    TITLE,
    MAP,
    BATTLE,
    MENU,
    GAMEOVER
};

/**
 * @brief Abstract base class for all engine scenes.
 */
class GameScene {
public:
    virtual ~GameScene() = default;
    
    virtual SceneType getType() const = 0;
    virtual std::string getName() const = 0;

    // Lifecycle hooks
    virtual void onCreate() {}
    virtual void onStart() {}
    virtual void onUpdate(float deltaTime) {}
    virtual void handleInput(const urpg::input::InputCore& input) {}
    virtual void onStop() {}
    virtual void onDestroy() {}

    bool isPaused() const { return m_isPaused; }
    void setPaused(bool paused) { m_isPaused = paused; }

protected:
    bool m_isPaused = false;
};

/**
 * @brief Authoritative manager for the engine's scene stack.
 * 
 * This system synchronizes Input, Audio, and UI context based on the
 * active gameplay scene.
 */
class SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    void pushScene(std::shared_ptr<GameScene> scene) {
        if (!m_stack.empty()) {
            m_stack.back()->onStop();
        }
        m_stack.push_back(scene);
        scene->onCreate();
        scene->onStart();
    }

    void popScene() {
        if (m_stack.empty()) return;
        
        auto oldScene = m_stack.back();
        oldScene->onStop();
        oldScene->onDestroy();
        m_stack.pop_back();

        if (!m_stack.empty()) {
            m_stack.back()->onStart();
        }
    }

    void gotoScene(std::shared_ptr<GameScene> scene) {
        while (!m_stack.empty()) {
            popScene();
        }
        pushScene(scene);
    }

    std::shared_ptr<GameScene> getActiveScene() const {
        return m_stack.empty() ? nullptr : m_stack.back();
    }

    size_t stackSize() const { return m_stack.size(); }

    void update(float deltaTime) {
        if (!m_stack.empty() && !m_stack.back()->isPaused()) {
            m_stack.back()->onUpdate(deltaTime);
        }
    }

private:
    std::vector<std::shared_ptr<GameScene>> m_stack;
};

} // namespace urpg::scene
