#pragma once

#include "engine/core/input/input_core.h"

#include <cstdint>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include <nlohmann/json.hpp>

namespace urpg::ui { class MenuAuthoringDocument; }
namespace urpg::dialogue { class DialogueGraph; }
namespace urpg::quest { class QuestObjectiveGraphDocument; }
namespace urpg::map { class TileLayerDocument; class ProjectWorldGraph; }
namespace urpg::localization { class LocaleCatalog; }

namespace urpg::accessibility {

enum class InclusiveDeviceKind { KeyboardMouse, Controller, TouchLike };
enum class InputOwnership { Shared, PlayerOne, PlayerTwo, Unassigned };
enum class InclusiveInputContext { Global, Editor, Runtime, Menu, Gameplay, TextEntry };

struct InclusiveDevice {
    std::string id;
    InclusiveDeviceKind kind = InclusiveDeviceKind::KeyboardMouse;
    InputOwnership owner = InputOwnership::Unassigned;
    bool connected = true;
    float deadzone = 0.15F;
    float axis_scale = 1.0F;
    std::string glyph_profile;
};

struct InclusiveBinding {
    InclusiveInputContext context = InclusiveInputContext::Global;
    input::InputAction action = input::InputAction::None;
    std::vector<std::string> controls;
    uint32_t repeat_delay_ms = 350;
    uint32_t repeat_interval_ms = 80;
};

struct InputConflict {
    InclusiveInputContext context = InclusiveInputContext::Global;
    input::InputAction existing = input::InputAction::None;
    input::InputAction requested = input::InputAction::None;
    std::vector<std::string> controls;
};

struct InputRouteResult {
    bool accepted = false;
    bool active_device_changed = false;
    input::InputAction action = input::InputAction::None;
    float normalized_value = 0.0F;
    std::string glyph;
    std::vector<std::string> diagnostics;
};

class UnifiedInputRouter {
public:
    bool connectDevice(InclusiveDevice device);
    bool disconnectDevice(std::string_view id);
    bool reconnectDevice(std::string_view id);
    bool setOwnership(std::string_view id, InputOwnership owner);
    bool calibrate(std::string_view id, float deadzone, float axis_scale);
    std::optional<InputConflict> conflictFor(const InclusiveBinding& binding) const;
    bool bind(InclusiveBinding binding, bool replace_conflict);
    void clearBindings() { bindings_.clear(); }
    InputRouteResult route(std::string_view device_id, InclusiveInputContext context,
                           const std::vector<std::string>& held_controls, float raw_axis,
                           uint64_t held_ms,
                           std::optional<InputOwnership> requested_owner = std::nullopt) const;
    InputRouteResult dispatch(input::InputCore& core, std::string_view device_id,
                              InclusiveInputContext context,
                              const std::vector<std::string>& controls,
                              input::ActionState state, float raw_axis, uint64_t held_ms,
                              std::optional<InputOwnership> requested_owner = std::nullopt) const;
    std::string activeDeviceId() const { return active_device_id_; }
    std::vector<std::string> recoveryActions() const;
    const std::vector<InclusiveBinding>& bindings() const { return bindings_; }

private:
    std::map<std::string, InclusiveDevice> devices_;
    std::vector<InclusiveBinding> bindings_;
    mutable std::string active_device_id_;
};

enum class TextScaleProfile { Compact, Standard, Large, ExtraLarge };
enum class ColorFilter { None, Protanopia, Deuteranopia, Tritanopia, Monochrome };

struct InclusiveSettings {
    TextScaleProfile text_scale = TextScaleProfile::Standard;
    bool high_contrast = false;
    ColorFilter color_filter = ColorFilter::None;
    bool non_color_cues = true;
    bool reduced_motion = false;
    float screen_shake = 1.0F;
    float flash_intensity = 1.0F;
    bool subtitles = true;
    bool captions = true;
    float caption_scale = 1.0F;
    float master_volume = 1.0F;
    float music_volume = 1.0F;
    float effects_volume = 1.0F;
    float voice_volume = 1.0F;
    bool mono_audio = false;

    bool isValid() const;
    nlohmann::json toJson() const;
    static std::optional<InclusiveSettings> fromJson(const nlohmann::json& json);
    static InclusiveSettings safeDefaults();
};

enum class TextDirection { LeftToRight, RightToLeft };
enum class GrammarVariant { Neutral, Masculine, Feminine, Other };

struct LocaleProfile {
    std::string id;
    std::string fallback_id;
    TextDirection direction = TextDirection::LeftToRight;
    std::string font_profile;
    std::set<char32_t> glyphs;
    bool ime_supported = false;
};

struct LocalizedVariant {
    std::string key;
    std::string locale;
    std::string text;
    std::string plural = "other";
    GrammarVariant grammar = GrammarVariant::Neutral;
    uint32_t revision = 1;
};

struct LocalizationResolution {
    bool valid = false;
    bool used_fallback = false;
    TextDirection direction = TextDirection::LeftToRight;
    std::string text;
    std::vector<std::string> diagnostics;
};

class InclusiveLocalizationModel {
public:
    bool defineLocale(LocaleProfile locale);
    bool addVariant(LocalizedVariant variant);
    LocalizationResolution resolve(std::string_view key, std::string_view locale, int64_t count,
                                   GrammarVariant grammar) const;
    std::string pluralCategory(std::string_view locale, int64_t count) const;
    std::vector<std::string> staleKeys(uint32_t source_revision) const;
    std::vector<std::string> missingGlyphs(std::string_view locale, std::u32string_view text) const;
    std::string formatNumber(std::string_view locale, double value) const;
    std::string formatDate(std::string_view locale, int year, int month, int day) const;
    bool supportsIme(std::string_view locale) const;
    bool importCatalog(const localization::LocaleCatalog& catalog, std::string fallback_id,
                       TextDirection direction, std::set<char32_t> glyphs,
                       bool ime_supported, uint32_t revision);
    bool importCatalog(const localization::LocaleCatalog& catalog, std::set<char32_t> glyphs);

private:
    std::map<std::string, LocaleProfile> locales_;
    std::vector<LocalizedVariant> variants_;
};

struct CaptionCue {
    std::string id;
    std::string speaker_id;
    std::string locale;
    std::string take_id;
    std::string text;
    std::vector<std::string> non_speech_cues;
    uint32_t start_ms = 0;
    uint32_t end_ms = 0;
    uint32_t voice_duration_ms = 0;
    bool muted_alternative = false;
};

struct CaptionAuditResult {
    bool complete = false;
    std::vector<std::string> diagnostics;
};

CaptionAuditResult auditCaptionTrack(const std::vector<CaptionCue>& cues,
                                     const std::set<std::string>& required_locales);

struct DialogueCaptionTrackInput {
    std::string locale;
    std::string default_take_id;
    const localization::LocaleCatalog* catalog = nullptr;
    std::map<std::string, uint32_t> start_ms_by_node;
    std::map<std::string, uint32_t> voice_duration_ms_by_asset;
    std::map<std::string, std::vector<std::string>> non_speech_cues_by_node;
    std::set<std::string> muted_alternative_voice_assets;
};

struct DialogueCaptionTrackBuild {
    std::vector<CaptionCue> cues;
    std::vector<std::string> diagnostics;
};

DialogueCaptionTrackBuild buildDialogueCaptionTrack(const dialogue::DialogueGraph& graph,
                                                     const DialogueCaptionTrackInput& input);

struct SemanticEditorNode {
    std::string id;
    std::string label;
    std::string kind;
    int order = 0;
    std::map<std::string, std::string> properties;
    std::vector<std::string> connections;
};

class SemanticEditorAlternative {
public:
    bool addNode(SemanticEditorNode node);
    bool updateProperty(std::string_view id, std::string key, std::string value);
    bool connect(std::string_view from, std::string_view to);
    bool reorder(std::string_view id, int order);
    std::vector<SemanticEditorNode> orderedNodes() const;
    std::vector<std::string> diagnostics() const;

private:
    std::vector<SemanticEditorNode> nodes_;
};

SemanticEditorAlternative semanticAlternativeForMenu(const ui::MenuAuthoringDocument& document);
SemanticEditorAlternative semanticAlternativeForDialogue(const dialogue::DialogueGraph& graph);
SemanticEditorAlternative semanticAlternativeForQuest(const quest::QuestObjectiveGraphDocument& document);
SemanticEditorAlternative semanticAlternativeForTileMap(const map::TileLayerDocument& document);
SemanticEditorAlternative semanticAlternativeForWorldMap(const map::ProjectWorldGraph& graph);

struct InclusiveUiSnapshot {
    std::string id;
    std::string label;
    bool focusable = false;
    int focus_order = -1;
    float contrast_ratio = 0.0F;
    int width = 0;
    int height = 0;
    bool clipped = false;
    bool localization_overflow = false;
    uint32_t motion_duration_ms = 0;
    bool motion_essential = false;
};

struct InclusiveAuditIssue {
    std::string code;
    std::string object_id;
    std::string message;
};

std::vector<InclusiveAuditIssue> auditInclusiveUi(const std::vector<InclusiveUiSnapshot>& elements,
                                                 bool touch_declared, bool reduced_motion);

} // namespace urpg::accessibility
