#pragma once

#include "engine/core/message/chatbot_component.h"

namespace urpg::ai {

/**
 * @brief Connectivity boundary for out-of-tree AI providers.
 *
 * The in-tree runtime exposes the provider-agnostic `IChatService` interface in
 * `chatbot_component.h` and the deterministic `MockChatService` test double in
 * `engine/core/message/mock_chat_service.h`.
 *
 * No live OpenAI, Anthropic, llama.cpp, or other transport/inference provider
 * is implemented in-tree. Concrete production providers should be supplied by
 * out-of-tree integrations that implement `IChatService`.
 */

} // namespace urpg::ai
