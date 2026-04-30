#pragma once

#include <nlohmann/json.hpp>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace urpg::localization {

class LocaleCatalog {
public:
    void loadFromJson(const nlohmann::json& json);
    void mergeFromJson(const nlohmann::json& json);

    std::optional<std::string> getKey(const std::string& key) const;
    bool hasKey(const std::string& key) const;
    std::vector<std::string> getAllKeys() const;

    std::string getLocaleCode() const;
    std::string getFontProfileId() const;
    bool hasFontProfile() const;
    size_t keyCount() const;
    void clear();

    static bool validateBundleJson(const nlohmann::json& json);

private:
    std::string m_localeCode;
    std::string m_fontProfileId;
    std::unordered_map<std::string, std::string> m_keys;
};

} // namespace urpg::localization
