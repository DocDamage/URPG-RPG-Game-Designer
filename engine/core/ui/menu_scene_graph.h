#pragma once

#include "ui_types.h"
#include "../input/input_core.h"
#include "../audio/audio_core.h"
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <optional>

namespace urpg::ui {

/**
 * @brief Represents a single UI pane or view within a scene.
 */
struct MenuPane {
    std::string id;
    std::string displayName;
    bool isVisible = true;
    bool isActive = false;
    std::vector<MenuCommandMeta> commands;
    int selectedCommandIndex = 0;
    std::string selectionSound = "se_cursor";

    void nextCommand(audio::AudioCore* audio = nullptr) {
        if (commands.empty()) return;
        selectedCommandIndex = (selectedCommandIndex + 1) % commands.size();
        if (audio) audio->playSound(selectionSound, audio::AudioCategory::System);
    }

    void prevCommand(audio::AudioCore* audio = nullptr) {
        if (commands.empty()) return;
        selectedCommandIndex = (selectedCommandIndex - 1 + commands.size()) % commands.size();
        if (audio) audio->playSound(selectionSound, audio::AudioCategory::System);
    }

    const MenuCommandMeta* getSelectedCommand() const {
        if (commands.empty()) return nullptr;
        return &commands[selectedCommandIndex];
    }
};

/**
 * @brief A high-level representation of a menu "Scene" (e.g., MainMenu, Inventory).
 */
class MenuScene {
public:
    MenuScene(const std::string& id) : m_id(id) {}

    const std::string& getId() const { return m_id; }

    void addPane(const MenuPane& pane) {
        m_panes.push_back(pane);
    }

    const std::vector<MenuPane>& getPanes() const { return m_panes; }

    std::optional<MenuPane*> getPane(const std::string& paneId) {
        for (auto& pane : m_panes) {
            if (pane.id == paneId) return &pane;
        }
        return std::nullopt;
    }

private:
    std::string m_id;
    std::vector<MenuPane> m_panes;
};

/**
 * @brief Authoritative graph/tree of menu scenes and their transitions.
 */
class MenuSceneGraph {
public:
    void pushScene(const std::string& sceneId) {
        if (m_scenes.count(sceneId)) {
            m_sceneStack.push_back(sceneId);
            if (m_audio) m_audio->playSound("se_ok", audio::AudioCategory::System);
        }
    }

    void popScene() {
        if (!m_sceneStack.empty()) {
            m_sceneStack.pop_back();
            if (m_audio) m_audio->playSound("se_cancel", audio::AudioCategory::System);
        }
    }

    void registerScene(std::shared_ptr<MenuScene> scene) {
        if (scene) {
            m_scenes[scene->getId()] = scene;
        }
    }

    std::shared_ptr<MenuScene> getActiveScene() const {
        if (m_sceneStack.empty()) return nullptr;
        auto it = m_scenes.find(m_sceneStack.back());
        return (it != m_scenes.end()) ? it->second : nullptr;
    }

    void handleInput(input::InputAction action, input::ActionState state) {
        if (state != input::ActionState::Pressed) return;

        auto scene = getActiveScene();
        if (!scene) return;

        // Try to handle in active pane
        for (auto& pane : const_cast<std::vector<MenuPane>&>(scene->getPanes())) {
            if (pane.isActive) {
                if (action == input::InputAction::MoveDown) pane.nextCommand(m_audio.get());
                else if (action == input::InputAction::MoveUp) pane.prevCommand(m_audio.get());
                break;
            }
        }
    }

    void setAudio(std::shared_ptr<audio::AudioCore> audio) { m_audio = audio; }

    size_t stackSize() const { return m_sceneStack.size(); }

private:
    std::map<std::string, std::shared_ptr<MenuScene>> m_scenes;
    std::vector<std::string> m_sceneStack;
    std::shared_ptr<audio::AudioCore> m_audio;
};

} // namespace urpg::ui
