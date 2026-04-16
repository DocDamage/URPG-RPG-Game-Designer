#pragma once

#include <string>
#include <vector>
#include <functional>

namespace urpg::editor {

/**
 * @brief Base class for simple editor utility actions.
 * Used for repetitive tasks like batch renaming or common audits.
 */
class EditorUtilityTask {
public:
    struct Context {
        std::string activeProject;
        std::vector<std::string> selectedFiles;
    };

    virtual ~EditorUtilityTask() = default;
    virtual std::string getName() const = 0;
    virtual void execute(const Context& ctx) = 0;
};

/**
 * @brief Manager to register and trigger productivity utilities.
 */
class EditorUtilityManager {
public:
    void registerTask(std::unique_ptr<EditorUtilityTask> task) {
        m_tasks.push_back(std::move(task));
    }

    const std::vector<std::unique_ptr<EditorUtilityTask>>& getTasks() const {
        return m_tasks;
    }

private:
    std::vector<std::unique_ptr<EditorUtilityTask>> m_tasks;
};

} // namespace urpg::editor
