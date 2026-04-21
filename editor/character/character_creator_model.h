#pragma once

#include "engine/core/character/character_identity.h"
#include <nlohmann/json.hpp>
#include <string>

namespace urpg::editor {

/**
 * @brief Editor model that owns a CharacterIdentity being edited.
 */
class CharacterCreatorModel {
public:
    void loadIdentity(const urpg::character::CharacterIdentity& identity);

    const urpg::character::CharacterIdentity& getIdentity() const { return m_identity; }

    void setName(const std::string& value);
    void setPortraitId(const std::string& value);
    void setBodySpriteId(const std::string& value);
    void setClassId(const std::string& value);
    void setBaseAttribute(const std::string& key, float value);
    void addAppearanceToken(const std::string& token);
    void removeAppearanceToken(const std::string& token);

    /** @brief Builds a snapshot JSON of the current identity plus dirty state. */
    nlohmann::json buildSnapshot() const;

private:
    urpg::character::CharacterIdentity m_identity;
    bool m_dirty = false;
};

} // namespace urpg::editor
