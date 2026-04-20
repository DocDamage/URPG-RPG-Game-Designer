#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>

#include "engine/core/presentation/presentation_types.h"

namespace urpg::editor {

/**
 * @brief Base class for simple editor utility actions.
 * Used for repetitive tasks like batch renaming or common audits.
 */
class EditorUtilityTask {
public:
    enum class ModeRequirement {
        Any,
        Classic2DOnly,
        SpatialOnly
    };

    struct Context {
        std::string activeProject;
        std::vector<std::string> selectedFiles;
        presentation::PresentationMode presentationMode = presentation::PresentationMode::Classic2D;
    };

    virtual ~EditorUtilityTask() = default;
    virtual std::string getName() const = 0;
    virtual void execute(const Context& ctx) = 0;
    virtual ModeRequirement requiredMode() const { return ModeRequirement::Any; }

    bool canRun(const Context& ctx) const {
        switch (requiredMode()) {
        case ModeRequirement::Any:
            return true;
        case ModeRequirement::Classic2DOnly:
            return ctx.presentationMode == presentation::PresentationMode::Classic2D;
        case ModeRequirement::SpatialOnly:
            return ctx.presentationMode == presentation::PresentationMode::Spatial;
        }
        return false;
    }
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

    std::vector<const EditorUtilityTask*> getRunnableTasks(const EditorUtilityTask::Context& ctx) const {
        std::vector<const EditorUtilityTask*> runnable;
        for (const auto& task : m_tasks) {
            if (task && task->canRun(ctx)) {
                runnable.push_back(task.get());
            }
        }
        return runnable;
    }

private:
    std::vector<std::unique_ptr<EditorUtilityTask>> m_tasks;
};

} // namespace urpg::editor
