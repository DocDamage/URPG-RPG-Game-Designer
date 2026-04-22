#include "editor/character/character_creator_panel.h"

namespace urpg::editor {

void CharacterCreatorPanel::bindModel(CharacterCreatorModel* model) {
    m_model = model;
    m_snapshot = nlohmann::json::object();
}

void CharacterCreatorPanel::render() {
    if (!m_model) {
        m_snapshot = nlohmann::json::object();
        return;
    }

    m_snapshot = m_model->buildSnapshot();
    const auto& identity = m_model->getIdentity();
    m_snapshot["identity_summary"] = {
        {"name", identity.getName()},
        {"display_name", identity.getDisplayName()},
        {"class_id", identity.getClassId()},
        {"portrait_id", identity.getPortraitId()},
        {"body_sprite_id", identity.getBodySpriteId()}
    };
    m_snapshot["appearance_token_count"] = identity.getAppearanceTokens().size();
    m_snapshot["preview_card"] = m_snapshot["preview"];
    m_snapshot["validation_summary"] = m_snapshot["validation"];
    m_snapshot["workflow_actions"] = m_snapshot["workflow"];
}

} // namespace urpg::editor
