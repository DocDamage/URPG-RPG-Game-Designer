#pragma once

#ifdef _WIN32

#include "editor/accessibility/native_editor_accessibility.h"

#include <functional>
#include <memory>
#include <string_view>

namespace urpg::editor_app {

class WindowsAccessibilityBridge {
public:
    using SnapshotProvider = std::function<editor::NativeAccessibilitySnapshot()>;
    using ActionHandler = std::function<bool(std::string_view)>;
    using ValueHandler = std::function<bool(std::string_view, std::string_view)>;

    WindowsAccessibilityBridge();
    ~WindowsAccessibilityBridge();
    WindowsAccessibilityBridge(const WindowsAccessibilityBridge&) = delete;
    WindowsAccessibilityBridge& operator=(const WindowsAccessibilityBridge&) = delete;

    bool install(void* sdl_window, SnapshotProvider snapshot_provider, ActionHandler action_handler,
                 ValueHandler value_handler = {});
    void uninstall();
    void notifyTreeChanged();
    void synchronizeTree();
    [[nodiscard]] bool installed() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace urpg::editor_app

#endif
