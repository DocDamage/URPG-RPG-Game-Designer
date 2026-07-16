#pragma once

#include "engine/core/events/event_dependency_graph.h"

#include <nlohmann/json.hpp>

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace urpg::events {

enum class EventTemplateKind : uint8_t { CommonEvent, Narrative };
enum class EventTemplateParameterType : uint8_t { String, Integer, Boolean, Reference };

struct EventTemplateParameter {
    std::string id;
    EventTemplateParameterType type = EventTemplateParameterType::String;
    bool required = true;
    nlohmann::json default_value;
    std::string reference_kind;
};

struct EventTemplateDefinition {
    std::string id;
    uint32_t version = 1;
    EventTemplateKind kind = EventTemplateKind::CommonEvent;
    std::vector<EventTemplateParameter> parameters;
    std::vector<EventCommand> commands;
};

struct EventTemplateOverride {
    std::string command_id;
    std::string field;
    nlohmann::json value;
};

struct EventTemplateInstance {
    std::string id;
    std::string template_id;
    uint32_t template_version = 0;
    std::string common_event_id;
    nlohmann::json bindings = nlohmann::json::object();
    std::vector<EventTemplateOverride> overrides;
};

struct EventTemplateUse {
    std::string instance_id;
    std::string common_event_id;
    std::vector<std::string> caller_source_ids;
};

struct EventTemplateImpact {
    bool valid = false;
    std::string code;
    std::size_t affected_instance_count = 0;
    std::size_t preserved_override_count = 0;
    std::size_t caller_count = 0;
    std::vector<std::string> affected_instance_ids;
    std::vector<std::string> caller_source_ids;
};

class EventTemplateLibrary {
public:
    bool registerDefinition(EventTemplateDefinition definition);
    std::vector<std::string> validateBindings(const std::string& template_id, const nlohmann::json& bindings) const;
    bool instantiate(EventTemplateInstance instance, EventDocument& document, std::string* error_code = nullptr);
    std::vector<EventTemplateUse> findUses(const std::string& template_id, const EventDocument& document) const;
    EventTemplateImpact previewUpdate(const EventTemplateDefinition& replacement,
                                      const EventDocument& document) const;
    bool applyUpdate(EventTemplateDefinition replacement, EventDocument& document,
                     EventTemplateImpact* applied = nullptr);
    EventTemplateImpact previewDelete(const std::string& template_id, const std::string& replacement_template_id,
                                      const EventDocument& document) const;
    bool deleteTemplate(const std::string& template_id, const std::string& replacement_template_id,
                        EventDocument& document, EventTemplateImpact* applied = nullptr);
    nlohmann::json toJson() const;
    static std::optional<EventTemplateLibrary> fromJson(const nlohmann::json& json);

    const std::map<std::string, EventTemplateDefinition>& definitions() const { return definitions_; }
    const std::map<std::string, EventTemplateInstance>& instances() const { return instances_; }

private:
    std::optional<CommonEventDefinition> render(const EventTemplateInstance& instance,
                                                const EventTemplateDefinition& definition,
                                                std::string* error_code = nullptr) const;
    std::map<std::string, EventTemplateDefinition> definitions_;
    std::map<std::string, EventTemplateInstance> instances_;
};

} // namespace urpg::events
