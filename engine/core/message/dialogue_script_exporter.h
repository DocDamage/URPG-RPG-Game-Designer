#pragma once

#include "engine/core/message/message_core.h"

#include <string>
#include <vector>

namespace urpg::message {

[[nodiscard]] std::string exportDialogueScript(const std::vector<DialoguePage>& pages);

} // namespace urpg::message
