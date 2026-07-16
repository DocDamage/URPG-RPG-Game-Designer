#include "engine/core/localization/locale_catalog.h"

#include <algorithm>
#include <stdexcept>
#include <tuple>

namespace urpg::localization {

namespace {

bool validPlural(const std::string& value) {
    return value == "zero" || value == "one" || value == "two" || value == "few" || value == "many" ||
           value == "other";
}

bool validGrammar(const std::string& value) {
    return value == "neutral" || value == "masculine" || value == "feminine" || value == "other";
}

bool validMessageValue(const nlohmann::json& value) {
    if (value.is_string()) return true;
    if (!value.is_object() || value.empty()) return false;
    for (const auto& [plural, pluralValue] : value.items()) {
        if (!validPlural(plural)) return false;
        if (pluralValue.is_string()) continue;
        if (!pluralValue.is_object() || pluralValue.empty()) return false;
        for (const auto& [grammar, text] : pluralValue.items()) {
            if (!validGrammar(grammar) || !text.is_string()) return false;
        }
    }
    return true;
}

std::vector<LocaleMessageVariant> readVariants(const nlohmann::json& value) {
    if (value.is_string()) return {{"other", "neutral", value.get<std::string>()}};
    std::vector<LocaleMessageVariant> variants;
    for (const auto& [plural, pluralValue] : value.items()) {
        if (pluralValue.is_string()) {
            variants.push_back({plural, "neutral", pluralValue.get<std::string>()});
            continue;
        }
        for (const auto& [grammar, text] : pluralValue.items()) {
            variants.push_back({plural, grammar, text.get<std::string>()});
        }
    }
    std::ranges::sort(variants, [](const auto& left, const auto& right) {
        return std::tie(left.plural, left.grammar) < std::tie(right.plural, right.grammar);
    });
    return variants;
}

std::optional<std::string> defaultText(const std::vector<LocaleMessageVariant>& variants) {
    for (const auto& wanted : {std::pair{"other", "neutral"}, std::pair{"one", "neutral"}}) {
        const auto found = std::ranges::find_if(variants, [&](const auto& variant) {
            return variant.plural == wanted.first && variant.grammar == wanted.second;
        });
        if (found != variants.end()) return found->text;
    }
    return variants.empty() ? std::nullopt : std::optional<std::string>(variants.front().text);
}

} // namespace

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
    if (json.contains("fallback_locale") && !json["fallback_locale"].is_string()) return false;
    if (json.contains("text_direction") &&
        (!json["text_direction"].is_string() ||
         (json["text_direction"] != "ltr" && json["text_direction"] != "rtl"))) return false;
    if (json.contains("ime_supported") && !json["ime_supported"].is_boolean()) return false;
    if (json.contains("source_revision") &&
        (!json["source_revision"].is_number_unsigned() || json["source_revision"].get<uint32_t>() == 0)) return false;
    for (const auto& [key, value] : json["keys"].items()) {
        if (key.empty() || !validMessageValue(value)) return false;
    }
    return true;
}

void LocaleCatalog::loadFromJson(const nlohmann::json& json) {
    if (!validateBundleJson(json)) {
        throw std::invalid_argument("Invalid locale bundle JSON");
    }

    m_localeCode = json["locale"].get<std::string>();
    m_fontProfileId = json.value("font_profile_id", "");
    m_fallbackLocale = json.value("fallback_locale", "");
    m_textDirection = json.value("text_direction", "ltr");
    m_supportsIme = json.value("ime_supported", false);
    m_sourceRevision = json.value("source_revision", 1u);
    m_keys.clear();
    m_variants.clear();

    for (const auto& [key, value] : json["keys"].items()) {
        auto variants = readVariants(value);
        m_variants[key] = variants;
        m_keys[key] = *defaultText(variants);
    }
}

void LocaleCatalog::mergeFromJson(const nlohmann::json& json) {
    if (!json.is_object() || !json.contains("keys") || !json["keys"].is_object()) {
        throw std::invalid_argument("Invalid locale merge JSON: missing 'keys' object");
    }

    for (const auto& [key, value] : json["keys"].items()) {
        if (key.empty() || !validMessageValue(value))
            throw std::invalid_argument("Invalid locale merge JSON: key values must be strings or plural/grammar objects");
        auto incoming = readVariants(value);
        auto& variants = m_variants[key];
        for (auto& variant : incoming) {
            std::erase_if(variants, [&](const auto& existing) {
                return existing.plural == variant.plural && existing.grammar == variant.grammar;
            });
            variants.push_back(std::move(variant));
        }
        std::ranges::sort(variants, [](const auto& left, const auto& right) {
            return std::tie(left.plural, left.grammar) < std::tie(right.plural, right.grammar);
        });
        m_keys[key] = *defaultText(variants);
    }
}

std::optional<std::string> LocaleCatalog::getVariant(const std::string& key, const std::string& plural,
                                                      const std::string& grammar) const {
    const auto found = m_variants.find(key);
    if (found == m_variants.end()) return std::nullopt;
    for (const auto& candidate : {std::pair{plural, grammar}, std::pair{plural, std::string("neutral")},
                                  std::pair{std::string("other"), grammar},
                                  std::pair{std::string("other"), std::string("neutral")}}) {
        const auto variant = std::ranges::find_if(found->second, [&](const auto& item) {
            return item.plural == candidate.first && item.grammar == candidate.second;
        });
        if (variant != found->second.end()) return variant->text;
    }
    return std::nullopt;
}

std::vector<LocaleMessageVariant> LocaleCatalog::getVariants(const std::string& key) const {
    const auto found = m_variants.find(key);
    return found == m_variants.end() ? std::vector<LocaleMessageVariant>{} : found->second;
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

std::string LocaleCatalog::getFallbackLocale() const { return m_fallbackLocale; }
std::string LocaleCatalog::getTextDirection() const { return m_textDirection; }
bool LocaleCatalog::supportsIme() const { return m_supportsIme; }
uint32_t LocaleCatalog::sourceRevision() const { return m_sourceRevision; }

bool LocaleCatalog::hasFontProfile() const {
    return !m_fontProfileId.empty();
}

size_t LocaleCatalog::keyCount() const {
    return m_keys.size();
}

void LocaleCatalog::clear() {
    m_localeCode.clear();
    m_fontProfileId.clear();
    m_fallbackLocale.clear();
    m_textDirection = "ltr";
    m_supportsIme = false;
    m_sourceRevision = 1;
    m_keys.clear();
    m_variants.clear();
}

} // namespace urpg::localization
