#include "engine/core/community/community_wysiwyg_feature.h"

#include <algorithm>

namespace urpg::community {

namespace {

CommunityFeatureAction actionFromJson(const nlohmann::json& json) {
    CommunityFeatureAction action;
    action.id = json.value("id", "");
    action.label = json.value("label", "");
    action.trigger = json.value("trigger", "");
    action.command = json.value("command", "");
    action.target = json.value("target", "");
    action.required_flags = json.value("required_flags", std::vector<std::string>{});
    action.parameters = json.value("parameters", std::map<std::string, std::string>{});
    return action;
}

nlohmann::json actionToJson(const CommunityFeatureAction& action) {
    return {{"id", action.id},
            {"label", action.label},
            {"trigger", action.trigger},
            {"command", action.command},
            {"target", action.target},
            {"required_flags", action.required_flags},
            {"parameters", action.parameters}};
}

bool hasAllFlags(const CommunityFeatureRuntimeState& state, const std::vector<std::string>& flags) {
    return std::all_of(flags.begin(), flags.end(), [&](const auto& flag) { return state.flags.contains(flag); });
}

void applyAction(CommunityFeatureRuntimeState& state, const CommunityFeatureAction& action) {
    state.emitted_commands.push_back(action.command + ":" + action.target);
    for (const auto& [key, value] : action.parameters) {
        state.variables[key] = value;
    }
}

nlohmann::json stateToJson(const CommunityFeatureRuntimeState& state) {
    return {{"flags", state.flags}, {"variables", state.variables}, {"emitted_commands", state.emitted_commands}};
}

} // namespace

std::vector<CommunityFeatureDiagnostic> CommunityWysiwygFeatureDocument::validate() const {
    std::vector<CommunityFeatureDiagnostic> diagnostics;
    if (schema_version != "urpg.community_wysiwyg.v1") {
        diagnostics.push_back({"invalid_schema_version", "Community WYSIWYG feature has an unsupported schema version.", id});
    }
    if (id.empty()) {
        diagnostics.push_back({"missing_document_id", "Community WYSIWYG feature is missing an id.", id});
    }
    if (feature_type.empty()) {
        diagnostics.push_back({"missing_feature_type", "Community WYSIWYG feature is missing a feature type.", id});
    }
    if (display_name.empty()) {
        diagnostics.push_back({"missing_display_name", "Community WYSIWYG feature is missing a display name.", id});
    }
    if (visual_layers.empty()) {
        diagnostics.push_back({"missing_visual_layers", "Community WYSIWYG feature needs at least one visual authoring layer.", id});
    }

    std::set<std::string> action_ids;
    for (const auto& action : actions) {
        if (action.id.empty()) {
            diagnostics.push_back({"missing_action_id", "Community WYSIWYG action is missing an id.", action.id});
        } else if (!action_ids.insert(action.id).second) {
            diagnostics.push_back({"duplicate_action_id", "Community WYSIWYG action id is duplicated.", action.id});
        }
        if (action.trigger.empty()) {
            diagnostics.push_back({"missing_action_trigger", "Community WYSIWYG action is missing a trigger.", action.id});
        }
        if (action.command.empty()) {
            diagnostics.push_back({"missing_action_command", "Community WYSIWYG action is missing a command.", action.id});
        }
        if (action.target.empty()) {
            diagnostics.push_back({"missing_action_target", "Community WYSIWYG action is missing a target.", action.id});
        }
    }
    return diagnostics;
}

CommunityFeaturePreview CommunityWysiwygFeatureDocument::preview(const CommunityFeatureRuntimeState& state,
                                                                 const std::string& trigger) const {
    CommunityFeaturePreview preview;
    preview.feature_type = feature_type;
    preview.trigger = trigger;
    preview.resulting_state = state;
    preview.diagnostics = validate();
    for (const auto& action : actions) {
        if (action.trigger == trigger && hasAllFlags(state, action.required_flags)) {
            preview.active_actions.push_back(action);
            applyAction(preview.resulting_state, action);
        }
    }
    if (preview.active_actions.empty()) {
        preview.diagnostics.push_back({"no_active_actions", "No community WYSIWYG actions activate for trigger: " + trigger, id});
    }
    return preview;
}

CommunityFeaturePreview CommunityWysiwygFeatureDocument::execute(CommunityFeatureRuntimeState& state,
                                                                 const std::string& trigger) const {
    auto result = preview(state, trigger);
    state = result.resulting_state;
    return result;
}

nlohmann::json CommunityWysiwygFeatureDocument::toJson() const {
    nlohmann::json json;
    json["schema_version"] = schema_version;
    json["id"] = id;
    json["feature_type"] = feature_type;
    json["display_name"] = display_name;
    json["visual_layers"] = visual_layers;
    json["actions"] = nlohmann::json::array();
    for (const auto& action : actions) {
        json["actions"].push_back(actionToJson(action));
    }
    return json;
}

CommunityWysiwygFeatureDocument CommunityWysiwygFeatureDocument::fromJson(const nlohmann::json& json) {
    CommunityWysiwygFeatureDocument document;
    document.schema_version = json.value("schema_version", "urpg.community_wysiwyg.v1");
    document.id = json.value("id", "");
    document.feature_type = json.value("feature_type", "");
    document.display_name = json.value("display_name", "");
    document.visual_layers = json.value("visual_layers", std::vector<std::string>{});
    for (const auto& action_json : json.value("actions", nlohmann::json::array())) {
        document.actions.push_back(actionFromJson(action_json));
    }
    return document;
}

std::vector<std::string> communityWysiwygFeatureTypes() {
    return {"smart_event_workflow",
            "event_template_library",
            "interaction_prompt_system",
            "message_log_history",
            "minimap_fog_of_war",
            "picture_hotspot_common_event",
            "common_event_menu_builder",
            "developer_debug_overlay",
            "switch_variable_inspector",
            "asset_dlc_library_manager",
            "hud_maker",
            "plugin_conflict_resolver"};
}

nlohmann::json communityFeaturePreviewToJson(const CommunityFeaturePreview& preview) {
    nlohmann::json json{{"feature_type", preview.feature_type},
                        {"trigger", preview.trigger},
                        {"resulting_state", stateToJson(preview.resulting_state)}};
    json["active_actions"] = nlohmann::json::array();
    for (const auto& action : preview.active_actions) {
        json["active_actions"].push_back(actionToJson(action));
    }
    json["diagnostics"] = nlohmann::json::array();
    for (const auto& diagnostic : preview.diagnostics) {
        json["diagnostics"].push_back({{"code", diagnostic.code}, {"message", diagnostic.message}, {"id", diagnostic.id}});
    }
    return json;
}

} // namespace urpg::community
