#include "engine/core/ai/ai_knowledge_base.h"

#include <algorithm>
#include <cctype>
#include <set>
#include <sstream>

namespace {

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

bool containsText(const std::string& haystack, const std::string& needle) {
    return lowerCopy(haystack).find(lowerCopy(needle)) != std::string::npos;
}

std::vector<std::string> tokens(const std::string& query) {
    std::vector<std::string> out;
    std::stringstream ss(lowerCopy(query));
    std::string token;
    while (ss >> token) {
        token.erase(std::remove_if(token.begin(), token.end(), [](unsigned char c) {
                        return !std::isalnum(c) && c != '_' && c != '-';
                    }),
                    token.end());
        if (!token.empty()) {
            out.push_back(token);
        }
    }
    return out;
}

bool matchesKeywords(const std::vector<std::string>& keywords, const std::vector<std::string>& queryTokens) {
    return std::any_of(queryTokens.begin(), queryTokens.end(), [&](const std::string& token) {
        return std::any_of(keywords.begin(), keywords.end(), [&](const std::string& keyword) {
            return containsText(keyword, token) || containsText(token, keyword);
        });
    });
}

urpg::ai::AiKnowledgeDiagnostic diagnostic(const std::string& code,
                                           const std::string& message,
                                           const std::string& target) {
    return {code, message, target};
}

nlohmann::json diagnosticsToJson(const std::vector<urpg::ai::AiKnowledgeDiagnostic>& diagnostics) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& item : diagnostics) {
        out.push_back({{"code", item.code}, {"message", item.message}, {"target", item.target}});
    }
    return out;
}

std::string stablePlanId(const std::string& request) {
    std::uint64_t hash = 1469598103934665603ull;
    for (const unsigned char ch : request) {
        hash ^= ch;
        hash *= 1099511628211ull;
    }
    std::ostringstream out;
    out << "ai_task_" << std::hex << hash;
    return out.str();
}

void pushUnique(std::vector<std::string>& values, const std::string& value) {
    if (std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

void appendObjectSummary(urpg::ai::ProjectKnowledgeIndex& index,
                         const nlohmann::json& root,
                         const std::string& key,
                         const std::string& type,
                         const std::string& title) {
    if (!root.contains(key)) {
        return;
    }
    const auto& value = root[key];
    const auto count = value.is_array() || value.is_object() ? value.size() : 1;
    index.addEntry({
        key,
        type,
        title,
        "/" + key,
        title + " entries available: " + std::to_string(count),
        {key, type, title},
        {{"count", count}},
    });
}

} // namespace

namespace urpg::ai {

nlohmann::json AppCapability::toJson() const {
    return {
        {"id", id},
        {"title", title},
        {"category", category},
        {"wysiwyg_surface", wysiwyg_surface},
        {"actions", actions},
        {"project_paths", project_paths},
        {"keywords", keywords},
    };
}

void AppCapabilityRegistry::registerCapability(AppCapability capability) {
    const auto it = std::find_if(capabilities_.begin(), capabilities_.end(), [&](const auto& existing) {
        return existing.id == capability.id;
    });
    if (it == capabilities_.end()) {
        capabilities_.push_back(std::move(capability));
    } else {
        *it = std::move(capability);
    }
}

const AppCapability* AppCapabilityRegistry::find(const std::string& id) const {
    const auto it = std::find_if(capabilities_.begin(), capabilities_.end(), [&](const auto& capability) {
        return capability.id == id;
    });
    return it == capabilities_.end() ? nullptr : &(*it);
}

std::vector<AppCapability> AppCapabilityRegistry::search(const std::string& query) const {
    const auto queryTokens = tokens(query);
    std::vector<AppCapability> out;
    for (const auto& capability : capabilities_) {
        if (containsText(capability.id, query) || containsText(capability.title, query) ||
            containsText(capability.category, query) || matchesKeywords(capability.keywords, queryTokens) ||
            matchesKeywords(capability.actions, queryTokens)) {
            out.push_back(capability);
        }
    }
    return out;
}

nlohmann::json AppCapabilityRegistry::toJson() const {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& capability : capabilities_) {
        out.push_back(capability.toJson());
    }
    return out;
}

AppCapabilityRegistry AppCapabilityRegistry::buildDefault() {
    AppCapabilityRegistry registry;
    const std::vector<AppCapability> defaults = {
        {"map_authoring", "Map Authoring", "Worldbuilding", "map lighting/weather/region preview",
         {"create_map", "place_tile", "paint_region", "preview_lighting", "preview_weather"},
         {"/maps", "/tiles", "/regions"}, {"map", "tile", "house", "lighting", "weather", "region"}},
        {"event_authoring", "Event Authoring", "Events", "event-command visual graph",
         {"add_event", "wire_event_command", "preview_event_flow", "validate_event"},
         {"/events", "/common_events", "/switches", "/variables"}, {"event", "logic", "switch", "variable", "door", "chest"}},
        {"dialogue_authoring", "Dialogue Authoring", "Narrative", "dialogue preview with portraits choices variables localization",
         {"edit_dialogue", "add_choice", "bind_portrait", "preview_localization"},
         {"/dialogue", "/localization", "/portraits"}, {"dialogue", "choice", "portrait", "localization", "npc"}},
        {"ability_authoring", "Ability Authoring", "Gameplay", "ability sandbox",
         {"add_ability", "set_cost", "set_cooldown", "preview_effects", "validate_tags"},
         {"/abilities", "/effects", "/gameplay_tags"}, {"ability", "skill", "cooldown", "cost", "tag", "effect"}},
        {"battle_vfx_authoring", "Battle VFX Authoring", "Battle", "battle animation/VFX timeline editor",
         {"add_vfx_track", "add_keyframe", "preview_battle_vfx", "validate_timeline"},
         {"/battle_vfx", "/timelines"}, {"battle", "vfx", "animation", "timeline", "keyframe"}},
        {"save_lab", "Save/Load Preview Lab", "Diagnostics", "save/load preview lab",
         {"preview_save", "migrate_save", "validate_save_compatibility"},
         {"/saves", "/save_migrations"}, {"save", "load", "migration", "compatibility"}},
        {"export_preview", "Export Preview", "Release", "export preview showing exactly what will ship",
         {"run_export_preview", "validate_bundle", "audit_assets"}, {"/export", "/assets"}, {"export", "ship", "bundle", "package"}},
        {"asset_pipeline", "Asset Pipeline", "Content", "asset library and license audit",
         {"import_asset", "audit_license", "promote_asset", "cleanup_unused_assets"}, {"/assets"}, {"asset", "license", "import", "cleanup"}},
        {"template_authoring", "Template Authoring", "Project", "template wizard and sample projects",
         {"create_template_project", "validate_template", "instantiate_template"}, {"/templates"}, {"template", "starter", "project", "genre"}},
        {"creator_command", "Creator Command", "AI", "selected-tile WYSIWYG creator command panel",
         {"plan_creator_command", "validate_creator_plan", "apply_creator_plan"}, {"/maps", "/creator_command_history"},
         {"chatbot", "ai", "generate", "make", "house", "shop", "inn", "npc", "puzzle"}},
    };
    for (const auto& capability : defaults) {
        registry.registerCapability(capability);
    }
    return registry;
}

nlohmann::json KnowledgeEntry::toJson() const {
    return {
        {"id", id},
        {"type", type},
        {"title", title},
        {"path", path},
        {"summary", summary},
        {"keywords", keywords},
        {"metadata", metadata},
    };
}

void ProjectKnowledgeIndex::addEntry(KnowledgeEntry entry) {
    entries_.push_back(std::move(entry));
}

std::vector<KnowledgeEntry> ProjectKnowledgeIndex::search(const std::string& query) const {
    const auto queryTokens = tokens(query);
    std::vector<KnowledgeEntry> out;
    for (const auto& entry : entries_) {
        if (containsText(entry.id, query) || containsText(entry.title, query) || containsText(entry.summary, query) ||
            matchesKeywords(entry.keywords, queryTokens)) {
            out.push_back(entry);
        }
    }
    return out;
}

nlohmann::json ProjectKnowledgeIndex::toJson() const {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& entry : entries_) {
        out.push_back(entry.toJson());
    }
    return out;
}

ProjectKnowledgeIndex ProjectKnowledgeIndex::buildFromProjectData(const nlohmann::json& projectData) {
    ProjectKnowledgeIndex index;
    if (!projectData.is_object()) {
        index.addEntry({"empty_project", "project", "Empty Project", "/", "No project data was supplied.", {"empty", "project"}});
        return index;
    }
    index.addEntry({
        projectData.value("project_id", "project"),
        "project",
        projectData.value("name", "URPG Project"),
        "/",
        "Project data is available to the AI planner.",
        {"project", "root"},
        {{"schema", projectData.value("schema", "")}},
    });
    appendObjectSummary(index, projectData, "maps", "map", "Maps");
    appendObjectSummary(index, projectData, "events", "event", "Events");
    appendObjectSummary(index, projectData, "dialogue", "dialogue", "Dialogue");
    appendObjectSummary(index, projectData, "abilities", "ability", "Abilities");
    appendObjectSummary(index, projectData, "assets", "asset", "Assets");
    appendObjectSummary(index, projectData, "templates", "template", "Templates");
    appendObjectSummary(index, projectData, "localization", "localization", "Localization");
    appendObjectSummary(index, projectData, "export", "export", "Export Settings");
    return index;
}

void DocumentationKnowledgeIndex::addEntry(KnowledgeEntry entry) {
    entries_.push_back(std::move(entry));
}

std::vector<KnowledgeEntry> DocumentationKnowledgeIndex::search(const std::string& query) const {
    const auto queryTokens = tokens(query);
    std::vector<KnowledgeEntry> out;
    for (const auto& entry : entries_) {
        if (containsText(entry.id, query) || containsText(entry.title, query) || containsText(entry.summary, query) ||
            matchesKeywords(entry.keywords, queryTokens)) {
            out.push_back(entry);
        }
    }
    return out;
}

nlohmann::json DocumentationKnowledgeIndex::toJson() const {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& entry : entries_) {
        out.push_back(entry.toJson());
    }
    return out;
}

DocumentationKnowledgeIndex DocumentationKnowledgeIndex::buildDefault() {
    DocumentationKnowledgeIndex index;
    index.addEntry({"agent_index", "doc", "Agent Knowledge Index", "docs/agent/INDEX.md",
                    "Canonical starting point for agent work.", {"agent", "index", "documentation"}});
    index.addEntry({"architecture_map", "doc", "Architecture Map", "docs/agent/ARCHITECTURE_MAP.md",
                    "Subsystem ownership and code location map.", {"architecture", "subsystem", "code"}});
    index.addEntry({"quality_gates", "doc", "Quality Gates", "docs/agent/QUALITY_GATES.md",
                    "Validation command map and expected gates.", {"test", "validation", "quality"}});
    index.addEntry({"ai_copilot", "doc", "AI Copilot Guide", "docs/integrations/AI_COPILOT_GUIDE.md",
                    "AI assistant, creator command, provider, and safety guidance.", {"ai", "chatbot", "provider", "creator"}});
    index.addEntry({"readiness", "doc", "Release Readiness Matrix", "docs/RELEASE_READINESS_MATRIX.md",
                    "Release status and readiness evidence.", {"release", "readiness", "status"}});
    return index;
}

nlohmann::json AiToolDefinition::toJson() const {
    return {
        {"id", id},
        {"title", title},
        {"capability_id", capability_id},
        {"mutates_project", mutates_project},
        {"requires_approval", requires_approval},
        {"required_fields", required_fields},
    };
}

nlohmann::json AiToolStep::toJson() const {
    return {
        {"id", id},
        {"tool_id", tool_id},
        {"summary", summary},
        {"arguments", arguments},
        {"approved", approved},
        {"rejected", rejected},
    };
}

nlohmann::json AiTaskPlan::toJson() const {
    nlohmann::json stepsJson = nlohmann::json::array();
    for (const auto& step : steps) {
        stepsJson.push_back(step.toJson());
    }
    return {
        {"schema", schema},
        {"id", id},
        {"user_request", user_request},
        {"capability_ids", capability_ids},
        {"steps", stepsJson},
        {"diagnostics", diagnosticsToJson(diagnostics)},
        {"ready_for_approval", ready_for_approval},
    };
}

nlohmann::json AiToolApplyResult::toJson() const {
    return {
        {"applied", applied},
        {"project_data", project_data},
        {"before_project_data", before_project_data},
        {"project_patch", project_patch},
        {"revert_patch", revert_patch},
        {"diagnostics", diagnosticsToJson(diagnostics)},
    };
}

nlohmann::json AiToolApprovalSummary::toJson() const {
    return {
        {"step_id", step_id},
        {"tool_id", tool_id},
        {"tool_title", tool_title},
        {"capability_id", capability_id},
        {"summary", summary},
        {"mutates_project", mutates_project},
        {"requires_approval", requires_approval},
        {"approved", approved},
        {"rejected", rejected},
        {"project_paths", project_paths},
        {"arguments", arguments},
    };
}

void AiToolRegistry::registerTool(AiToolDefinition tool) {
    const auto it = std::find_if(tools_.begin(), tools_.end(), [&](const auto& existing) {
        return existing.id == tool.id;
    });
    if (it == tools_.end()) {
        tools_.push_back(std::move(tool));
    } else {
        *it = std::move(tool);
    }
}

const AiToolDefinition* AiToolRegistry::find(const std::string& id) const {
    const auto it = std::find_if(tools_.begin(), tools_.end(), [&](const auto& tool) {
        return tool.id == id;
    });
    return it == tools_.end() ? nullptr : &(*it);
}

std::vector<AiToolDefinition> AiToolRegistry::mutatingToolsRequiringApproval() const {
    std::vector<AiToolDefinition> out;
    for (const auto& tool : tools_) {
        if (tool.mutates_project && tool.requires_approval) {
            out.push_back(tool);
        }
    }
    return out;
}

std::vector<AiToolApprovalSummary> AiToolRegistry::pendingApprovalSteps(
    const AiTaskPlan& plan,
    const AppCapabilityRegistry& capabilities) const {
    std::vector<AiToolApprovalSummary> out;
    for (const auto& step : plan.steps) {
        const auto* tool = find(step.tool_id);
        if (tool == nullptr || !tool->requires_approval || step.approved || step.rejected) {
            continue;
        }
        std::vector<std::string> projectPaths;
        if (const auto* capability = capabilities.find(tool->capability_id)) {
            projectPaths = capability->project_paths;
        }
        out.push_back({
            step.id,
            step.tool_id,
            tool->title,
            tool->capability_id,
            step.summary,
            tool->mutates_project,
            tool->requires_approval,
            step.approved,
            step.rejected,
            projectPaths,
            step.arguments,
        });
    }
    return out;
}

nlohmann::json AiToolRegistry::approvalManifest(const AiTaskPlan& plan,
                                                const AppCapabilityRegistry& capabilities) const {
    nlohmann::json pending = nlohmann::json::array();
    for (const auto& item : pendingApprovalSteps(plan, capabilities)) {
        pending.push_back(item.toJson());
    }
    nlohmann::json mutating = nlohmann::json::array();
    for (const auto& tool : mutatingToolsRequiringApproval()) {
        mutating.push_back(tool.toJson());
    }
    return {
        {"plan_id", plan.id},
        {"pending_count", pending.size()},
        {"pending_steps", pending},
        {"mutating_tools_requiring_approval", mutating},
    };
}

std::vector<AiKnowledgeDiagnostic> AiToolRegistry::validatePlan(const AiTaskPlan& plan) const {
    std::vector<AiKnowledgeDiagnostic> diagnostics = plan.diagnostics;
    if (plan.schema != "urpg.ai_task_plan.v1") {
        diagnostics.push_back(diagnostic("ai_plan_schema_invalid", "AI task plan schema is unsupported.", plan.id));
    }
    if (plan.steps.empty()) {
        diagnostics.push_back(diagnostic("ai_plan_empty", "AI task plan contains no tool steps.", plan.id));
    }
    for (const auto& step : plan.steps) {
        const auto* tool = find(step.tool_id);
        if (tool == nullptr) {
            diagnostics.push_back(diagnostic("ai_tool_unknown", "AI plan references an unknown tool.", step.tool_id));
            continue;
        }
        for (const auto& field : tool->required_fields) {
            if (!step.arguments.contains(field)) {
                diagnostics.push_back(diagnostic("ai_tool_missing_argument", "AI tool step is missing a required argument.", step.id + ":" + field));
            }
        }
        if (step.rejected) {
            diagnostics.push_back(diagnostic("ai_tool_rejected", "AI tool step was rejected and cannot be applied.", step.id));
        }
        if (tool->requires_approval && !step.approved) {
            diagnostics.push_back(diagnostic("ai_tool_unapproved", "Mutating AI tool step requires approval before apply.", step.id));
        }
    }
    return diagnostics;
}

AiToolApplyResult AiToolRegistry::applyApprovedPlan(const AiTaskPlan& plan, const nlohmann::json& projectData) const {
    AiToolApplyResult result;
    result.project_data = projectData.is_object() ? projectData : nlohmann::json::object();
    result.before_project_data = result.project_data;
    result.diagnostics = validatePlan(plan);
    if (!result.diagnostics.empty()) {
        return result;
    }
    auto& applications = result.project_data["ai_tool_applications"];
    if (!applications.is_array()) {
        applications = nlohmann::json::array();
    }
    for (const auto& step : plan.steps) {
        const auto* tool = find(step.tool_id);
        if (tool == nullptr) {
            continue;
        }
        applications.push_back(step.toJson());
        if (step.tool_id == "create_map") {
            auto& maps = result.project_data["maps"];
            if (!maps.is_object()) {
                maps = nlohmann::json::object();
            }
            const auto mapId = step.arguments.value("map_id", "generated_map");
            maps[mapId]["id"] = mapId;
            maps[mapId]["width"] = step.arguments.value("width", 32);
            maps[mapId]["height"] = step.arguments.value("height", 32);
            maps[mapId]["source"] = "ai_tool_registry";
        } else if (step.tool_id == "place_tile") {
            auto& maps = result.project_data["maps"];
            if (!maps.is_object()) {
                maps = nlohmann::json::object();
            }
            const auto mapId = step.arguments.value("map_id", "generated_map");
            auto& edits = maps[mapId]["tile_edits"];
            if (!edits.is_array()) {
                edits = nlohmann::json::array();
            }
            edits.push_back(step.arguments);
        } else if (step.tool_id == "paint_region") {
            auto& regions = result.project_data["regions"];
            if (!regions.is_array()) {
                regions = nlohmann::json::array();
            }
            regions.push_back(step.arguments);
        } else if (step.tool_id == "configure_environment") {
            auto& environments = result.project_data["environments"];
            if (!environments.is_object()) {
                environments = nlohmann::json::object();
            }
            environments[step.arguments.value("map_id", "current_map")] = step.arguments;
        } else if (step.tool_id == "add_event") {
            auto& events = result.project_data["events"];
            if (!events.is_array()) {
                events = nlohmann::json::array();
            }
            events.push_back(step.arguments);
        } else if (step.tool_id == "edit_dialogue") {
            auto& dialogue = result.project_data["dialogue"];
            if (!dialogue.is_object()) {
                dialogue = nlohmann::json::object();
            }
            dialogue[step.arguments.value("dialogue_id", "generated_dialogue")] = step.arguments;
        } else if (step.tool_id == "add_localization_entry") {
            auto& localization = result.project_data["localization"];
            if (!localization.is_object()) {
                localization = nlohmann::json::object();
            }
            localization[step.arguments.value("locale", "en-US")][step.arguments.value("key", "generated.key")] =
                step.arguments.value("text", "");
        } else if (step.tool_id == "add_quest") {
            auto& quests = result.project_data["quests"];
            if (!quests.is_array()) {
                quests = nlohmann::json::array();
            }
            quests.push_back(step.arguments);
        } else if (step.tool_id == "set_npc_schedule") {
            auto& schedules = result.project_data["npc_schedules"];
            if (!schedules.is_object()) {
                schedules = nlohmann::json::object();
            }
            schedules[step.arguments.value("npc_id", "generated_npc")] = step.arguments["schedule"];
        } else if (step.tool_id == "add_ability") {
            auto& abilities = result.project_data["abilities"];
            if (!abilities.is_array()) {
                abilities = nlohmann::json::array();
            }
            abilities.push_back(step.arguments);
        } else if (step.tool_id == "add_vfx_keyframe") {
            auto& timelines = result.project_data["battle_vfx"];
            if (!timelines.is_object()) {
                timelines = nlohmann::json::object();
            }
            auto& keyframes = timelines[step.arguments.value("timeline_id", "generated_timeline")]["keyframes"];
            if (!keyframes.is_array()) {
                keyframes = nlohmann::json::array();
            }
            keyframes.push_back(step.arguments);
        } else if (step.tool_id == "configure_save_preview") {
            auto& saveLabs = result.project_data["save_labs"];
            if (!saveLabs.is_array()) {
                saveLabs = nlohmann::json::array();
            }
            saveLabs.push_back(step.arguments);
        } else if (step.tool_id == "import_asset_record") {
            auto& assets = result.project_data["assets"];
            if (!assets.is_array()) {
                assets = nlohmann::json::array();
            }
            assets.push_back(step.arguments);
        } else if (step.tool_id == "create_template_project") {
            auto& templates = result.project_data["template_instances"];
            if (!templates.is_array()) {
                templates = nlohmann::json::array();
            }
            templates.push_back(step.arguments);
        } else if (step.tool_id == "run_validation") {
            result.project_data["last_ai_validation"] = {{"requested_by_plan", plan.id}, {"status", "queued"}};
        } else if (step.tool_id == "run_export_preview") {
            result.project_data["last_ai_export_preview"] = {{"requested_by_plan", plan.id}, {"status", "queued"}};
        } else if (step.tool_id == "plan_creator_command") {
            auto& commands = result.project_data["creator_command_requests"];
            if (!commands.is_array()) {
                commands = nlohmann::json::array();
            }
            commands.push_back(step.arguments);
        }
    }
    result.applied = true;
    result.project_patch = nlohmann::json::diff(result.before_project_data, result.project_data);
    result.revert_patch = nlohmann::json::diff(result.project_data, result.before_project_data);
    return result;
}

nlohmann::json AiToolRegistry::toJson() const {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& tool : tools_) {
        out.push_back(tool.toJson());
    }
    return out;
}

AiToolRegistry AiToolRegistry::buildDefault() {
    AiToolRegistry registry;
    registry.registerTool({"create_map", "Create Map", "map_authoring", true, true, {"map_id", "width", "height"}});
    registry.registerTool({"place_tile", "Place Tile", "map_authoring", true, true, {"map_id", "layer_id", "x", "y", "tile_id"}});
    registry.registerTool({"paint_region", "Paint Region", "map_authoring", true, true, {"map_id", "region_id", "x", "y", "rule"}});
    registry.registerTool({"configure_environment", "Configure Environment", "map_authoring", true, true, {"map_id", "weather", "lighting_profile"}});
    registry.registerTool({"add_event", "Add Event", "event_authoring", true, true, {"event_id", "map_id", "x", "y", "commands"}});
    registry.registerTool({"edit_dialogue", "Edit Dialogue", "dialogue_authoring", true, true, {"dialogue_id", "lines"}});
    registry.registerTool({"add_localization_entry", "Add Localization Entry", "dialogue_authoring", true, true, {"locale", "key", "text"}});
    registry.registerTool({"add_quest", "Add Quest", "dialogue_authoring", true, true, {"quest_id", "objectives"}});
    registry.registerTool({"set_npc_schedule", "Set NPC Schedule", "event_authoring", true, true, {"npc_id", "schedule"}});
    registry.registerTool({"add_ability", "Add Ability", "ability_authoring", true, true, {"ability_id", "cost", "cooldown", "effects"}});
    registry.registerTool({"add_vfx_keyframe", "Add VFX Keyframe", "battle_vfx_authoring", true, true, {"timeline_id", "time", "effect"}});
    registry.registerTool({"configure_save_preview", "Configure Save Preview", "save_lab", true, true, {"save_id", "scenario"}});
    registry.registerTool({"import_asset_record", "Import Asset Record", "asset_pipeline", true, true, {"asset_id", "path", "license"}});
    registry.registerTool({"create_template_project", "Create Template Project", "template_authoring", true, true, {"template_id", "project_id"}});
    registry.registerTool({"run_validation", "Run Validation", "export_preview", false, false, {"scope"}});
    registry.registerTool({"run_export_preview", "Run Export Preview", "export_preview", false, false, {"profile"}});
    registry.registerTool({"plan_creator_command", "Plan Creator Command", "creator_command", true, true, {"prompt", "map_id", "tile_x", "tile_y"}});
    return registry;
}

AiTaskPlan AiTaskPlanner::planTask(const std::string& userRequest,
                                   const AppCapabilityRegistry& capabilities,
                                   const ProjectKnowledgeIndex& projectIndex,
                                   const DocumentationKnowledgeIndex& docs,
                                   const AiToolRegistry& tools) const {
    (void)projectIndex;
    (void)docs;
    AiTaskPlan plan;
    plan.id = stablePlanId(userRequest);
    plan.user_request = userRequest;
    const auto lowered = lowerCopy(userRequest);
    auto addCapability = [&](const std::string& id) {
        if (capabilities.find(id) == nullptr) {
            plan.diagnostics.push_back(diagnostic("ai_capability_missing", "Required capability is not registered.", id));
        } else {
            pushUnique(plan.capability_ids, id);
        }
    };
    auto addStep = [&](AiToolStep step) {
        if (tools.find(step.tool_id) == nullptr) {
            plan.diagnostics.push_back(diagnostic("ai_tool_missing", "Required tool is not registered.", step.tool_id));
        }
        plan.steps.push_back(std::move(step));
    };

    if (lowered.find("house") != std::string::npos || lowered.find("shop") != std::string::npos ||
        lowered.find("inn") != std::string::npos ||
        (lowered.find("npc") != std::string::npos && lowered.find("schedule") == std::string::npos) ||
        lowered.find("chest") != std::string::npos || lowered.find("puzzle") != std::string::npos) {
        addCapability("creator_command");
        addCapability("map_authoring");
        addCapability("event_authoring");
        addStep({"step_creator_command", "plan_creator_command", "Generate selected-tile map edits and runtime logic.",
                 {{"prompt", userRequest}, {"map_id", "current_map"}, {"tile_x", 0}, {"tile_y", 0}}, false});
    } else if (lowered.find("dialogue") != std::string::npos || lowered.find("conversation") != std::string::npos) {
        addCapability("dialogue_authoring");
        addStep({"step_dialogue", "edit_dialogue", "Create or update dialogue with localization-ready lines.",
                 {{"dialogue_id", "generated_dialogue"}, {"lines", nlohmann::json::array({userRequest})}}, false});
    } else if (lowered.find("localization") != std::string::npos || lowered.find("translate") != std::string::npos) {
        addCapability("dialogue_authoring");
        addStep({"step_localization", "add_localization_entry", "Add a localization-ready string entry.",
                 {{"locale", "en-US"}, {"key", "generated.string"}, {"text", userRequest}}, false});
    } else if (lowered.find("quest") != std::string::npos) {
        addCapability("dialogue_authoring");
        addCapability("event_authoring");
        addStep({"step_quest", "add_quest", "Create a quest record with objectives.",
                 {{"quest_id", "generated_quest"}, {"objectives", nlohmann::json::array({userRequest})}}, false});
    } else if (lowered.find("schedule") != std::string::npos) {
        addCapability("event_authoring");
        addStep({"step_npc_schedule", "set_npc_schedule", "Create an NPC schedule preview record.",
                 {{"npc_id", "generated_npc"}, {"schedule", nlohmann::json::array({{{"time", "day"}, {"activity", userRequest}}})}}, false});
    } else if (lowered.find("ability") != std::string::npos || lowered.find("skill") != std::string::npos) {
        addCapability("ability_authoring");
        addStep({"step_ability", "add_ability", "Create an authored ability with visible cost, cooldown, tags, and effects.",
                 {{"ability_id", "generated_ability"}, {"cost", 10}, {"cooldown", 3}, {"effects", nlohmann::json::array({userRequest})}}, false});
    } else if (lowered.find("vfx") != std::string::npos || lowered.find("battle animation") != std::string::npos) {
        addCapability("battle_vfx_authoring");
        addStep({"step_vfx", "add_vfx_keyframe", "Add a battle VFX timeline keyframe.",
                 {{"timeline_id", "generated_vfx"}, {"time", 0.0}, {"effect", userRequest}}, false});
    } else if (lowered.find("save") != std::string::npos) {
        addCapability("save_lab");
        addStep({"step_save_lab", "configure_save_preview", "Configure a save/load preview scenario.",
                 {{"save_id", "generated_save_case"}, {"scenario", userRequest}}, false});
    } else if (lowered.find("asset") != std::string::npos || lowered.find("import") != std::string::npos) {
        addCapability("asset_pipeline");
        addStep({"step_asset", "import_asset_record", "Create an asset import record for review.",
                 {{"asset_id", "generated_asset"}, {"path", "project-configured"}, {"license", "review_required"}}, false});
    } else if (lowered.find("template") != std::string::npos || lowered.find("starter") != std::string::npos) {
        addCapability("template_authoring");
        addStep({"step_template", "create_template_project", "Create a template project instance record.",
                 {{"template_id", "generated_template"}, {"project_id", "generated_project"}}, false});
    } else if (lowered.find("lighting") != std::string::npos || lowered.find("weather") != std::string::npos) {
        addCapability("map_authoring");
        addStep({"step_environment", "configure_environment", "Configure map lighting and weather preview data.",
                 {{"map_id", "current_map"}, {"weather", "rain"}, {"lighting_profile", "generated_lighting"}}, false});
    } else if (lowered.find("region") != std::string::npos) {
        addCapability("map_authoring");
        addStep({"step_region", "paint_region", "Paint a region rule into the map authoring data.",
                 {{"map_id", "current_map"}, {"region_id", "generated_region"}, {"x", 0}, {"y", 0}, {"rule", userRequest}}, false});
    } else if (lowered.find("create map") != std::string::npos || lowered.find("new map") != std::string::npos) {
        addCapability("map_authoring");
        addStep({"step_map", "create_map", "Create a new map record.",
                 {{"map_id", "generated_map"}, {"width", 32}, {"height", 32}}, false});
    } else if (lowered.find("export") != std::string::npos || lowered.find("ship") != std::string::npos) {
        addCapability("export_preview");
        addStep({"step_export_preview", "run_export_preview", "Preview exactly what will ship.",
                 {{"profile", "default"}}, true});
    } else {
        const auto matches = capabilities.search(userRequest);
        if (!matches.empty()) {
            addCapability(matches.front().id);
        }
        addStep({"step_validation", "run_validation", "Inspect project state before choosing a mutating tool.",
                 {{"scope", "project"}}, true});
    }

    const auto validation = tools.validatePlan(plan);
    plan.ready_for_approval = plan.diagnostics.empty() && !plan.steps.empty();
    if (!validation.empty()) {
        for (const auto& item : validation) {
            if (item.code != "ai_tool_unapproved") {
                plan.diagnostics.push_back(item);
            }
        }
    }
    return plan;
}

nlohmann::json AiKnowledgeSnapshot::toJson() const {
    return {
        {"capabilities", capabilities.toJson()},
        {"project_index", project_index.toJson()},
        {"docs_index", docs_index.toJson()},
        {"tools", tools.toJson()},
    };
}

AiKnowledgeSnapshot buildDefaultAiKnowledgeSnapshot(const nlohmann::json& projectData) {
    return {
        AppCapabilityRegistry::buildDefault(),
        ProjectKnowledgeIndex::buildFromProjectData(projectData),
        DocumentationKnowledgeIndex::buildDefault(),
        AiToolRegistry::buildDefault(),
    };
}

} // namespace urpg::ai
