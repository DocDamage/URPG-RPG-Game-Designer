#pragma once

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace urpg::editor {

/**
 * @brief A single executable command in the editor.
 * Part of Wave 5: Editor State Management (5.2).
 */
class EditorCommand {
  public:
    virtual ~EditorCommand() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
    virtual std::string getName() const = 0;
};

/**
 * @brief Persistent History Manager for the editor.
 * Essential for UI productivity and error recovery.
 */
class UndoHistory {
  public:
    /**
     * @brief Add a command to history and execute it.
     */
    void pushCommand(std::unique_ptr<EditorCommand> command) {
        command->execute();
        m_undoStack.push_back(std::move(command));

        // Clear the redo stack after a new action
        m_redoStack.clear();

        // Maintain max undo history size
        if (m_undoStack.size() > m_maxHistory) {
            m_undoStack.pop_front();
        }
    }

    /**
     * @brief Step back in time.
     */
    void undo() {
        if (m_undoStack.empty())
            return;

        auto cmd = std::move(m_undoStack.back());
        m_undoStack.pop_back();
        cmd->undo();
        m_redoStack.push_back(std::move(cmd));
    }

    /**
     * @brief Step forward in time.
     */
    void redo() {
        if (m_redoStack.empty())
            return;

        auto cmd = std::move(m_redoStack.back());
        m_redoStack.pop_back();
        cmd->execute();
        m_undoStack.push_back(std::move(cmd));
    }

    bool canUndo() const { return !m_undoStack.empty(); }
    bool canRedo() const { return !m_redoStack.empty(); }

  private:
    std::deque<std::unique_ptr<EditorCommand>> m_undoStack;
    std::deque<std::unique_ptr<EditorCommand>> m_redoStack;
    size_t m_maxHistory = 100;
};

} // namespace urpg::editor
