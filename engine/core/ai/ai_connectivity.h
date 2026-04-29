#pragma once

#include "engine/core/message/chatbot_component.h"

namespace urpg::ai {

/**
 * @brief Connectivity boundary for out-of-tree AI providers.
 *
 * The in-tree runtime exposes the provider-agnostic `IChatService` interface in
 * `chatbot_component.h`, the deterministic `MockChatService` test double in
 * `engine/core/message/mock_chat_service.h`, and a bounded curl-backed
 * OpenAI-compatible chat transport in `openai_compatible_chat_service.h`.
 *
 * The OpenAI-compatible service targets ChatGPT/OpenAI-compatible endpoints,
 * Kimi-compatible gateways, Ollama, LM Studio, OpenRouter, vLLM, LocalAI, and
 * similar `/v1/chat/completions` servers when configured by the host. Provider
 * secrets are runtime configuration, not hardcoded engine state. Non-compatible
 * production providers should still be supplied by out-of-tree integrations that
 * implement `IChatService`.
 */

} // namespace urpg::ai
