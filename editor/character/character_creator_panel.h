#pragma once

#include "editor/character/character_creator_model.h"
#include <nlohmann/json.hpp>

namespace urpg::editor {

/**
 * @brief Editor panel that renders a CharacterCreatorModel and builds a snapshot.
 */
class CharacterCreatorPanel {
public:
    void bindModel(CharacterCreatorModel* model);

    void render();

    const nlohmann::json& lastRenderSnapshot() const { return m_snapshot; }

private:
    CharacterCreatorModel* m_model = nullptr;
    nlohmann::json m_snapshot;
};

} // namespace urpg::editor
