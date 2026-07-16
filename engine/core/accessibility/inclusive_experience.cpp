#include "engine/core/accessibility/inclusive_experience.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/map/project_world_graph.h"
#include "engine/core/map/tile_layer_document.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/quest/quest_objective_graph.h"
#include "engine/core/ui/menu_authoring_document.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

namespace urpg::accessibility {

namespace {

bool validId(std::string_view id) {
    return !id.empty() && id.size() <= 128 && std::all_of(id.begin(), id.end(), [](unsigned char ch) {
        return std::isalnum(ch) != 0 || ch == '_' || ch == '-' || ch == '.';
    });
}

std::vector<std::string> normalizedControls(std::vector<std::string> controls) {
    std::sort(controls.begin(), controls.end());
    controls.erase(std::unique(controls.begin(), controls.end()), controls.end());
    return controls;
}

bool inUnit(float value) { return std::isfinite(value) && value >= 0.0F && value <= 1.0F; }

std::string localeLanguage(std::string_view locale) {
    const auto separator = locale.find_first_of("-_");
    std::string language(locale.substr(0, separator));
    std::ranges::transform(language, language.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return language;
}

uint64_t absoluteCount(int64_t value) {
    return value < 0 ? static_cast<uint64_t>(-(value + 1)) + 1u : static_cast<uint64_t>(value);
}

std::string localizedInteger(std::string_view locale, int64_t value) {
    const std::string raw = std::to_string(value);
    if (localeLanguage(locale) != "ar") return raw;
    static constexpr const char* digits[] = {"٠", "١", "٢", "٣", "٤", "٥", "٦", "٧", "٨", "٩"};
    std::string localized;
    for (const char ch : raw) {
        if (ch >= '0' && ch <= '9') localized += digits[ch - '0'];
        else localized.push_back(ch);
    }
    return localized;
}

GrammarVariant grammarVariant(std::string_view grammar) {
    if (grammar == "masculine") return GrammarVariant::Masculine;
    if (grammar == "feminine") return GrammarVariant::Feminine;
    if (grammar == "other") return GrammarVariant::Other;
    return GrammarVariant::Neutral;
}

} // namespace

bool UnifiedInputRouter::connectDevice(InclusiveDevice device) {
    if (!validId(device.id) || !inUnit(device.deadzone) || device.deadzone >= 0.95F ||
        !std::isfinite(device.axis_scale) || device.axis_scale <= 0.0F || device.axis_scale > 4.0F ||
        device.glyph_profile.empty()) return false;
    device.connected = true;
    devices_[device.id] = std::move(device);
    return true;
}

bool UnifiedInputRouter::disconnectDevice(std::string_view id) {
    auto found = devices_.find(std::string(id));
    if (found == devices_.end() || !found->second.connected) return false;
    found->second.connected = false;
    if (active_device_id_ == id) active_device_id_.clear();
    return true;
}

bool UnifiedInputRouter::reconnectDevice(std::string_view id) {
    auto found = devices_.find(std::string(id));
    if (found == devices_.end() || found->second.connected) return false;
    found->second.connected = true;
    return true;
}

bool UnifiedInputRouter::setOwnership(std::string_view id, InputOwnership owner) {
    auto found = devices_.find(std::string(id));
    if (found == devices_.end()) return false;
    found->second.owner = owner;
    return true;
}

bool UnifiedInputRouter::calibrate(std::string_view id, float deadzone, float axis_scale) {
    auto found = devices_.find(std::string(id));
    if (found == devices_.end() || !inUnit(deadzone) || deadzone >= 0.95F ||
        !std::isfinite(axis_scale) || axis_scale <= 0.0F || axis_scale > 4.0F) return false;
    found->second.deadzone = deadzone;
    found->second.axis_scale = axis_scale;
    return true;
}

std::optional<InputConflict> UnifiedInputRouter::conflictFor(const InclusiveBinding& candidate) const {
    const auto controls = normalizedControls(candidate.controls);
    for (const auto& binding : bindings_) {
        if (binding.context == candidate.context && binding.action != candidate.action &&
            normalizedControls(binding.controls) == controls) {
            return InputConflict{candidate.context, binding.action, candidate.action, controls};
        }
    }
    return std::nullopt;
}

bool UnifiedInputRouter::bind(InclusiveBinding binding, bool replace_conflict) {
    binding.controls = normalizedControls(std::move(binding.controls));
    if (binding.action == input::InputAction::None || binding.controls.empty() || binding.controls.size() > 4 ||
        std::any_of(binding.controls.begin(), binding.controls.end(), [](const auto& item) { return !validId(item); }) ||
        binding.repeat_delay_ms > 5000 || binding.repeat_interval_ms < 16 || binding.repeat_interval_ms > 1000)
        return false;
    if (const auto conflict = conflictFor(binding)) {
        if (!replace_conflict) return false;
        std::erase_if(bindings_, [&](const InclusiveBinding& existing) {
            return existing.context == conflict->context && existing.action == conflict->existing &&
                   normalizedControls(existing.controls) == conflict->controls;
        });
    }
    std::erase_if(bindings_, [&](const InclusiveBinding& existing) {
        return existing.context == binding.context && existing.action == binding.action &&
               normalizedControls(existing.controls) == binding.controls;
    });
    bindings_.push_back(std::move(binding));
    return true;
}

InputRouteResult UnifiedInputRouter::route(std::string_view device_id, InclusiveInputContext context,
                                           const std::vector<std::string>& held_controls, float raw_axis,
                                           uint64_t held_ms,
                                           std::optional<InputOwnership> requested_owner) const {
    InputRouteResult result;
    const auto device = devices_.find(std::string(device_id));
    if (device == devices_.end() || !device->second.connected) {
        result.diagnostics.push_back("Input device is disconnected or unknown.");
        return result;
    }
    if (requested_owner && device->second.owner != InputOwnership::Shared &&
        device->second.owner != *requested_owner) {
        result.diagnostics.push_back("Input device is owned by a different player.");
        return result;
    }
    const auto controls = normalizedControls(held_controls);
    const auto binding = std::find_if(bindings_.begin(), bindings_.end(), [&](const InclusiveBinding& item) {
        return (item.context == context || item.context == InclusiveInputContext::Global) &&
               std::includes(controls.begin(), controls.end(), item.controls.begin(), item.controls.end());
    });
    if (binding == bindings_.end()) {
        result.diagnostics.push_back("No semantic binding matches the active context and chord.");
        return result;
    }
    const float magnitude = std::abs(raw_axis);
    if (magnitude > device->second.deadzone) {
        result.normalized_value = std::copysign(std::clamp((magnitude - device->second.deadzone) /
            (1.0F - device->second.deadzone) * device->second.axis_scale, 0.0F, 1.0F), raw_axis);
    }
    const bool repeat_due = held_ms == 0 ||
        (held_ms >= binding->repeat_delay_ms &&
         (held_ms - binding->repeat_delay_ms) % binding->repeat_interval_ms == 0);
    if (!repeat_due) {
        result.diagnostics.push_back("Input repeat delay has not elapsed.");
        return result;
    }
    result.accepted = true;
    result.action = binding->action;
    result.active_device_changed = active_device_id_ != device_id;
    active_device_id_ = std::string(device_id);
    result.glyph = device->second.glyph_profile + ":" + binding->controls.back();
    return result;
}

InputRouteResult UnifiedInputRouter::dispatch(input::InputCore& core, std::string_view device_id,
                                              InclusiveInputContext context,
                                              const std::vector<std::string>& controls,
                                              input::ActionState state, float raw_axis,
                                              uint64_t held_ms,
                                              std::optional<InputOwnership> requested_owner) const {
    auto result = route(device_id, context, controls, raw_axis,
                        state == input::ActionState::Released ? 0 : held_ms, requested_owner);
    if (result.accepted) core.updateActionState(result.action, state);
    return result;
}

std::vector<std::string> UnifiedInputRouter::recoveryActions() const {
    bool keyboard = false;
    bool controller = false;
    for (const auto& [id, device] : devices_) {
        (void)id;
        if (!device.connected) continue;
        keyboard |= device.kind == InclusiveDeviceKind::KeyboardMouse;
        controller |= device.kind == InclusiveDeviceKind::Controller;
    }
    std::vector<std::string> actions{"reset-bindings", "open-calibration"};
    if (keyboard) actions.push_back("continue-keyboard-mouse");
    if (controller) actions.push_back("continue-controller");
    if (!keyboard && !controller) actions.push_back("wait-for-device");
    return actions;
}

bool InclusiveSettings::isValid() const {
    return inUnit(screen_shake) && inUnit(flash_intensity) && caption_scale >= 0.75F && caption_scale <= 2.5F &&
           inUnit(master_volume) && inUnit(music_volume) && inUnit(effects_volume) && inUnit(voice_volume) &&
           (!reduced_motion || screen_shake == 0.0F) && non_color_cues;
}

nlohmann::json InclusiveSettings::toJson() const {
    return {{"schema", "urpg.inclusive_settings.v1"}, {"text_scale", static_cast<int>(text_scale)},
        {"high_contrast", high_contrast}, {"color_filter", static_cast<int>(color_filter)},
        {"non_color_cues", non_color_cues}, {"reduced_motion", reduced_motion},
        {"screen_shake", screen_shake}, {"flash_intensity", flash_intensity},
        {"subtitles", subtitles}, {"captions", captions}, {"caption_scale", caption_scale},
        {"master_volume", master_volume}, {"music_volume", music_volume},
        {"effects_volume", effects_volume}, {"voice_volume", voice_volume}, {"mono_audio", mono_audio}};
}

std::optional<InclusiveSettings> InclusiveSettings::fromJson(const nlohmann::json& json) {
    try {
        if (!json.is_object() || json.value("schema", "") != "urpg.inclusive_settings.v1") return std::nullopt;
        InclusiveSettings value;
        const int scale = json.value("text_scale", -1);
        const int filter = json.value("color_filter", -1);
        if (scale < 0 || scale > static_cast<int>(TextScaleProfile::ExtraLarge) ||
            filter < 0 || filter > static_cast<int>(ColorFilter::Monochrome)) return std::nullopt;
        value.text_scale = static_cast<TextScaleProfile>(scale);
        value.high_contrast = json.value("high_contrast", false);
        value.color_filter = static_cast<ColorFilter>(filter);
        value.non_color_cues = json.value("non_color_cues", true);
        value.reduced_motion = json.value("reduced_motion", false);
        value.screen_shake = json.value("screen_shake", 1.0F);
        value.flash_intensity = json.value("flash_intensity", 1.0F);
        value.subtitles = json.value("subtitles", true);
        value.captions = json.value("captions", true);
        value.caption_scale = json.value("caption_scale", 1.0F);
        value.master_volume = json.value("master_volume", 1.0F);
        value.music_volume = json.value("music_volume", 1.0F);
        value.effects_volume = json.value("effects_volume", 1.0F);
        value.voice_volume = json.value("voice_volume", 1.0F);
        value.mono_audio = json.value("mono_audio", false);
        return value.isValid() ? std::optional(value) : std::nullopt;
    } catch (...) { return std::nullopt; }
}

InclusiveSettings InclusiveSettings::safeDefaults() {
    InclusiveSettings value;
    value.non_color_cues = true;
    value.subtitles = true;
    value.captions = true;
    return value;
}

bool InclusiveLocalizationModel::defineLocale(LocaleProfile locale) {
    if (!validId(locale.id) || locale.font_profile.empty() || locale.fallback_id == locale.id) return false;
    locales_[locale.id] = std::move(locale);
    return true;
}

bool InclusiveLocalizationModel::addVariant(LocalizedVariant variant) {
    if (!validId(variant.key) || !locales_.contains(variant.locale) || variant.text.empty() || variant.revision == 0 ||
        (variant.plural != "zero" && variant.plural != "one" && variant.plural != "two" &&
         variant.plural != "few" && variant.plural != "many" && variant.plural != "other")) return false;
    std::erase_if(variants_, [&](const LocalizedVariant& item) {
        return item.key == variant.key && item.locale == variant.locale && item.plural == variant.plural &&
               item.grammar == variant.grammar;
    });
    variants_.push_back(std::move(variant));
    return true;
}

LocalizationResolution InclusiveLocalizationModel::resolve(std::string_view key, std::string_view locale,
                                                            int64_t count, GrammarVariant grammar) const {
    LocalizationResolution result;
    std::set<std::string> visited;
    std::string current(locale);
    while (!current.empty() && visited.insert(current).second) {
        const auto profile = locales_.find(current);
        if (profile == locales_.end()) break;
        const std::string plural = pluralCategory(current, count);
        result.direction = profile->second.direction;
        auto selected = variants_.end();
        for (const auto& candidate : {std::pair{plural, grammar}, std::pair{plural, GrammarVariant::Neutral},
                                      std::pair{std::string("other"), grammar},
                                      std::pair{std::string("other"), GrammarVariant::Neutral}}) {
            selected = std::find_if(variants_.begin(), variants_.end(), [&](const LocalizedVariant& item) {
                return item.key == key && item.locale == current && item.plural == candidate.first &&
                       item.grammar == candidate.second;
            });
            if (selected != variants_.end()) break;
        }
        if (selected != variants_.end()) {
            result.valid = true;
            result.used_fallback = current != locale;
            result.text = selected->text;
            size_t marker = 0;
            const auto renderedCount = localizedInteger(current, count);
            while ((marker = result.text.find("{count}", marker)) != std::string::npos) {
                result.text.replace(marker, 7, renderedCount);
                marker += renderedCount.size();
            }
            return result;
        }
        current = profile->second.fallback_id;
    }
    result.diagnostics.push_back("Localization key or grammatical variant is missing: " + std::string(key));
    return result;
}

std::string InclusiveLocalizationModel::pluralCategory(std::string_view locale, int64_t count) const {
    const std::string language = localeLanguage(locale);
    const uint64_t n = absoluteCount(count);
    const uint64_t mod10 = n % 10u;
    const uint64_t mod100 = n % 100u;
    if (language == "ar") {
        if (n == 0) return "zero";
        if (n == 1) return "one";
        if (n == 2) return "two";
        if (mod100 >= 3 && mod100 <= 10) return "few";
        if (mod100 >= 11 && mod100 <= 99) return "many";
        return "other";
    }
    if (language == "ru" || language == "uk" || language == "be") {
        if (mod10 == 1 && mod100 != 11) return "one";
        if (mod10 >= 2 && mod10 <= 4 && (mod100 < 12 || mod100 > 14)) return "few";
        if (mod10 == 0 || mod10 >= 5 || (mod100 >= 11 && mod100 <= 14)) return "many";
        return "other";
    }
    if (language == "pl") {
        if (n == 1) return "one";
        if (mod10 >= 2 && mod10 <= 4 && (mod100 < 12 || mod100 > 14)) return "few";
        return "many";
    }
    if (language == "cs" || language == "sk") {
        if (n == 1) return "one";
        if (n >= 2 && n <= 4) return "few";
        return "other";
    }
    if (language == "sl") {
        if (mod100 == 1) return "one";
        if (mod100 == 2) return "two";
        if (mod100 == 3 || mod100 == 4) return "few";
        return "other";
    }
    if (language == "fr" && (n == 0 || n == 1)) return "one";
    return n == 1 ? "one" : "other";
}

std::vector<std::string> InclusiveLocalizationModel::staleKeys(uint32_t source_revision) const {
    std::set<std::string> stale;
    for (const auto& variant : variants_) if (variant.revision < source_revision) stale.insert(variant.key + "@" + variant.locale);
    return {stale.begin(), stale.end()};
}

std::vector<std::string> InclusiveLocalizationModel::missingGlyphs(std::string_view locale,
                                                                   std::u32string_view text) const {
    std::vector<std::string> missing;
    const auto profile = locales_.find(std::string(locale));
    if (profile == locales_.end()) return {"locale-profile-missing"};
    for (char32_t glyph : text) if (!profile->second.glyphs.contains(glyph)) missing.push_back("U+" + std::to_string(glyph));
    return missing;
}

std::string InclusiveLocalizationModel::formatNumber(std::string_view locale, double value) const {
    if (!std::isfinite(value)) return {};
    std::ostringstream stream;
    stream.imbue(std::locale::classic());
    stream << std::fixed << std::setprecision(2) << std::abs(value);
    const auto raw = stream.str();
    const auto decimalAt = raw.find('.');
    std::string integer = raw.substr(0, decimalAt);
    std::string grouped;
    for (size_t index = 0; index < integer.size(); ++index) {
        if (index > 0 && (integer.size() - index) % 3 == 0) grouped.push_back(',');
        grouped.push_back(integer[index]);
    }
    std::string ascii = (std::signbit(value) ? "-" : "") + grouped + raw.substr(decimalAt);
    const auto language = localeLanguage(locale);
    const std::string decimal = language == "ar" ? "٫" :
        (language == "fr" || language == "de" || language == "es" || language == "it" || language == "pt" ||
         language == "ru" || language == "pl") ? "," : ".";
    const std::string group = language == "ar" ? "٬" : language == "de" ? "." :
        (language == "fr" || language == "ru") ? " " : ",";
    std::string rendered;
    static constexpr const char* arabicDigits[] = {"٠", "١", "٢", "٣", "٤", "٥", "٦", "٧", "٨", "٩"};
    for (const char ch : ascii) {
        if (ch >= '0' && ch <= '9' && language == "ar") rendered += arabicDigits[ch - '0'];
        else if (ch == '.') rendered += decimal;
        else if (ch == ',') rendered += group;
        else rendered.push_back(ch);
    }
    return rendered;
}

std::string InclusiveLocalizationModel::formatDate(std::string_view locale, int year, int month, int day) const {
    if (year < 1 || month < 1 || month > 12 || day < 1 || day > 31) return {};
    std::ostringstream stream;
    stream << std::setfill('0');
    const auto language = localeLanguage(locale);
    if (locale.starts_with("en-US")) stream << std::setw(2) << month << '/' << std::setw(2) << day << '/' << year;
    else if (language == "ja" || language == "zh" || language == "ko")
        stream << year << '/' << std::setw(2) << month << '/' << std::setw(2) << day;
    else stream << std::setw(2) << day << '/' << std::setw(2) << month << '/' << year;
    if (language != "ar") return stream.str();
    std::string rendered;
    static constexpr const char* arabicDigits[] = {"٠", "١", "٢", "٣", "٤", "٥", "٦", "٧", "٨", "٩"};
    for (const char ch : stream.str()) {
        if (ch >= '0' && ch <= '9') rendered += arabicDigits[ch - '0'];
        else rendered.push_back(ch);
    }
    return rendered;
}

bool InclusiveLocalizationModel::supportsIme(std::string_view locale) const {
    const auto found = locales_.find(std::string(locale));
    return found != locales_.end() && found->second.ime_supported;
}

bool InclusiveLocalizationModel::importCatalog(const localization::LocaleCatalog& catalog,
                                                std::string fallback_id,
                                                TextDirection direction,
                                                std::set<char32_t> glyphs,
                                                bool ime_supported,
                                                uint32_t revision) {
    if (catalog.getLocaleCode().empty() || !catalog.hasFontProfile() || revision == 0) return false;
    const auto locale_id = catalog.getLocaleCode();
    if (!defineLocale({locale_id, std::move(fallback_id), direction,
                       catalog.getFontProfileId(), std::move(glyphs), ime_supported})) return false;
    for (const auto& key : catalog.getAllKeys()) {
        const auto variants = catalog.getVariants(key);
        if (variants.empty()) return false;
        for (const auto& variant : variants) {
            if (!addVariant({key, locale_id, variant.text, variant.plural, grammarVariant(variant.grammar), revision}))
                return false;
        }
        if (variants.size() == 1 && variants.front().plural == "other" &&
            !addVariant({key, locale_id, variants.front().text, "one", GrammarVariant::Neutral, revision})) return false;
    }
    return true;
}

bool InclusiveLocalizationModel::importCatalog(const localization::LocaleCatalog& catalog,
                                                std::set<char32_t> glyphs) {
    return importCatalog(catalog, catalog.getFallbackLocale(),
                         catalog.getTextDirection() == "rtl" ? TextDirection::RightToLeft
                                                              : TextDirection::LeftToRight,
                         std::move(glyphs), catalog.supportsIme(), catalog.sourceRevision());
}

CaptionAuditResult auditCaptionTrack(const std::vector<CaptionCue>& cues,
                                     const std::set<std::string>& required_locales) {
    CaptionAuditResult result;
    std::set<std::string> ids;
    std::set<std::string> locales;
    for (const auto& cue : cues) {
        if (!validId(cue.id) || !ids.insert(cue.id).second) result.diagnostics.push_back("Caption IDs must be stable and unique.");
        if (cue.speaker_id.empty()) result.diagnostics.push_back(cue.id + ": speaker identity is missing.");
        if (cue.text.empty()) result.diagnostics.push_back(cue.id + ": caption text is missing.");
        if (cue.take_id.empty()) result.diagnostics.push_back(cue.id + ": locale take is missing.");
        if (!cue.muted_alternative) result.diagnostics.push_back(cue.id + ": muted alternative is missing.");
        if (cue.end_ms <= cue.start_ms) result.diagnostics.push_back(cue.id + ": caption timing is invalid.");
        const uint32_t caption_duration = cue.end_ms > cue.start_ms ? cue.end_ms - cue.start_ms : 0;
        if (cue.voice_duration_ms > 0 && std::abs(static_cast<int64_t>(caption_duration) - cue.voice_duration_ms) > 250)
            result.diagnostics.push_back(cue.id + ": caption and voice timing differ by more than 250 ms.");
        locales.insert(cue.locale);
    }
    for (const auto& locale : required_locales) if (!locales.contains(locale))
        result.diagnostics.push_back("Required caption locale is missing: " + locale);
    result.complete = result.diagnostics.empty();
    return result;
}

DialogueCaptionTrackBuild buildDialogueCaptionTrack(const dialogue::DialogueGraph& graph,
                                                     const DialogueCaptionTrackInput& input) {
    DialogueCaptionTrackBuild result;
    if (!validId(input.locale) || !validId(input.default_take_id)) {
        result.diagnostics.push_back("Caption track locale and default take ID are required.");
        return result;
    }
    uint32_t sequential_start = 0;
    for (const auto& [id, node] : graph.nodes()) {
        if (node.voice_asset_id.empty() && node.caption_localization_key.empty()) continue;
        CaptionCue cue;
        cue.id = id + "." + input.locale;
        cue.speaker_id = node.speaker_id;
        cue.locale = input.locale;
        cue.take_id = input.default_take_id;
        cue.text = node.text_preview;
        if (!node.caption_localization_key.empty()) {
            if (input.catalog) {
                const auto localized = input.catalog->getKey(node.caption_localization_key);
                if (localized) cue.text = *localized;
                else result.diagnostics.push_back(id + ": caption localization key is missing from the catalog.");
            } else {
                result.diagnostics.push_back(id + ": no locale catalog was provided for the caption key.");
            }
        }
        const auto start = input.start_ms_by_node.find(id);
        cue.start_ms = start == input.start_ms_by_node.end() ? sequential_start : start->second;
        const auto duration = input.voice_duration_ms_by_asset.find(node.voice_asset_id);
        cue.voice_duration_ms = duration == input.voice_duration_ms_by_asset.end() ? 0 : duration->second;
        if (cue.voice_duration_ms == 0)
            result.diagnostics.push_back(id + ": voice duration evidence is missing.");
        cue.end_ms = cue.start_ms + cue.voice_duration_ms;
        sequential_start = cue.end_ms;
        const auto non_speech = input.non_speech_cues_by_node.find(id);
        if (non_speech != input.non_speech_cues_by_node.end()) cue.non_speech_cues = non_speech->second;
        cue.muted_alternative = input.muted_alternative_voice_assets.contains(node.voice_asset_id);
        result.cues.push_back(std::move(cue));
    }
    if (result.cues.empty()) result.diagnostics.push_back("Dialogue graph has no voice or caption references to build.");
    return result;
}

bool SemanticEditorAlternative::addNode(SemanticEditorNode node) {
    if (!validId(node.id) || node.label.empty() || node.kind.empty() ||
        std::any_of(nodes_.begin(), nodes_.end(), [&](const auto& item) { return item.id == node.id; })) return false;
    nodes_.push_back(std::move(node));
    return true;
}

bool SemanticEditorAlternative::updateProperty(std::string_view id, std::string key, std::string value) {
    const auto found = std::find_if(nodes_.begin(), nodes_.end(), [&](const auto& item) { return item.id == id; });
    if (found == nodes_.end() || key.empty()) return false;
    found->properties[std::move(key)] = std::move(value);
    return true;
}

bool SemanticEditorAlternative::connect(std::string_view from, std::string_view to) {
    const auto source = std::find_if(nodes_.begin(), nodes_.end(), [&](const auto& item) { return item.id == from; });
    const auto target = std::find_if(nodes_.begin(), nodes_.end(), [&](const auto& item) { return item.id == to; });
    if (source == nodes_.end() || target == nodes_.end() || from == to) return false;
    if (std::find(source->connections.begin(), source->connections.end(), to) == source->connections.end())
        source->connections.emplace_back(to);
    return true;
}

bool SemanticEditorAlternative::reorder(std::string_view id, int order) {
    const auto found = std::find_if(nodes_.begin(), nodes_.end(), [&](const auto& item) { return item.id == id; });
    if (found == nodes_.end() || order < 0 || order > 100000) return false;
    found->order = order;
    return true;
}

std::vector<SemanticEditorNode> SemanticEditorAlternative::orderedNodes() const {
    auto result = nodes_;
    std::stable_sort(result.begin(), result.end(), [](const auto& left, const auto& right) { return left.order < right.order; });
    return result;
}

std::vector<std::string> SemanticEditorAlternative::diagnostics() const {
    std::vector<std::string> result;
    for (const auto& node : nodes_) for (const auto& connection : node.connections)
        if (std::none_of(nodes_.begin(), nodes_.end(), [&](const auto& item) { return item.id == connection; }))
            result.push_back(node.id + ": connection target is missing: " + connection);
    return result;
}

SemanticEditorAlternative semanticAlternativeForMenu(const ui::MenuAuthoringDocument& document) {
    SemanticEditorAlternative result;
    for (size_t index = 0; index < document.nodes().size(); ++index) {
        const auto& node = document.nodes()[index];
        SemanticEditorNode semantic;
        semantic.id = node.id;
        semantic.label = node.accessible_label.empty() ? (node.label.empty() ? node.id : node.label)
                                                       : node.accessible_label;
        semantic.kind = node.kind == ui::MenuElementKind::Button ? "button"
                      : node.kind == ui::MenuElementKind::Text ? "text"
                      : node.kind == ui::MenuElementKind::Image ? "image"
                      : node.kind == ui::MenuElementKind::Progress ? "progress"
                      : node.kind == ui::MenuElementKind::List ? "list"
                      : node.kind == ui::MenuElementKind::Slot ? "slot" : "panel";
        semantic.order = static_cast<int>(index);
        semantic.properties = {{"visible", node.visible ? "true" : "false"},
                               {"enabled", node.enabled ? "true" : "false"},
                               {"x", std::to_string(node.layout.x)}, {"y", std::to_string(node.layout.y)},
                               {"width", std::to_string(node.layout.width)},
                               {"height", std::to_string(node.layout.height)}};
        (void)result.addNode(std::move(semantic));
    }
    for (const auto& node : document.nodes()) if (!node.parent_id.empty()) (void)result.connect(node.parent_id, node.id);
    return result;
}

SemanticEditorAlternative semanticAlternativeForDialogue(const dialogue::DialogueGraph& graph) {
    SemanticEditorAlternative result;
    int order = 0;
    for (const auto& [id, node] : graph.nodes()) {
        (void)result.addNode({id, node.speaker_name.empty() ? id : node.speaker_name + ": " + node.text_preview,
                             node.ending ? "ending" : "dialogue", order++,
                             {{"speaker_id", node.speaker_id}, {"localization_key", node.localization_key}}, {}});
    }
    for (const auto& [id, node] : graph.nodes())
        for (const auto& choice : node.choices) (void)result.connect(id, choice.target_node_id);
    return result;
}

SemanticEditorAlternative semanticAlternativeForQuest(const quest::QuestObjectiveGraphDocument& document) {
    SemanticEditorAlternative result;
    for (size_t index = 0; index < document.nodes.size(); ++index) {
        const auto& node = document.nodes[index];
        (void)result.addNode({node.id, node.title.empty() ? node.id : node.title, node.type,
                             static_cast<int>(index), {{"objective_id", node.objective_id},
                             {"localization_key", node.localization_key}}, {}});
    }
    for (const auto& link : document.links) (void)result.connect(link.from, link.to);
    return result;
}

SemanticEditorAlternative semanticAlternativeForTileMap(const map::TileLayerDocument& document) {
    SemanticEditorAlternative result;
    for (const auto& layer : document.layers()) {
        (void)result.addNode({layer.id, layer.id, "tile_layer", layer.draw_order,
                             {{"visible", layer.visible ? "true" : "false"},
                              {"locked", layer.locked ? "true" : "false"},
                              {"collision", layer.collision ? "true" : "false"},
                              {"navigation", layer.navigation ? "true" : "false"}}, {}});
    }
    return result;
}

SemanticEditorAlternative semanticAlternativeForWorldMap(const map::ProjectWorldGraph& graph) {
    SemanticEditorAlternative result;
    for (size_t index = 0; index < graph.maps().size(); ++index) {
        const auto& item = graph.maps()[index];
        (void)result.addNode({item.id, item.label.empty() ? item.id : item.label, "world_map",
                             static_cast<int>(index), {}, {}});
    }
    for (const auto& route : graph.routes()) (void)result.connect(route.source_map_id, route.target_map_id);
    return result;
}

std::vector<InclusiveAuditIssue> auditInclusiveUi(const std::vector<InclusiveUiSnapshot>& elements,
                                                 bool touch_declared, bool reduced_motion) {
    std::vector<InclusiveAuditIssue> issues;
    std::set<int> focus_orders;
    for (const auto& item : elements) {
        auto add = [&](std::string code, std::string message) {
            issues.push_back({std::move(code), item.id, std::move(message)});
        };
        if (item.focusable && item.label.empty()) add("missing_label", "Focusable element has no semantic label.");
        if (item.focusable && (item.focus_order < 0 || !focus_orders.insert(item.focus_order).second))
            add("focus_order", "Focusable element has an invalid or duplicate order.");
        if (item.contrast_ratio < 4.5F) add("contrast", "Text contrast is below 4.5:1.");
        if (touch_declared && item.focusable && (item.width < 44 || item.height < 44))
            add("hit_target", "Interactive target is smaller than 44 by 44.");
        if (item.clipped) add("clipping", "Element content is clipped.");
        if (item.localization_overflow) add("localization_overflow", "Localized content overflows its bounds.");
        if (reduced_motion && item.motion_duration_ms > 0 && !item.motion_essential)
            add("unsafe_motion", "Non-essential motion remains enabled in reduced-motion mode.");
    }
    return issues;
}

} // namespace urpg::accessibility
