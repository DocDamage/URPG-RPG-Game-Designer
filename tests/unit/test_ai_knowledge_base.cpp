#include "editor/ai/ai_assistant_panel.h"
#include "engine/core/ai/ai_knowledge_base.h"
#include "engine/core/ai/wysiwyg_chatbot_coverage.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/message/chatbot_component.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <memory>

namespace {

class ToolCommandChatService : public urpg::ai::IChatService {
public:
    explicit ToolCommandChatService(std::string command) : command_(std::move(command)) {}

    void requestResponse(const std::vector<urpg::ai::ChatMessage>& history, ChatCallback callback) override {
        (void)history;
        callback("I prepared a tool plan.", command_);
    }

private:
    std::string command_;
};

} // namespace

TEST_CASE("AI knowledge snapshot indexes app capabilities docs tools and project data",
          "[ai_knowledge][ai_assistant]") {
    const nlohmann::json project = {
        {"project_id", "p1"},
        {"name", "Knowledge Project"},
        {"maps", {{"town", {{"width", 20}, {"height", 20}}}}},
        {"events", nlohmann::json::array({{{"event_id", "door"}}})},
        {"dialogue", {{"intro", {{"lines", nlohmann::json::array({"Hello"})}}}}},
        {"abilities", nlohmann::json::array({{{"ability_id", "fire"}}})},
        {"assets", nlohmann::json::array({{{"id", "hero"}}})},
        {"project_files", nlohmann::json::array({
            {{"id", "project_json"}, {"path", "game/project.urpg.json"}, {"title", "Project Manifest"}, {"summary", "Root project file."}},
        })},
        {"schemas", nlohmann::json::array({
            {{"id", "ability_schema"}, {"path", "content/schemas/gameplay_ability.schema.json"}, {"title", "Ability Schema"}, {"summary", "Ability authoring contract."}},
        })},
        {"readiness_reports", nlohmann::json::array({
            {{"id", "release_matrix"}, {"path", "docs/RELEASE_READINESS_MATRIX.md"}, {"status", "PARTIAL"}, {"summary", "Release readiness evidence."}},
        })},
        {"validation_reports", nlohmann::json::array({
            {{"id", "local_gate"}, {"path", "reports/local_gate.json"}, {"status", "passed"}, {"summary", "Local gate validation output."}},
        })},
        {"asset_catalogs", nlohmann::json::array({
            {{"id", "main_catalog"}, {"path", "imports/reports/asset_intake/catalog.json"}, {"asset_count", 42}, {"duplicate_group_count", 3}, {"summary", "Indexed asset catalog."}},
        })},
        {"docs", nlohmann::json::array({
            {{"id", "combat_doc"}, {"path", "docs/BATTLE_CORE_NATIVE_SPEC.md"}, {"title", "Battle Spec"}, {"summary", "Battle authoring docs."}},
        })},
        {"template_specs", nlohmann::json::array({
            {{"id", "monster_collector"}, {"path", "docs/templates/monster_collector_rpg.md"}, {"status", "starter"}, {"summary", "Monster collector template spec."}},
        })},
        {"filesystem_documents", nlohmann::json::array({
            {
                {"id", "event_runtime_cpp"},
                {"path", "engine/core/events/event_runtime.cpp"},
                {"kind", "source"},
                {"content", "Event runtime executes command graphs and validates switch conditions."},
                {"indexed_at_epoch", 100},
                {"modified_at_epoch", 90},
                {"age_days", 2},
                {"max_age_days", 14},
            },
            {
                {"id", "asset_report"},
                {"path", "imports/reports/asset_intake/summary.json"},
                {"kind", "report"},
                {"content", "Asset catalog report lists promoted sprites and duplicate groups."},
                {"indexed_at_epoch", 100},
                {"modified_at_epoch", 120},
                {"age_days", 40},
                {"max_age_days", 14},
            },
        })},
        {"ingested_docs", nlohmann::json::array({
            {
                {"id", "readme"},
                {"path", "README.md"},
                {"content", "URPG Maker provides WYSIWYG RPG authoring with deterministic native runtime ownership."},
            },
        })},
    };

    const auto snapshot = urpg::ai::buildDefaultAiKnowledgeSnapshot(project);

    REQUIRE(snapshot.capabilities.capabilities().size() >= 10);
    REQUIRE(snapshot.tools.tools().size() >= 8);
    REQUIRE(snapshot.docs_index.entries().size() >= 5);
    REQUIRE(snapshot.project_index.entries().size() >= 6);
    REQUIRE(snapshot.capabilities.find("creator_command") != nullptr);
    REQUIRE(snapshot.tools.find("add_event") != nullptr);
    REQUIRE_FALSE(snapshot.project_index.search("dialogue").empty());
    REQUIRE_FALSE(snapshot.project_index.search("ability schema").empty());
    REQUIRE_FALSE(snapshot.project_index.search("release readiness").empty());
    REQUIRE_FALSE(snapshot.project_index.search("local gate validation").empty());
    REQUIRE_FALSE(snapshot.project_index.search("asset catalog duplicate").empty());
    REQUIRE_FALSE(snapshot.project_index.search("monster collector template").empty());
    REQUIRE_FALSE(snapshot.project_index.search("event runtime switch conditions").empty());
    REQUIRE_FALSE(snapshot.project_index.search("deterministic native runtime ownership").empty());
    REQUIRE_FALSE(snapshot.docs_index.search("copilot").empty());
    REQUIRE_FALSE(snapshot.docs_index.search("asset dlc library manager").empty());
    REQUIRE_FALSE(snapshot.docs_index.search("release checklist dashboard").empty());
    REQUIRE(snapshot.toJson()["capabilities"].size() == snapshot.capabilities.capabilities().size());
    const auto catalogMatches = snapshot.project_index.search("main_catalog");
    const auto catalogIt = std::find_if(catalogMatches.begin(), catalogMatches.end(), [](const auto& entry) {
        return entry.id == "asset_catalogs:main_catalog";
    });
    REQUIRE(catalogIt != catalogMatches.end());
    REQUIRE(catalogIt->metadata["asset_count"] == 42);
    REQUIRE(catalogIt->metadata["duplicate_group_count"] == 3);

    const auto staleMatches = snapshot.project_index.search("asset_report");
    const auto staleIt = std::find_if(staleMatches.begin(), staleMatches.end(), [](const auto& entry) {
        return entry.id == "filesystem_documents:asset_report";
    });
    REQUIRE(staleIt != staleMatches.end());
    REQUIRE(staleIt->metadata["freshness"] == "stale");
    REQUIRE(staleIt->metadata["stale"] == true);
    REQUIRE(staleIt->metadata["direct_ingestion"] == true);

    const auto freshMatches = snapshot.project_index.search("event_runtime_cpp");
    const auto freshIt = std::find_if(freshMatches.begin(), freshMatches.end(), [](const auto& entry) {
        return entry.id == "filesystem_documents:event_runtime_cpp";
    });
    REQUIRE(freshIt != freshMatches.end());
    REQUIRE(freshIt->metadata["freshness"] == "fresh");
    REQUIRE(freshIt->metadata["content_excerpt"].get<std::string>().find("Event runtime") != std::string::npos);
}

TEST_CASE("AI chatbot knowledge covers release WYSIWYG panels features and asset actions",
          "[ai_knowledge][ai_assistant][wysiwyg][coverage]") {
    const auto snapshot = urpg::ai::buildDefaultAiKnowledgeSnapshot();
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-007"},
        {"source_root", "imports/raw/urpg_stuff"},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"normalized_path", "asset://src-007/characters/hero.png"},
                 {"preview_path", "imports/raw/urpg_stuff/characters/hero.png"},
                 {"preview_kind", "image"},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"tags", {"hero", "kind:image"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }}});
    REQUIRE(library.promoteAsset("imports/raw/urpg_stuff/characters/hero.png").success);

    const auto report = urpg::ai::buildWysiwygChatbotCoverageReport(snapshot, library.snapshot());

    REQUIRE(report.passed);
    REQUIRE(report.release_panel_count > 0);
    REQUIRE(report.release_panel_count == report.searchable_panel_count);
    REQUIRE(report.capability_count == report.capability_with_tool_count);
    REQUIRE(report.capability_count == report.capability_with_wysiwyg_surface_count);
    REQUIRE(report.asset_panel_registered);
    REQUIRE(report.asset_chatbot_tool_registered);
    REQUIRE(report.asset_library_actions_available);
    REQUIRE(report.toJson()["missing"].empty());
}

TEST_CASE("AI task planner creates safe reviewable tool plans for creator tasks",
          "[ai_knowledge][ai_assistant][tools]") {
    const auto snapshot = urpg::ai::buildDefaultAiKnowledgeSnapshot();
    urpg::ai::AiTaskPlanner planner;

    const auto plan = planner.planTask("make a house with a door and event logic",
                                       snapshot.capabilities,
                                       snapshot.project_index,
                                       snapshot.docs_index,
                                       snapshot.tools);

    REQUIRE(plan.schema == "urpg.ai_task_plan.v1");
    REQUIRE(plan.ready_for_approval);
    REQUIRE(plan.capability_ids.size() >= 3);
    REQUIRE(plan.steps.size() == 1);
    REQUIRE(plan.steps[0].tool_id == "plan_creator_command");

    const auto approval = snapshot.tools.approvalManifest(plan, snapshot.capabilities);
    REQUIRE(approval["pending_count"] == 1);
    REQUIRE(approval["pending_steps"][0]["tool_id"] == "plan_creator_command");
    REQUIRE(approval["pending_steps"][0]["project_paths"].size() >= 1);

    const auto blocked = snapshot.tools.applyApprovedPlan(plan, {{"project_id", "p1"}});
    REQUIRE_FALSE(blocked.applied);
    REQUIRE_FALSE(blocked.diagnostics.empty());

    auto approved = plan;
    approved.steps[0].approved = true;
    const auto applied = snapshot.tools.applyApprovedPlan(approved, {{"project_id", "p1"}});
    REQUIRE(applied.applied);
    REQUIRE(applied.project_data["creator_command_requests"].size() == 1);
    REQUIRE(applied.project_data["ai_tool_applications"].size() == 1);
}

TEST_CASE("AI tool registry lists every mutating tool that needs approval",
          "[ai_knowledge][ai_assistant][tools][approval]") {
    const auto tools = urpg::ai::AiToolRegistry::buildDefault();
    const auto mutating = tools.mutatingToolsRequiringApproval();

    REQUIRE(mutating.size() >= 15);
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "create_map"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "place_tile"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "paint_region"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "configure_environment"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "add_event"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "edit_dialogue"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "add_localization_entry"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "add_quest"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "set_npc_schedule"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "add_ability"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "add_vfx_keyframe"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "configure_save_preview"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "import_asset_record"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "create_template_project"; }));
    REQUIRE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "plan_creator_command"; }));
    REQUIRE_FALSE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "run_validation"; }));
    REQUIRE_FALSE(std::any_of(mutating.begin(), mutating.end(), [](const auto& tool) { return tool.id == "run_export_preview"; }));
}

TEST_CASE("AI task planner covers broader WYSIWYG tool lanes",
          "[ai_knowledge][ai_assistant][tools]") {
    const auto snapshot = urpg::ai::buildDefaultAiKnowledgeSnapshot();
    urpg::ai::AiTaskPlanner planner;

    const std::vector<std::pair<std::string, std::string>> cases = {
        {"add localization for the town intro", "add_localization_entry"},
        {"create quest for helping the farmer", "add_quest"},
        {"set npc schedule for the merchant", "set_npc_schedule"},
        {"add battle vfx keyframe", "add_vfx_keyframe"},
        {"make save lab scenario", "configure_save_preview"},
        {"import asset record", "import_asset_record"},
        {"create template starter", "create_template_project"},
        {"configure lighting and weather", "configure_environment"},
        {"paint region rule", "paint_region"},
        {"create map for dungeon", "create_map"},
    };

    for (const auto& [request, toolId] : cases) {
        const auto plan = planner.planTask(request, snapshot.capabilities, snapshot.project_index,
                                           snapshot.docs_index, snapshot.tools);
        INFO(request);
        REQUIRE(plan.ready_for_approval);
        REQUIRE(plan.steps.size() == 1);
        REQUIRE(plan.steps[0].tool_id == toolId);
    }
}

TEST_CASE("AI tool registry applies approved map event dialogue ability and export steps",
          "[ai_knowledge][ai_assistant][tools]") {
    const auto tools = urpg::ai::AiToolRegistry::buildDefault();
    urpg::ai::AiTaskPlan plan;
    plan.id = "manual_plan";
    plan.user_request = "build a complete starter flow";
    plan.steps = {
        {"create_map", "create_map", "Create the town map.", {{"map_id", "town"}, {"width", 30}, {"height", 22}}, true},
        {"place_tile", "place_tile", "Place a visible marker tile.", {{"map_id", "town"}, {"layer_id", "terrain"}, {"x", 4}, {"y", 5}, {"tile_id", 2}}, true},
        {"add_event", "add_event", "Add intro event.", {{"event_id", "intro_event"}, {"map_id", "town"}, {"x", 4}, {"y", 5}, {"commands", nlohmann::json::array({"message"})}}, true},
        {"dialogue", "edit_dialogue", "Add intro dialogue.", {{"dialogue_id", "intro"}, {"lines", nlohmann::json::array({"Welcome."})}}, true},
        {"ability", "add_ability", "Add starter ability.", {{"ability_id", "spark"}, {"cost", 3}, {"cooldown", 1}, {"effects", nlohmann::json::array({"damage"})}}, true},
        {"validate", "run_validation", "Queue validation.", {{"scope", "project"}}, true},
        {"export", "run_export_preview", "Queue export preview.", {{"profile", "default"}}, true},
    };

    REQUIRE(tools.validatePlan(plan).empty());
    const auto result = tools.applyApprovedPlan(plan, {{"project_id", "p1"}});

    REQUIRE(result.applied);
    REQUIRE(result.project_data["maps"]["town"]["width"] == 30);
    REQUIRE(result.project_data["maps"]["town"]["tile_edits"].size() == 1);
    REQUIRE(result.project_data["events"].size() == 1);
    REQUIRE(result.project_data["dialogue"]["intro"]["lines"].size() == 1);
    REQUIRE(result.project_data["abilities"].size() == 1);
    REQUIRE(result.project_data["last_ai_validation"]["status"] == "passed");
    REQUIRE(result.project_data["last_ai_validation"]["scope"] == "project");
    REQUIRE(result.project_data["last_ai_validation"]["validator_count"] == 2);
    REQUIRE(result.project_data["last_ai_validation"]["validators"][0]["validator"] == "event_graph_validator");
    REQUIRE(result.project_data["last_ai_validation"]["validators"][1]["validator"] == "ability_sandbox_validator");
    REQUIRE(result.project_data["ai_validation_reports"].size() == 1);
    REQUIRE(result.project_data["last_ai_export_preview"]["status"] == "queued");
}

TEST_CASE("AI tool registry applies broader WYSIWYG project records",
          "[ai_knowledge][ai_assistant][tools]") {
    const auto tools = urpg::ai::AiToolRegistry::buildDefault();
    urpg::ai::AiTaskPlan plan;
    plan.id = "broad_plan";
    plan.user_request = "configure broad wysiwyg systems";
    plan.steps = {
        {"region", "paint_region", "Paint region.", {{"map_id", "town"}, {"region_id", "safe_zone"}, {"x", 1}, {"y", 2}, {"rule", "no_encounters"}}, true},
        {"environment", "configure_environment", "Set weather.", {{"map_id", "town"}, {"weather", "rain"}, {"lighting_profile", "night"}}, true},
        {"localization", "add_localization_entry", "Add text.", {{"locale", "en-US"}, {"key", "intro.hello"}, {"text", "Hello"}}, true},
        {"quest", "add_quest", "Add quest.", {{"quest_id", "q1"}, {"objectives", nlohmann::json::array({"talk"})}}, true},
        {"schedule", "set_npc_schedule", "Add schedule.", {{"npc_id", "merchant"}, {"schedule", nlohmann::json::array({{{"time", "day"}, {"activity", "shop"}}})}}, true},
        {"vfx", "add_vfx_keyframe", "Add VFX.", {{"timeline_id", "slash"}, {"time", 0.5}, {"effect", "spark"}}, true},
        {"save", "configure_save_preview", "Add save scenario.", {{"save_id", "slot1"}, {"scenario", "corrupt save"}}, true},
        {"asset", "import_asset_record", "Add asset.", {{"asset_id", "hero"}, {"path", "assets/hero.png"}, {"license", "CC0"}}, true},
        {"template", "create_template_project", "Add template.", {{"template_id", "jrpg"}, {"project_id", "new_game"}}, true},
    };

    REQUIRE(tools.validatePlan(plan).empty());
    const auto result = tools.applyApprovedPlan(plan, {{"project_id", "p1"}});

    REQUIRE(result.applied);
    REQUIRE(result.project_data["regions"].size() == 1);
    REQUIRE(result.project_data["environments"]["town"]["weather"] == "rain");
    REQUIRE(result.project_data["localization"]["en-US"]["intro.hello"] == "Hello");
    REQUIRE(result.project_data["quests"].size() == 1);
    REQUIRE(result.project_data["npc_schedules"]["merchant"].size() == 1);
    REQUIRE(result.project_data["battle_vfx"]["slash"]["keyframes"].size() == 1);
    REQUIRE(result.project_data["save_labs"].size() == 1);
    REQUIRE(result.project_data["assets"].size() == 1);
    REQUIRE(result.project_data["template_instances"].size() == 1);
    REQUIRE(result.project_data["ai_tool_previews"].size() == 3);
    REQUIRE(result.project_data["ai_tool_previews"][0]["kind"] == "lighting_weather_preview");
    REQUIRE(result.project_data["ai_tool_previews"][1]["kind"] == "vfx_timeline_edit");
    REQUIRE(result.project_data["ai_tool_previews"][2]["kind"] == "asset_import_promotion");
    REQUIRE(result.project_data["asset_promotion_requests"][0]["status"] == "review_required");
}

TEST_CASE("AI tool registry emits concrete subsystem preview artifacts",
          "[ai_knowledge][ai_assistant][tools]") {
    const auto tools = urpg::ai::AiToolRegistry::buildDefault();
    urpg::ai::AiTaskPlan plan;
    plan.id = "concrete_tool_plan";
    plan.user_request = "build concrete subsystem previews";
    plan.steps = {
        {"event", "add_event", "Add event graph.", {{"event_id", "door"}, {"map_id", "town"}, {"x", 2}, {"y", 3}, {"commands", nlohmann::json::array({"show_text", "transfer_player"})}}, true},
        {"ability", "add_ability", "Compose ability sandbox.", {{"ability_id", "spark"}, {"cost", 4}, {"cooldown", 2}, {"effects", nlohmann::json::array({"damage"})}}, true},
        {"export", "run_export_preview", "Configure export preview.", {{"profile", "windows_debug"}}, true},
    };

    REQUIRE(tools.validatePlan(plan).empty());
    const auto result = tools.applyApprovedPlan(plan, {{"project_id", "p1"}});

    REQUIRE(result.applied);
    REQUIRE(result.project_data["ai_tool_previews"].size() == 3);
    REQUIRE(result.project_data["ai_tool_previews"][0]["kind"] == "event_graph_authoring");
    REQUIRE(result.project_data["ai_tool_previews"][0]["payload"]["node_count"] == 2);
    REQUIRE(result.project_data["ai_tool_previews"][1]["kind"] == "ability_sandbox_composition");
    REQUIRE(result.project_data["ai_tool_previews"][1]["payload"]["preview_surface"] == "ability_sandbox");
    REQUIRE(result.project_data["ai_tool_previews"][2]["kind"] == "export_preview_configuration");
    REQUIRE(result.project_data["last_ai_export_preview"]["profile"] == "windows_debug");
    REQUIRE(result.project_data["last_ai_export_preview"]["preview_surface"] == "export_preview_panel");
}

TEST_CASE("AI run_validation executes concrete preview validators",
          "[ai_knowledge][ai_assistant][tools][validation]") {
    const auto tools = urpg::ai::AiToolRegistry::buildDefault();
    urpg::ai::AiTaskPlan plan;
    plan.id = "validation_plan";
    plan.user_request = "validate concrete previews";
    plan.steps = {
        {"event", "add_event", "Add incomplete event graph.", {{"event_id", "empty"}, {"map_id", "town"}, {"x", 1}, {"y", 1}, {"commands", nlohmann::json::array()}}, true},
        {"asset", "import_asset_record", "Review asset import.", {{"asset_id", "hero"}, {"path", "project-configured"}, {"license", "review_required"}}, true},
        {"validate", "run_validation", "Run validators.", {{"scope", "ai_tool_previews"}}, true},
    };

    REQUIRE(tools.validatePlan(plan).empty());
    const auto result = tools.applyApprovedPlan(plan, {{"project_id", "p1"}});

    REQUIRE(result.applied);
    REQUIRE(result.project_data["last_ai_validation"]["status"] == "failed");
    REQUIRE(result.project_data["last_ai_validation"]["scope"] == "ai_tool_previews");
    REQUIRE(result.project_data["last_ai_validation"]["preview_artifact_count"] == 2);
    REQUIRE(result.project_data["last_ai_validation"]["validator_count"] == 2);
    REQUIRE(result.project_data["last_ai_validation"]["issue_count"].get<std::size_t>() >= 3);
    REQUIRE(result.project_data["last_ai_validation"]["validators"][0]["status"] == "failed");
    REQUIRE(result.project_data["last_ai_validation"]["validators"][0]["issues"][0]["code"] == "event_graph_empty");
    REQUIRE(result.project_data["last_ai_validation"]["validators"][1]["validator"] == "asset_import_promotion_validator");
    REQUIRE(result.project_data["last_ai_validation"]["validators"][1]["status"] == "warning");
    REQUIRE(result.project_data["ai_tool_previews"].back()["kind"] == "validation_execution");
    REQUIRE(result.project_data["ai_tool_previews"].back()["payload"]["validator_count"] == 2);
}

TEST_CASE("AI assistant panel exposes knowledge and task plan snapshots",
          "[ai_knowledge][ai_assistant][editor]") {
    urpg::editor::AiAssistantPanel panel;
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-008"},
        {"source_root", "imports/raw/curated"},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/curated/ui/menu.png"},
                 {"normalized_path", "asset://src-008/ui/menu.png"},
                 {"preview_path", "imports/raw/curated/ui/menu.png"},
                 {"preview_kind", "image"},
                 {"media_kind", "image"},
                 {"category", "ui"},
                 {"tags", {"menu", "kind:image"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }}});
    REQUIRE(library.promoteAsset("imports/raw/curated/ui/menu.png").success);

    urpg::ai::AiAssistantConfig config;
    config.enabled = true;
    config.providerId = "local_deterministic";
    panel.setConfig(config, true);
    panel.setProjectData({{"project_id", "p1"}, {"maps", {{"town", {{"width", 20}, {"height", 20}}}}}});
    panel.setAssetLibrarySnapshot(library.snapshot());
    panel.setTaskRequest("create dialogue for the town intro");
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["status"]["reason"] == "ready");
    REQUIRE(snapshot["knowledge"]["capability_count"].get<size_t>() >= 10);
    REQUIRE(snapshot["knowledge"]["tool_count"].get<size_t>() >= 8);
    REQUIRE(snapshot["task_plan"]["steps"].size() == 1);
    REQUIRE(snapshot["task_plan"]["steps"][0]["tool_id"] == "edit_dialogue");
    REQUIRE(snapshot["approval"]["pending_count"] == 1);
    REQUIRE(snapshot["approval"]["pending_steps"][0]["tool_id"] == "edit_dialogue");
    REQUIRE(snapshot["controls"]["approve_all_button"]["enabled"] == true);
    REQUIRE(snapshot["controls"]["apply_button"]["enabled"] == false);
    REQUIRE(snapshot["controls"]["step_controls"][0]["approve_button"]["enabled"] == true);
    REQUIRE(snapshot["validation"]["valid"] == false);
    REQUIRE(snapshot["validation"]["blocked_reason"] == "ai_tool_unapproved");
    REQUIRE(snapshot["apply_preview"]["would_apply"] == false);
    REQUIRE(snapshot["apply_preview"]["project_patch_count"] == 0);
    REQUIRE(snapshot["apply_history"]["count"] == 0);
    REQUIRE(snapshot["apply_history"]["can_revert_latest"] == false);
    REQUIRE(snapshot["rationale_rows"].size() == 1);
    REQUIRE(snapshot["rationale_rows"][0]["step_id"] == "step_dialogue");
    REQUIRE(snapshot["rationale_rows"][0]["state"] == "needs_review");
    REQUIRE(snapshot["rationale_rows"][0]["rationale"].get<std::string>().find("requires review") != std::string::npos);
    REQUIRE(snapshot["wysiwyg_chatbot_coverage"]["passed"] == true);
    REQUIRE(snapshot["wysiwyg_chatbot_coverage"]["asset_library_actions_available"] == true);
    REQUIRE(snapshot["wysiwyg_chatbot_coverage"]["release_panel_count"].get<size_t>() > 0);
}

TEST_CASE("AI assistant panel approves and applies task plans",
          "[ai_knowledge][ai_assistant][editor][approval]") {
    urpg::editor::AiAssistantPanel panel;
    urpg::ai::AiAssistantConfig config;
    config.enabled = true;
    config.providerId = "local_deterministic";
    panel.setConfig(config, true);
    panel.setProjectData({{"project_id", "p1"}});
    panel.setTaskRequest("create dialogue for the town intro");
    panel.render();

    REQUIRE_FALSE(panel.applyApprovedPlan());
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["applied"] == false);
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["diagnostics"][0]["code"] == "ai_tool_unapproved");

    REQUIRE(panel.approveStep("step_dialogue"));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["approval"]["pending_count"] == 0);
    REQUIRE(panel.lastRenderSnapshot()["rationale_rows"][0]["state"] == "approved");
    REQUIRE(panel.lastRenderSnapshot()["apply_preview"]["would_apply"] == true);
    REQUIRE(panel.lastRenderSnapshot()["apply_preview"]["diff_rows"].size() > 0);
    REQUIRE(panel.lastRenderSnapshot()["apply_preview"]["diff_rows"][0]["tone"] == "added");
    REQUIRE(panel.lastRenderSnapshot()["apply_preview"]["diff_rows"][0]["root"] == "ai_tool_applications");

    REQUIRE(panel.applyApprovedPlan());
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["applied"] == true);
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["project_data"]["dialogue"]["generated_dialogue"]["lines"].size() == 1);
    REQUIRE_FALSE(panel.lastRenderSnapshot()["last_apply"]["project_patch"].empty());
    REQUIRE_FALSE(panel.lastRenderSnapshot()["last_apply"]["revert_patch"].empty());

    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["controls"]["revert_button"]["enabled"] == true);
    REQUIRE(panel.lastRenderSnapshot()["controls"]["undo_stack"]["available"] == true);
    REQUIRE(panel.lastRenderSnapshot()["controls"]["undo_stack"]["count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["has_changes"] == true);
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["forward_patch_count"].get<size_t>() > 0);
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["revert_patch_count"].get<size_t>() > 0);
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["row_count"].get<size_t>() > 0);
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["rows"][0]["operation"] == "add");
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["rows"][0].contains("before"));
    REQUIRE(panel.lastRenderSnapshot()["result_diff"]["rows"][0].contains("after"));
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["can_revert_latest"] == true);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["can_revert"] == true);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["project_patch_count"].get<size_t>() > 0);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["revert_patch_count"].get<size_t>() > 0);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["diff_rows"].size() > 0);

    REQUIRE(panel.revertLastAppliedPlan());
    REQUIRE(panel.lastRenderSnapshot()["last_revert"]["reverted"] == true);
    REQUIRE_FALSE(panel.lastRenderSnapshot()["last_revert"]["project_data"].contains("dialogue"));

    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["count"] == 0);
    REQUIRE(panel.lastRenderSnapshot()["controls"]["undo_stack"]["available"] == false);
}

TEST_CASE("AI assistant panel rejects proposed task steps",
          "[ai_knowledge][ai_assistant][editor][approval]") {
    urpg::editor::AiAssistantPanel panel;
    panel.setProjectData({{"project_id", "p1"}});
    panel.setTaskRequest("create dialogue for the town intro");
    panel.render();

    REQUIRE(panel.rejectStep("step_dialogue"));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["task_plan"]["steps"][0]["rejected"] == true);
    REQUIRE(panel.lastRenderSnapshot()["approval"]["pending_count"] == 0);
    REQUIRE_FALSE(panel.applyApprovedPlan());
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["diagnostics"][0]["code"] == "ai_tool_rejected");
}

TEST_CASE("AI assistant panel approve all handles creator command plans",
          "[ai_knowledge][ai_assistant][editor][approval]") {
    urpg::editor::AiAssistantPanel panel;
    panel.setProjectData({{"project_id", "p1"}});
    panel.setTaskRequest("make a house with a door and event logic");
    panel.render();

    REQUIRE(panel.lastRenderSnapshot()["approval"]["pending_count"] == 1);
    REQUIRE(panel.approveAllPendingSteps() == 1);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["approval"]["pending_count"] == 0);

    REQUIRE(panel.applyApprovedPlan());
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["project_data"]["creator_command_requests"].size() == 1);
}

TEST_CASE("Chatbot component plans approves and applies AI tool commands",
          "[ai_knowledge][ai_assistant][chatbot]") {
    auto service = std::make_shared<ToolCommandChatService>("AI_TASK:create dialogue for the town intro");
    urpg::ai::ChatbotComponent chatbot(service);
    urpg::assets::AssetLibrary library;
    library.ingestPromotionCatalog(nlohmann::json{
        {"source_id", "SRC-009"},
        {"source_root", "imports/raw/curated"},
        {"assets",
         {
             {
                 {"source_path", "imports/raw/curated/characters/hero.png"},
                 {"normalized_path", "asset://src-009/characters/hero.png"},
                 {"preview_path", "imports/raw/curated/characters/hero.png"},
                 {"preview_kind", "image"},
                 {"preview_width", 128},
                 {"preview_height", 96},
                 {"media_kind", "image"},
                 {"category", "characters"},
                 {"tags", {"hero", "kind:image"}},
                 {"license", "user_attested_free_for_game_use_pending_per_pack_attribution"},
             },
         }}});
    REQUIRE(library.promoteAsset("imports/raw/curated/characters/hero.png").success);
    chatbot.setProjectData({{"project_id", "p1"}});
    chatbot.setAssetLibrarySnapshot(library.snapshot());

    bool callbackCalled = false;
    chatbot.getResponse("please make intro dialogue", [&](urpg::message::DialoguePage page) {
        callbackCalled = true;
        REQUIRE(page.command == "AI_TASK:create dialogue for the town intro");
    });

    REQUIRE(callbackCalled);
    REQUIRE(chatbot.lastAiToolSnapshot()["wysiwyg_chatbot_coverage"]["passed"] == true);
    REQUIRE(chatbot.lastAiToolSnapshot()["wysiwyg_chatbot_coverage"]["asset_library_actions_available"] == true);
    REQUIRE(chatbot.lastAiToolSnapshot()["asset_action_rows"].size() == 1);
    REQUIRE(chatbot.lastAiToolSnapshot()["asset_action_rows"][0]["recommended_action"] == "ready");
    REQUIRE(chatbot.lastAiToolSnapshot()["asset_action_rows"][0]["promote_button"]["disabled_reason"] ==
            "asset_already_promoted");
    REQUIRE(chatbot.lastAiToolSnapshot()["asset_preview_rows"].size() == 1);
    REQUIRE(chatbot.lastAiToolSnapshot()["asset_preview_rows"][0]["thumbnail"]["ready"] == true);
    REQUIRE(chatbot.lastAiToolSnapshot()["asset_preview_rows"][0]["thumbnail"]["width"] == 128);
    REQUIRE(chatbot.lastAiToolSnapshot()["task_plan"]["steps"][0]["tool_id"] == "edit_dialogue");
    REQUIRE(chatbot.lastAiToolSnapshot()["approval"]["pending_count"] == 1);

    const auto approved = chatbot.executeTool("AI_APPROVE_STEP:step_dialogue");
    REQUIRE(approved["approved"] == true);
    REQUIRE(approved["approval"]["pending_count"] == 0);

    const auto applied = chatbot.executeTool("AI_APPLY");
    REQUIRE(applied["last_apply"]["applied"] == true);
    REQUIRE(chatbot.projectData()["dialogue"]["generated_dialogue"]["lines"].size() == 1);
}

TEST_CASE("Chatbot component rejects AI tool commands before apply",
          "[ai_knowledge][ai_assistant][chatbot]") {
    urpg::ai::ChatbotComponent chatbot(std::make_shared<ToolCommandChatService>("AI_TASK:make a house"));
    chatbot.setProjectData({{"project_id", "p1"}});

    chatbot.executeTool("AI_TASK:make a house");
    const auto rejected = chatbot.executeTool("AI_REJECT_STEP:step_creator_command");
    REQUIRE(rejected["rejected"] == true);
    REQUIRE(rejected["task_plan"]["steps"][0]["rejected"] == true);

    const auto applied = chatbot.executeTool("AI_APPLY");
    REQUIRE(applied["last_apply"]["applied"] == false);
    REQUIRE(applied["last_apply"]["diagnostics"][0]["code"] == "ai_tool_rejected");
}
