#include "engine/core/localization/locale_catalog.h"

#include <algorithm>
#include <stdexcept>

namespace urpg::localization {

bool LocaleCatalog::validateBundleJson(const nlohmann::json& json) {
    if (!json.is_object()) {
        return false;
    }
    if (!json.contains("locale") || !json["locale"].is_string()) {
        return false;
    }
    if (!json.contains("keys") || !json["keys"].is_object()) {
        return false;
    }
    if (json.contains("font_profile_id") && !json["font_profile_id"].is_string()) {
        return false;
    }
    for (const auto& [key, value] : json["keys"].items()) {
        if (!value.is_string()) {
            return false;
        }
    }
    return true;
}

void LocaleCatalog::loadFromJson(const nlohmann::json& json) {
    if (!validateBundleJson(json)) {
        throw std::invalid_argument("Invalid locale bundle JSON");
    }

    m_localeCode = json["locale"].get<std::string>();
    m_fontProfileId = json.value("font_profile_id", "");
    m_keys.clear();

    for (const auto& [key, value] : json["keys"].items()) {
        m_keys[key] = value.get<std::string>();
    }
}

void LocaleCatalog::mergeFromJson(const nlohmann::json& json) {
    if (!json.is_object() || !json.contains("keys") || !json["keys"].is_object()) {
        throw std::invalid_argument("Invalid locale merge JSON: missing 'keys' object");
    }

    for (const auto& [key, value] : json["keys"].items()) {
        if (!value.is_string()) {
            throw std::invalid_argument("Invalid locale merge JSON: all values in 'keys' must be strings");
        }
        m_keys[key] = value.get<std::string>();
    }
}

std::optional<std::string> LocaleCatalog::getKey(const std::string& key) const {
    const auto it = m_keys.find(key);
    if (it == m_keys.end()) {
        return std::nullopt;
    }
    return it->second;
}

bool LocaleCatalog::hasKey(const std::string& key) const {
    return m_keys.count(key) > 0;
}

std::vector<std::string> LocaleCatalog::getAllKeys() const {
    std::vector<std::string> keys;
    keys.reserve(m_keys.size());
    for (const auto& [key, value] : m_keys) {
        (void)value;
        keys.push_back(key);
    }
    std::sort(keys.begin(), keys.end());
    return keys;
}

std::string LocaleCatalog::getLocaleCode() const {
    return m_localeCode;
}

std::string LocaleCatalog::getFontProfileId() const {
    return m_fontProfileId;
}

bool LocaleCatalog::hasFontProfile() const {
    return !m_fontProfileId.empty();
}

size_t LocaleCatalog::keyCount() const {
    return m_keys.size();
}

void LocaleCatalog::clear() {
    m_localeCode.clear();
    m_fontProfileId.clear();
    m_keys.clear();
}

} // namespace urpg::localization
