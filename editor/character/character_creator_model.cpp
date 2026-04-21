#include "editor/character/character_creator_model.h"

namespace urpg::editor {

void CharacterCreatorModel::loadIdentity(const urpg::character::CharacterIdentity& identity) {
    m_identity = identity;
    m_dirty = false;
}

void CharacterCreatorModel::setName(const std::string& value) {
    m_identity.setName(value);
    m_dirty = true;
}

void CharacterCreatorModel::setPortraitId(const std::string& value) {
    m_identity.setPortraitId(value);
    m_dirty = true;
}

void CharacterCreatorModel::setBodySpriteId(const std::string& value) {
    m_identity.setBodySpriteId(value);
    m_dirty = true;
}

void CharacterCreatorModel::setClassId(const std::string& value) {
    m_identity.setClassId(value);
    m_dirty = true;
}

void CharacterCreatorModel::setBaseAttribute(const std::string& key, float value) {
    m_identity.setAttribute(key, value);
    m_dirty = true;
}

void CharacterCreatorModel::addAppearanceToken(const std::string& token) {
    m_identity.addAppearanceToken(token);
    m_dirty = true;
}

void CharacterCreatorModel::removeAppearanceToken(const std::string& token) {
    m_identity.removeAppearanceToken(token);
    m_dirty = true;
}

nlohmann::json CharacterCreatorModel::buildSnapshot() const {
    nlohmann::json snapshot;
    snapshot["identity"] = m_identity.toJson();
    snapshot["is_dirty"] = m_dirty;
    return snapshot;
}

} // namespace urpg::editor
