#include "engine/core/ability/ability_orchestration.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace urpg::ability {
namespace {

class OrchestratedAbility final : public GameplayAbility {
public:
    OrchestratedAbility(AuthoredAbilityAsset asset, std::vector<std::string> requiredTags,
                        std::vector<std::string> blockingTags)
        : asset_(std::move(asset)),
          required_tags_(std::move(requiredTags)),
          blocking_tags_(std::move(blockingTags)),
          pattern_(std::make_shared<urpg::PatternField>(asset_.pattern)) {
        id = asset_.ability_id;
        cooldownTime = asset_.cooldown_seconds;
        mpCost = asset_.mp_cost;
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
    return diagnostics;
}

nlohmann::json AbilityOrchestrationDocument::toJson() const {
    nlohmann::json abilityJson = ability;
    nlohmann::json targetJson = nlohmann::json::array();
    for (const auto& target : targets) {
        targetJson.push_back(actorToJson(target));
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
    return document;
}

AbilityOrchestrationResult runAbilityOrchestration(const AbilityOrchestrationDocument& document) {
    AbilityOrchestrationResult result;
    result.document_id = document.id;
    result.ability_id = document.ability.ability_id;
    result.mode = document.mode;
    result.source_mp_before = document.source.mp;

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

    auto ability = std::make_shared<OrchestratedAbility>(document.ability, document.required_tags,
                                                        document.blocking_tags);
    result.diagnostics = patternDiagnostics(document, *ability);
    if (!result.diagnostics.empty()) {
        result.source_mp_after = sourceAsc.getAttribute("MP", 0.0f);
        result.cooldown_after = sourceAsc.getCooldownRemaining(document.ability.ability_id);
        result.targets = buildTargetResults(document, targetComponents, *ability);
        return result;
    }

    result.activation_attempted = true;
    if (document.mode == AbilityOrchestrationMode::Battle) {
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
        if (result.activation_executed) {
            ability->applyEffectTargets(context);
        }
    } else {
        const auto check = ability->evaluateActivation(sourceAsc, context);
        result.blocking_reason = check.reason;
        result.activation_executed = sourceAsc.tryActivateAbility(*ability, context);
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
        {"diagnostics", diagnosticsJson},
        {"battleSnapshot", result.battle_snapshot},
    };
}

} // namespace urpg::ability
