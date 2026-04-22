#include "engine/core/character/character_identity.h"

#include <algorithm>
#include <stdexcept>

namespace urpg::character {

nlohmann::json CharacterIdentity::toJson() const {
    nlohmann::json j;
    j["schemaVersion"] = "1.0.0";
    j["name"] = m_name;
    j["portraitId"] = m_portraitId;
    j["bodySpriteId"] = m_bodySpriteId;
    j["classId"] = m_classId;
    j["baseAttributes"] = m_baseAttributes;
    j["appearanceTokens"] = m_appearanceTokens;
    return j;
}

CharacterIdentity CharacterIdentity::fromJson(const nlohmann::json& j) {
    if (!j.contains("schemaVersion")) {
        throw std::invalid_argument("CharacterIdentity JSON missing required field: schemaVersion");
    }
    if (j["schemaVersion"] != "1.0.0") {
        throw std::invalid_argument("CharacterIdentity JSON has unsupported schemaVersion");
    }

    CharacterIdentity identity;
    identity.m_name = j.value("name", "");
    identity.m_portraitId = j.value("portraitId", "");
    identity.m_bodySpriteId = j.value("bodySpriteId", "");
    identity.m_classId = j.value("classId", "");

    if (j.contains("baseAttributes") && j["baseAttributes"].is_object()) {
        for (const auto& [key, val] : j["baseAttributes"].items()) {
            identity.m_baseAttributes[key] = val.get<float>();
        }
    }

    if (j.contains("appearanceTokens") && j["appearanceTokens"].is_array()) {
        identity.m_appearanceTokens = j["appearanceTokens"].get<std::vector<std::string>>();
    }

    return identity;
}

std::string CharacterIdentity::getDisplayName() const {
    if (!m_name.empty()) {
        return m_name;
    }
    return "Unnamed";
}

void CharacterIdentity::setAttribute(const std::string& key, float value) {
    m_baseAttributes[key] = value;
}

float CharacterIdentity::getAttribute(const std::string& key) const {
    auto it = m_baseAttributes.find(key);
    if (it != m_baseAttributes.end()) {
        return it->second;
    }
    return 0.0f;
}

void CharacterIdentity::addAppearanceToken(const std::string& token) {
    m_appearanceTokens.push_back(token);
}

void CharacterIdentity::removeAppearanceToken(const std::string& token) {
    auto it = std::remove(m_appearanceTokens.begin(), m_appearanceTokens.end(), token);
    m_appearanceTokens.erase(it, m_appearanceTokens.end());
}

} // namespace urpg::character
