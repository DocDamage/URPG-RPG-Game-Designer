#include "engine/core/ai/creator_command_planner.h"
#include "editor/ai/creator_command_panel.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

namespace {

class FakeCreatorHttpClient final : public urpg::net::IHttpClient {
  public:
    urpg::net::HttpResponse postJson(const urpg::net::HttpRequest& request) override {
        lastRequest = request;
        return {200, R"({"output_text":"{\"schema\":\"urpg.creator_command_plan.v1\",\"intent\":\"noop\",\"can_apply\":false}"})", ""};
    }

    urpg::net::HttpRequest lastRequest;
};

bool hasLogic(const urpg::ai::CreatorCommandPlan& plan, const std::string& kind) {
    return std::any_of(plan.logic_edits.begin(), plan.logic_edits.end(), [&](const auto& edit) {
        return edit.kind == kind;
    });
}

} // namespace

TEST_CASE("creator command planner builds a complete house plan from selected tile intent",
          "[creator_command][ai][wysiwyg]") {
    urpg::ai::CreatorCommandPlanner planner;
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house here";
    request.project_id = "p1";
    request.map_id = "town";
    request.tile_x = 4;
    request.tile_y = 5;
    request.width = 20;
    request.height = 20;
    request.selected_tile_id = 2;

    const auto plan = planner.plan(request);

    REQUIRE(plan.intent == "make_house");
    REQUIRE(plan.can_apply);
    REQUIRE(plan.diagnostics.empty());
    REQUIRE(plan.tile_edits.size() >= 40);
    REQUIRE(plan.prop_edits.size() == 2);
    REQUIRE(hasLogic(plan, "transfer_player"));
    REQUIRE(hasLogic(plan, "message"));
    REQUIRE(hasLogic(plan, "set_variable"));
    REQUIRE(plan.toJson()["schema"] == "urpg.creator_command_plan.v1");

    const auto preview = planner.previewDocument(request, plan);
    REQUIRE(preview.tileAt("collision", 6, 8).value() == 0);
    REQUIRE_FALSE(preview.validateNavigation().empty());
}

TEST_CASE("creator command planner exposes ChatGPT Gemini and Kimi provider profiles",
          "[creator_command][ai][providers]") {
    const auto profiles = urpg::ai::defaultCreatorAiProviderProfiles();
    REQUIRE(profiles.size() >= 20);

    const auto chatgpt = urpg::ai::creatorAiProviderProfile(urpg::ai::CreatorAiProvider::ChatGpt);
    const auto gemini = urpg::ai::creatorAiProviderProfile(urpg::ai::CreatorAiProvider::Gemini);
    const auto kimi = urpg::ai::creatorAiProviderProfile(urpg::ai::CreatorAiProvider::Kimi);
    const auto anthropic = urpg::ai::creatorAiProviderProfile(urpg::ai::CreatorAiProvider::AnthropicClaude);
    const auto ollama = urpg::ai::creatorAiProviderProfile(urpg::ai::CreatorAiProvider::Ollama);
    const auto lmStudio = urpg::ai::creatorAiProviderProfile(urpg::ai::CreatorAiProvider::LmStudio);

    REQUIRE(chatgpt.id == "chatgpt");
    REQUIRE(chatgpt.api_key_env == "OPENAI_API_KEY");
    REQUIRE(gemini.id == "gemini");
    REQUIRE(gemini.api_key_env == "GEMINI_API_KEY");
    REQUIRE(kimi.id == "kimi");
    REQUIRE(kimi.api_key_env == "KIMI_API_KEY");
    REQUIRE(kimi.network_required);
    REQUIRE(anthropic.id == "anthropic");
    REQUIRE(anthropic.api_key_env == "ANTHROPIC_API_KEY");
    REQUIRE(ollama.id == "ollama");
    REQUIRE(ollama.local_provider);
    REQUIRE(lmStudio.id == "lmstudio");
    REQUIRE(lmStudio.endpoint.find("127.0.0.1") != std::string::npos);

    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house";
    request.map_id = "town";
    request.provider = urpg::ai::CreatorAiProvider::ChatGpt;
    REQUIRE(urpg::ai::buildCreatorProviderRequest(request)["body"].contains("input"));

    request.provider = urpg::ai::CreatorAiProvider::Gemini;
    REQUIRE(urpg::ai::buildCreatorProviderRequest(request)["body"].contains("contents"));

    request.provider = urpg::ai::CreatorAiProvider::Kimi;
    REQUIRE(urpg::ai::buildCreatorProviderRequest(request)["body"].contains("messages"));

    request.provider = urpg::ai::CreatorAiProvider::AnthropicClaude;
    REQUIRE(urpg::ai::buildCreatorProviderRequest(request)["body"].contains("max_tokens"));

    request.provider = urpg::ai::CreatorAiProvider::Ollama;
    REQUIRE(urpg::ai::buildCreatorProviderRequest(request)["body"].contains("messages"));
}

TEST_CASE("creator command provider transport builds redacted native HTTP diagnostic without hardcoded secrets",
          "[creator_command][ai][providers][transport]") {
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house";
    request.map_id = "town";
    request.provider = urpg::ai::CreatorAiProvider::ChatGpt;

    urpg::ai::CreatorProviderTransportConfig config;
    config.api_key = "test-key";
    config.request_path = "build/creator_request.json";
    config.response_path = "build/creator_response.json";
    config.execute = false;

    const auto command = urpg::ai::buildCreatorProviderCurlCommand(request, config);
    REQUIRE(command.find("native_http_post") != std::string::npos);
    REQUIRE(command.find("api.openai.com") != std::string::npos);
    REQUIRE(command.find("test-key") == std::string::npos);
    REQUIRE(command.find("[redacted]") != std::string::npos);

    const auto result = urpg::ai::invokeCreatorProvider(request, config);
    REQUIRE_FALSE(result.attempted);
    REQUIRE_FALSE(result.success);
    REQUIRE(result.message == "dry_run");
    REQUIRE(result.toJson()["command"].get<std::string>().find("api.openai.com") != std::string::npos);
}

TEST_CASE("creator command provider execution posts through injectable native HTTP client",
          "[creator_command][ai][providers][transport][creator]") {
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house";
    request.map_id = "town";
    request.provider = urpg::ai::CreatorAiProvider::ChatGpt;

    urpg::ai::CreatorProviderTransportConfig config;
    config.api_key = "creator-secret";
    config.request_path = "build/creator_native_http_request.json";
    config.response_path = "build/creator_native_http_response.json";
    config.execute = true;

    FakeCreatorHttpClient http;
    const auto result = urpg::ai::invokeCreatorProvider(request, config, &http);

    REQUIRE(result.attempted);
    REQUIRE(result.success);
    REQUIRE(result.command.find("creator-secret") == std::string::npos);
    REQUIRE(http.lastRequest.url.find("api.openai.com") != std::string::npos);
    REQUIRE(http.lastRequest.headers.count("Authorization") == 1);
    REQUIRE(http.lastRequest.headers.at("Authorization").find("creator-secret") != std::string::npos);
}

TEST_CASE("creator command provider responses import into safe creator plans",
          "[creator_command][ai][providers][parse]") {
    urpg::ai::CreatorCommandPlanner planner;
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house";
    request.map_id = "town";
    request.width = 20;
    request.height = 20;
    const auto plan = planner.plan(request);
    const auto planJson = plan.toJson();
    const auto dumped = planJson.dump();

    nlohmann::json chatResponse;
    chatResponse["choices"] = nlohmann::json::array({{{"message", {{"content", dumped}}}}});
    nlohmann::json geminiResponse;
    geminiResponse["candidates"] = nlohmann::json::array({
        {{"content", {{"parts", nlohmann::json::array({{{"text", "```json\n" + dumped + "\n```"}}})}}}},
    });
    nlohmann::json anthropicResponse;
    anthropicResponse["content"] = nlohmann::json::array({{{"text", "Result:\n" + dumped}}});

    REQUIRE(urpg::ai::parseCreatorCommandPlan(
                urpg::ai::extractCreatorPlanJsonFromProviderResponse(chatResponse)).intent == "make_house");
    REQUIRE(urpg::ai::parseCreatorCommandPlan(
                urpg::ai::extractCreatorPlanJsonFromProviderResponse(geminiResponse)).logic_edits.size() == 3);
    REQUIRE(urpg::ai::parseCreatorCommandPlan(
                urpg::ai::extractCreatorPlanJsonFromProviderResponse(anthropicResponse)).can_apply);

    const auto malformed = urpg::ai::extractCreatorPlanJsonFromProviderResponse(nlohmann::json{{"content", "not json"}});
    const auto malformedPlan = urpg::ai::parseCreatorCommandPlan(malformed);
    REQUIRE(malformedPlan.intent == "provider_parse_failed");
    REQUIRE_FALSE(malformedPlan.can_apply);
    REQUIRE_FALSE(malformedPlan.diagnostics.empty());
}

TEST_CASE("creator command validation rejects unsafe provider plans before apply",
          "[creator_command][ai][providers][apply]") {
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house";
    request.map_id = "town";
    request.width = 10;
    request.height = 10;

    auto malicious = urpg::ai::parseCreatorCommandPlan({
        {"schema", "urpg.creator_command_plan.v1"},
        {"intent", "make_house"},
        {"can_apply", true},
        {"tile_edits", {{{"layer_id", "terrain"}, {"x", 99}, {"y", 99}, {"tile_id", 2}}}},
        {"prop_edits", nlohmann::json::array()},
        {"logic_edits", nlohmann::json::array()},
    });

    const auto diagnostics = urpg::ai::validateCreatorCommandPlan(request, malicious);
    REQUIRE_FALSE(diagnostics.empty());

    const auto result = urpg::ai::applyCreatorCommandPlan(request, malicious, {{"project_id", "p1"}});
    REQUIRE_FALSE(result.applied);
    REQUIRE(result.project_data["project_id"] == "p1");
}

TEST_CASE("creator command compatibility apply refuses detached project JSON mutation",
          "[creator_command][ai][providers][apply]") {
    urpg::ai::CreatorCommandPlanner planner;
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "make a house";
    request.project_id = "p1";
    request.map_id = "town";
    request.width = 20;
    request.height = 20;
    const auto plan = planner.plan(request);

    const auto result = urpg::ai::applyCreatorCommandPlan(request, plan, {{"project_id", "p1"}});
    REQUIRE_FALSE(result.applied);
    const nlohmann::json expected_project = {{"project_id", "p1"}};
    REQUIRE(result.project_data == expected_project);
    REQUIRE(std::any_of(result.diagnostics.begin(), result.diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "creator_generic_project_mutation_removed";
    }));
}

TEST_CASE("creator command deterministic planner supports common creator intents",
          "[creator_command][ai][wysiwyg][intents]") {
    urpg::ai::CreatorCommandPlanner planner;
    urpg::ai::CreatorCommandRequest request;
    request.map_id = "town";
    request.width = 24;
    request.height = 24;
    request.tile_x = 4;
    request.tile_y = 4;

    const std::vector<std::string> prompts = {
        "make a shop",
        "make an inn",
        "make a dungeon room",
        "make an npc",
        "make a quest giver",
        "make treasure chest",
        "make locked door",
        "make puzzle",
        "make farm plot",
    };

    for (const auto& prompt : prompts) {
        request.prompt = prompt;
        const auto plan = planner.plan(request);
        INFO(prompt);
        REQUIRE(plan.can_apply);
        REQUIRE(plan.diagnostics.empty());
        REQUIRE_FALSE(plan.logic_edits.empty());
        REQUIRE(urpg::ai::validateCreatorCommandPlan(request, plan).empty());
    }
}

TEST_CASE("creator command panel snapshots selected tile preview and native unavailable state",
          "[creator_command][ai][wysiwyg][editor]") {
    urpg::editor::CreatorCommandPanel panel;
    urpg::ai::CreatorCommandRequest request;
    request.prompt = "Make a house with logic";
    request.map_id = "town";
    request.tile_x = 3;
    request.tile_y = 4;
    request.width = 18;
    request.height = 18;
    request.provider = urpg::ai::CreatorAiProvider::Gemini;

    panel.setRequest(request);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["selected_tile"]["x"] == 3);
    REQUIRE(snapshot["plan"]["provider_id"] == "gemini");
    REQUIRE(snapshot["plan"]["can_apply"] == true);
    REQUIRE(snapshot["plan"]["logic_edits"].size() == 3);
    REQUIRE(snapshot["apply_preview"]["would_apply"] == false);
    REQUIRE(snapshot["apply_preview"]["code"] == "creator_native_domain_not_available");
    REQUIRE(snapshot["provider_transport"]["message"] == "dry_run");
    REQUIRE(snapshot["provider_request"]["provider"] == "gemini");
    REQUIRE(snapshot["preview"]["layer_count"] == 3);

    REQUIRE_FALSE(panel.applyCurrentPlan());
    REQUIRE(panel.lastRenderSnapshot()["last_apply"]["code"] == "creator_native_domain_not_available");
}
