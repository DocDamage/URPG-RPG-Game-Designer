#include "engine/core/localization/locale_catalog.h"

#include <algorithm>
#include <array>
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

bool decodeUtf8(std::string_view text, std::vector<char32_t>& output) {
    bool allValid = true;
    for (size_t cursor = 0; cursor < text.size();) {
        bool valid = true;
        const auto lead = static_cast<unsigned char>(text[cursor]);
        char32_t value = U'\uFFFD';
        size_t length = 1;
        if (lead < 0x80) {
            value = lead;
        } else if ((lead >> 5) == 0x6 && cursor + 1 < text.size()) {
            const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
            if ((b1 & 0xC0) == 0x80) {
                value = static_cast<char32_t>(((lead & 0x1F) << 6) | (b1 & 0x3F));
                length = 2;
                if (value < 0x80) valid = false;
            } else valid = false;
        } else if ((lead >> 4) == 0xE && cursor + 2 < text.size()) {
            const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
            const auto b2 = static_cast<unsigned char>(text[cursor + 2]);
            if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80) {
                value = static_cast<char32_t>(((lead & 0x0F) << 12) | ((b1 & 0x3F) << 6) | (b2 & 0x3F));
                length = 3;
                if (value < 0x800 || (value >= 0xD800 && value <= 0xDFFF)) valid = false;
            } else valid = false;
        } else if ((lead >> 3) == 0x1E && cursor + 3 < text.size()) {
            const auto b1 = static_cast<unsigned char>(text[cursor + 1]);
            const auto b2 = static_cast<unsigned char>(text[cursor + 2]);
            const auto b3 = static_cast<unsigned char>(text[cursor + 3]);
            if ((b1 & 0xC0) == 0x80 && (b2 & 0xC0) == 0x80 && (b3 & 0xC0) == 0x80) {
                value = static_cast<char32_t>(((lead & 0x07) << 18) | ((b1 & 0x3F) << 12) |
                                              ((b2 & 0x3F) << 6) | (b3 & 0x3F));
                length = 4;
                if (value < 0x10000 || value > 0x10FFFF) valid = false;
            } else valid = false;
        } else {
            valid = false;
        }
        if (!valid) {
            value = U'\uFFFD';
            allValid = false;
        }
        output.push_back(value);
        cursor += length;
    }
    return allValid;
}

void appendUtf8(std::string& output, char32_t value) {
    if (value <= 0x7F) output.push_back(static_cast<char>(value));
    else if (value <= 0x7FF) {
        output.push_back(static_cast<char>(0xC0 | (value >> 6)));
        output.push_back(static_cast<char>(0x80 | (value & 0x3F)));
    } else if (value <= 0xFFFF) {
        output.push_back(static_cast<char>(0xE0 | (value >> 12)));
        output.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (value & 0x3F)));
    } else {
        output.push_back(static_cast<char>(0xF0 | (value >> 18)));
        output.push_back(static_cast<char>(0x80 | ((value >> 12) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | ((value >> 6) & 0x3F)));
        output.push_back(static_cast<char>(0x80 | (value & 0x3F)));
    }
}

std::string encodeUtf8(const std::vector<char32_t>& values) {
    std::string result;
    for (const auto value : values) appendUtf8(result, value);
    return result;
}

bool isCombining(char32_t cp) {
    return (cp >= 0x0300 && cp <= 0x036F) || (cp >= 0x0591 && cp <= 0x05BD) ||
           (cp >= 0x064B && cp <= 0x065F) || (cp >= 0xFE00 && cp <= 0xFE0F) ||
           (cp >= 0x1F3FB && cp <= 0x1F3FF);
}

bool isRtlStrong(char32_t cp) {
    return (cp >= 0x0590 && cp <= 0x08FF) || (cp >= 0xFB1D && cp <= 0xFDFF) ||
           (cp >= 0xFE70 && cp <= 0xFEFF);
}

bool isLtrStrong(char32_t cp) {
    return (cp >= U'A' && cp <= U'Z') || (cp >= U'a' && cp <= U'z') ||
           (cp >= 0x00C0 && cp <= 0x02AF) || (cp >= 0x0370 && cp <= 0x052F) ||
           (cp >= 0x3040 && cp <= 0x30FF) || (cp >= 0x3400 && cp <= 0x9FFF);
}

bool isCjk(char32_t cp) {
    return (cp >= 0x2E80 && cp <= 0x9FFF) || (cp >= 0xF900 && cp <= 0xFAFF) ||
           (cp >= 0x3040 && cp <= 0x30FF) || (cp >= 0xAC00 && cp <= 0xD7AF);
}

size_t displayColumns(char32_t cp) {
    if (isCombining(cp) || cp == 0x200D) return 0;
    if (cp == U'\t') return 4;
    if (isCjk(cp) || cp >= 0x1F000) return 2;
    return 1;
}

struct ArabicForm {
    char32_t base;
    char32_t isolated;
    char32_t final;
    char32_t initial;
    char32_t medial;
    bool joinsNext;
};

constexpr std::array kArabicForms = {
    ArabicForm{0x0621, 0xFE80, 0, 0, 0, false}, ArabicForm{0x0622, 0xFE81, 0xFE82, 0, 0, false},
    ArabicForm{0x0623, 0xFE83, 0xFE84, 0, 0, false}, ArabicForm{0x0624, 0xFE85, 0xFE86, 0, 0, false},
    ArabicForm{0x0625, 0xFE87, 0xFE88, 0, 0, false}, ArabicForm{0x0626, 0xFE89, 0xFE8A, 0xFE8B, 0xFE8C, true},
    ArabicForm{0x0627, 0xFE8D, 0xFE8E, 0, 0, false}, ArabicForm{0x0628, 0xFE8F, 0xFE90, 0xFE91, 0xFE92, true},
    ArabicForm{0x0629, 0xFE93, 0xFE94, 0, 0, false}, ArabicForm{0x062A, 0xFE95, 0xFE96, 0xFE97, 0xFE98, true},
    ArabicForm{0x062B, 0xFE99, 0xFE9A, 0xFE9B, 0xFE9C, true}, ArabicForm{0x062C, 0xFE9D, 0xFE9E, 0xFE9F, 0xFEA0, true},
    ArabicForm{0x062D, 0xFEA1, 0xFEA2, 0xFEA3, 0xFEA4, true}, ArabicForm{0x062E, 0xFEA5, 0xFEA6, 0xFEA7, 0xFEA8, true},
    ArabicForm{0x062F, 0xFEA9, 0xFEAA, 0, 0, false}, ArabicForm{0x0630, 0xFEAB, 0xFEAC, 0, 0, false},
    ArabicForm{0x0631, 0xFEAD, 0xFEAE, 0, 0, false}, ArabicForm{0x0632, 0xFEAF, 0xFEB0, 0, 0, false},
    ArabicForm{0x0633, 0xFEB1, 0xFEB2, 0xFEB3, 0xFEB4, true}, ArabicForm{0x0634, 0xFEB5, 0xFEB6, 0xFEB7, 0xFEB8, true},
    ArabicForm{0x0635, 0xFEB9, 0xFEBA, 0xFEBB, 0xFEBC, true}, ArabicForm{0x0636, 0xFEBD, 0xFEBE, 0xFEBF, 0xFEC0, true},
    ArabicForm{0x0637, 0xFEC1, 0xFEC2, 0xFEC3, 0xFEC4, true}, ArabicForm{0x0638, 0xFEC5, 0xFEC6, 0xFEC7, 0xFEC8, true},
    ArabicForm{0x0639, 0xFEC9, 0xFECA, 0xFECB, 0xFECC, true}, ArabicForm{0x063A, 0xFECD, 0xFECE, 0xFECF, 0xFED0, true},
    ArabicForm{0x0641, 0xFED1, 0xFED2, 0xFED3, 0xFED4, true}, ArabicForm{0x0642, 0xFED5, 0xFED6, 0xFED7, 0xFED8, true},
    ArabicForm{0x0643, 0xFED9, 0xFEDA, 0xFEDB, 0xFEDC, true}, ArabicForm{0x0644, 0xFEDD, 0xFEDE, 0xFEDF, 0xFEE0, true},
    ArabicForm{0x0645, 0xFEE1, 0xFEE2, 0xFEE3, 0xFEE4, true}, ArabicForm{0x0646, 0xFEE5, 0xFEE6, 0xFEE7, 0xFEE8, true},
    ArabicForm{0x0647, 0xFEE9, 0xFEEA, 0xFEEB, 0xFEEC, true}, ArabicForm{0x0648, 0xFEED, 0xFEEE, 0, 0, false},
    ArabicForm{0x0649, 0xFEEF, 0xFEF0, 0, 0, false}, ArabicForm{0x064A, 0xFEF1, 0xFEF2, 0xFEF3, 0xFEF4, true},
};

const ArabicForm* arabicForm(char32_t cp) {
    const auto found = std::ranges::find_if(kArabicForms, [&](const auto& item) { return item.base == cp; });
    return found == kArabicForms.end() ? nullptr : &*found;
}

std::vector<char32_t> shapeArabic(std::vector<char32_t> values) {
    const auto logical = values;
    for (size_t index = 0; index < values.size(); ++index) {
        const auto* current = arabicForm(logical[index]);
        if (!current) continue;
        size_t previous = index;
        while (previous > 0 && isCombining(logical[previous - 1])) --previous;
        size_t next = index + 1;
        while (next < logical.size() && isCombining(logical[next])) ++next;
        const auto* previousForm = previous > 0 ? arabicForm(logical[previous - 1]) : nullptr;
        const auto* nextForm = next < logical.size() ? arabicForm(logical[next]) : nullptr;
        const bool joinsPrevious = current->final != 0 && previousForm && previousForm->joinsNext;
        const bool joinsNext = current->joinsNext && nextForm && nextForm->final != 0;
        if (joinsPrevious && joinsNext && current->medial) values[index] = current->medial;
        else if (joinsPrevious && current->final) values[index] = current->final;
        else if (joinsNext && current->initial) values[index] = current->initial;
        else values[index] = current->isolated;
    }
    return values;
}

struct TextCluster {
    std::vector<char32_t> values;
    size_t columns = 0;
};

std::vector<TextCluster> makeClusters(const std::vector<char32_t>& values) {
    std::vector<TextCluster> clusters;
    for (const auto cp : values) {
        const bool attaches = isCombining(cp) || cp == 0x200D ||
                              (!clusters.empty() && !clusters.back().values.empty() &&
                               clusters.back().values.back() == 0x200D);
        if (attaches && !clusters.empty()) {
            clusters.back().values.push_back(cp);
            clusters.back().columns += displayColumns(cp);
        } else {
            clusters.push_back({{cp}, displayColumns(cp)});
        }
    }
    return clusters;
}

char32_t mirrored(char32_t cp) {
    switch (cp) {
    case U'(': return U')'; case U')': return U'(';
    case U'[': return U']'; case U']': return U'[';
    case U'{': return U'}'; case U'}': return U'{';
    case U'<': return U'>'; case U'>': return U'<';
    default: return cp;
    }
}

std::string clustersToText(const std::vector<TextCluster>& clusters) {
    std::string text;
    for (const auto& cluster : clusters) for (const auto cp : cluster.values) appendUtf8(text, cp);
    return text;
}

std::string visualOrder(const std::vector<TextCluster>& logical, LocaleTextDirection paragraphDirection) {
    struct Run { bool rtl; std::vector<TextCluster> clusters; };
    std::vector<Run> runs;
    bool activeRtl = paragraphDirection == LocaleTextDirection::RightToLeft;
    for (const auto& cluster : logical) {
        const char32_t base = cluster.values.empty() ? U' ' : cluster.values.front();
        const bool strongRtl = isRtlStrong(base);
        const bool strongLtr = isLtrStrong(base) || (base >= U'0' && base <= U'9') ||
                               (base >= 0x0660 && base <= 0x0669);
        const bool rtl = strongRtl ? true : strongLtr ? false : activeRtl;
        activeRtl = rtl;
        if (runs.empty() || runs.back().rtl != rtl) runs.push_back({rtl, {}});
        runs.back().clusters.push_back(cluster);
    }
    for (auto& run : runs) {
        if (!run.rtl) continue;
        std::reverse(run.clusters.begin(), run.clusters.end());
        for (auto& cluster : run.clusters) {
            if (!cluster.values.empty()) cluster.values.front() = mirrored(cluster.values.front());
        }
    }
    if (paragraphDirection == LocaleTextDirection::RightToLeft) std::reverse(runs.begin(), runs.end());
    std::string text;
    for (const auto& run : runs) text += clustersToText(run.clusters);
    return text;
}

bool breakAfter(const TextCluster& cluster) {
    if (cluster.values.empty()) return false;
    const auto cp = cluster.values.front();
    return cp == U' ' || cp == U'\t' || cp == U'-' || cp == U'/' || isCjk(cp);
}

bool whitespaceCluster(const TextCluster& cluster) {
    return !cluster.values.empty() && (cluster.values.front() == U' ' || cluster.values.front() == U'\t');
}

} // namespace

LocaleTextLayoutResult layoutLocaleText(std::string_view text, LocaleTextDirection direction, size_t maxColumns) {
    LocaleTextLayoutResult result;
    std::vector<char32_t> codepoints;
    const bool validUtf8 = decodeUtf8(text, codepoints);
    if (!validUtf8) result.diagnostics.push_back("localization_text_invalid_utf8_replaced");
    if (direction == LocaleTextDirection::Auto) {
        direction = LocaleTextDirection::LeftToRight;
        for (const auto cp : codepoints) {
            if (isRtlStrong(cp)) { direction = LocaleTextDirection::RightToLeft; break; }
            if (isLtrStrong(cp)) break;
        }
    }
    result.direction = direction;

    std::vector<char32_t> paragraph;
    const auto flushParagraph = [&](const std::vector<char32_t>& logicalValues) {
        const auto shaped = shapeArabic(logicalValues);
        auto clusters = makeClusters(shaped);
        const auto logicalClusters = makeClusters(logicalValues);
        size_t start = 0;
        while (start < clusters.size() || (clusters.empty() && result.lines.empty())) {
            size_t end = start;
            size_t columns = 0;
            size_t lastBreak = start;
            while (end < clusters.size()) {
                const size_t nextColumns = columns + clusters[end].columns;
                if (maxColumns > 0 && nextColumns > maxColumns && end > start) break;
                columns = nextColumns;
                ++end;
                if (breakAfter(clusters[end - 1])) lastBreak = end;
                if (maxColumns > 0 && columns >= maxColumns) break;
            }
            if (end < clusters.size() && lastBreak > start) end = lastBreak;
            if (end == start && end < clusters.size()) ++end;
            std::vector<TextCluster> line(clusters.begin() + static_cast<std::ptrdiff_t>(start),
                                          clusters.begin() + static_cast<std::ptrdiff_t>(end));
            std::vector<TextCluster> logicalLine(logicalClusters.begin() + static_cast<std::ptrdiff_t>(start),
                                                 logicalClusters.begin() + static_cast<std::ptrdiff_t>(end));
            while (!line.empty() && whitespaceCluster(line.back())) line.pop_back();
            while (!logicalLine.empty() && whitespaceCluster(logicalLine.back())) logicalLine.pop_back();
            size_t lineColumns = 0;
            for (const auto& cluster : line) lineColumns += cluster.columns;
            result.lines.push_back({clustersToText(logicalLine), visualOrder(line, direction), lineColumns});
            start = end;
            while (start < clusters.size() && whitespaceCluster(clusters[start])) ++start;
            if (clusters.empty()) break;
        }
    };
    for (const auto cp : codepoints) {
        if (cp == U'\r') continue;
        if (cp == U'\n') { flushParagraph(paragraph); paragraph.clear(); }
        else paragraph.push_back(cp);
    }
    flushParagraph(paragraph);
    result.valid = validUtf8;
    return result;
}

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

LocaleTextLayoutResult LocaleCatalog::layoutText(std::string_view text, size_t maxColumns) const {
    const auto direction = m_textDirection == "rtl" ? LocaleTextDirection::RightToLeft
                                                     : LocaleTextDirection::LeftToRight;
    return layoutLocaleText(text, direction, maxColumns);
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
    m_fallbackLocale.clear();
    m_textDirection = "ltr";
    m_supportsIme = false;
    m_sourceRevision = 1;
    m_keys.clear();
    m_variants.clear();
}

} // namespace urpg::localization
