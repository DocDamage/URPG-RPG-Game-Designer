#include "engine/core/ability/ability_sandbox.h"

#include <algorithm>

namespace urpg::ability {
namespace {

class SandboxAbility final : public GameplayAbility {
public:
    SandboxAbility(AuthoredAbilityAsset asset, std::vector<std::string> required_tags,
                   std::vector<std::string> blocking_tags)
        : asset_(std::move(asset)),
          required_tags_(std::move(required_tags)),
          blocking_tags_(std::move(blocking_tags)),
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
        commitAbility(source);

        urpg::GameplayEffect effect;
        effect.id = asset_.effect_id;
        effect.name = asset_.effect_id;
        effect.duration = asset_.effect_duration;

        urpg::GameplayEffectModifier modifier;
        modifier.attributeName = asset_.effect_attribute;
        modifier.operation = asset_.effect_operation;
        modifier.value = asset_.effect_value;
        effect.modifiers.push_back(modifier);

        source.applyEffect(effect);
    }

private:
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

} // namespace

std::vector<AbilitySandboxDiagnostic> AbilitySandboxDocument::validate() const {
    std::vector<AbilitySandboxDiagnostic> diagnostics;
    if (id.empty()) {
        diagnostics.push_back({"missing_sandbox_id", "Ability sandbox requires an id.", ""});
    }
    if (ability.ability_id.empty()) {
        diagnostics.push_back({"missing_ability_id", "Ability sandbox requires an ability id.", ""});
    }
    if (ability.cooldown_seconds < 0.0f) {
        diagnostics.push_back({"invalid_cooldown", "Ability cooldown cannot be negative.", ability.ability_id});
    }
    if (ability.mp_cost < 0.0f) {
        diagnostics.push_back({"invalid_cost", "Ability MP cost cannot be negative.", ability.ability_id});
    }
    if (ability.effect_id.empty()) {
        diagnostics.push_back({"missing_effect_id", "Ability sandbox requires an effect id.", ability.ability_id});
    }
    if (ability.effect_attribute.empty()) {
        diagnostics.push_back({"missing_effect_attribute", "Ability sandbox requires an effect attribute.",
                               ability.ability_id});
    }
    if (source_mp < 0.0f) {
        diagnostics.push_back({"invalid_source_mp", "Ability sandbox source MP cannot be negative.", id});
    }
    if (activation_attempts <= 0) {
        diagnostics.push_back({"invalid_activation_attempts", "Ability sandbox requires at least one activation attempt.",
                               id});
    }
    if (seconds_between_attempts < 0.0f) {
        diagnostics.push_back({"invalid_attempt_interval", "Ability sandbox attempt interval cannot be negative.", id});
    }
    for (const auto& tag : required_tags) {
        if (tag.empty()) {
            diagnostics.push_back({"empty_required_tag", "Ability sandbox required tag cannot be empty.", id});
        }
    }
    for (const auto& tag : blocking_tags) {
        if (tag.empty()) {
            diagnostics.push_back({"empty_blocking_tag", "Ability sandbox blocking tag cannot be empty.", id});
        }
    }
    return diagnostics;
}

nlohmann::json AbilitySandboxDocument::toJson() const {
    nlohmann::json ability_json = ability;
    return {
        {"schema", "urpg.ability_sandbox.v1"},
        {"id", id},
        {"ability", ability_json},
        {"source_mp", source_mp},
        {"source_attack", source_attack},
        {"activation_attempts", activation_attempts},
        {"seconds_between_attempts", seconds_between_attempts},
        {"source_tags", source_tags},
        {"required_tags", required_tags},
        {"blocking_tags", blocking_tags},
    };
}

AbilitySandboxDocument AbilitySandboxDocument::fromJson(const nlohmann::json& json) {
    AbilitySandboxDocument document;
    if (!json.is_object()) {
        return document;
    }
    document.id = json.value("id", "");
    if (const auto ability = json.find("ability"); ability != json.end() && ability->is_object()) {
        document.ability = ability->get<AuthoredAbilityAsset>();
    }
    document.source_mp = json.value("source_mp", 100.0f);
    document.source_attack = json.value("source_attack", 10.0f);
    document.activation_attempts = json.value("activation_attempts", 1);
    document.seconds_between_attempts = json.value("seconds_between_attempts", 0.0f);
    document.source_tags = stringsFromJsonArray(json.value("source_tags", nlohmann::json::array()));
    document.required_tags = stringsFromJsonArray(json.value("required_tags", nlohmann::json::array()));
    document.blocking_tags = stringsFromJsonArray(json.value("blocking_tags", nlohmann::json::array()));
    return document;
}

AbilitySandboxResult RunAbilitySandbox(const AbilitySandboxDocument& document) {
    AbilitySandboxResult result;
    result.diagnostics = document.validate();
    result.mp_before = document.source_mp;
    result.effect_attribute_before = document.source_attack;
    if (!result.diagnostics.empty()) {
        return result;
    }

    AbilitySystemComponent asc;
    asc.setAttribute("MP", document.source_mp);
    asc.setAttribute(document.ability.effect_attribute, document.source_attack);
    for (const auto& tag : document.source_tags) {
        asc.addTag(GameplayTag(tag));
    }

    SandboxAbility ability(document.ability, document.required_tags, document.blocking_tags);
    result.runtime_trace.push_back("load_ability:" + document.ability.ability_id);
    result.runtime_trace.push_back("source_mp:" + std::to_string(document.source_mp));
    for (int32_t attempt = 0; attempt < document.activation_attempts; ++attempt) {
        if (attempt > 0 && document.seconds_between_attempts > 0.0f) {
            asc.update(document.seconds_between_attempts);
            result.runtime_trace.push_back("tick:" + std::to_string(document.seconds_between_attempts));
        }

        AbilitySandboxActivationStep step;
        step.attempt_index = static_cast<size_t>(attempt + 1);
        step.mp_before = asc.getAttribute("MP", 0.0f);
        step.cooldown_before = asc.getCooldownRemaining(document.ability.ability_id);
        const auto check = ability.evaluateActivation(asc);
        step.allowed = check.allowed;
        step.blocking_reason = check.reason;
        step.executed = asc.tryActivateAbility(ability);
        step.mp_after = asc.getAttribute("MP", 0.0f);
        step.cooldown_after = asc.getCooldownRemaining(document.ability.ability_id);
        step.effect_attribute_after = asc.getAttribute(document.ability.effect_attribute, document.source_attack);
        step.active_effect_count = asc.getActiveEffectCount();
        if (!step.executed && step.blocking_reason.empty()) {
            step.blocking_reason = "activation_failed";
        }
        result.runtime_trace.push_back(std::string("attempt:") + std::to_string(step.attempt_index) + ":" +
                                       (step.executed ? "executed" : "blocked") +
                                       (step.blocking_reason.empty() ? "" : ":" + step.blocking_reason));
        result.activation_steps.push_back(step);
    }

    if (!result.activation_steps.empty()) {
        const auto& first = result.activation_steps.front();
        result.activation_allowed = first.allowed;
        result.blocking_reason = first.blocking_reason;
        result.activation_executed = first.executed;
    }
    result.mp_after = asc.getAttribute("MP", 0.0f);
    result.cooldown_after = asc.getCooldownRemaining(document.ability.ability_id);
    result.effect_attribute_after = asc.getAttribute(document.ability.effect_attribute, document.source_attack);
    result.active_effect_count = asc.getActiveEffectCount();
    result.execution_history_count = asc.getAbilityExecutionHistory().size();
    for (const auto& tag : asc.getTags().getTags()) {
        result.visible_tags.push_back(tag.getName());
    }
    std::sort(result.visible_tags.begin(), result.visible_tags.end());
    result.active_tag_count = result.visible_tags.size();
    if (!result.activation_executed && result.blocking_reason.empty()) {
        result.blocking_reason = "activation_failed";
    }
    return result;
}

} // namespace urpg::ability
