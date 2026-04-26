#pragma once

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <functional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace urpg::editor {

struct EditorFrameContext {
    uint64_t frame_index = 0;
    double delta_seconds = 0.0;
    bool headless = false;
};

struct EditorPanelDescriptor {
    std::string id;
    std::string title;
    std::string category;
    bool visible = true;
    bool enabled = true;
    bool rendered_last_frame = false;
    uint64_t render_count = 0;
};

struct EditorShellSnapshot {
    bool running = false;
    bool frame_active = false;
    bool headless = false;
    uint64_t frame_index = 0;
    std::string active_panel_id;
    std::filesystem::path project_root;
    std::string runtime_preview_id;
    std::vector<EditorPanelDescriptor> panels;
};

/**
 * @brief Runtime editor host for panel registration, navigation, and frame dispatch.
 *
 * The shell owns editor-level state and invokes panel render callbacks from a
 * bounded frame lifecycle. Platform/UI backends can wrap beginFrame/endFrame
 * with their ImGui setup and teardown while headless smoke runs still verify
 * that panels are registered and reachable.
 */
class EditorShell {
  public:
    using RenderCallback = std::function<void(const EditorFrameContext&)>;

    bool addPanel(EditorPanelDescriptor descriptor, RenderCallback render_callback) {
        if (descriptor.id.empty() || !render_callback) {
            return false;
        }

        if (findPanel(descriptor.id) != nullptr) {
            return false;
        }

        descriptor.rendered_last_frame = false;
        descriptor.render_count = 0;
        m_registered_panels.push_back({std::move(descriptor), std::move(render_callback)});
        if (m_active_panel_id.empty()) {
            m_active_panel_id = m_registered_panels.back().descriptor.id;
        }
        return true;
    }

    bool start(bool headless = false) {
        if (m_running) {
            return false;
        }

        m_headless = headless;
        m_running = true;
        m_frame_active = false;
        m_frame_index = 0;
        return true;
    }

    void shutdown() {
        m_frame_active = false;
        m_running = false;
    }

    bool isRunning() const { return m_running; }

    bool beginFrame(double delta_seconds) {
        if (!m_running || m_frame_active) {
            return false;
        }

        m_frame_active = true;
        m_current_context = {m_frame_index, delta_seconds, m_headless};
        for (auto& panel : m_registered_panels) {
            panel.descriptor.rendered_last_frame = false;
        }
        return true;
    }

    bool renderActivePanel() {
        if (!m_frame_active || m_active_panel_id.empty()) {
            return false;
        }

        auto* panel = findPanel(m_active_panel_id);
        if (!panel || !panel->descriptor.enabled || !panel->descriptor.visible) {
            return false;
        }

        panel->render_callback(m_current_context);
        panel->descriptor.rendered_last_frame = true;
        ++panel->descriptor.render_count;
        return true;
    }

    size_t renderVisiblePanels() {
        if (!m_frame_active) {
            return 0;
        }

        size_t rendered_count = 0;
        for (auto& panel : m_registered_panels) {
            if (!panel.descriptor.enabled || !panel.descriptor.visible) {
                continue;
            }

            panel.render_callback(m_current_context);
            panel.descriptor.rendered_last_frame = true;
            ++panel.descriptor.render_count;
            ++rendered_count;
        }
        return rendered_count;
    }

    bool endFrame() {
        if (!m_frame_active) {
            return false;
        }

        m_frame_active = false;
        ++m_frame_index;
        return true;
    }

    bool renderFrame(double delta_seconds) {
        if (!beginFrame(delta_seconds)) {
            return false;
        }

        const bool rendered = renderActivePanel();
        return endFrame() && rendered;
    }

    bool setActivePanel(std::string_view id) {
        auto* panel = findPanel(id);
        if (!panel || !panel->descriptor.enabled) {
            return false;
        }

        m_active_panel_id = panel->descriptor.id;
        return true;
    }

    bool setPanelVisible(std::string_view id, bool visible) {
        auto* panel = findPanel(id);
        if (!panel) {
            return false;
        }

        panel->descriptor.visible = visible;
        return true;
    }

    bool openPanel(std::string_view id) {
        if (!setPanelVisible(id, true)) {
            return false;
        }
        return setActivePanel(id);
    }

    const std::string& activePanelId() const { return m_active_panel_id; }

    std::vector<EditorPanelDescriptor> panels() const {
        std::vector<EditorPanelDescriptor> result;
        result.reserve(m_registered_panels.size());
        for (const auto& panel : m_registered_panels) {
            result.push_back(panel.descriptor);
        }
        return result;
    }

    bool hasPanel(std::string_view id) const { return findPanel(id) != nullptr; }

    void setProjectRoot(std::filesystem::path project_root) { m_project_root = std::move(project_root); }

    const std::filesystem::path& projectRoot() const { return m_project_root; }

    void setRuntimePreviewId(std::string runtime_preview_id) { m_runtime_preview_id = std::move(runtime_preview_id); }

    const std::string& runtimePreviewId() const { return m_runtime_preview_id; }

    EditorShellSnapshot snapshot() const {
        return {
            m_running,         m_frame_active, m_headless,           m_frame_index,
            m_active_panel_id, m_project_root, m_runtime_preview_id, panels(),
        };
    }

    void renderUI() { (void)renderFrame(0.0); }

  private:
    struct RegisteredPanel {
        EditorPanelDescriptor descriptor;
        RenderCallback render_callback;
    };

    RegisteredPanel* findPanel(std::string_view id) {
        auto it = std::find_if(m_registered_panels.begin(), m_registered_panels.end(),
                               [id](const RegisteredPanel& panel) { return panel.descriptor.id == id; });
        return it == m_registered_panels.end() ? nullptr : &(*it);
    }

    const RegisteredPanel* findPanel(std::string_view id) const {
        auto it = std::find_if(m_registered_panels.begin(), m_registered_panels.end(),
                               [id](const RegisteredPanel& panel) { return panel.descriptor.id == id; });
        return it == m_registered_panels.end() ? nullptr : &(*it);
    }

    std::vector<RegisteredPanel> m_registered_panels;
    EditorFrameContext m_current_context;
    std::filesystem::path m_project_root;
    std::string m_runtime_preview_id;
    std::string m_active_panel_id;
    bool m_running = false;
    bool m_frame_active = false;
    bool m_headless = false;
    uint64_t m_frame_index = 0;
};

} // namespace urpg::editor
