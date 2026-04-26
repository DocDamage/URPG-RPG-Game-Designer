#pragma once

#include "engine/core/character/character_identity.h"
#include "engine/core/ecs/world.h"

#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace urpg::character {

inline constexpr const char* kCreatedProtagonistSaveKey = "_created_protagonist";

struct CreatedProtagonistSaveState {
    EntityID entity = 0;
    CharacterIdentity identity;
    bool valid = false;
    std::vector<std::string> diagnostics;
};

nlohmann::json buildCreatedProtagonistSaveJson(EntityID entity, const CharacterIdentity& identity);

bool attachCreatedProtagonistToSaveDocument(nlohmann::json& document, EntityID entity,
                                            const CharacterIdentity& identity,
                                            std::vector<std::string>* diagnostics = nullptr);

std::optional<CreatedProtagonistSaveState>
loadCreatedProtagonistFromSaveDocument(const nlohmann::json& document, std::vector<std::string>* diagnostics = nullptr);

} // namespace urpg::character
