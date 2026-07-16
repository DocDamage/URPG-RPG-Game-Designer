#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <optional>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace urpg::localization {

struct LocaleMessageVariant {
    std::string plural = "other";
    std::string grammar = "neutral";
    std::string text;
};

enum class LocaleTextDirection { Auto, LeftToRight, RightToLeft };

struct LocaleTextLine {
    std::string logicalText;
    std::string visualText;
    size_t columns = 0;
};

struct LocaleTextLayoutResult {
    bool valid = false;
    LocaleTextDirection direction = LocaleTextDirection::LeftToRight;
    std::vector<LocaleTextLine> lines;
    std::vector<std::string> diagnostics;
};

LocaleTextLayoutResult layoutLocaleText(std::string_view text,
                                        LocaleTextDirection direction = LocaleTextDirection::Auto,
                                        size_t maxColumns = 0);

class LocaleCatalog {
public:
    void loadFromJson(const nlohmann::json& json);
    void mergeFromJson(const nlohmann::json& json);

    std::optional<std::string> getKey(const std::string& key) const;
    std::optional<std::string> getVariant(const std::string& key, const std::string& plural,
                                          const std::string& grammar = "neutral") const;
    std::vector<LocaleMessageVariant> getVariants(const std::string& key) const;
    bool hasKey(const std::string& key) const;
    std::vector<std::string> getAllKeys() const;

    std::string getLocaleCode() const;
    std::string getFontProfileId() const;
    std::string getFallbackLocale() const;
    std::string getTextDirection() const;
    bool supportsIme() const;
    uint32_t sourceRevision() const;
    LocaleTextLayoutResult layoutText(std::string_view text, size_t maxColumns = 0) const;
    bool hasFontProfile() const;
    size_t keyCount() const;
    void clear();

    static bool validateBundleJson(const nlohmann::json& json);

private:
    std::string m_localeCode;
    std::string m_fontProfileId;
    std::string m_fallbackLocale;
    std::string m_textDirection = "ltr";
    bool m_supportsIme = false;
    uint32_t m_sourceRevision = 1;
    std::unordered_map<std::string, std::string> m_keys;
    std::map<std::string, std::vector<LocaleMessageVariant>> m_variants;
};

} // namespace urpg::localization
