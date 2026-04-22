#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::character {

/**
 * @brief Represents the persistent identity of a playable or NPC character.
 */
class CharacterIdentity {
public:
    CharacterIdentity() = default;

    /** @brief Serializes identity to JSON with schemaVersion. */
    nlohmann::json toJson() const;

    /** @brief Deserializes identity from JSON and validates schemaVersion. */
    static CharacterIdentity fromJson(const nlohmann::json& j);

    /** @brief Returns the display name (defaults to "Unnamed" if empty). */
    std::string getDisplayName() const;

    void setName(const std::string& value) { m_name = value; }
    const std::string& getName() const { return m_name; }

    void setPortraitId(const std::string& value) { m_portraitId = value; }
    const std::string& getPortraitId() const { return m_portraitId; }

    void setBodySpriteId(const std::string& value) { m_bodySpriteId = value; }
    const std::string& getBodySpriteId() const { return m_bodySpriteId; }

    void setClassId(const std::string& value) { m_classId = value; }
    const std::string& getClassId() const { return m_classId; }

    /** @brief Sets a base attribute value. */
    void setAttribute(const std::string& key, float value);

    /** @brief Gets a base attribute value, or 0.0f if missing. */
    float getAttribute(const std::string& key) const;

    void setBaseAttributes(const std::unordered_map<std::string, float>& value) { m_baseAttributes = value; }
    const std::unordered_map<std::string, float>& getBaseAttributes() const { return m_baseAttributes; }

    /** @brief Adds an appearance token. */
    void addAppearanceToken(const std::string& token);

    /** @brief Removes an appearance token. */
    void removeAppearanceToken(const std::string& token);

    void setAppearanceTokens(const std::vector<std::string>& value) { m_appearanceTokens = value; }
    const std::vector<std::string>& getAppearanceTokens() const { return m_appearanceTokens; }

private:
    std::string m_name;
    std::string m_portraitId;
    std::string m_bodySpriteId;
    std::string m_classId;
    std::unordered_map<std::string, float> m_baseAttributes;
    std::vector<std::string> m_appearanceTokens;
};

} // namespace urpg::character
