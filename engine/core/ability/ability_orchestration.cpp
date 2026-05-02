#include "engine/core/ability/ability_orchestration.h"

#include "engine/core/ability/ability_condition_evaluator.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <utility>

namespace urpg::ability {
namespace {

class OrchestratedAbility final : public GameplayAbility {
public:
    OrchestratedAbility(AuthoredAbilityAsset asset, std::vector<std::string> requiredTags,
                        std::vector<std::string> blockingTags, bool taskGraphMode = false)
        : asset_(std::move(asset)),
          required_tags_(std::move(requiredTags)),
          blocking_tags_(std::move(blockingTags)),
          task_graph_mode_(taskGraphMode) {
        id = asset_.ability_id;
        cooldownTime = asset_.cooldown_seconds;
        mpCost = asset_.mp_cost;
        if (!asset_.pattern.getPoints().empty()) {
            pattern_ = std::make_shared<urpg::PatternField>(asset_.pattern);
        }
    }

    const std::string& getId() const override { return id; }

    const ActivationInfo& getActivationInfo() const override {
        activation_info_ = {};
        activation_info_.cooldownSeconds = asset_.cooldown_seconds;
        activation_info_.mpCost = static_cast<int32_t>(asset_.mp_cost);
        activation_info_.pattern = pattern_;
        for (const auto& tag : required_tags_) {
            activation_info_.requiredTags.addTag(GameplayTag(tag));
        }
        for (const auto& tag : blocking_tags_) {
            activation_info_.blockingTags.addTag(GameplayTag(tag));
        }
        return activation_info_;
    }

    void activate(AbilitySystemComponent& source) override {
        GameplayAbility::AbilityExecutionContext context;
        activate(source, context);
    }

    void activate(AbilitySystemComponent& source, const AbilityExecutionContext& context) override {
        commitAbility(source);
        if (task_graph_mode_) {
            return;
        }
        applyEffectTargets(context);
    }

    void applyEffectTargets(const AbilityExecutionContext& context) const {
        if (context.targets.empty()) {
            return;
        }
        const auto effect = buildEffect();
        for (const auto& target : context.targets) {
            if (target.abilitySystem) {
                target.abilitySystem->applyEffect(effect);
            }
        }
    }

private:
    urpg::GameplayEffect buildEffect() const {
        urpg::GameplayEffect effect;
        effect.id = asset_.effect_id;
        effect.name = asset_.effect_id;
        effect.duration = asset_.effect_duration;

        urpg::GameplayEffectModifier modifier;
        modifier.attributeName = asset_.effect_attribute;
        modifier.operation = asset_.effect_operation;
        modifier.value = asset_.effect_value;
        effect.modifiers.push_back(modifier);
        return effect;
    }

    AuthoredAbilityAsset asset_;
    std::vector<std::string> required_tags_;
    std::vector<std::string> blocking_tags_;
    std::shared_ptr<urpg::PatternField> pattern_;
    bool task_graph_mode_ = false;
    mutable ActivationInfo activation_info_;
};

std::vector<std::string> stringsFromJsonArray(const nlohmann::json& json) {
    std::vector<std::string> values;
    if (!json.is_array()) {
        return values;
    }
    for (const auto& value : json) {
        if (value.is_string()) {
            values.push_back(value.get<std::string>());
        }
    }
    return values;
}

std::string toLower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

AbilityOrchestrationActor actorFromJson(const nlohmann::json& json) {
    AbilityOrchestrationActor actor;
    if (!json.is_object()) {
        return actor;
    }
    actor.id = json.value("id", "");
    actor.mp = json.value("mp", actor.mp);
    actor.effect_attribute_base = json.value("effect_attribute_base", actor.effect_attribute_base);
    actor.x = json.value("x", actor.x);
    actor.y = json.value("y", actor.y);
    actor.tags = stringsFromJsonArray(json.value("tags", nlohmann::json::array()));
    return actor;
}

nlohmann::json actorToJson(const AbilityOrchestrationActor& actor) {
    return {
        {"id", actor.id},
        {"mp", actor.mp},
        {"effect_attribute_base", actor.effect_attribute_base},
        {"x", actor.x},
        {"y", actor.y},
        {"tags", actor.tags},
    };
}

void addTags(AbilitySystemComponent& asc, const std::vector<std::string>& tags) {
    for (const auto& tag : tags) {
        asc.addTag(GameplayTag(tag));
    }
}

AbilityOrchestrationTask taskFromJson(const nlohmann::json& json) {
    AbilityOrchestrationTask task;
    if (!json.is_object()) {
        return task;
    }
    task.id = json.value("id", "");
    task.kind = json.value("kind", "");
    task.action = json.value("action", "");
    task.timeout_ms = json.value("timeout_ms", task.timeout_ms);
    task.condition = json.value("condition", "");
    task.on_true = json.value("on_true", "");
    task.on_false = json.value("on_false", "");
    task.effect_id = json.value("effect_id", "");
    task.cue_id = json.value("cue_id", "");
    task.target = json.value("target", "");
    task.next = json.value("next", "");
    task.depends_on = stringsFromJsonArray(json.value("depends_on", nlohmann::json::array()));
    task.skip_cooldown_on_cancel = json.value("skip_cooldown_on_cancel", task.skip_cooldown_on_cancel);
    return task;
}

nlohmann::json taskToJson(const AbilityOrchestrationTask& task) {
    nlohmann::json json = {
        {"id", task.id},
        {"kind", task.kind},
    };
    if (!task.action.empty()) {
        json["action"] = task.action;
    }
    if (task.timeout_ms > 0) {
        json["timeout_ms"] = task.timeout_ms;
    }
    if (!task.condition.empty()) {
        json["condition"] = task.condition;
    }
    if (!task.on_true.empty()) {
        json["on_true"] = task.on_true;
    }
    if (!task.on_false.empty()) {
        json["on_false"] = task.on_false;
    }
    if (!task.effect_id.empty()) {
        json["effect_id"] = task.effect_id;
    }
    if (!task.cue_id.empty()) {
        json["cue_id"] = task.cue_id;
    }
    if (!task.target.empty()) {
        json["target"] = task.target;
    }
    if (!task.next.empty()) {
        json["next"] = task.next;
    }
    if (!task.depends_on.empty()) {
        json["depends_on"] = task.depends_on;
    }
    if (task.skip_cooldown_on_cancel) {
        json["skip_cooldown_on_cancel"] = true;
    }
    return json;
}

bool hasTaskId(const std::unordered_set<std::string>& ids, const std::string& id) {
    return !id.empty() && ids.find(id) != ids.end();
}

AbilitySystemComponent buildConditionSource(const AbilityOrchestrationDocument& document) {
    AbilitySystemComponent asc;
    asc.setAttribute("MP", document.source.mp);
    asc.setAttribute("HP", document.source.effect_attribute_base);
    asc.setAttribute(document.ability.effect_attribute, document.source.effect_attribute_base);
    addTags(asc, document.source.tags);
    return asc;
}

std::vector<std::unique_ptr<AbilitySystemComponent>>
buildConditionTargets(const AbilityOrchestrationDocument& document,
                      GameplayAbility::AbilityExecutionContext& context) {
    std::vector<std::unique_ptr<AbilitySystemComponent>> targets;
    for (const auto& target : document.targets) {
        auto asc = std::make_unique<AbilitySystemComponent>();
        asc->setAttribute("MP", target.mp);
        asc->setAttribute("HP", target.effect_attribute_base);
        asc->setAttribute(document.ability.effect_attribute, target.effect_attribute_base);
        addTags(*asc, target.tags);
        context.targets.push_back({asc.get(), nullptr, target.id});
        targets.push_back(std::move(asc));
    }
    return targets;
}

std::vector<AbilityTaskPreviewRow> buildTaskPreviewRows(const AbilityOrchestrationDocument& document) {
    std::vector<AbilityTaskPreviewRow> rows;
    rows.reserve(document.tasks.size());
    for (const auto& task : document.tasks) {
        AbilityTaskPreviewRow row;
        row.id = task.id;
        row.kind = task.kind;
        row.status = "ready";
        row.executable = true;

        if (task.kind == "wait_input") {
            row.detail = "Wait for input action '" + task.action + "'";
        } else if (task.kind == "wait_event") {
            row.detail = "Wait for event action '" + task.action + "'";
        } else if (task.kind == "wait_projectile_collision") {
            row.detail = "Wait for projectile collision target '" + task.target + "'";
        } else if (task.kind == "cost") {
            row.detail = "Commit authored MP cost";
        } else if (task.kind == "delay") {
            row.detail = "Delay for " + std::to_string(task.timeout_ms) + "ms";
        } else if (task.kind == "cooldown") {
            row.detail = "Commit authored cooldown";
        } else if (task.kind == "branch_on_condition") {
            row.detail = "Branch on " + task.condition + " true=" + task.on_true + " false=" + task.on_false;
        } else if (task.kind == "apply_effect") {
            row.detail = "Apply effect '" + task.effect_id + "' to " + task.target;
        } else if (task.kind == "play_cue") {
            row.detail = "Play cue '" + task.cue_id + "'";
        } else {
            row.status = "blocked";
            row.executable = false;
            row.disabled_reason = "Unsupported ability task kind.";
            row.detail = task.kind;
        }

        rows.push_back(std::move(row));
    }
    return rows;
}

urpg::GameplayEffect buildTaskEffect(const AuthoredAbilityAsset& asset, const AbilityOrchestrationTask& task) {
    urpg::GameplayEffect effect;
    effect.id = task.effect_id.empty() ? asset.effect_id : task.effect_id;
    effect.name = effect.id;
    effect.duration = asset.effect_duration;

    urpg::GameplayEffectModifier modifier;
    modifier.attributeName = asset.effect_attribute;
    modifier.operation = asset.effect_operation;
    modifier.value = asset.effect_value;
    effect.modifiers.push_back(modifier);
    return effect;
}

void applyTaskEffect(const AbilityOrchestrationDocument& document,
                     const AbilityOrchestrationTask& task,
                     const GameplayAbility::AbilityExecutionContext& context) {
    const auto effect = buildTaskEffect(document.ability, task);
    const auto lowerTarget = toLower(task.target);
    for (size_t index = 0; index < context.targets.size(); ++index) {
        const auto& target = context.targets[index];
        const bool apply = lowerTarget.empty() || lowerTarget == "all" ||
                           (lowerTarget == "primary" && index == 0) ||
                           toLower(target.runtimeId) == lowerTarget;
        if (apply && target.abilitySystem) {
            target.abilitySystem->applyEffect(effect);
        }
    }
}

bool isSupportedTaskKind(const std::string& kind) {
    return kind == "cost" || kind == "wait_input" || kind == "wait_event" ||
           kind == "wait_projectile_collision" || kind == "delay" ||
           kind == "apply_effect" || kind == "play_cue" ||
           kind == "branch_on_condition" || kind == "cooldown";
}

bool taskRequestsCancellation(const AbilityOrchestrationDocument& document) {
    return std::any_of(document.tasks.begin(), document.tasks.end(), [](const auto& task) {
        return task.kind == "wait_input" && task.skip_cooldown_on_cancel && toLower(task.action) == "cancel";
    });
}

struct TaskGraphRunState {
    const AbilityOrchestrationDocument& document;
    const std::unordered_map<std::string, const AbilityOrchestrationTask*>& taskById;
    AbilitySystemComponent& sourceAsc;
    const GameplayAbility::AbilityExecutionContext& context;
    AbilityOrchestrationResult& result;
    std::unordered_set<std::string> completed;
    std::unordered_set<std::string> visited;
    bool cancelled = false;
    size_t sequence = 0;

    void addEvent(const AbilityOrchestrationTask& task, std::string status, std::string detail = {}) {
        result.task_execution_events.push_back({++sequence, task.id, task.kind, std::move(status), std::move(detail)});
    }

    bool dependenciesSatisfied(const AbilityOrchestrationTask& task) const {
        return std::all_of(task.depends_on.begin(), task.depends_on.end(), [this](const auto& dependency) {
            return completed.find(dependency) != completed.end();
        });
    }

    bool run(const std::string& id) {
        const auto it = taskById.find(id);
        if (it == taskById.end() || visited.find(id) != visited.end() || cancelled) {
            return !cancelled;
        }
        const auto& task = *it->second;
        if (!dependenciesSatisfied(task)) {
            return true;
        }

        visited.insert(id);
        addEvent(task, "started");

        if (task.kind == "wait_input" || task.kind == "wait_event" || task.kind == "wait_projectile_collision" ||
            task.kind == "delay") {
            addEvent(task, "waiting", task.action.empty() ? task.target : task.action);
            if (task.kind == "wait_input" && task.skip_cooldown_on_cancel && toLower(task.action) == "cancel") {
                addEvent(task, "cancelled", "input cancel");
                cancelled = true;
                result.activation_executed = false;
                result.blocking_reason = "cancelled";
                return false;
            }
            addEvent(task, "completed");
        } else if (task.kind == "branch_on_condition") {
            AbilityConditionEvaluator evaluator;
            const auto decision = evaluator.evaluate(task.condition, sourceAsc, &context);
            const bool branchTrue = decision.parsed && decision.value;
            const auto& next = branchTrue ? task.on_true : task.on_false;
            addEvent(task, "branched", std::string(branchTrue ? "true -> " : "false -> ") + next);
            completed.insert(task.id);
            return run(next);
        } else if (task.kind == "apply_effect") {
            applyTaskEffect(document, task, context);
            addEvent(task, "completed", task.effect_id.empty() ? document.ability.effect_id : task.effect_id);
        } else if (task.kind == "play_cue") {
            addEvent(task, "completed", task.cue_id);
        } else {
            addEvent(task, "completed");
        }

        completed.insert(task.id);
        if (!task.next.empty()) {
            return run(task.next);
        }
        return true;
    }
};

void runTaskGraph(const AbilityOrchestrationDocument& document,
                  AbilitySystemComponent& sourceAsc,
                  const GameplayAbility::AbilityExecutionContext& context,
                  AbilityOrchestrationResult& result) {
    if (document.tasks.empty()) {
        return;
    }

    std::unordered_map<std::string, const AbilityOrchestrationTask*> taskById;
    std::unordered_set<std::string> referenced;
    for (const auto& task : document.tasks) {
        taskById[task.id] = &task;
        if (!task.next.empty()) {
            referenced.insert(task.next);
        }
        if (!task.on_true.empty()) {
            referenced.insert(task.on_true);
        }
        if (!task.on_false.empty()) {
            referenced.insert(task.on_false);
        }
        for (const auto& dependency : task.depends_on) {
            referenced.insert(task.id);
            (void)dependency;
        }
    }

    TaskGraphRunState state{document, taskById, sourceAsc, context, result, {}, {}, false, 0};
    for (const auto& task : document.tasks) {
        if (referenced.find(task.id) != referenced.end() && task.depends_on.empty()) {
            continue;
        }
        state.run(task.id);
        for (const auto& candidate : document.tasks) {
            if (!candidate.depends_on.empty() && state.dependenciesSatisfied(candidate)) {
                state.run(candidate.id);
            }
        }
        if (state.cancelled) {
            break;
        }
    }
}

nlohmann::json taskPreviewRowsToJson(const std::vector<AbilityTaskPreviewRow>& rows) {
    nlohmann::json json = nlohmann::json::array();
    for (const auto& row : rows) {
        json.push_back({
            {"id", row.id},
            {"kind", row.kind},
            {"status", row.status},
            {"detail", row.detail},
            {"executable", row.executable},
            {"disabledReason", row.disabled_reason},
        });
    }
    return json;
}

std::vector<AbilityOrchestrationDiagnostic> patternDiagnostics(const AbilityOrchestrationDocument& document,
                                                               const OrchestratedAbility& ability) {
    std::vector<AbilityOrchestrationDiagnostic> diagnostics;
    AbilitySystemComponent source;
    for (const auto& target : document.targets) {
        if (source.isTargetInPattern(ability, document.source.x, document.source.y, target.x, target.y)) {
            continue;
        }
        diagnostics.push_back({
            "target_out_of_pattern",
            "Ability orchestration target is outside the authored ability pattern.",
            target.id,
        });
    }
    return diagnostics;
}

std::vector<AbilityOrchestrationTargetResult>
buildTargetResults(const AbilityOrchestrationDocument& document,
                   const std::vector<std::unique_ptr<AbilitySystemComponent>>& targetComponents,
                   const OrchestratedAbility& ability) {
    std::vector<AbilityOrchestrationTargetResult> results;
    for (size_t index = 0; index < document.targets.size(); ++index) {
        const auto& target = document.targets[index];
        const auto& asc = *targetComponents[index];
        results.push_back({
            target.id,
            target.effect_attribute_base,
            asc.getAttribute(document.ability.effect_attribute, target.effect_attribute_base),
            asc.getActiveEffectCount(),
            AbilitySystemComponent{}.isTargetInPattern(ability, document.source.x, document.source.y, target.x,
                                                       target.y),
        });
    }
    return results;
}

} // namespace

const char* abilityOrchestrationModeName(AbilityOrchestrationMode mode) {
    switch (mode) {
    case AbilityOrchestrationMode::Battle:
        return "battle";
    case AbilityOrchestrationMode::Map:
        return "map";
    }
    return "battle";
}

AbilityOrchestrationMode abilityOrchestrationModeFromString(const std::string& value) {
    if (value == "map") {
        return AbilityOrchestrationMode::Map;
    }
    return AbilityOrchestrationMode::Battle;
}

std::vector<AbilityOrchestrationDiagnostic> AbilityOrchestrationDocument::validate() const {
    std::vector<AbilityOrchestrationDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_orchestration_id", "Ability orchestration requires an id.", ""});
    }
    if (source.id.empty()) {
        diagnostics.push_back({"missing_source_id", "Ability orchestration requires a source actor.", id});
    }
    if (targets.empty()) {
        diagnostics.push_back({"missing_targets", "Ability orchestration requires at least one target.", id});
    }
    if (ability.ability_id.empty()) {
        diagnostics.push_back({"missing_ability_id", "Ability orchestration requires an ability id.", id});
    }
    if (ability.cooldown_seconds < 0.0f) {
        diagnostics.push_back({"invalid_cooldown", "Ability cooldown cannot be negative.", ability.ability_id});
    }
    if (ability.mp_cost < 0.0f) {
        diagnostics.push_back({"invalid_cost", "Ability MP cost cannot be negative.", ability.ability_id});
    }
    if (ability.effect_id.empty()) {
        diagnostics.push_back({"missing_effect_id", "Ability orchestration requires an effect id.", ability.ability_id});
    }
    if (ability.effect_attribute.empty()) {
        diagnostics.push_back({"missing_effect_attribute", "Ability orchestration requires an effect attribute.",
                               ability.ability_id});
    }
    if (source.mp < 0.0f) {
        diagnostics.push_back({"invalid_source_mp", "Ability orchestration source MP cannot be negative.", source.id});
    }
    for (const auto& tag : required_tags) {
        if (tag.empty()) {
            diagnostics.push_back({"empty_required_tag", "Ability orchestration required tag cannot be empty.", id});
        }
    }
    for (const auto& tag : blocking_tags) {
        if (tag.empty()) {
            diagnostics.push_back({"empty_blocking_tag", "Ability orchestration blocking tag cannot be empty.", id});
        }
    }
    for (const auto& target : targets) {
        if (target.id.empty()) {
            diagnostics.push_back({"missing_target_id", "Ability orchestration target requires an id.", id});
        }
    }
    std::unordered_set<std::string> taskIds;
    std::unordered_map<std::string, AbilityOrchestrationTask> taskById;
    for (const auto& task : tasks) {
        if (task.id.empty()) {
            diagnostics.push_back({"ability_task_missing_id", "Ability orchestration task requires an id.", id});
            continue;
        }
        if (!taskIds.insert(task.id).second) {
            diagnostics.push_back({"ability_task_duplicate_id", "Ability orchestration task id must be unique.", task.id});
        }
        taskById[task.id] = task;
    }

    AbilityConditionEvaluator evaluator;
    auto sourceAsc = buildConditionSource(*this);
    GameplayAbility::AbilityExecutionContext context;
    const auto conditionTargets = buildConditionTargets(*this, context);
    (void)conditionTargets;

    for (const auto& task : tasks) {
        if (!task.next.empty() && !hasTaskId(taskIds, task.next)) {
            diagnostics.push_back({"ability_task_missing_next",
                                   "Ability orchestration task next must reference a task id in the same document.",
                                   task.id});
        }
        for (const auto& dependency : task.depends_on) {
            if (!hasTaskId(taskIds, dependency)) {
                diagnostics.push_back({"ability_task_missing_dependency",
                                       "Ability orchestration task dependency must reference a task id in the same document.",
                                       task.id});
            }
        }
        if (task.kind == "branch_on_condition") {
            if (!hasTaskId(taskIds, task.on_true) || !hasTaskId(taskIds, task.on_false)) {
                diagnostics.push_back({"ability_task_branch_missing_target",
                                       "Branch task on_true and on_false must reference task ids in the same document.",
                                       task.id});
            }
            const auto condition = evaluator.evaluate(task.condition, sourceAsc, &context);
            if (!condition.parsed) {
                diagnostics.push_back({"ability_task_branch_condition_invalid",
                                       "Branch task condition uses unsupported grammar.", task.id});
            }
            const auto trueIt = taskById.find(task.on_true);
            const auto falseIt = taskById.find(task.on_false);
            const auto pointsBack = [&task](const AbilityOrchestrationTask& next) {
                return next.kind == "branch_on_condition" && (next.on_true == task.id || next.on_false == task.id);
            };
            if ((trueIt != taskById.end() && pointsBack(trueIt->second)) ||
                (falseIt != taskById.end() && pointsBack(falseIt->second))) {
                diagnostics.push_back({"ability_task_branch_cycle",
                                       "Branch task cannot create an immediate two-node cycle.", task.id});
            }
        } else if (task.kind == "script") {
            diagnostics.push_back({"ability_task_script_unsupported",
                                   "Ability orchestration rejects arbitrary script task strings.", task.id});
        } else if (!isSupportedTaskKind(task.kind)) {
            diagnostics.push_back({"ability_task_kind_unsupported",
                                   "Ability orchestration task kind is unsupported.", task.id});
        }
    }

    enum class VisitState { Visiting, Done };
    std::unordered_map<std::string, VisitState> visitState;
    auto referencesFor = [&taskById](const AbilityOrchestrationTask& task) {
        std::vector<std::string> refs;
        if (!task.next.empty()) {
            refs.push_back(task.next);
        }
        if (!task.on_true.empty()) {
            refs.push_back(task.on_true);
        }
        if (!task.on_false.empty()) {
            refs.push_back(task.on_false);
        }
        for (const auto& other : taskById) {
            if (std::find(other.second.depends_on.begin(), other.second.depends_on.end(), task.id) !=
                other.second.depends_on.end()) {
                refs.push_back(other.first);
            }
        }
        return refs;
    };
    bool foundCycle = false;
    std::function<void(const std::string&)> visit = [&](const std::string& taskId) {
        if (foundCycle) {
            return;
        }
        const auto stateIt = visitState.find(taskId);
        if (stateIt != visitState.end()) {
            if (stateIt->second == VisitState::Visiting) {
                diagnostics.push_back({"ability_task_graph_cycle",
                                       "Ability orchestration task graph cannot contain cycles.", taskId});
                foundCycle = true;
            }
            return;
        }
        const auto taskIt = taskById.find(taskId);
        if (taskIt == taskById.end()) {
            return;
        }
        visitState[taskId] = VisitState::Visiting;
        for (const auto& ref : referencesFor(taskIt->second)) {
            visit(ref);
        }
        visitState[taskId] = VisitState::Done;
    };
    for (const auto& task : tasks) {
        visit(task.id);
    }
    return diagnostics;
}

nlohmann::json AbilityOrchestrationDocument::toJson() const {
    nlohmann::json abilityJson = ability;
    nlohmann::json targetJson = nlohmann::json::array();
    for (const auto& target : targets) {
        targetJson.push_back(actorToJson(target));
    }
    nlohmann::json taskJson = nlohmann::json::array();
    for (const auto& task : tasks) {
        taskJson.push_back(taskToJson(task));
    }
    return {
        {"schema", "urpg.ability_orchestration.v1"},
        {"id", id},
        {"mode", abilityOrchestrationModeName(mode)},
        {"ability", abilityJson},
        {"source", actorToJson(source)},
        {"targets", targetJson},
        {"required_tags", required_tags},
        {"blocking_tags", blocking_tags},
        {"battle_turn", battle_turn},
        {"battle_speed", battle_speed},
        {"battle_priority", battle_priority},
        {"tasks", taskJson},
    };
}

AbilityOrchestrationDocument AbilityOrchestrationDocument::fromJson(const nlohmann::json& json) {
    AbilityOrchestrationDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", "");
    document.mode = abilityOrchestrationModeFromString(json.value("mode", std::string("battle")));
    if (const auto ability = json.find("ability"); ability != json.end() && ability->is_object()) {
        document.ability = ability->get<AuthoredAbilityAsset>();
    }
    if (const auto source = json.find("source"); source != json.end() && source->is_object()) {
        document.source = actorFromJson(*source);
    }
    if (const auto targets = json.find("targets"); targets != json.end() && targets->is_array()) {
        document.targets.clear();
        for (const auto& target : *targets) {
            document.targets.push_back(actorFromJson(target));
        }
    }
    document.required_tags = stringsFromJsonArray(json.value("required_tags", nlohmann::json::array()));
    document.blocking_tags = stringsFromJsonArray(json.value("blocking_tags", nlohmann::json::array()));
    document.battle_turn = json.value("battle_turn", document.battle_turn);
    document.battle_speed = json.value("battle_speed", document.battle_speed);
    document.battle_priority = json.value("battle_priority", document.battle_priority);
    if (const auto tasks = json.find("tasks"); tasks != json.end() && tasks->is_array()) {
        document.tasks.clear();
        for (const auto& task : *tasks) {
            document.tasks.push_back(taskFromJson(task));
        }
    }
    return document;
}

AbilityOrchestrationResult runAbilityOrchestration(const AbilityOrchestrationDocument& document) {
    AbilityOrchestrationResult result;
    result.document_id = document.id;
    result.ability_id = document.ability.ability_id;
    result.mode = document.mode;
    result.source_mp_before = document.source.mp;
    result.task_preview_rows = buildTaskPreviewRows(document);

    result.diagnostics = document.validate();
    if (!result.diagnostics.empty()) {
        return result;
    }

    result.valid = true;
    AbilitySystemComponent sourceAsc;
    sourceAsc.setAttribute("MP", document.source.mp);
    sourceAsc.setAttribute(document.ability.effect_attribute, document.source.effect_attribute_base);
    addTags(sourceAsc, document.source.tags);

    std::vector<std::unique_ptr<AbilitySystemComponent>> targetComponents;
    targetComponents.reserve(document.targets.size());
    GameplayAbility::AbilityExecutionContext context;
    context.sourceRuntimeId = document.source.id;
    for (const auto& target : document.targets) {
        auto targetAsc = std::make_unique<AbilitySystemComponent>();
        targetAsc->setAttribute("MP", target.mp);
        targetAsc->setAttribute(document.ability.effect_attribute, target.effect_attribute_base);
        addTags(*targetAsc, target.tags);
        context.targets.push_back({targetAsc.get(), nullptr, target.id});
        targetComponents.push_back(std::move(targetAsc));
    }

    const bool hasTaskGraph = !document.tasks.empty();
    auto ability = std::make_shared<OrchestratedAbility>(document.ability, document.required_tags,
                                                        document.blocking_tags, hasTaskGraph);
    result.diagnostics = patternDiagnostics(document, *ability);
    if (!result.diagnostics.empty()) {
        result.source_mp_after = sourceAsc.getAttribute("MP", 0.0f);
        result.cooldown_after = sourceAsc.getCooldownRemaining(document.ability.ability_id);
        result.targets = buildTargetResults(document, targetComponents, *ability);
        return result;
    }

    result.activation_attempted = true;
    const bool cancelledBeforeCommit = taskRequestsCancellation(document);
    if (cancelledBeforeCommit) {
        runTaskGraph(document, sourceAsc, context, result);
    } else if (document.mode == AbilityOrchestrationMode::Battle) {
        sourceAsc.grantAbility(ability);
        AbilityBattleQueue queue;
        queue.enqueue({document.source.id, {}, document.ability.ability_id, document.battle_speed,
                       document.battle_priority});
        const auto snapshot = queue.flush({{document.source.id, &sourceAsc}}, document.battle_turn);
        result.battle_snapshot = snapshot.toJson();
        result.battle_commands_received = snapshot.commands_received;
        result.battle_commands_executed = snapshot.commands_executed;
        result.battle_commands_blocked = snapshot.commands_blocked;
        result.activation_executed = snapshot.commands_executed == 1;
        if (!snapshot.outcomes.empty()) {
            result.blocking_reason = snapshot.outcomes.front().reason;
        }
        if (result.activation_executed && hasTaskGraph) {
            runTaskGraph(document, sourceAsc, context, result);
        } else if (result.activation_executed) {
            ability->applyEffectTargets(context);
        }
    } else {
        const auto check = ability->evaluateActivation(sourceAsc, context);
        result.blocking_reason = check.reason;
        result.activation_executed = sourceAsc.tryActivateAbility(*ability, context);
        if (result.activation_executed && hasTaskGraph) {
            runTaskGraph(document, sourceAsc, context, result);
        }
    }

    result.source_mp_after = sourceAsc.getAttribute("MP", 0.0f);
    result.cooldown_after = sourceAsc.getCooldownRemaining(document.ability.ability_id);
    result.targets = buildTargetResults(document, targetComponents, *ability);
    return result;
}

nlohmann::json abilityOrchestrationResultToJson(const AbilityOrchestrationResult& result) {
    nlohmann::json targetJson = nlohmann::json::array();
    for (const auto& target : result.targets) {
        targetJson.push_back({
            {"id", target.id},
            {"effectAttributeBefore", target.effect_attribute_before},
            {"effectAttributeAfter", target.effect_attribute_after},
            {"activeEffectCount", target.active_effect_count},
            {"inPattern", target.in_pattern},
        });
    }

    nlohmann::json diagnosticsJson = nlohmann::json::array();
    for (const auto& diagnostic : result.diagnostics) {
        diagnosticsJson.push_back({
            {"code", diagnostic.code},
            {"message", diagnostic.message},
            {"target", diagnostic.target},
        });
    }

    nlohmann::json taskExecutionJson = nlohmann::json::array();
    for (const auto& event : result.task_execution_events) {
        taskExecutionJson.push_back({
            {"sequence", event.sequence},
            {"taskId", event.task_id},
            {"kind", event.kind},
            {"status", event.status},
            {"detail", event.detail},
        });
    }

    return {
        {"documentId", result.document_id},
        {"abilityId", result.ability_id},
        {"mode", abilityOrchestrationModeName(result.mode)},
        {"valid", result.valid},
        {"activationAttempted", result.activation_attempted},
        {"activationExecuted", result.activation_executed},
        {"blockingReason", result.blocking_reason},
        {"sourceMpBefore", result.source_mp_before},
        {"sourceMpAfter", result.source_mp_after},
        {"cooldownAfter", result.cooldown_after},
        {"battleCommandsReceived", result.battle_commands_received},
        {"battleCommandsExecuted", result.battle_commands_executed},
        {"battleCommandsBlocked", result.battle_commands_blocked},
        {"targets", targetJson},
        {"taskPreviewRows", taskPreviewRowsToJson(result.task_preview_rows)},
        {"taskExecutionEvents", taskExecutionJson},
        {"diagnostics", diagnosticsJson},
        {"battleSnapshot", result.battle_snapshot},
    };
}

} // namespace urpg::ability
