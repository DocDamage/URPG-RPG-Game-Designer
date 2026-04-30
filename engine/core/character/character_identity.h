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

    void setPortraitAssetId(const std::string& value) { m_portraitAssetId = value; }
    const std::string& getPortraitAssetId() const { return m_portraitAssetId; }

    void setFieldSpriteAssetId(const std::string& value) { m_fieldSpriteAssetId = value; }
    const std::string& getFieldSpriteAssetId() const { return m_fieldSpriteAssetId; }

    void setBattleSpriteAssetId(const std::string& value) { m_battleSpriteAssetId = value; }
    const std::string& getBattleSpriteAssetId() const { return m_battleSpriteAssetId; }

    void addLayeredPartAssetId(const std::string& value);
    void setLayeredPartAssetIds(const std::vector<std::string>& value) { m_layeredPartAssetIds = value; }
    const std::vector<std::string>& getLayeredPartAssetIds() const { return m_layeredPartAssetIds; }

    void setClassId(const std::string& value) { m_classId = value; }
    const std::string& getClassId() const { return m_classId; }

    void setSpeciesId(const std::string& value) { m_speciesId = value; }
    const std::string& getSpeciesId() const { return m_speciesId; }

    void setOriginId(const std::string& value) { m_originId = value; }
    const std::string& getOriginId() const { return m_originId; }

    void setBackgroundId(const std::string& value) { m_backgroundId = value; }
    const std::string& getBackgroundId() const { return m_backgroundId; }

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
    std::string m_portraitAssetId;
    std::string m_fieldSpriteAssetId;
    std::string m_battleSpriteAssetId;
    std::string m_classId;
    std::string m_speciesId;
    std::string m_originId;
    std::string m_backgroundId;
    std::unordered_map<std::string, float> m_baseAttributes;
    std::vector<std::string> m_appearanceTokens;
    std::vector<std::string> m_layeredPartAssetIds;
};

} // namespace urpg::character
