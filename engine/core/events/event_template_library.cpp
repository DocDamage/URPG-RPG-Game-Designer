#include "engine/core/events/event_template_library.h"

#include <algorithm>
#include <set>

namespace urpg::events {
namespace {

std::string token(const std::string& id) { return "${" + id + "}"; }

std::string valueText(const nlohmann::json& value) {
    if (value.is_string()) return value.get<std::string>();
    if (value.is_boolean()) return value.get<bool>() ? "true" : "false";
    if (value.is_number_integer()) return std::to_string(value.get<int64_t>());
    return value.dump();
}

void substitute(std::string& text, const nlohmann::json& bindings) {
    for (const auto& [id, value] : bindings.items()) {
        const auto needle = token(id);
        std::size_t position = 0;
        while ((position = text.find(needle, position)) != std::string::npos) {
            const auto replacement = valueText(value);
            text.replace(position, needle.size(), replacement);
            position += replacement.size();
        }
    }
}

void substituteJson(nlohmann::json& value, const nlohmann::json& bindings) {
    if (value.is_string()) {
        auto text = value.get<std::string>();
        for (const auto& [id, binding] : bindings.items()) {
            if (text == token(id)) {
                value = binding;
                return;
            }
        }
        substitute(text, bindings);
        value = std::move(text);
    } else if (value.is_array()) {
        for (auto& child : value) substituteJson(child, bindings);
    } else if (value.is_object()) {
        for (auto& [_, child] : value.items()) substituteJson(child, bindings);
    }
}

const char* kindId(const EventTemplateKind kind) {
    return kind == EventTemplateKind::Narrative ? "narrative" : "common_event";
}

const char* parameterTypeId(const EventTemplateParameterType type) {
    switch (type) {
        case EventTemplateParameterType::String: return "string";
        case EventTemplateParameterType::Integer: return "integer";
        case EventTemplateParameterType::Boolean: return "boolean";
        case EventTemplateParameterType::Reference: return "reference";
    }
    return "string";
}

std::optional<EventTemplateParameterType> parameterType(const std::string& id) {
    if (id == "string") return EventTemplateParameterType::String;
    if (id == "integer") return EventTemplateParameterType::Integer;
    if (id == "boolean") return EventTemplateParameterType::Boolean;
    if (id == "reference") return EventTemplateParameterType::Reference;
    return std::nullopt;
}

nlohmann::json commandJson(const EventCommand& command) {
    return {{"id", command.id}, {"kind", toString(command.kind)}, {"target", command.target},
            {"value", command.value}, {"amount", command.amount},
            {"read_save_fields", command.read_save_fields}, {"write_save_fields", command.write_save_fields},
            {"payload", command.payload}, {"compat_fallback", command.compat_fallback}};
}

EventCommand commandFromJson(const nlohmann::json& json) {
    return {json.value("id", ""), eventCommandKindFromString(json.value("kind", "unsupported")),
            json.value("target", ""), json.value("value", ""), json.value("amount", int64_t{0}),
            json.value("read_save_fields", std::set<std::string>{}),
            json.value("write_save_fields", std::set<std::string>{}),
            json.value("payload", nlohmann::json::object()),
            json.value("compat_fallback", nlohmann::json::object())};
}

} // namespace

bool EventTemplateLibrary::registerDefinition(EventTemplateDefinition definition) {
    if (definition.id.empty() || definition.version == 0 || definition.commands.empty() ||
        definitions_.contains(definition.id)) return false;
    std::set<std::string> parameter_ids;
    for (const auto& parameter : definition.parameters) {
        if (parameter.id.empty() || !parameter_ids.insert(parameter.id).second ||
            (parameter.type == EventTemplateParameterType::Reference && parameter.reference_kind.empty())) return false;
    }
    definitions_.emplace(definition.id, std::move(definition));
    return true;
}

std::vector<std::string> EventTemplateLibrary::validateBindings(const std::string& template_id,
                                                                const nlohmann::json& bindings) const {
    std::vector<std::string> errors;
    const auto definition = definitions_.find(template_id);
    if (definition == definitions_.end()) return {"event_template_missing"};
    if (!bindings.is_object()) return {"event_template_bindings_not_object"};
    std::set<std::string> declared;
    for (const auto& parameter : definition->second.parameters) {
        declared.insert(parameter.id);
        const auto value = bindings.find(parameter.id);
        if (value == bindings.end()) {
            if (parameter.required && parameter.default_value.is_null()) errors.push_back("missing_parameter:" + parameter.id);
            continue;
        }
        const bool valid = (parameter.type == EventTemplateParameterType::String && value->is_string()) ||
                           (parameter.type == EventTemplateParameterType::Integer && value->is_number_integer()) ||
                           (parameter.type == EventTemplateParameterType::Boolean && value->is_boolean()) ||
                           (parameter.type == EventTemplateParameterType::Reference && value->is_string() && !value->get<std::string>().empty());
        if (!valid) errors.push_back("invalid_parameter_type:" + parameter.id);
    }
    for (const auto& [id, _] : bindings.items()) {
        if (!declared.contains(id)) errors.push_back("unknown_parameter:" + id);
    }
    return errors;
}

std::optional<CommonEventDefinition> EventTemplateLibrary::render(const EventTemplateInstance& instance,
                                                                  const EventTemplateDefinition& definition,
                                                                  std::string* error_code) const {
    auto bindings = instance.bindings;
    for (const auto& parameter : definition.parameters) {
        if (!bindings.contains(parameter.id) && !parameter.default_value.is_null()) bindings[parameter.id] = parameter.default_value;
    }
    const auto errors = validateBindings(definition.id, bindings);
    if (!errors.empty()) {
        if (error_code != nullptr) *error_code = errors.front();
        return std::nullopt;
    }
    CommonEventDefinition result{instance.common_event_id, definition.commands};
    for (auto& command : result.commands) {
        substitute(command.id, bindings);
        substitute(command.target, bindings);
        substitute(command.value, bindings);
        substituteJson(command.payload, bindings);
    }
    for (const auto& override_value : instance.overrides) {
        const auto command = std::find_if(result.commands.begin(), result.commands.end(), [&](const EventCommand& candidate) {
            return candidate.id == override_value.command_id;
        });
        if (command == result.commands.end()) {
            if (error_code != nullptr) *error_code = "event_template_override_command_missing";
            return std::nullopt;
        }
        if (override_value.field == "target" && override_value.value.is_string()) command->target = override_value.value.get<std::string>();
        else if (override_value.field == "value" && override_value.value.is_string()) command->value = override_value.value.get<std::string>();
        else if (override_value.field == "amount" && override_value.value.is_number_integer()) command->amount = override_value.value.get<int64_t>();
        else if (override_value.field == "payload") command->payload = override_value.value;
        else {
            if (error_code != nullptr) *error_code = "event_template_override_invalid";
            return std::nullopt;
        }
    }
    return result;
}

bool EventTemplateLibrary::instantiate(EventTemplateInstance instance, EventDocument& document,
                                       std::string* error_code) {
    const auto definition = definitions_.find(instance.template_id);
    if (instance.id.empty() || instance.common_event_id.empty() || definition == definitions_.end() ||
        instances_.contains(instance.id) || document.commonEvents().contains(instance.common_event_id)) {
        if (error_code != nullptr) *error_code = "event_template_instance_conflict";
        return false;
    }
    auto rendered = render(instance, definition->second, error_code);
    if (!rendered) return false;
    instance.template_version = definition->second.version;
    document.addCommonEvent(std::move(*rendered));
    instances_.emplace(instance.id, std::move(instance));
    return true;
}

std::vector<EventTemplateUse> EventTemplateLibrary::findUses(const std::string& template_id,
                                                             const EventDocument& document) const {
    const auto dependencies = EventDependencyGraph::build(document);
    std::vector<EventTemplateUse> result;
    for (const auto& [instance_id, instance] : instances_) {
        if (instance.template_id != template_id) continue;
        EventTemplateUse use{instance_id, instance.common_event_id, {}};
        for (const auto& edge : dependencies.edges()) {
            if (edge.target_type == "common_event" && edge.target_id == instance.common_event_id) {
                use.caller_source_ids.push_back(edge.source_id);
            }
        }
        result.push_back(std::move(use));
    }
    return result;
}

EventTemplateImpact EventTemplateLibrary::previewUpdate(const EventTemplateDefinition& replacement,
                                                        const EventDocument& document) const {
    EventTemplateImpact impact;
    const auto current = definitions_.find(replacement.id);
    if (current == definitions_.end()) { impact.code = "event_template_missing"; return impact; }
    if (replacement.version <= current->second.version) { impact.code = "event_template_version_not_newer"; return impact; }
    auto probe = *this;
    probe.definitions_[replacement.id] = replacement;
    for (const auto& [id, instance] : instances_) {
        if (instance.template_id != replacement.id) continue;
        std::string error;
        if (!probe.render(instance, replacement, &error)) { impact.code = error; return impact; }
        impact.affected_instance_ids.push_back(id);
        impact.preserved_override_count += instance.overrides.size();
    }
    for (const auto& use : findUses(replacement.id, document)) {
        impact.caller_source_ids.insert(impact.caller_source_ids.end(), use.caller_source_ids.begin(), use.caller_source_ids.end());
    }
    std::sort(impact.caller_source_ids.begin(), impact.caller_source_ids.end());
    impact.caller_source_ids.erase(std::unique(impact.caller_source_ids.begin(), impact.caller_source_ids.end()), impact.caller_source_ids.end());
    impact.affected_instance_count = impact.affected_instance_ids.size();
    impact.caller_count = impact.caller_source_ids.size();
    impact.valid = true;
    impact.code = "event_template_update_ready";
    return impact;
}

bool EventTemplateLibrary::applyUpdate(EventTemplateDefinition replacement, EventDocument& document,
                                       EventTemplateImpact* applied) {
    const auto impact = previewUpdate(replacement, document);
    if (!impact.valid) { if (applied != nullptr) *applied = impact; return false; }
    definitions_[replacement.id] = replacement;
    for (auto& [_, instance] : instances_) {
        if (instance.template_id != replacement.id) continue;
        auto rendered = render(instance, replacement);
        if (!rendered) return false;
        instance.template_version = replacement.version;
        document.addCommonEvent(std::move(*rendered));
    }
    if (applied != nullptr) *applied = impact;
    return true;
}

EventTemplateImpact EventTemplateLibrary::previewDelete(const std::string& template_id,
                                                        const std::string& replacement_template_id,
                                                        const EventDocument& document) const {
    EventTemplateImpact impact;
    if (!definitions_.contains(template_id)) { impact.code = "event_template_missing"; return impact; }
    const auto uses = findUses(template_id, document);
    for (const auto& use : uses) {
        impact.affected_instance_ids.push_back(use.instance_id);
        impact.caller_source_ids.insert(impact.caller_source_ids.end(), use.caller_source_ids.begin(), use.caller_source_ids.end());
        impact.preserved_override_count += instances_.at(use.instance_id).overrides.size();
    }
    impact.affected_instance_count = impact.affected_instance_ids.size();
    impact.caller_count = impact.caller_source_ids.size();
    if (!uses.empty() && replacement_template_id.empty()) { impact.code = "event_template_delete_has_uses"; return impact; }
    if (!replacement_template_id.empty() && !definitions_.contains(replacement_template_id)) {
        impact.code = "event_template_replacement_missing"; return impact;
    }
    if (!replacement_template_id.empty()) {
        const auto& replacement = definitions_.at(replacement_template_id);
        for (const auto& use : uses) {
            auto candidate = instances_.at(use.instance_id);
            candidate.template_id = replacement_template_id;
            std::string error;
            if (!render(candidate, replacement, &error)) {
                impact.code = error;
                return impact;
            }
        }
    }
    impact.valid = true;
    impact.code = "event_template_delete_ready";
    return impact;
}

bool EventTemplateLibrary::deleteTemplate(const std::string& template_id,
                                          const std::string& replacement_template_id,
                                          EventDocument& document, EventTemplateImpact* applied) {
    const auto impact = previewDelete(template_id, replacement_template_id, document);
    if (!impact.valid) { if (applied != nullptr) *applied = impact; return false; }
    std::vector<std::string> erase_ids;
    std::map<std::string, CommonEventDefinition> replacements;
    if (!replacement_template_id.empty()) {
        const auto& replacement = definitions_.at(replacement_template_id);
        for (const auto& [id, instance] : instances_) {
            if (instance.template_id != template_id) continue;
            auto rendered = render(instance, replacement);
            if (!rendered) return false;
            replacements.emplace(id, std::move(*rendered));
        }
    }
    for (auto& [id, instance] : instances_) {
        if (instance.template_id != template_id) continue;
        if (replacement_template_id.empty()) {
            document.removeCommonEvent(instance.common_event_id);
            erase_ids.push_back(id);
            continue;
        }
        instance.template_id = replacement_template_id;
        const auto& replacement = definitions_.at(replacement_template_id);
        instance.template_version = replacement.version;
        document.addCommonEvent(std::move(replacements.at(id)));
    }
    for (const auto& id : erase_ids) instances_.erase(id);
    definitions_.erase(template_id);
    if (applied != nullptr) *applied = impact;
    return true;
}

nlohmann::json EventTemplateLibrary::toJson() const {
    nlohmann::json definitions = nlohmann::json::array();
    for (const auto& [_, definition] : definitions_) {
        nlohmann::json parameters = nlohmann::json::array();
        for (const auto& parameter : definition.parameters) parameters.push_back({{"id", parameter.id}, {"type", parameterTypeId(parameter.type)}, {"required", parameter.required}, {"default", parameter.default_value}, {"reference_kind", parameter.reference_kind}});
        nlohmann::json commands = nlohmann::json::array();
        for (const auto& command : definition.commands) commands.push_back(commandJson(command));
        definitions.push_back({{"id", definition.id}, {"version", definition.version}, {"kind", kindId(definition.kind)}, {"parameters", parameters}, {"commands", commands}});
    }
    nlohmann::json instances = nlohmann::json::array();
    for (const auto& [_, instance] : instances_) {
        nlohmann::json overrides = nlohmann::json::array();
        for (const auto& value : instance.overrides) overrides.push_back({{"command_id", value.command_id}, {"field", value.field}, {"value", value.value}});
        instances.push_back({{"id", instance.id}, {"template_id", instance.template_id}, {"template_version", instance.template_version}, {"common_event_id", instance.common_event_id}, {"bindings", instance.bindings}, {"overrides", overrides}});
    }
    return {{"schema_version", "urpg.event_template_library.v1"}, {"definitions", definitions}, {"instances", instances}};
}

std::optional<EventTemplateLibrary> EventTemplateLibrary::fromJson(const nlohmann::json& json) {
    try {
        if (json.value("schema_version", "") != "urpg.event_template_library.v1") return std::nullopt;
        EventTemplateLibrary library;
        for (const auto& item : json.at("definitions")) {
            EventTemplateDefinition definition;
            definition.id = item.at("id").get<std::string>();
            definition.version = item.at("version").get<uint32_t>();
            definition.kind = item.value("kind", "common_event") == "narrative" ? EventTemplateKind::Narrative : EventTemplateKind::CommonEvent;
            for (const auto& value : item.at("parameters")) {
                const auto type = parameterType(value.at("type").get<std::string>());
                if (!type) return std::nullopt;
                definition.parameters.push_back({value.at("id").get<std::string>(), *type, value.value("required", true), value.value("default", nlohmann::json{}), value.value("reference_kind", "")});
            }
            for (const auto& value : item.at("commands")) definition.commands.push_back(commandFromJson(value));
            if (!library.registerDefinition(std::move(definition))) return std::nullopt;
        }
        for (const auto& item : json.at("instances")) {
            EventTemplateInstance instance;
            instance.id = item.at("id").get<std::string>(); instance.template_id = item.at("template_id").get<std::string>();
            instance.template_version = item.at("template_version").get<uint32_t>(); instance.common_event_id = item.at("common_event_id").get<std::string>();
            instance.bindings = item.at("bindings");
            for (const auto& value : item.at("overrides")) instance.overrides.push_back({value.at("command_id").get<std::string>(), value.at("field").get<std::string>(), value.at("value")});
            if (!library.definitions_.contains(instance.template_id) || library.instances_.contains(instance.id)) return std::nullopt;
            library.instances_.emplace(instance.id, std::move(instance));
        }
        return library;
    } catch (const nlohmann::json::exception&) { return std::nullopt; }
}

} // namespace urpg::events
