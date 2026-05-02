#pragma once

#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/ability/ability_battle_integration.h"

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace urpg::ability {

enum class AbilityOrchestrationMode { Battle, Map };

struct AbilityOrchestrationActor {
    std::string id;
    float mp = 100.0f;
    float effect_attribute_base = 0.0f;
    int32_t x = 0;
    int32_t y = 0;
    std::vector<std::string> tags;
};

struct AbilityOrchestrationDiagnostic {
    std::string code;
    std::string message;
    std::string target;
};

struct AbilityOrchestrationTask {
    std::string id;
    std::string kind;
    std::string action;
    int32_t timeout_ms = 0;
    std::string condition;
    std::string on_true;
    std::string on_false;
    std::string effect_id;
    std::string cue_id;
    std::string target;
    std::string next;
    std::vector<std::string> depends_on;
    bool skip_cooldown_on_cancel = false;
};

struct AbilityTaskPreviewRow {
    std::string id;
    std::string kind;
    std::string status;
    std::string detail;
    bool executable = false;
    std::string disabled_reason;
};

struct AbilityTaskExecutionEvent {
    size_t sequence = 0;
    std::string task_id;
    std::string kind;
    std::string status;
    std::string detail;
};

struct AbilityOrchestrationDocument {
    std::string id;
    AbilityOrchestrationMode mode = AbilityOrchestrationMode::Battle;
    AuthoredAbilityAsset ability;
    AbilityOrchestrationActor source;
    std::vector<AbilityOrchestrationActor> targets;
    std::vector<std::string> required_tags;
    std::vector<std::string> blocking_tags;
    int32_t battle_turn = 1;
    int32_t battle_speed = 0;
    int32_t battle_priority = 0;
    std::vector<AbilityOrchestrationTask> tasks;

    std::vector<AbilityOrchestrationDiagnostic> validate() const;
    nlohmann::json toJson() const;

    static AbilityOrchestrationDocument fromJson(const nlohmann::json& json);
};

struct AbilityOrchestrationTargetResult {
    std::string id;
    float effect_attribute_before = 0.0f;
    float effect_attribute_after = 0.0f;
    size_t active_effect_count = 0;
    bool in_pattern = true;
};

struct AbilityOrchestrationResult {
    std::string document_id;
    std::string ability_id;
    AbilityOrchestrationMode mode = AbilityOrchestrationMode::Battle;
    bool valid = false;
    bool activation_attempted = false;
    bool activation_executed = false;
    std::string blocking_reason;
    float source_mp_before = 0.0f;
    float source_mp_after = 0.0f;
    float cooldown_after = 0.0f;
    size_t battle_commands_received = 0;
    size_t battle_commands_executed = 0;
    size_t battle_commands_blocked = 0;
    std::vector<AbilityOrchestrationTargetResult> targets;
    std::vector<AbilityTaskPreviewRow> task_preview_rows;
    std::vector<AbilityTaskExecutionEvent> task_execution_events;
    std::vector<AbilityOrchestrationDiagnostic> diagnostics;
    nlohmann::json battle_snapshot = nlohmann::json::object();
};

AbilityOrchestrationResult runAbilityOrchestration(const AbilityOrchestrationDocument& document);
nlohmann::json abilityOrchestrationResultToJson(const AbilityOrchestrationResult& result);

const char* abilityOrchestrationModeName(AbilityOrchestrationMode mode);
AbilityOrchestrationMode abilityOrchestrationModeFromString(const std::string& value);

} // namespace urpg::ability
