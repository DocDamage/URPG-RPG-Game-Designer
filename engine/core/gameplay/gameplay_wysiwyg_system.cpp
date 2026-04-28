#include "engine/core/gameplay/gameplay_wysiwyg_system.h"

#include <algorithm>
#include <utility>

namespace urpg::gameplay {

namespace {

GameplayWysiwygRule ruleFromJson(const nlohmann::json& json) {
    GameplayWysiwygRule rule;
    rule.id = json.value("id", "");
    rule.label = json.value("label", "");
    rule.trigger = json.value("trigger", "");
    rule.target = json.value("target", "");
    rule.effect = json.value("effect", "");
    rule.value = json.value("value", 0);
    rule.duration = json.value("duration", 0);
    rule.required_flags = json.value("required_flags", std::vector<std::string>{});
    rule.grants_flags = json.value("grants_flags", std::vector<std::string>{});
    rule.variable_writes = json.value("variable_writes", std::map<std::string, std::string>{});
    rule.resource_delta = json.value("resource_delta", std::map<std::string, int32_t>{});
    return rule;
}

nlohmann::json ruleToJson(const GameplayWysiwygRule& rule) {
    return {{"id", rule.id},
            {"label", rule.label},
            {"trigger", rule.trigger},
            {"target", rule.target},
            {"effect", rule.effect},
            {"value", rule.value},
            {"duration", rule.duration},
            {"required_flags", rule.required_flags},
            {"grants_flags", rule.grants_flags},
            {"variable_writes", rule.variable_writes},
            {"resource_delta", rule.resource_delta}};
}

bool hasAllFlags(const GameplayWysiwygState& state, const std::vector<std::string>& flags) {
    return std::all_of(flags.begin(), flags.end(), [&](const auto& flag) { return state.flags.contains(flag); });
}

void applyRule(GameplayWysiwygState& state, const GameplayWysiwygRule& rule) {
    for (const auto& flag : rule.grants_flags) {
        state.flags.insert(flag);
    }
    for (const auto& [name, value] : rule.variable_writes) {
        state.variables[name] = value;
    }
    for (const auto& [name, delta] : rule.resource_delta) {
        state.resources[name] += delta;
    }
}

nlohmann::json stateToJson(const GameplayWysiwygState& state) {
    return {{"flags", state.flags}, {"variables", state.variables}, {"resources", state.resources}};
}

} // namespace

std::vector<GameplayWysiwygDiagnostic> GameplayWysiwygDocument::validate() const {
    std::vector<GameplayWysiwygDiagnostic> diagnostics;
    if (schema_version != "urpg.gameplay_wysiwyg.v1") {
        diagnostics.push_back({"invalid_schema_version", "Gameplay WYSIWYG document has an unsupported schema version.", id});
    }
    if (id.empty()) {
        diagnostics.push_back({"missing_document_id", "Gameplay WYSIWYG document is missing an id.", id});
    }
    if (feature_type.empty()) {
        diagnostics.push_back({"missing_feature_type", "Gameplay WYSIWYG document is missing a feature type.", id});
    }
    if (display_name.empty()) {
        diagnostics.push_back({"missing_display_name", "Gameplay WYSIWYG document is missing a display name.", id});
    }
    if (visual_layers.empty()) {
        diagnostics.push_back({"missing_visual_layers", "Gameplay WYSIWYG document must define at least one visual authoring layer.", id});
    }

    std::set<std::string> rule_ids;
    for (const auto& rule : rules) {
        if (rule.id.empty()) {
            diagnostics.push_back({"missing_rule_id", "Gameplay WYSIWYG rule is missing an id.", rule.id});
        } else if (!rule_ids.insert(rule.id).second) {
            diagnostics.push_back({"duplicate_rule_id", "Gameplay WYSIWYG rule id is duplicated.", rule.id});
        }
        if (rule.label.empty()) {
            diagnostics.push_back({"missing_rule_label", "Gameplay WYSIWYG rule is missing a label.", rule.id});
        }
        if (rule.trigger.empty()) {
            diagnostics.push_back({"missing_rule_trigger", "Gameplay WYSIWYG rule is missing a trigger.", rule.id});
        }
        if (rule.target.empty()) {
            diagnostics.push_back({"missing_rule_target", "Gameplay WYSIWYG rule is missing a target.", rule.id});
        }
        if (rule.effect.empty()) {
            diagnostics.push_back({"missing_rule_effect", "Gameplay WYSIWYG rule is missing an effect.", rule.id});
        }
        if (rule.duration < 0) {
            diagnostics.push_back({"negative_rule_duration", "Gameplay WYSIWYG rule duration cannot be negative.", rule.id});
        }
    }
    return diagnostics;
}

GameplayWysiwygPreview GameplayWysiwygDocument::preview(const GameplayWysiwygState& state, const std::string& trigger) const {
    GameplayWysiwygPreview preview;
    preview.feature_id = id;
    preview.feature_type = feature_type;
    preview.trigger = trigger;
    preview.resulting_state = state;
    preview.diagnostics = validate();

    for (const auto& rule : rules) {
        if (rule.trigger != trigger || !hasAllFlags(state, rule.required_flags)) {
            continue;
        }
        preview.active_rules.push_back(rule);
        preview.events.push_back({rule.id, rule.target, rule.effect, rule.value, rule.duration});
        applyRule(preview.resulting_state, rule);
    }
    if (preview.active_rules.empty()) {
        preview.diagnostics.push_back({"no_active_rules", "No gameplay WYSIWYG rules activate for trigger: " + trigger, id});
    }
    return preview;
}

GameplayWysiwygPreview GameplayWysiwygDocument::execute(GameplayWysiwygState& state, const std::string& trigger) const {
    auto result = preview(state, trigger);
    state = result.resulting_state;
    return result;
}

nlohmann::json GameplayWysiwygDocument::toJson() const {
    nlohmann::json json;
    json["schema_version"] = schema_version;
    json["id"] = id;
    json["feature_type"] = feature_type;
    json["display_name"] = display_name;
    json["visual_layers"] = visual_layers;
    json["rules"] = nlohmann::json::array();
    for (const auto& rule : rules) {
        json["rules"].push_back(ruleToJson(rule));
    }
    return json;
}

GameplayWysiwygDocument GameplayWysiwygDocument::fromJson(const nlohmann::json& json) {
    GameplayWysiwygDocument document;
    document.schema_version = json.value("schema_version", "urpg.gameplay_wysiwyg.v1");
    document.id = json.value("id", "");
    document.feature_type = json.value("feature_type", "");
    document.display_name = json.value("display_name", "");
    document.visual_layers = json.value("visual_layers", std::vector<std::string>{});
    for (const auto& rule_json : json.value("rules", nlohmann::json::array())) {
        document.rules.push_back(ruleFromJson(rule_json));
    }
    return document;
}

std::vector<std::string> gameplayWysiwygFeatureTypes() {
    return {"status_effect_designer",
            "enemy_ai_behavior_tree",
            "boss_phase_script",
            "equipment_set_bonus",
            "dungeon_room_flow",
            "companion_banter",
            "quest_choice_consequence",
            "shop_economy_sim_lab",
            "puzzle_mechanic_builder",
            "world_state_timeline",
            "tactical_terrain_effects",
            "procedural_content_rules"};
}

nlohmann::json gameplayWysiwygPreviewToJson(const GameplayWysiwygPreview& preview) {
    nlohmann::json json{{"feature_id", preview.feature_id},
                        {"feature_type", preview.feature_type},
                        {"trigger", preview.trigger},
                        {"resulting_state", stateToJson(preview.resulting_state)}};
    json["active_rules"] = nlohmann::json::array();
    for (const auto& rule : preview.active_rules) {
        json["active_rules"].push_back(ruleToJson(rule));
    }
    json["events"] = nlohmann::json::array();
    for (const auto& event : preview.events) {
        json["events"].push_back({{"rule_id", event.rule_id},
                                  {"target", event.target},
                                  {"effect", event.effect},
                                  {"value", event.value},
                                  {"duration", event.duration}});
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return json;
}

} // namespace urpg::gameplay
