#include "engine/core/character/character_creation_screen.h"

#include "engine/core/character/character_creation_rules.h"
#include "engine/core/character/character_identity_validator.h"
#include "engine/core/ecs/actor_manager.h"
#include "engine/core/render/render_layer.h"

#include <algorithm>
#include <sstream>
#include <utility>

namespace urpg::character {

namespace {

std::string issueCode(CharacterIdentityIssueCategory category) {
    switch (category) {
    case CharacterIdentityIssueCategory::MissingName:
        return "missing_name";
    case CharacterIdentityIssueCategory::MissingClass:
        return "missing_class";
    case CharacterIdentityIssueCategory::UnknownClass:
        return "unknown_class";
    case CharacterIdentityIssueCategory::UnknownPortrait:
        return "unknown_portrait";
    case CharacterIdentityIssueCategory::UnknownBodySprite:
        return "unknown_body_sprite";
    case CharacterIdentityIssueCategory::UnknownAppearanceToken:
        return "unknown_appearance_token";
    case CharacterIdentityIssueCategory::DuplicateAppearanceToken:
        return "duplicate_appearance_token";
    case CharacterIdentityIssueCategory::CreationRuleViolation:
        return "creation_rule_violation";
    }

    return "unknown_issue";
}

const std::vector<std::string>& orderedAttributeKeys() {
    static const std::vector<std::string> kKeys = {"STR", "VIT", "INT", "AGI"};
    return kKeys;
}

template <typename T>
size_t wrapIndex(size_t current, int delta, const std::vector<T>& values) {
    if (values.empty()) {
        return 0;
    }
    const auto size = static_cast<int>(values.size());
    int next = static_cast<int>(current) + delta;
    while (next < 0) {
        next += size;
    }
    return static_cast<size_t>(next % size);
}

size_t wrapStep(size_t current, int delta, size_t stepCount) {
    const auto size = static_cast<int>(stepCount);
    int next = static_cast<int>(current) + delta;
    while (next < 0) {
        next += size;
    }
    return static_cast<size_t>(next % size);
}

} // namespace

CharacterCreationScreen::CharacterCreationScreen()
    : m_catalog(defaultCharacterIdentityCatalog()),
      m_presets(defaultCharacterClassPresets()),
      m_nameSuggestions(defaultCharacterNameSuggestions()) {
    if (!m_nameSuggestions.empty()) {
        syncNameSelection(0);
    }
    if (!m_presets.empty()) {
        syncClassSelection(0);
    }
}

void CharacterCreationScreen::setName(const std::string& value) {
    m_identity.setName(value);
    const auto it = std::find(m_nameSuggestions.begin(), m_nameSuggestions.end(), value);
    if (it != m_nameSuggestions.end()) {
        m_nameIndex = static_cast<size_t>(std::distance(m_nameSuggestions.begin(), it));
    }
    pushEvent(CharacterCreationEventType::SelectionChanged, "Character name updated.");
}

void CharacterCreationScreen::handleInput(const urpg::input::InputCore& input,
                                          urpg::World& world,
                                          urpg::ActorManager& actorManager) {
    if (input.isActionJustPressed(urpg::input::InputAction::MoveUp)) {
        cycleStep(-1);
        return;
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveDown)) {
        cycleStep(+1);
        return;
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveLeft)) {
        cycleCurrentSelection(-1);
        return;
    }
    if (input.isActionJustPressed(urpg::input::InputAction::MoveRight)) {
        cycleCurrentSelection(+1);
        return;
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
        if (m_activeStep == Step::Appearance) {
            toggleHighlightedAppearance();
            return;
        }
        if (m_activeStep == Step::Review) {
            finalizeCreation(world, actorManager);
            return;
        }
        cycleStep(+1);
        return;
    }
    if (input.isActionJustPressed(urpg::input::InputAction::Cancel)) {
        if (m_activeStep == Step::Name) {
            pushEvent(CharacterCreationEventType::CreationCancelled,
                      "Character creation cancelled before finalization.");
            return;
        }
        cycleStep(-1);
    }
}

void CharacterCreationScreen::draw(urpg::SpriteBatcher& batcher) {
    (void)batcher;

    const auto* preset = activeClassPreset();
    const auto themeR = preset ? preset->previewR : 0.25f;
    const auto themeG = preset ? preset->previewG : 0.25f;
    const auto themeB = preset ? preset->previewB : 0.25f;

    submitRect(16.0f, 16.0f, 608.0f, 332.0f, 0.07f, 0.08f, 0.11f, 0.96f, 1);
    submitRect(28.0f, 52.0f, 250.0f, 268.0f, 0.11f, 0.13f, 0.18f, 0.98f, 2);
    submitRect(292.0f, 52.0f, 320.0f, 268.0f, 0.10f, 0.12f, 0.16f, 0.98f, 2);
    submitRect(292.0f, 52.0f, 320.0f, 40.0f, themeR, themeG, themeB, 0.50f, 3);

    submitText("Create Character", 34.0f, 24.0f, 4);
    submitText("Preset Runtime Workflow", 390.0f, 64.0f, 4);

    float rowY = 76.0f;
    for (size_t i = 0; i <= static_cast<size_t>(Step::Review); ++i) {
        const auto step = static_cast<Step>(i);
        if (step == m_activeStep) {
            submitRect(36.0f, rowY - 4.0f, 232.0f, 28.0f, themeR, themeG, themeB, 0.35f, 3);
        }
        submitText(stepLabel(step) + ": " + stepValue(step), 44.0f, rowY, 4);
        rowY += 34.0f;
    }

    submitText("Preview Card", 312.0f, 102.0f, 4);
    submitRect(312.0f, 126.0f, 92.0f, 92.0f, themeR, themeG, themeB, 0.60f, 3);
    submitRect(424.0f, 126.0f, 160.0f, 92.0f, 0.18f, 0.20f, 0.24f, 1.0f, 3);

    submitText("Portrait", 332.0f, 164.0f, 4);
    submitText(m_identity.getPortraitId().empty() ? "portrait_unset" : m_identity.getPortraitId(), 316.0f, 192.0f, 4);
    submitText("Body Sprite", 444.0f, 144.0f, 4);
    submitText(m_identity.getBodySpriteId().empty() ? "body_unset" : m_identity.getBodySpriteId(), 444.0f, 172.0f, 4);
    submitText("Name: " + m_identity.getDisplayName(), 312.0f, 234.0f, 4);
    submitText("Class: " + m_identity.getClassId(), 312.0f, 258.0f, 4);
    submitText("Tokens: " + buildAppearanceSummary(), 312.0f, 282.0f, 4);

    float barY = 304.0f;
    for (const auto& key : orderedAttributeKeys()) {
        submitAttributeBar(key, m_identity.getAttribute(key), 312.0f, barY, 240.0f, 4);
        barY += 18.0f;
    }

    submitText("Confirm: advance / toggle / finalize", 34.0f, 328.0f, 4);
    submitText("Cancel: go back", 332.0f, 328.0f, 4);
}

nlohmann::json CharacterCreationScreen::buildSnapshot() const {
    const auto validation = buildValidationSnapshot();
    return {
        {"identity", m_identity.toJson()},
        {"validation", validation},
        {"preview", buildPreviewSnapshot()},
        {"workflow", {
            {"active_step", stepLabel(m_activeStep)},
            {"can_finalize", validation.value("is_valid", false)},
            {"last_created_entity", m_lastCreatedEntity.has_value()
                                        ? nlohmann::json(*m_lastCreatedEntity)
                                        : nlohmann::json(nullptr)},
        }},
        {"catalog", {
            {"class_ids", m_catalog.classIds},
            {"portrait_ids", m_catalog.portraitIds},
            {"body_sprite_ids", m_catalog.bodySpriteIds},
            {"appearance_tokens", m_catalog.appearanceTokens},
            {"name_suggestions", m_nameSuggestions},
        }},
        {"creation_rules", characterCreationRulesToJson(defaultCharacterCreationRules())},
    };
}

std::vector<CharacterCreationEvent> CharacterCreationScreen::consumeEvents() {
    auto events = std::move(m_events);
    m_events.clear();
    return events;
}

void CharacterCreationScreen::cycleStep(int delta) {
    const auto next = wrapStep(static_cast<size_t>(m_activeStep), delta, static_cast<size_t>(Step::Review) + 1);
    m_activeStep = static_cast<Step>(next);
    pushEvent(CharacterCreationEventType::StepChanged, "Active character creation step changed.");
}

void CharacterCreationScreen::cycleCurrentSelection(int delta) {
    if (m_activeStep == Step::Review) {
        return;
    }

    switch (m_activeStep) {
    case Step::Name:
        if (!m_nameSuggestions.empty()) {
            syncNameSelection(wrapIndex(m_nameIndex, delta, m_nameSuggestions));
        }
        break;
    case Step::Class:
        if (!m_presets.empty()) {
            syncClassSelection(wrapIndex(m_classIndex, delta, m_presets));
        }
        break;
    case Step::Portrait:
        if (!m_catalog.portraitIds.empty()) {
            m_portraitIndex = wrapIndex(m_portraitIndex, delta, m_catalog.portraitIds);
            m_identity.setPortraitId(m_catalog.portraitIds[m_portraitIndex]);
        }
        break;
    case Step::Body:
        if (!m_catalog.bodySpriteIds.empty()) {
            m_bodyIndex = wrapIndex(m_bodyIndex, delta, m_catalog.bodySpriteIds);
            m_identity.setBodySpriteId(m_catalog.bodySpriteIds[m_bodyIndex]);
        }
        break;
    case Step::Appearance:
        if (!m_catalog.appearanceTokens.empty()) {
            m_appearanceIndex = wrapIndex(m_appearanceIndex, delta, m_catalog.appearanceTokens);
        }
        break;
    case Step::Review:
        break;
    }

    pushEvent(CharacterCreationEventType::SelectionChanged,
              "Character creation selection changed for the active step.");
}

void CharacterCreationScreen::toggleHighlightedAppearance() {
    if (m_catalog.appearanceTokens.empty()) {
        return;
    }

    const auto& token = m_catalog.appearanceTokens[m_appearanceIndex];
    const auto& activeTokens = m_identity.getAppearanceTokens();
    const bool alreadySelected =
        std::find(activeTokens.begin(), activeTokens.end(), token) != activeTokens.end();
    if (alreadySelected) {
        m_identity.removeAppearanceToken(token);
        pushEvent(CharacterCreationEventType::SelectionChanged,
                  "Appearance token removed from the runtime preview.");
        return;
    }

    m_identity.addAppearanceToken(token);
    pushEvent(CharacterCreationEventType::SelectionChanged,
              "Appearance token added to the runtime preview.");
}

void CharacterCreationScreen::finalizeCreation(urpg::World& world, urpg::ActorManager& actorManager) {
    const auto validation = buildValidationSnapshot();
    if (!validation.value("is_valid", false)) {
        pushEvent(CharacterCreationEventType::ValidationFailed,
                  "Character creation is not valid yet.");
        return;
    }

    urpg::CharacterSpawner::Request request;
    request.identity = m_identity;

    const auto result = urpg::CharacterSpawner::spawn(world, actorManager, request);
    if (!result.success) {
        pushEvent(CharacterCreationEventType::ValidationFailed,
                  "Character creation spawn request failed.");
        return;
    }

    m_lastCreatedEntity = result.entity;
    pushEvent(CharacterCreationEventType::CreationCompleted,
              "Character creation finalized and actor spawned.",
              result.entity);
    if (m_completionHandler) {
        m_completionHandler(result.entity, m_identity);
    }
}

void CharacterCreationScreen::submitText(const std::string& text, float x, float y, int32_t zOrder) const {
    urpg::TextCommand command;
    command.text = text;
    command.x = x;
    command.y = y;
    command.zOrder = zOrder;
    urpg::RenderLayer::getInstance().submit(command);
}

void CharacterCreationScreen::submitRect(float x,
                                         float y,
                                         float w,
                                         float h,
                                         float r,
                                         float g,
                                         float b,
                                         float a,
                                         int32_t zOrder) const {
    urpg::RectCommand command;
    command.x = x;
    command.y = y;
    command.w = w;
    command.h = h;
    command.r = r;
    command.g = g;
    command.b = b;
    command.a = a;
    command.zOrder = zOrder;
    urpg::RenderLayer::getInstance().submit(command);
}

void CharacterCreationScreen::submitAttributeBar(const std::string& label,
                                                 float value,
                                                 float x,
                                                 float y,
                                                 float width,
                                                 int32_t zOrder) const {
    const auto* preset = activeClassPreset();
    const float themeR = preset ? preset->previewR : 0.25f;
    const float themeG = preset ? preset->previewG : 0.25f;
    const float themeB = preset ? preset->previewB : 0.25f;

    submitText(label, x, y, zOrder);
    submitRect(x + 44.0f, y + 4.0f, width, 8.0f, 0.18f, 0.20f, 0.24f, 1.0f, zOrder - 1);

    const float fill = std::clamp(value / 20.0f, 0.0f, 1.0f) * width;
    submitRect(x + 44.0f, y + 4.0f, fill, 8.0f, themeR, themeG, themeB, 1.0f, zOrder);
}

void CharacterCreationScreen::syncClassSelection(size_t presetIndex) {
    if (m_presets.empty()) {
        return;
    }

    m_classIndex = presetIndex;
    const auto& preset = m_presets[m_classIndex];
    applyCharacterClassPreset(m_identity, preset.id, true, true);

    const auto portraitIt = std::find(m_catalog.portraitIds.begin(), m_catalog.portraitIds.end(), preset.portraitId);
    if (portraitIt != m_catalog.portraitIds.end()) {
        m_portraitIndex = static_cast<size_t>(std::distance(m_catalog.portraitIds.begin(), portraitIt));
    }

    const auto bodyIt = std::find(m_catalog.bodySpriteIds.begin(), m_catalog.bodySpriteIds.end(), preset.bodySpriteId);
    if (bodyIt != m_catalog.bodySpriteIds.end()) {
        m_bodyIndex = static_cast<size_t>(std::distance(m_catalog.bodySpriteIds.begin(), bodyIt));
    }

    m_appearanceIndex = 0;
}

void CharacterCreationScreen::syncNameSelection(size_t nameIndex) {
    if (m_nameSuggestions.empty()) {
        return;
    }

    m_nameIndex = nameIndex;
    m_identity.setName(m_nameSuggestions[m_nameIndex]);
}

void CharacterCreationScreen::pushEvent(CharacterCreationEventType type, std::string detail, EntityID entity) {
    m_events.push_back({type, stepLabel(m_activeStep), std::move(detail), entity});
}

nlohmann::json CharacterCreationScreen::buildValidationSnapshot() const {
    CharacterIdentityValidator validator;
    const auto issues = validator.validate(m_identity, m_catalog);

    nlohmann::json issueRows = nlohmann::json::array();
    for (const auto& issue : issues) {
        issueRows.push_back({
            {"field", issue.field},
            {"code", issueCode(issue.category)},
            {"message", issue.message},
        });
    }

    return {
        {"is_valid", issues.empty()},
        {"issue_count", issueRows.size()},
        {"issues", issueRows},
    };
}

nlohmann::json CharacterCreationScreen::buildPreviewSnapshot() const {
    const auto* preset = activeClassPreset();
    return {
        {"display_name", m_identity.getDisplayName()},
        {"class_id", m_identity.getClassId()},
        {"portrait_id", m_identity.getPortraitId()},
        {"body_sprite_id", m_identity.getBodySpriteId()},
        {"primary_attribute", primaryAttribute()},
        {"attribute_total", attributeTotal()},
        {"appearance_tokens", m_identity.getAppearanceTokens()},
        {"highlighted_appearance_token",
         m_catalog.appearanceTokens.empty() ? nlohmann::json(nullptr)
                                            : nlohmann::json(m_catalog.appearanceTokens[m_appearanceIndex])},
        {"preview_theme", {
            {"r", preset ? preset->previewR : 0.25f},
            {"g", preset ? preset->previewG : 0.25f},
            {"b", preset ? preset->previewB : 0.25f},
        }},
    };
}

std::string CharacterCreationScreen::buildAppearanceSummary() const {
    const auto& tokens = m_identity.getAppearanceTokens();
    if (tokens.empty()) {
        return "none";
    }

    std::ostringstream stream;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0) {
            stream << ", ";
        }
        stream << tokens[i];
    }
    return stream.str();
}

std::string CharacterCreationScreen::stepLabel(Step step) const {
    switch (step) {
    case Step::Name:
        return "Name";
    case Step::Class:
        return "Class";
    case Step::Portrait:
        return "Portrait";
    case Step::Body:
        return "Body";
    case Step::Appearance:
        return "Appearance";
    case Step::Review:
        return "Review";
    }

    return "Unknown";
}

std::string CharacterCreationScreen::stepValue(Step step) const {
    switch (step) {
    case Step::Name:
        return m_identity.getDisplayName();
    case Step::Class:
        return m_identity.getClassId();
    case Step::Portrait:
        return m_identity.getPortraitId();
    case Step::Body:
        return m_identity.getBodySpriteId();
    case Step::Appearance:
        if (m_catalog.appearanceTokens.empty()) {
            return "none";
        }
        return m_catalog.appearanceTokens[m_appearanceIndex];
    case Step::Review:
        return buildValidationSnapshot().value("is_valid", false) ? "Ready" : "Needs fixes";
    }

    return {};
}

float CharacterCreationScreen::attributeTotal() const {
    float total = 0.0f;
    for (const auto& key : orderedAttributeKeys()) {
        total += m_identity.getAttribute(key);
    }
    return total;
}

std::string CharacterCreationScreen::primaryAttribute() const {
    std::string bestKey;
    float bestValue = -1.0f;
    for (const auto& key : orderedAttributeKeys()) {
        const float value = m_identity.getAttribute(key);
        if (value > bestValue) {
            bestValue = value;
            bestKey = key;
        }
    }
    return bestKey;
}

const CharacterClassPreset* CharacterCreationScreen::activeClassPreset() const {
    return findCharacterClassPreset(m_identity.getClassId());
}

} // namespace urpg::character
