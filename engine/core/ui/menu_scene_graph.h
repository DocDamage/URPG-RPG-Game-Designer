#pragma once

#include "ui_types.h"
#include "menu_route_resolver.h"
#include "menu_command_registry.h"
#include "../input/input_core.h"
#include "../audio/audio_core.h"
#include <algorithm>
#include <functional>
#include <iterator>
#include <string_view>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include <optional>

namespace urpg::ui {

struct MenuDesignCanvas {
    int width = 1280;
    int height = 720;

    bool isValid() const {
        return width >= 64 && width <= 8192 && height >= 64 && height <= 8192;
    }
};

struct MenuPaneLayout {
    int x = 0;
    int y = 0;
    int width = 320;
    int height = 180;
    int z_order = 0;
    // A negative value preserves legacy pane insertion order.
    int focus_order = -1;
    // Native responsive anchors are evaluated from the authored design canvas
    // against a selected runtime target canvas. A pane with opposite anchors
    // stretches while a single trailing anchor preserves its trailing margin.
    bool anchor_left = true;
    bool anchor_top = true;
    bool anchor_right = false;
    bool anchor_bottom = false;
    int min_width = 1;
    int min_height = 1;

    bool isValid() const {
        return x >= -8192 && x <= 8192 && y >= -8192 && y <= 8192 &&
               width >= 1 && width <= 8192 && height >= 1 && height <= 8192 &&
               z_order >= -1024 && z_order <= 1024 && focus_order >= -1 && focus_order <= 4096 &&
               min_width >= 1 && min_width <= width && min_height >= 1 && min_height <= height;
    }
};

inline MenuPaneLayout resolveMenuPaneLayoutForCanvas(const MenuPaneLayout& authored_layout,
                                                     const MenuDesignCanvas& authored_canvas,
                                                     const MenuDesignCanvas& target_canvas) {
    auto resolved = authored_layout;
    if (!authored_canvas.isValid() || !target_canvas.isValid()) {
        return resolved;
    }
    const int delta_width = target_canvas.width - authored_canvas.width;
    const int delta_height = target_canvas.height - authored_canvas.height;
    if (authored_layout.anchor_left && authored_layout.anchor_right) {
        resolved.width = std::max(authored_layout.min_width, authored_layout.width + delta_width);
    } else if (!authored_layout.anchor_left && authored_layout.anchor_right) {
        resolved.x = authored_layout.x + delta_width;
    }
    if (authored_layout.anchor_top && authored_layout.anchor_bottom) {
        resolved.height = std::max(authored_layout.min_height, authored_layout.height + delta_height);
    } else if (!authored_layout.anchor_top && authored_layout.anchor_bottom) {
        resolved.y = authored_layout.y + delta_height;
    }
    return resolved;
}

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
    MenuPaneLayout layout;

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

    const MenuDesignCanvas& getDesignCanvas() const { return m_designCanvas; }
    bool setDesignCanvas(MenuDesignCanvas canvas) {
        if (!canvas.isValid()) {
            return false;
        }
        m_designCanvas = canvas;
        return true;
    }

    void addPane(const MenuPane& pane) {
        m_panes.push_back(pane);
    }

    const std::vector<MenuPane>& getPanes() const { return m_panes; }
    std::vector<MenuPane>& getPanesMutable() { return m_panes; }

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
    MenuDesignCanvas m_designCanvas;
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

    void clearRegisteredScenes() {
        m_sceneStack.clear();
        m_scenes.clear();
        clearLastBlockedCommand();
    }

    bool restoreActiveScene(const std::string& sceneId) {
        if (m_scenes.count(sceneId) == 0) {
            return false;
        }
        m_sceneStack.clear();
        m_sceneStack.push_back(sceneId);
        clearLastBlockedCommand();
        return true;
    }

    std::shared_ptr<MenuScene> getActiveScene() const {
        if (m_sceneStack.empty()) return nullptr;
        auto it = m_scenes.find(m_sceneStack.back());
        return (it != m_scenes.end()) ? it->second : nullptr;
    }

    const std::map<std::string, std::shared_ptr<MenuScene>>& getRegisteredScenes() const {
        return m_scenes;
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
        const auto ordered = navigablePaneIndexes(panes);
        return ordered.empty() ? -1 : static_cast<int>(ordered.front());
    }

    void advanceActivePane(std::vector<MenuPane>& panes, int currentIndex, int direction) {
        if (panes.empty() || direction == 0) {
            return;
        }

        const auto ordered = navigablePaneIndexes(panes);
        if (ordered.empty()) {
            return;
        }

        const auto current = std::find(ordered.begin(), ordered.end(), static_cast<size_t>(currentIndex));
        const size_t current_order = current == ordered.end()
            ? 0
            : static_cast<size_t>(std::distance(ordered.begin(), current));
        const int raw = static_cast<int>(current_order) + direction;
        const int count = static_cast<int>(ordered.size());
        const size_t next_order = static_cast<size_t>((raw % count + count) % count);
        if (ordered[next_order] == static_cast<size_t>(currentIndex)) {
            return;
        }

        panes[static_cast<size_t>(currentIndex)].isActive = false;
        MenuPane& candidate = panes[ordered[next_order]];
        candidate.isActive = true;
        if (m_audio) m_audio->playSound(candidate.selectionSound, audio::AudioCategory::System);
    }

    std::vector<size_t> navigablePaneIndexes(const std::vector<MenuPane>& panes) const {
        std::vector<size_t> indexes;
        for (size_t index = 0; index < panes.size(); ++index) {
            if (isPaneNavigable(panes[index])) {
                indexes.push_back(index);
            }
        }

        std::stable_sort(indexes.begin(), indexes.end(), [&panes](size_t left, size_t right) {
            const int left_order = panes[left].layout.focus_order;
            const int right_order = panes[right].layout.focus_order;
            if (left_order < 0 && right_order < 0) {
                return false;
            }
            if (left_order < 0) {
                return false;
            }
            if (right_order < 0) {
                return true;
            }
            return left_order < right_order;
        });
        return indexes;
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
