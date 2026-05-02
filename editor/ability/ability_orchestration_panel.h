#pragma once

#include "engine/core/ability/ability_orchestration.h"

#include <nlohmann/json.hpp>
#include <string>

namespace urpg::editor {

class AbilityOrchestrationPanel {
public:
    struct Snapshot {
        bool disabled = true;
        bool rendered = false;
        std::string orchestration_id;
        std::string mode;
        std::string ability_id;
        bool activation_executed = false;
        float source_mp_before = 0.0f;
        float source_mp_after = 0.0f;
        float cooldown_after = 0.0f;
        size_t target_count = 0;
        size_t task_count = 0;
        size_t diagnostic_count = 0;
        std::string status_message;
        std::vector<urpg::ability::AbilityTaskPreviewRow> task_preview_rows;
        std::vector<urpg::ability::AbilityTaskExecutionEvent> task_execution_events;
        std::vector<std::string> validation_messages;
        bool can_save = false;
        bool can_load = true;
        bool can_apply = false;
        bool can_revert = false;
        nlohmann::json saved_project_json = nlohmann::json::object();
        nlohmann::json applied_project_json = nlohmann::json::object();
        nlohmann::json runtime_result_json = nlohmann::json::object();
    };

    void loadDocument(urpg::ability::AbilityOrchestrationDocument document);
    bool loadProjectData(const nlohmann::json& json);
    nlohmann::json saveProjectData() const;
    bool addTask(urpg::ability::AbilityOrchestrationTask task);
    bool applyPreview();
    bool revertLastApply();
    void render();

    bool hasRenderedFrame() const { return snapshot_.rendered; }
    const Snapshot& snapshot() const { return snapshot_; }
    const urpg::ability::AbilityOrchestrationResult& result() const { return result_; }

private:
    void refreshPreview();

    bool has_document_ = false;
    bool has_applied_project_ = false;
    urpg::ability::AbilityOrchestrationDocument document_;
    urpg::ability::AbilityOrchestrationResult result_;
    nlohmann::json applied_project_json_ = nlohmann::json::object();
    Snapshot snapshot_;
};

} // namespace urpg::editor
