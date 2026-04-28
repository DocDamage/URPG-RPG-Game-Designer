#pragma once

#include "engine/core/map/tile_layer_document.h"

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace urpg::ai {

enum class CreatorAiProvider {
    LocalDeterministic,
    ChatGpt,
    Gemini,
    Kimi,
    AnthropicClaude,
    Mistral,
    Cohere,
    Groq,
    Perplexity,
    XAiGrok,
    DeepSeek,
    TogetherAi,
    OpenRouter,
    AzureOpenAi,
    AwsBedrock,
    Ollama,
    LmStudio,
    LlamaCpp,
    Vllm,
    TextGenerationWebUi,
    LocalAi
};

struct CreatorAiProviderProfile {
    CreatorAiProvider provider = CreatorAiProvider::LocalDeterministic;
    std::string id;
    std::string display_name;
    std::string endpoint;
    std::string api_key_env;
    std::string request_format;
    std::string auth_scheme;
    bool local_provider = false;
    bool network_required = false;
};

struct CreatorProviderTransportConfig {
    bool execute = false;
    std::string curl_executable = "curl";
    std::string api_key;
    std::string request_path;
    std::string response_path;
    int timeout_seconds = 45;
};

struct CreatorProviderTransportResult {
    bool attempted = false;
    bool success = false;
    int exit_code = -1;
    std::string command;
    std::string response_path;
    std::string message;
    nlohmann::json toJson() const;
};

struct CreatorCommandRequest {
    std::string prompt;
    std::string project_id;
    std::string map_id;
    int32_t tile_x = 0;
    int32_t tile_y = 0;
    int32_t width = 32;
    int32_t height = 32;
    int32_t selected_tile_id = 2;
    CreatorAiProvider provider = CreatorAiProvider::LocalDeterministic;
};

struct CreatorTileEdit {
    std::string layer_id;
    int32_t x = 0;
    int32_t y = 0;
    int32_t tile_id = 0;
};

struct CreatorPropEdit {
    std::string id;
    std::string asset_id;
    int32_t tile_x = 0;
    int32_t tile_y = 0;
};

struct CreatorLogicEdit {
    std::string id;
    std::string kind;
    std::string trigger;
    int32_t tile_x = 0;
    int32_t tile_y = 0;
    nlohmann::json payload = nlohmann::json::object();
};

struct CreatorCommandPlan {
    std::string schema = "urpg.creator_command_plan.v1";
    std::string intent;
    std::string provider_id;
    bool provider_network_required = false;
    bool deterministic_fallback_used = true;
    bool can_apply = false;
    std::vector<CreatorTileEdit> tile_edits;
    std::vector<CreatorPropEdit> prop_edits;
    std::vector<CreatorLogicEdit> logic_edits;
    std::vector<urpg::map::MapDiagnostic> diagnostics;
    nlohmann::json toJson() const;
};

struct CreatorCommandApplyResult {
    bool applied = false;
    nlohmann::json project_data = nlohmann::json::object();
    std::vector<urpg::map::MapDiagnostic> diagnostics;
    nlohmann::json toJson() const;
};

std::vector<CreatorAiProviderProfile> defaultCreatorAiProviderProfiles();
CreatorAiProviderProfile creatorAiProviderProfile(CreatorAiProvider provider);
CreatorAiProviderProfile creatorAiProviderProfileById(const std::string& providerId);
nlohmann::json buildCreatorProviderRequest(const CreatorCommandRequest& request);
std::string buildCreatorProviderCurlCommand(const CreatorCommandRequest& request,
                                            const CreatorProviderTransportConfig& config);
CreatorProviderTransportResult invokeCreatorProvider(const CreatorCommandRequest& request,
                                                     const CreatorProviderTransportConfig& config);
CreatorCommandPlan parseCreatorCommandPlan(const nlohmann::json& json);
nlohmann::json extractCreatorPlanJsonFromProviderResponse(const nlohmann::json& response);
std::vector<urpg::map::MapDiagnostic> validateCreatorCommandPlan(const CreatorCommandRequest& request,
                                                                  const CreatorCommandPlan& plan);
CreatorCommandApplyResult applyCreatorCommandPlan(const CreatorCommandRequest& request,
                                                  const CreatorCommandPlan& plan,
                                                  const nlohmann::json& projectData);

class CreatorCommandPlanner {
public:
    CreatorCommandPlan plan(const CreatorCommandRequest& request) const;
    urpg::map::TileLayerDocument previewDocument(const CreatorCommandRequest& request,
                                                 const CreatorCommandPlan& plan) const;

private:
    CreatorCommandPlan planFootprint(const CreatorCommandRequest& request,
                                     const std::string& intent,
                                     int32_t width,
                                     int32_t height,
                                     int32_t wall_tile,
                                     int32_t floor_tile,
                                     int32_t accent_tile,
                                     std::vector<CreatorPropEdit> props,
                                     std::vector<CreatorLogicEdit> logic) const;
    CreatorCommandPlan planHouse(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planShop(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planInn(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planDungeonRoom(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planNpc(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planQuestGiver(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planTreasureChest(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planLockedDoor(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planPuzzle(const CreatorCommandRequest& request) const;
    CreatorCommandPlan planFarmPlot(const CreatorCommandRequest& request) const;
    CreatorCommandPlan unsupportedIntent(const CreatorCommandRequest& request) const;
};

} // namespace urpg::ai
