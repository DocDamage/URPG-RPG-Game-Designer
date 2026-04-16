#pragma once

#include "ui_types.h"
#include "menu_route_resolver.h"
#include "menu_command_registry.h"
#include "../input/input_core.h"
#include "../audio/audio_core.h"
#include <functional>
#include <string_view>
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
    std::string confirmSound = "se_ok";
    std::string blockedSound = "se_buzzer";

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
    MenuScene(const std::string& id) : m_id(id) {
        // Subscribe to all state changes to trigger UI refreshes
        m_stateHandle = GlobalStateHub::getInstance().subscribe("*", [this](const std::string&, const GlobalStateHub::Value&) {
            this->m_needsRefresh = true;
        });
    }

    ~MenuScene() {
        GlobalStateHub::getInstance().unsubscribe(m_stateHandle);
    }

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

    bool needsRefresh() const { return m_needsRefresh; }
    void clearRefresh() { m_needsRefresh = false; }

private:
    std::string m_id;
    std::vector<MenuPane> m_panes;
    uint32_t m_stateHandle = 0;
    bool m_needsRefresh = false;
};

/**
 * @brief Authoritative graph/tree of menu scenes and their transitions.
 */
class MenuSceneGraph {
public:
    using CommandEnabledEvaluator = std::function<bool(const MenuCommandMeta&)>;
    using CommandVisibleEvaluator = std::function<bool(const MenuCommandMeta&)>;
    using CommandDisabledReasonEvaluator = std::function<std::string(const MenuCommandMeta&)>;
    using CommandBlockedHandler = std::function<void(const MenuCommandMeta&, std::string_view)>;

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

        auto& panes = const_cast<std::vector<MenuPane>&>(scene->getPanes());
        const int activePaneIndex = ensureActivePaneNavigable(panes);
        if (activePaneIndex < 0) {
            return;
        }

        if (action == input::InputAction::MoveRight) {
            advanceActivePane(panes, activePaneIndex, +1);
            return;
        }
        if (action == input::InputAction::MoveLeft) {
            advanceActivePane(panes, activePaneIndex, -1);
            return;
        }

        MenuPane& pane = panes[static_cast<size_t>(activePaneIndex)];
        if (action == input::InputAction::MoveDown) pane.nextCommand(m_audio.get());
        else if (action == input::InputAction::MoveUp) pane.prevCommand(m_audio.get());
        else if (action == input::InputAction::Confirm) {
            const MenuCommandMeta* selected = pane.getSelectedCommand();
            if (!selected || !m_routeResolver) {
                if (m_audio) m_audio->playSound(pane.blockedSound, audio::AudioCategory::System);
                if (selected) {
                    recordBlockedCommand(*selected, "Route resolver unavailable.");
                }
                return;
            }

            if (m_commandEnabledEvaluator && !m_commandEnabledEvaluator(*selected)) {
                if (m_audio) m_audio->playSound(pane.blockedSound, audio::AudioCategory::System);
                const std::string reason = m_commandDisabledReasonEvaluator
                    ? m_commandDisabledReasonEvaluator(*selected)
                    : std::string("Command is currently disabled.");
                recordBlockedCommand(*selected, reason);
                return;
            }

            const bool resolved = m_routeResolver->resolve(*selected);
            if (m_audio) {
                m_audio->playSound(
                    resolved ? pane.confirmSound : pane.blockedSound,
                    audio::AudioCategory::System
                );
            }
            if (resolved) {
                clearLastBlockedCommand();
            } else {
                recordBlockedCommand(*selected, "No route resolved for command.");
            }
        }
        else if (action == input::InputAction::Cancel) {
            if (m_sceneStack.size() > 1 || m_allowRootCancelPop) {
                popScene();
            } else if (m_audio) {
                m_audio->playSound(pane.blockedSound, audio::AudioCategory::System);
            }
        }
    }

    void setAudio(std::shared_ptr<audio::AudioCore> audio) { m_audio = audio; }
    void setRouteResolver(const MenuRouteResolver* resolver) { m_routeResolver = resolver; }
    void setCommandEnabledEvaluator(CommandEnabledEvaluator evaluator) {
        m_commandEnabledEvaluator = std::move(evaluator);
    }
    void setCommandVisibleEvaluator(CommandVisibleEvaluator evaluator) {
        m_commandVisibleEvaluator = std::move(evaluator);
    }
    void setCommandDisabledReasonEvaluator(CommandDisabledReasonEvaluator evaluator) {
        m_commandDisabledReasonEvaluator = std::move(evaluator);
    }
    void setCommandBlockedHandler(CommandBlockedHandler handler) {
        m_commandBlockedHandler = std::move(handler);
    }
    void clearCommandStateEvaluators() {
        m_commandEnabledEvaluator = nullptr;
        m_commandVisibleEvaluator = nullptr;
        m_commandDisabledReasonEvaluator = nullptr;
    }
    void setCommandStateFromRegistry(
        const MenuCommandRegistry& registry,
        const MenuCommandRegistry::SwitchState& switches,
        const MenuCommandRegistry::VariableState& variables) {
        const auto switchState = switches;
        const auto variableState = variables;
        setCommandVisibleEvaluator(
            [&registry, switchState, variableState](const MenuCommandMeta& command) {
                return registry.isVisible(command, switchState, variableState);
            }
        );
        setCommandEnabledEvaluator(
            [&registry, switchState, variableState](const MenuCommandMeta& command) {
                return registry.isEnabled(command, switchState, variableState);
            }
        );
    }
    void setAllowRootCancelPop(bool allow) { m_allowRootCancelPop = allow; }

    size_t stackSize() const { return m_sceneStack.size(); }
    const std::string& getLastBlockedCommandId() const { return m_lastBlockedCommandId; }
    const std::string& getLastBlockedReason() const { return m_lastBlockedReason; }
    void clearLastBlockedCommand() {
        m_lastBlockedCommandId.clear();
        m_lastBlockedReason.clear();
    }

private:
    void recordBlockedCommand(const MenuCommandMeta& command, std::string reason) {
        m_lastBlockedCommandId = command.id;
        m_lastBlockedReason = std::move(reason);
        if (m_commandBlockedHandler) {
            m_commandBlockedHandler(command, m_lastBlockedReason);
        }
    }

    static int findActivePaneIndex(const std::vector<MenuPane>& panes) {
        for (size_t i = 0; i < panes.size(); ++i) {
            if (panes[i].isActive) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    int ensureActivePaneNavigable(std::vector<MenuPane>& panes) const {
        const int activePaneIndex = findActivePaneIndex(panes);
        if (activePaneIndex >= 0 &&
            isPaneNavigable(panes[static_cast<size_t>(activePaneIndex)])) {
            return activePaneIndex;
        }

        const int firstNavigable = findFirstNavigablePaneIndex(panes);
        for (auto& pane : panes) {
            pane.isActive = false;
        }
        if (firstNavigable >= 0) {
            panes[static_cast<size_t>(firstNavigable)].isActive = true;
        }
        return firstNavigable;
    }

    int findFirstNavigablePaneIndex(const std::vector<MenuPane>& panes) const {
        for (size_t i = 0; i < panes.size(); ++i) {
            if (isPaneNavigable(panes[i])) {
                return static_cast<int>(i);
            }
        }
        return -1;
    }

    void advanceActivePane(std::vector<MenuPane>& panes, int currentIndex, int direction) {
        if (panes.empty() || direction == 0) {
            return;
        }

        const size_t count = panes.size();
        for (size_t step = 1; step <= count; ++step) {
            const int raw = currentIndex + (direction * static_cast<int>(step));
            int wrapped = raw % static_cast<int>(count);
            if (wrapped < 0) {
                wrapped += static_cast<int>(count);
            }

            MenuPane& candidate = panes[static_cast<size_t>(wrapped)];
            if (!isPaneNavigable(candidate)) {
                continue;
            }

            panes[static_cast<size_t>(currentIndex)].isActive = false;
            candidate.isActive = true;
            if (m_audio) m_audio->playSound(candidate.selectionSound, audio::AudioCategory::System);
            return;
        }
    }

    bool isPaneNavigable(const MenuPane& pane) const {
        if (!pane.isVisible || pane.commands.empty()) {
            return false;
        }

        for (const auto& command : pane.commands) {
            const bool visible = m_commandVisibleEvaluator ? m_commandVisibleEvaluator(command) : true;
            const bool enabled = m_commandEnabledEvaluator ? m_commandEnabledEvaluator(command) : true;
            if (visible && enabled) {
                return true;
            }
        }

        return false;
    }

    std::map<std::string, std::shared_ptr<MenuScene>> m_scenes;
    std::vector<std::string> m_sceneStack;
    std::shared_ptr<audio::AudioCore> m_audio;
    const MenuRouteResolver* m_routeResolver = nullptr;
    CommandEnabledEvaluator m_commandEnabledEvaluator;
    CommandVisibleEvaluator m_commandVisibleEvaluator;
    CommandDisabledReasonEvaluator m_commandDisabledReasonEvaluator;
    CommandBlockedHandler m_commandBlockedHandler;
    std::string m_lastBlockedCommandId;
    std::string m_lastBlockedReason;
    bool m_allowRootCancelPop = false;
};

} // namespace urpg::ui
