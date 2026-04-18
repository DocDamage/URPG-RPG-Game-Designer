#pragma once

#include <string>
#include <vector>
#include <memory>

namespace urpg::editor {

/**
 * @brief Represents a line of log/output in the scripting console.
 */
struct ConsoleLine {
    enum class Type { Info, Warning, Error, Input };
    Type type;
    std::string text;
};

/**
 * @brief ImGui-based console for script execution and debug output.
 * 
 * Wave 7.4 implementation: Scripting Console (ImGui).
 */
class ScriptingConsole {
public:
    static ScriptingConsole& instance() {
        static ScriptingConsole inst;
        return inst;
    }

    void draw();
    void log(ConsoleLine::Type type, const std::string& text);
    void clear();

private:
    ScriptingConsole();
    
    char m_inputBuf[1024];
    std::vector<ConsoleLine> m_history;
    bool m_scrollToBottom;

    void executeCommand(const std::string& command);
};

} // namespace urpg::editor
