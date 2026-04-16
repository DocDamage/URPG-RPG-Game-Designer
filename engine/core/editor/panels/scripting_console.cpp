#include "scripting_console.h"
#include "../script_bridge.h"
#include "imgui.h"
#include <iostream>
#include <algorithm>

namespace urpg::editor {

ScriptingConsole::ScriptingConsole() : m_scrollToBottom(false) {
    memset(m_inputBuf, 0, sizeof(m_inputBuf));
    log(ConsoleLine::Type::Info, "URPG Scripting Console initialized.");
    log(ConsoleLine::Type::Info, "QuickJS Bridge Active. Type commands below.");
}

void ScriptingConsole::draw() {
    ImGui::Begin("Scripting Console");

    // Clear and Scroll buttons
    if (ImGui::Button("Clear")) clear();
    ImGui::SameLine();
    bool copyToClipboard = ImGui::Button("Copy");
    ImGui::Separator();

    // Console output history
    const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_HorizontalScrollbar);

    if (copyToClipboard) ImGui::LogToClipboard();

    for (const auto& line : m_history) {
        ImVec4 color;
        bool hasColor = true;
        
        switch (line.type) {
            case ConsoleLine::Type::Error: color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); break;
            case ConsoleLine::Type::Warning: color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f); break;
            case ConsoleLine::Type::Input: color = ImVec4(0.4f, 0.7f, 1.0f, 1.0f); break;
            default: hasColor = false; break;
        }

        if (hasColor) ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(line.text.c_str());
        if (hasColor) ImGui::PopStyleColor();
    }

    if (m_scrollToBottom || ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    m_scrollToBottom = false;

    ImGui::EndChild();
    ImGui::Separator();

    // Command Input
    bool reclaimFocus = false;
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue;
    if (ImGui::InputText("Input", m_inputBuf, IM_ARRAYSIZE(m_inputBuf), inputFlags)) {
        std::string cmd(m_inputBuf);
        if (!cmd.empty()) {
            executeCommand(cmd);
        }
        memset(m_inputBuf, 0, sizeof(m_inputBuf));
        reclaimFocus = true;
    }

    // Auto-focus on window apparition
    ImGui::SetItemDefaultFocus();
    if (reclaimFocus) ImGui::SetKeyboardFocusHere(-1);

    ImGui::End();
}

void ScriptingConsole::log(ConsoleLine::Type type, const std::string& text) {
    m_history.push_back({type, text});
    m_scrollToBottom = true;
}

void ScriptingConsole::clear() {
    m_history.clear();
}

void ScriptingConsole::executeCommand(const std::string& command) {
    log(ConsoleLine::Type::Input, "> " + command);
    
    // Evaluate via ScriptBridge
    auto result = ScriptBridge::instance().eval(command);
    
    // Convert BridgeValue to string for logging
    if (std::holds_alternative<int>(result)) {
        log(ConsoleLine::Type::Info, "result: " + std::to_string(std::get<int>(result)));
    } else if (std::holds_alternative<float>(result)) {
        log(ConsoleLine::Type::Info, "result: " + std::to_string(std::get<float>(result)));
    } else if (std::holds_alternative<std::string>(result)) {
        log(ConsoleLine::Type::Info, "result: " + std::get<std::string>(result));
    } else if (std::holds_alternative<bool>(result)) {
        log(ConsoleLine::Type::Info, std::get<bool>(result) ? "result: true" : "result: false");
    }

    // Capture bridge errors
    std::string err = ScriptBridge::instance().getLastError();
    if (!err.empty()) {
        log(ConsoleLine::Type::Error, "[JS Error] " + err);
        ScriptBridge::instance().clearError();
    }
}

} // namespace urpg::editor
