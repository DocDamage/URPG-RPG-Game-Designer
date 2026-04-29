#include "editor/ai/ai_assistant_panel.h"
#include "engine/core/ai/ai_knowledge_base.h"
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
    };

    const auto snapshot = urpg::ai::buildDefaultAiKnowledgeSnapshot(project);

    REQUIRE(snapshot.capabilities.capabilities().size() >= 10);
    REQUIRE(snapshot.tools.tools().size() >= 8);
    REQUIRE(snapshot.docs_index.entries().size() >= 5);
    REQUIRE(snapshot.project_index.entries().size() >= 6);
    REQUIRE(snapshot.capabilities.find("creator_command") != nullptr);
    REQUIRE(snapshot.tools.find("add_event") != nullptr);
    REQUIRE_FALSE(snapshot.project_index.search("dialogue").empty());
    REQUIRE_FALSE(snapshot.docs_index.search("copilot").empty());
    REQUIRE(snapshot.toJson()["capabilities"].size() == snapshot.capabilities.capabilities().size());
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
    REQUIRE(result.project_data["last_ai_validation"]["status"] == "queued");
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
}

TEST_CASE("AI assistant panel exposes knowledge and task plan snapshots",
          "[ai_knowledge][ai_assistant][editor]") {
    urpg::editor::AiAssistantPanel panel;
    urpg::ai::AiAssistantConfig config;
    config.enabled = true;
    config.providerId = "local_deterministic";
    panel.setConfig(config, true);
    panel.setProjectData({{"project_id", "p1"}, {"maps", {{"town", {{"width", 20}, {"height", 20}}}}}});
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
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["count"] == 1);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["can_revert_latest"] == true);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["can_revert"] == true);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["project_patch_count"].get<size_t>() > 0);
    REQUIRE(panel.lastRenderSnapshot()["apply_history"]["entries"][0]["revert_patch_count"].get<size_t>() > 0);

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
    chatbot.setProjectData({{"project_id", "p1"}});

    bool callbackCalled = false;
    chatbot.getResponse("please make intro dialogue", [&](urpg::message::DialoguePage page) {
        callbackCalled = true;
        REQUIRE(page.command == "AI_TASK:create dialogue for the town intro");
    });

    REQUIRE(callbackCalled);
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
