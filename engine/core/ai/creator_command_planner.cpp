#include "engine/core/ai/creator_command_planner.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>

namespace {

std::string lowerCopy(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return value;
}

nlohmann::json diagnosticsToJson(const std::vector<urpg::map::MapDiagnostic>& diagnostics) {
    nlohmann::json out = nlohmann::json::array();
    for (const auto& diagnostic : diagnostics) {
        out.push_back({
            {"code", diagnostic.code},
            {"message", diagnostic.message},
            {"x", diagnostic.x},
            {"y", diagnostic.y},
            {"target", diagnostic.target},
        });
    }
    return out;
}

std::vector<urpg::map::MapDiagnostic> diagnosticsFromJson(const nlohmann::json& json) {
    std::vector<urpg::map::MapDiagnostic> out;
    for (const auto& item : json) {
        out.push_back({
            item.value("code", ""),
            item.value("message", ""),
            item.value("x", -1),
            item.value("y", -1),
            item.value("target", ""),
        });
    }
    return out;
}

std::string firstStringContent(const nlohmann::json& item) {
    if (item.is_string()) {
        return item.get<std::string>();
    }
    if (item.is_object()) {
        if (item.contains("text") && item["text"].is_string()) {
            return item["text"].get<std::string>();
        }
        if (item.contains("parts")) {
            return firstStringContent(item["parts"]);
        }
        if (item.contains("content")) {
            return firstStringContent(item["content"]);
        }
    }
    if (item.is_array()) {
        for (const auto& child : item) {
            const auto value = firstStringContent(child);
            if (!value.empty()) {
                return value;
            }
        }
    }
    return {};
}

std::optional<std::string> extractJsonObjectText(const std::string& text) {
    const auto start = text.find('{');
    if (start == std::string::npos) {
        return std::nullopt;
    }
    int32_t depth = 0;
    bool inString = false;
    bool escaped = false;
    for (size_t i = start; i < text.size(); ++i) {
        const char ch = text[i];
        if (escaped) {
            escaped = false;
            continue;
        }
        if (ch == '\\' && inString) {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            inString = !inString;
            continue;
        }
        if (inString) {
            continue;
        }
        if (ch == '{') {
            ++depth;
        } else if (ch == '}') {
            --depth;
            if (depth == 0) {
                return text.substr(start, i - start + 1);
            }
        }
    }
    return std::nullopt;
}

nlohmann::json parseJsonFromText(const std::string& text) {
    const auto objectText = extractJsonObjectText(text);
    if (!objectText.has_value()) {
        return {};
    }
    return nlohmann::json::parse(*objectText, nullptr, false);
}

urpg::map::MapDiagnostic makeDiagnostic(const std::string& code,
                                        const std::string& message,
                                        int32_t x,
                                        int32_t y,
                                        const std::string& target) {
    return {code, message, x, y, target};
}

bool hasLogicKind(const urpg::ai::CreatorCommandPlan& plan, const std::string& kind) {
    return std::any_of(plan.logic_edits.begin(), plan.logic_edits.end(), [&](const auto& edit) {
        return edit.kind == kind;
    });
}

bool hasAnyLogicKind(const urpg::ai::CreatorCommandPlan& plan, std::initializer_list<const char*> kinds) {
    return std::any_of(kinds.begin(), kinds.end(), [&](const char* kind) {
        return hasLogicKind(plan, kind);
    });
}

std::string quoteCommandArg(const std::string& value) {
    std::string out = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            out += "\\\"";
        } else {
            out += ch;
        }
    }
    out += "\"";
    return out;
}

std::string authHeader(const urpg::ai::CreatorAiProviderProfile& profile, const std::string& apiKey) {
    if (profile.auth_scheme.empty() || apiKey.empty()) {
        return {};
    }
    if (profile.auth_scheme == "bearer") {
        return "Authorization: Bearer " + apiKey;
    }
    if (profile.auth_scheme == "x-goog-api-key") {
        return "x-goog-api-key: " + apiKey;
    }
    if (profile.auth_scheme == "x-api-key") {
        return "x-api-key: " + apiKey;
    }
    if (profile.auth_scheme == "api-key") {
        return "api-key: " + apiKey;
    }
    return {};
}

} // namespace

namespace urpg::ai {

std::vector<CreatorAiProviderProfile> defaultCreatorAiProviderProfiles() {
    return {
        {CreatorAiProvider::LocalDeterministic, "local_deterministic", "Local deterministic planner", "", "",
         "urpg.creator_command_plan.v1", "", true, false},
        {CreatorAiProvider::ChatGpt, "chatgpt", "ChatGPT / OpenAI", "https://api.openai.com/v1/responses",
         "OPENAI_API_KEY", "openai_responses_json_schema", "bearer", false, true},
        {CreatorAiProvider::Gemini, "gemini", "Google Gemini",
         "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-pro:generateContent", "GEMINI_API_KEY",
         "gemini_contents_json_schema", "x-goog-api-key", false, true},
        {CreatorAiProvider::Kimi, "kimi", "Kimi / Moonshot AI", "https://api.moonshot.ai/v1/chat/completions",
         "KIMI_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::AnthropicClaude, "anthropic", "Anthropic Claude", "https://api.anthropic.com/v1/messages",
         "ANTHROPIC_API_KEY", "anthropic_messages_json_schema", "x-api-key", false, true},
        {CreatorAiProvider::Mistral, "mistral", "Mistral AI", "https://api.mistral.ai/v1/chat/completions",
         "MISTRAL_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::Cohere, "cohere", "Cohere", "https://api.cohere.com/v2/chat",
         "COHERE_API_KEY", "cohere_chat_json_schema", "bearer", false, true},
        {CreatorAiProvider::Groq, "groq", "Groq", "https://api.groq.com/openai/v1/chat/completions",
         "GROQ_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::Perplexity, "perplexity", "Perplexity", "https://api.perplexity.ai/chat/completions",
         "PERPLEXITY_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::XAiGrok, "xai", "xAI Grok", "https://api.x.ai/v1/chat/completions",
         "XAI_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::DeepSeek, "deepseek", "DeepSeek", "https://api.deepseek.com/chat/completions",
         "DEEPSEEK_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::TogetherAi, "together", "Together AI", "https://api.together.xyz/v1/chat/completions",
         "TOGETHER_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::OpenRouter, "openrouter", "OpenRouter", "https://openrouter.ai/api/v1/chat/completions",
         "OPENROUTER_API_KEY", "openai_chat_completions_json_schema", "bearer", false, true},
        {CreatorAiProvider::AzureOpenAi, "azure_openai", "Azure OpenAI", "https://{resource}.openai.azure.com/openai/deployments/{deployment}/chat/completions?api-version=2024-02-15-preview",
         "AZURE_OPENAI_API_KEY", "openai_chat_completions_json_schema", "api-key", false, true},
        {CreatorAiProvider::AwsBedrock, "aws_bedrock", "AWS Bedrock", "https://bedrock-runtime.{region}.amazonaws.com/model/{modelId}/invoke",
         "AWS_BEDROCK_BEARER_TOKEN", "bedrock_invoke_json_schema", "bearer", false, true},
        {CreatorAiProvider::Ollama, "ollama", "Ollama local", "http://127.0.0.1:11434/v1/chat/completions",
         "", "openai_chat_completions_json_schema", "", true, true},
        {CreatorAiProvider::LmStudio, "lmstudio", "LM Studio local", "http://127.0.0.1:1234/v1/chat/completions",
         "", "openai_chat_completions_json_schema", "", true, true},
        {CreatorAiProvider::LlamaCpp, "llamacpp", "llama.cpp server", "http://127.0.0.1:8080/v1/chat/completions",
         "", "openai_chat_completions_json_schema", "", true, true},
        {CreatorAiProvider::Vllm, "vllm", "vLLM OpenAI-compatible server", "http://127.0.0.1:8000/v1/chat/completions",
         "", "openai_chat_completions_json_schema", "", true, true},
        {CreatorAiProvider::TextGenerationWebUi, "text_generation_webui", "text-generation-webui local", "http://127.0.0.1:5000/v1/chat/completions",
         "", "openai_chat_completions_json_schema", "", true, true},
        {CreatorAiProvider::LocalAi, "localai", "LocalAI OpenAI-compatible server", "http://127.0.0.1:8080/v1/chat/completions",
         "", "openai_chat_completions_json_schema", "", true, true},
    };
}

CreatorAiProviderProfile creatorAiProviderProfile(CreatorAiProvider provider) {
    const auto profiles = defaultCreatorAiProviderProfiles();
    const auto it = std::find_if(profiles.begin(), profiles.end(), [&](const CreatorAiProviderProfile& profile) {
        return profile.provider == provider;
    });
    return it == profiles.end() ? profiles.front() : *it;
}

CreatorAiProviderProfile creatorAiProviderProfileById(const std::string& providerId) {
    const auto profiles = defaultCreatorAiProviderProfiles();
    const auto it = std::find_if(profiles.begin(), profiles.end(), [&](const CreatorAiProviderProfile& profile) {
        return profile.id == providerId;
    });
    return it == profiles.end() ? profiles.front() : *it;
}

nlohmann::json buildCreatorProviderRequest(const CreatorCommandRequest& request) {
    const auto profile = creatorAiProviderProfile(request.provider);
    const nlohmann::json contract = {
        {"schema", "urpg.creator_command_plan.v1"},
        {"required_intents", {
            "make_house",
            "make_shop",
            "make_inn",
            "make_dungeon_room",
            "make_npc",
            "make_quest_giver",
            "make_treasure_chest",
            "make_locked_door",
            "make_puzzle",
            "make_farm_plot",
        }},
        {"required_outputs", {
            "terrain_tiles",
            "collision_tiles",
            "decor_tiles",
            "prop_edits",
            "runtime_logic_edits",
            "transfer_player_logic",
            "inspect_message_logic",
            "shop_open_logic",
            "party_recovery_logic",
            "quest_logic",
            "item_reward_logic",
            "locked_door_logic",
            "switch_or_variable_logic",
            "farm_crop_logic",
            "diagnostics",
        }},
        {"rules", {
            "Return JSON only.",
            "Do not write files directly.",
            "All tile edits must stay inside map bounds.",
            "Every generated building must include collision, a passable door, and reviewable logic edits.",
            "Every generated plan must be safe to preview before the editor applies it.",
        }},
    };
    const nlohmann::json context = {
        {"project_id", request.project_id},
        {"map_id", request.map_id},
        {"selected_tile", {{"x", request.tile_x}, {"y", request.tile_y}, {"tile_id", request.selected_tile_id}}},
        {"map_size", {{"width", request.width}, {"height", request.height}}},
        {"prompt", request.prompt},
        {"contract", contract},
    };

    if (profile.provider == CreatorAiProvider::ChatGpt) {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"model", "project-configured"},
                {"input", {
                    {{"role", "system"}, {"content", "You are URPG Creator Command Planner. Return a strict creator command plan JSON object."}},
                    {{"role", "user"}, {"content", context.dump()}},
                }},
            }},
        };
    }
    if (profile.provider == CreatorAiProvider::Gemini) {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"contents", {{
                    {"role", "user"},
                    {"parts", {{{"text", "Return strict URPG creator command plan JSON for this request: " + context.dump()}}}},
                }}},
            }},
        };
    }
    if (profile.provider == CreatorAiProvider::Kimi) {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"model", profile.local_provider ? "local-model" : "project-configured"},
                {"messages", {
                    {{"role", "system"}, {"content", "Return JSON only using urpg.creator_command_plan.v1."}},
                    {{"role", "user"}, {"content", context.dump()}},
                }},
            }},
        };
    }
    if (profile.request_format == "anthropic_messages_json_schema") {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"model", "project-configured"},
                {"max_tokens", 2048},
                {"system", "Return JSON only using urpg.creator_command_plan.v1."},
                {"messages", {{{"role", "user"}, {"content", context.dump()}}}},
            }},
        };
    }
    if (profile.request_format == "cohere_chat_json_schema") {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"model", "project-configured"},
                {"messages", {
                    {{"role", "system"}, {"content", "Return JSON only using urpg.creator_command_plan.v1."}},
                    {{"role", "user"}, {"content", context.dump()}},
                }},
            }},
        };
    }
    if (profile.request_format == "bedrock_invoke_json_schema") {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"anthropic_version", "bedrock-2023-05-31"},
                {"max_tokens", 2048},
                {"system", "Return JSON only using urpg.creator_command_plan.v1."},
                {"messages", {{{"role", "user"}, {"content", context.dump()}}}},
            }},
        };
    }
    if (profile.request_format == "openai_chat_completions_json_schema") {
        return {
            {"provider", profile.id},
            {"endpoint", profile.endpoint},
            {"api_key_env", profile.api_key_env},
            {"auth_scheme", profile.auth_scheme},
            {"body", {
                {"model", profile.local_provider ? "local-model" : "project-configured"},
                {"messages", {
                    {{"role", "system"}, {"content", "Return JSON only using urpg.creator_command_plan.v1."}},
                    {{"role", "user"}, {"content", context.dump()}},
                }},
            }},
        };
    }
    return {
        {"provider", profile.id},
        {"endpoint", profile.endpoint},
        {"api_key_env", profile.api_key_env},
        {"auth_scheme", profile.auth_scheme},
        {"body", context},
    };
}

nlohmann::json CreatorProviderTransportResult::toJson() const {
    return {
        {"attempted", attempted},
        {"success", success},
        {"exit_code", exit_code},
        {"command", command},
        {"response_path", response_path},
        {"message", message},
    };
}

std::string buildCreatorProviderCurlCommand(const CreatorCommandRequest& request,
                                            const CreatorProviderTransportConfig& config) {
    const auto profile = creatorAiProviderProfile(request.provider);
    const auto payload = buildCreatorProviderRequest(request);
    const std::string requestPath = config.request_path.empty() ? "creator_command_request.json" : config.request_path;
    const std::string responsePath = config.response_path.empty() ? "creator_command_response.json" : config.response_path;

    std::ostringstream command;
    command << quoteCommandArg(config.curl_executable.empty() ? "curl" : config.curl_executable)
            << " --fail --silent --show-error"
            << " --max-time " << std::max(1, config.timeout_seconds)
            << " -X POST"
            << " -H " << quoteCommandArg("Content-Type: application/json");
    const auto header = authHeader(profile, config.api_key);
    if (!header.empty()) {
        command << " -H " << quoteCommandArg(header);
    }
    command << " --data-binary @" << quoteCommandArg(requestPath)
            << " -o " << quoteCommandArg(responsePath)
            << " " << quoteCommandArg(payload.value("endpoint", profile.endpoint));
    return command.str();
}

CreatorProviderTransportResult invokeCreatorProvider(const CreatorCommandRequest& request,
                                                     const CreatorProviderTransportConfig& config) {
    CreatorProviderTransportResult result;
    const auto payload = buildCreatorProviderRequest(request);
    const std::string requestPath = config.request_path.empty() ? "creator_command_request.json" : config.request_path;
    result.response_path = config.response_path.empty() ? "creator_command_response.json" : config.response_path;
    result.command = buildCreatorProviderCurlCommand(request, config);

    if (!config.execute) {
        result.message = "dry_run";
        return result;
    }

    std::ofstream requestFile(requestPath);
    if (!requestFile.is_open()) {
        result.message = "request_file_open_failed";
        return result;
    }
    requestFile << payload["body"].dump();
    requestFile.close();

    result.attempted = true;
    result.exit_code = std::system(result.command.c_str());
    result.success = result.exit_code == 0 && std::filesystem::exists(result.response_path);
    result.message = result.success ? "provider_response_written" : "provider_command_failed";
    return result;
}

nlohmann::json CreatorCommandPlan::toJson() const {
    nlohmann::json tiles = nlohmann::json::array();
    for (const auto& edit : tile_edits) {
        tiles.push_back({{"layer_id", edit.layer_id}, {"x", edit.x}, {"y", edit.y}, {"tile_id", edit.tile_id}});
    }

    nlohmann::json props = nlohmann::json::array();
    for (const auto& edit : prop_edits) {
        props.push_back({{"id", edit.id}, {"asset_id", edit.asset_id}, {"tile_x", edit.tile_x}, {"tile_y", edit.tile_y}});
    }

    nlohmann::json logic = nlohmann::json::array();
    for (const auto& edit : logic_edits) {
        logic.push_back({
            {"id", edit.id},
            {"kind", edit.kind},
            {"trigger", edit.trigger},
            {"tile_x", edit.tile_x},
            {"tile_y", edit.tile_y},
            {"payload", edit.payload},
        });
    }

    return {
        {"schema", schema},
        {"intent", intent},
        {"provider_id", provider_id},
        {"provider_network_required", provider_network_required},
        {"deterministic_fallback_used", deterministic_fallback_used},
        {"can_apply", can_apply},
        {"tile_edits", tiles},
        {"prop_edits", props},
        {"logic_edits", logic},
        {"diagnostics", diagnosticsToJson(diagnostics)},
    };
}

nlohmann::json CreatorCommandApplyResult::toJson() const {
    return {
        {"applied", applied},
        {"project_data", project_data},
        {"diagnostics", diagnosticsToJson(diagnostics)},
    };
}

CreatorCommandPlan parseCreatorCommandPlan(const nlohmann::json& json) {
    CreatorCommandPlan plan;
    plan.schema = json.value("schema", "urpg.creator_command_plan.v1");
    plan.intent = json.value("intent", "");
    plan.provider_id = json.value("provider_id", "");
    plan.provider_network_required = json.value("provider_network_required", false);
    plan.deterministic_fallback_used = json.value("deterministic_fallback_used", false);
    plan.can_apply = json.value("can_apply", false);
    for (const auto& item : json.value("tile_edits", nlohmann::json::array())) {
        plan.tile_edits.push_back({
            item.value("layer_id", item.value("layer", "")),
            item.value("x", 0),
            item.value("y", 0),
            item.value("tile_id", 0),
        });
    }
    for (const auto& item : json.value("prop_edits", nlohmann::json::array())) {
        plan.prop_edits.push_back({
            item.value("id", ""),
            item.value("asset_id", ""),
            item.value("tile_x", 0),
            item.value("tile_y", 0),
        });
    }
    for (const auto& item : json.value("logic_edits", nlohmann::json::array())) {
        plan.logic_edits.push_back({
            item.value("id", ""),
            item.value("kind", ""),
            item.value("trigger", ""),
            item.value("tile_x", 0),
            item.value("tile_y", 0),
            item.value("payload", nlohmann::json::object()),
        });
    }
    plan.diagnostics = diagnosticsFromJson(json.value("diagnostics", nlohmann::json::array()));
    return plan;
}

nlohmann::json extractCreatorPlanJsonFromProviderResponse(const nlohmann::json& response) {
    if (response.value("schema", "") == "urpg.creator_command_plan.v1") {
        return response;
    }
    auto failed = [](const std::string& code, const std::string& message) {
        return nlohmann::json{
            {"schema", "urpg.creator_command_plan.v1"},
            {"intent", "provider_parse_failed"},
            {"provider_id", ""},
            {"provider_network_required", true},
            {"deterministic_fallback_used", false},
            {"can_apply", false},
            {"tile_edits", nlohmann::json::array()},
            {"prop_edits", nlohmann::json::array()},
            {"logic_edits", nlohmann::json::array()},
            {"diagnostics", {{{"code", code}, {"message", message}, {"x", -1}, {"y", -1}, {"target", "provider_response"}}}},
        };
    };
    auto parseCandidate = [&](const std::string& text) -> nlohmann::json {
        const auto parsed = parseJsonFromText(text);
        if (parsed.is_object()) {
            return parsed;
        }
        return failed("creator_provider_json_malformed", "Provider response did not contain a parseable JSON object.");
    };
    if (response.contains("output_text") && response["output_text"].is_string()) {
        return parseCandidate(response["output_text"].get<std::string>());
    }
    if (response.contains("output") && response["output"].is_array()) {
        for (const auto& output : response["output"]) {
            const auto text = firstStringContent(output);
            if (!text.empty()) {
                return parseCandidate(text);
            }
        }
    }
    if (response.contains("choices") && response["choices"].is_array() && !response["choices"].empty()) {
        const auto& choice = response["choices"].front();
        if (choice.contains("message")) {
            return parseCandidate(firstStringContent(choice["message"]));
        }
        if (choice.contains("text")) {
            return parseCandidate(firstStringContent(choice["text"]));
        }
    }
    if (response.contains("candidates") && response["candidates"].is_array() && !response["candidates"].empty()) {
        return parseCandidate(firstStringContent(response["candidates"].front()));
    }
    if (response.contains("message")) {
        return parseCandidate(firstStringContent(response["message"]));
    }
    if (response.contains("content")) {
        return parseCandidate(firstStringContent(response["content"]));
    }
    return failed("creator_provider_response_unsupported", "Provider response shape is not supported by the creator command importer.");
}

std::vector<urpg::map::MapDiagnostic> validateCreatorCommandPlan(const CreatorCommandRequest& request,
                                                                 const CreatorCommandPlan& plan) {
    std::vector<urpg::map::MapDiagnostic> diagnostics = plan.diagnostics;
    if (plan.schema != "urpg.creator_command_plan.v1") {
        diagnostics.push_back(makeDiagnostic("creator_schema_invalid", "Creator command plan has an unsupported schema.", -1, -1, plan.intent));
    }
    if (!plan.can_apply) {
        diagnostics.push_back(makeDiagnostic("creator_plan_not_apply_ready", "Creator command plan is not marked apply-ready.", -1, -1, plan.intent));
    }
    if (plan.tile_edits.empty() && plan.prop_edits.empty() && plan.logic_edits.empty()) {
        diagnostics.push_back(makeDiagnostic("creator_plan_empty", "Creator command plan contains no edits.", -1, -1, plan.intent));
    }
    for (const auto& edit : plan.tile_edits) {
        if (edit.layer_id.empty()) {
            diagnostics.push_back(makeDiagnostic("creator_tile_layer_missing", "Tile edit is missing a layer id.", edit.x, edit.y, plan.intent));
        }
        if (edit.x < 0 || edit.y < 0 || edit.x >= request.width || edit.y >= request.height) {
            diagnostics.push_back(makeDiagnostic("creator_tile_out_of_bounds", "Tile edit is outside map bounds.", edit.x, edit.y, plan.intent));
        }
        if (edit.tile_id < 0) {
            diagnostics.push_back(makeDiagnostic("creator_tile_id_invalid", "Tile edit cannot use a negative tile id.", edit.x, edit.y, plan.intent));
        }
    }
    for (const auto& edit : plan.prop_edits) {
        if (edit.id.empty() || edit.asset_id.empty()) {
            diagnostics.push_back(makeDiagnostic("creator_prop_incomplete", "Prop edit must have id and asset id.", edit.tile_x, edit.tile_y, plan.intent));
        }
        if (edit.tile_x < 0 || edit.tile_y < 0 || edit.tile_x >= request.width || edit.tile_y >= request.height) {
            diagnostics.push_back(makeDiagnostic("creator_prop_out_of_bounds", "Prop edit is outside map bounds.", edit.tile_x, edit.tile_y, plan.intent));
        }
    }
    for (const auto& edit : plan.logic_edits) {
        if (edit.id.empty() || edit.kind.empty() || edit.trigger.empty()) {
            diagnostics.push_back(makeDiagnostic("creator_logic_incomplete", "Logic edit must have id, kind, and trigger.", edit.tile_x, edit.tile_y, plan.intent));
        }
        if (edit.tile_x < 0 || edit.tile_y < 0 || edit.tile_x >= request.width || edit.tile_y >= request.height) {
            diagnostics.push_back(makeDiagnostic("creator_logic_out_of_bounds", "Logic edit is outside map bounds.", edit.tile_x, edit.tile_y, plan.intent));
        }
    }
    if (plan.intent == "make_house" && (!hasLogicKind(plan, "transfer_player") || !hasLogicKind(plan, "message") || !hasLogicKind(plan, "set_variable"))) {
        diagnostics.push_back(makeDiagnostic("creator_house_logic_missing", "House plans require transfer, message, and save-variable logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_shop" && (!hasLogicKind(plan, "transfer_player") || !hasLogicKind(plan, "message") || !hasLogicKind(plan, "shop_open"))) {
        diagnostics.push_back(makeDiagnostic("creator_shop_logic_missing", "Shop plans require transfer, message, and shop-open logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_inn" && (!hasLogicKind(plan, "transfer_player") || !hasLogicKind(plan, "message") || !hasLogicKind(plan, "recover_party"))) {
        diagnostics.push_back(makeDiagnostic("creator_inn_logic_missing", "Inn plans require transfer, message, and party-recovery logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_quest_giver" && (!hasLogicKind(plan, "message") || !hasAnyLogicKind(plan, {"start_quest", "set_variable"}))) {
        diagnostics.push_back(makeDiagnostic("creator_quest_logic_missing", "Quest giver plans require dialogue and quest-state logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_treasure_chest" && (!hasLogicKind(plan, "grant_item") || !hasLogicKind(plan, "set_variable"))) {
        diagnostics.push_back(makeDiagnostic("creator_chest_logic_missing", "Treasure chest plans require item grant and opened-state logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_locked_door" && !hasAnyLogicKind(plan, {"conditional_transfer", "locked_door"})) {
        diagnostics.push_back(makeDiagnostic("creator_locked_door_logic_missing", "Locked door plans require conditional transfer or locked-door logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_puzzle" && !hasAnyLogicKind(plan, {"set_switch", "set_variable"})) {
        diagnostics.push_back(makeDiagnostic("creator_puzzle_logic_missing", "Puzzle plans require switch or variable logic.", request.tile_x, request.tile_y, plan.intent));
    } else if (plan.intent == "make_farm_plot" && !hasAnyLogicKind(plan, {"plant_crop", "set_variable"})) {
        diagnostics.push_back(makeDiagnostic("creator_farm_logic_missing", "Farm plots require crop planting or state logic.", request.tile_x, request.tile_y, plan.intent));
    }
    return diagnostics;
}

CreatorCommandApplyResult applyCreatorCommandPlan(const CreatorCommandRequest& request,
                                                 const CreatorCommandPlan& plan,
                                                 const nlohmann::json& projectData) {
    CreatorCommandApplyResult result;
    result.project_data = projectData.is_object() ? projectData : nlohmann::json::object();
    result.diagnostics = validateCreatorCommandPlan(request, plan);
    if (!result.diagnostics.empty()) {
        return result;
    }
    result.project_data["schema"] = result.project_data.value("schema", "urpg.project.creator_command.v1");
    auto& maps = result.project_data["maps"];
    if (!maps.is_object()) {
        maps = nlohmann::json::object();
    }
    auto& map = maps[request.map_id.empty() ? "default_map" : request.map_id];
    map["width"] = request.width;
    map["height"] = request.height;
    auto& tileEdits = map["tile_edits"];
    auto& propEdits = map["prop_edits"];
    auto& logicEdits = map["logic_edits"];
    if (!tileEdits.is_array()) tileEdits = nlohmann::json::array();
    if (!propEdits.is_array()) propEdits = nlohmann::json::array();
    if (!logicEdits.is_array()) logicEdits = nlohmann::json::array();
    const auto planJson = plan.toJson();
    for (const auto& item : planJson["tile_edits"]) tileEdits.push_back(item);
    for (const auto& item : planJson["prop_edits"]) propEdits.push_back(item);
    for (const auto& item : planJson["logic_edits"]) logicEdits.push_back(item);
    auto& history = result.project_data["creator_command_history"];
    if (!history.is_array()) {
        history = nlohmann::json::array();
    }
    history.push_back({
        {"intent", plan.intent},
        {"provider_id", plan.provider_id},
        {"prompt", request.prompt},
        {"map_id", request.map_id},
        {"applied", true},
        {"tile_edit_count", plan.tile_edits.size()},
        {"prop_edit_count", plan.prop_edits.size()},
        {"logic_edit_count", plan.logic_edits.size()},
    });
    result.applied = true;
    return result;
}

CreatorCommandPlan CreatorCommandPlanner::plan(const CreatorCommandRequest& request) const {
    const auto prompt = lowerCopy(request.prompt);
    if (prompt.find("shop") != std::string::npos || prompt.find("store") != std::string::npos) return planShop(request);
    if (prompt.find("inn") != std::string::npos) return planInn(request);
    if (prompt.find("dungeon") != std::string::npos || prompt.find("room") != std::string::npos) return planDungeonRoom(request);
    if (prompt.find("quest giver") != std::string::npos) return planQuestGiver(request);
    if (prompt.find("npc") != std::string::npos || prompt.find("villager") != std::string::npos) return planNpc(request);
    if (prompt.find("treasure") != std::string::npos || prompt.find("chest") != std::string::npos) return planTreasureChest(request);
    if (prompt.find("locked door") != std::string::npos || prompt.find("lock") != std::string::npos) return planLockedDoor(request);
    if (prompt.find("puzzle") != std::string::npos) return planPuzzle(request);
    if (prompt.find("farm") != std::string::npos || prompt.find("crop") != std::string::npos) return planFarmPlot(request);
    if (prompt.find("house") != std::string::npos || prompt.find("home") != std::string::npos || prompt.find("building") != std::string::npos) {
        return planHouse(request);
    }
    return unsupportedIntent(request);
}

CreatorCommandPlan CreatorCommandPlanner::planFootprint(const CreatorCommandRequest& request,
                                                        const std::string& intent,
                                                        int32_t width,
                                                        int32_t height,
                                                        int32_t wall_tile,
                                                        int32_t floor_tile,
                                                        int32_t accent_tile,
                                                        std::vector<CreatorPropEdit> props,
                                                        std::vector<CreatorLogicEdit> logic) const {
    CreatorCommandPlan plan;
    const auto profile = creatorAiProviderProfile(request.provider);
    plan.intent = intent;
    plan.provider_id = profile.id;
    plan.provider_network_required = profile.network_required;
    plan.deterministic_fallback_used = true;
    const int32_t left = request.tile_x;
    const int32_t top = request.tile_y;
    if (left < 0 || top < 0 || left + width > request.width || top + height > request.height) {
        plan.diagnostics.push_back({"creator_plan_out_of_bounds", "Generated footprint does not fit on the selected tile.", left, top, request.map_id});
        return plan;
    }
    const int32_t doorX = left + width / 2;
    const int32_t doorY = top + height - 1;
    const bool building = intent == "make_house" || intent == "make_shop" || intent == "make_inn" ||
                          intent == "make_dungeon_room";
    for (int32_t y = top; y < top + height; ++y) {
        for (int32_t x = left; x < left + width; ++x) {
            const bool border = x == left || y == top || x == left + width - 1 || y == top + height - 1;
            const bool door = building && x == doorX && y == doorY;
            plan.tile_edits.push_back({"terrain", x, y, border ? (door ? 14 : wall_tile) : floor_tile});
            plan.tile_edits.push_back({"collision", x, y, border && !door ? 1 : 0});
        }
    }
    plan.tile_edits.push_back({"decor", left + width / 2, top + height / 2, accent_tile});
    for (auto prop : props) {
        prop.tile_x += left;
        prop.tile_y += top;
        plan.prop_edits.push_back(std::move(prop));
    }
    for (auto edit : logic) {
        edit.tile_x += left;
        edit.tile_y += top;
        plan.logic_edits.push_back(std::move(edit));
    }
    plan.can_apply = true;
    return plan;
}

urpg::map::TileLayerDocument CreatorCommandPlanner::previewDocument(const CreatorCommandRequest& request,
                                                                    const CreatorCommandPlan& plan) const {
    urpg::map::TileLayerDocument document(request.width, request.height);
    document.addLayer({"terrain", true, false, false, true, 0, {}});
    document.addLayer({"collision", true, false, true, false, 1, {}});
    document.addLayer({"decor", true, false, false, false, 2, {}});

    for (const auto& edit : plan.tile_edits) {
        document.setTile(edit.layer_id, edit.x, edit.y, edit.tile_id);
    }
    return document;
}

CreatorCommandPlan CreatorCommandPlanner::planHouse(const CreatorCommandRequest& request) const {
    CreatorCommandPlan plan;
    const auto profile = creatorAiProviderProfile(request.provider);
    plan.intent = "make_house";
    plan.provider_id = profile.id;
    plan.provider_network_required = profile.network_required;
    plan.deterministic_fallback_used = true;

    constexpr int32_t house_w = 5;
    constexpr int32_t house_h = 4;
    const int32_t left = request.tile_x;
    const int32_t top = request.tile_y;
    const int32_t door_x = left + house_w / 2;
    const int32_t door_y = top + house_h - 1;

    if (left < 0 || top < 0 || left + house_w > request.width || top + house_h > request.height) {
        plan.diagnostics.push_back({"creator_plan_out_of_bounds", "House footprint does not fit on the selected tile.", left, top, request.map_id});
        return plan;
    }

    for (int32_t y = top; y < top + house_h; ++y) {
        for (int32_t x = left; x < left + house_w; ++x) {
            const bool border = x == left || y == top || x == left + house_w - 1 || y == top + house_h - 1;
            const bool door = x == door_x && y == door_y;
            const int32_t terrain_tile = border ? (door ? 14 : 11) : request.selected_tile_id;
            plan.tile_edits.push_back({"terrain", x, y, terrain_tile});
            plan.tile_edits.push_back({"collision", x, y, border && !door ? 1 : 0});
        }
    }

    plan.tile_edits.push_back({"decor", left + 1, top + 1, 31});
    plan.tile_edits.push_back({"decor", left + 3, top + 1, 31});
    plan.prop_edits.push_back({"house_shell", "house_wooden_small", left + 2, top + 1});
    plan.prop_edits.push_back({"front_door", "door_wooden", door_x, door_y});

    plan.logic_edits.push_back({
        "enter_house",
        "transfer_player",
        "confirm_interact",
        door_x,
        door_y,
        {
            {"target_map", request.map_id + "_house_interior"},
            {"target_x", 2},
            {"target_y", 4},
            {"requires_facing", "north"},
        },
    });
    plan.logic_edits.push_back({
        "inspect_house_sign",
        "message",
        "inspect",
        left,
        door_y,
        {
            {"speaker", "System"},
            {"text", "A newly built house. The door leads inside."},
            {"localization_key", "house.inspect"},
        },
    });
    plan.logic_edits.push_back({
        "house_save_flag",
        "set_variable",
        "on_apply",
        door_x,
        door_y,
        {
            {"variable", "world.house_built"},
            {"value", true},
        },
    });

    plan.can_apply = true;
    return plan;
}

CreatorCommandPlan CreatorCommandPlanner::planShop(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_shop", 6, 4, 21, 2, 42,
                         {{"shop_counter", "shop_counter_basic", 3, 1}, {"merchant", "npc_merchant", 2, 2}},
                         {
                             {"enter_shop", "transfer_player", "confirm_interact", 3, 3, {{"target_map", request.map_id + "_shop_interior"}, {"target_x", 3}, {"target_y", 5}}},
                             {"shopkeeper_greeting", "message", "inspect", 2, 2, {{"speaker", "Merchant"}, {"text", "Welcome. Take a look around."}, {"localization_key", "shop.greeting"}}},
                             {"open_shop", "shop_open", "confirm_interact", 2, 2, {{"stock_table", "starter_general_store"}}},
                         });
}

CreatorCommandPlan CreatorCommandPlanner::planInn(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_inn", 6, 5, 22, 2, 43,
                         {{"inn_bed", "bed_single", 2, 2}, {"innkeeper", "npc_innkeeper", 3, 2}},
                         {
                             {"enter_inn", "transfer_player", "confirm_interact", 3, 4, {{"target_map", request.map_id + "_inn_interior"}, {"target_x", 3}, {"target_y", 5}}},
                             {"innkeeper_greeting", "message", "inspect", 3, 2, {{"speaker", "Innkeeper"}, {"text", "Rest here and recover your strength."}, {"localization_key", "inn.greeting"}}},
                             {"rest_at_inn", "recover_party", "confirm_interact", 3, 2, {{"cost", 25}, {"fade", true}}},
                         });
}

CreatorCommandPlan CreatorCommandPlanner::planDungeonRoom(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_dungeon_room", 7, 5, 51, 52, 53,
                         {{"enemy_spawn", "enemy_slime", 3, 2}},
                         {{"start_encounter", "battle", "on_enter", 3, 2, {{"encounter_id", "starter_dungeon_encounter"}}}});
}

CreatorCommandPlan CreatorCommandPlanner::planNpc(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_npc", 1, 1, request.selected_tile_id, request.selected_tile_id, 0,
                         {{"npc_villager", "npc_villager", 0, 0}},
                         {{"talk_to_npc", "message", "confirm_interact", 0, 0, {{"text", "Hello there."}, {"localization_key", "npc.hello"}}}});
}

CreatorCommandPlan CreatorCommandPlanner::planQuestGiver(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_quest_giver", 1, 1, request.selected_tile_id, request.selected_tile_id, 0,
                         {{"quest_giver", "npc_quest_giver", 0, 0}},
                         {
                             {"quest_intro", "message", "confirm_interact", 0, 0, {{"speaker", "Quest Giver"}, {"text", "Could you help me gather supplies?"}, {"localization_key", "quest.starter_intro"}}},
                             {"offer_quest", "start_quest", "confirm_interact", 0, 0, {{"quest_id", "starter_help_request"}}},
                             {"quest_state", "set_variable", "on_apply", 0, 0, {{"variable", "quest.starter.available"}, {"value", true}}},
                         });
}

CreatorCommandPlan CreatorCommandPlanner::planTreasureChest(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_treasure_chest", 1, 1, request.selected_tile_id, request.selected_tile_id, 0,
                         {{"treasure_chest", "chest_wooden", 0, 0}},
                         {
                             {"open_chest", "grant_item", "confirm_interact", 0, 0, {{"item_id", "potion"}, {"quantity", 3}, {"self_switch", "A"}}},
                             {"chest_opened", "set_variable", "after_interact", 0, 0, {{"variable", "chest.opened"}, {"value", true}}},
                         });
}

CreatorCommandPlan CreatorCommandPlanner::planLockedDoor(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_locked_door", 1, 1, 14, 14, 0,
                         {{"locked_door", "door_locked", 0, 0}},
                         {
                             {"unlock_door", "locked_door", "confirm_interact", 0, 0, {{"required_item", "bronze_key"}, {"failure_message", "The door is locked."}}},
                             {"enter_locked_room", "conditional_transfer", "after_unlock", 0, 0, {{"target_map", request.map_id + "_locked_room"}, {"target_x", 2}, {"target_y", 4}}},
                         });
}

CreatorCommandPlan CreatorCommandPlanner::planPuzzle(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_puzzle", 3, 3, request.selected_tile_id, request.selected_tile_id, 61,
                         {{"puzzle_switch", "floor_switch", 1, 1}},
                         {{"solve_puzzle", "set_switch", "confirm_interact", 1, 1, {{"switch", "puzzle_solved"}, {"value", true}}}});
}

CreatorCommandPlan CreatorCommandPlanner::planFarmPlot(const CreatorCommandRequest& request) const {
    return planFootprint(request, "make_farm_plot", 5, 4, 71, 72, 73,
                         {{"crop_marker", "crop_seedling", 2, 2}},
                         {
                             {"plant_crop", "plant_crop", "confirm_interact", 2, 2, {{"seed_item", "turnip_seed"}, {"growth_days", 3}}},
                             {"harvest_crop", "grant_item", "confirm_interact", 2, 2, {{"item_id", "turnip"}, {"quantity", 1}, {"regrow_days", 3}}},
                             {"farm_state", "set_variable", "on_apply", 2, 2, {{"variable", "farm.plot.active"}, {"value", true}}},
                         });
}

CreatorCommandPlan CreatorCommandPlanner::unsupportedIntent(const CreatorCommandRequest& request) const {
    CreatorCommandPlan plan;
    const auto profile = creatorAiProviderProfile(request.provider);
    plan.intent = "unsupported";
    plan.provider_id = profile.id;
    plan.provider_network_required = profile.network_required;
    plan.deterministic_fallback_used = true;
    plan.diagnostics.push_back({"creator_intent_unsupported",
                                "This deterministic planner currently supports house/building creation intents.",
                                request.tile_x,
                                request.tile_y,
                                request.map_id});
    return plan;
}

} // namespace urpg::ai
