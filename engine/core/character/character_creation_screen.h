#pragma once

#include "engine/core/character/character_identity_catalog.h"
#include "engine/core/character/character_identity_system.h"
#include "engine/core/input/input_core.h"

#include <functional>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace urpg {
class ActorManager;
class SpriteBatcher;
class World;
} // namespace urpg

namespace urpg::character {

enum class CharacterCreationEventType : uint8_t {
    StepChanged = 0,
    SelectionChanged = 1,
    CreationCompleted = 2,
    CreationCancelled = 3,
    ValidationFailed = 4,
};

struct CharacterCreationEvent {
    CharacterCreationEventType type = CharacterCreationEventType::SelectionChanged;
    std::string activeStep;
    std::string detail;
    EntityID entity = 0;
};

/**
 * @brief Bounded runtime Create-a-Character screen with a live preview card.
 */
class CharacterCreationScreen {
public:
    enum class Step : uint8_t {
        Name = 0,
        Class = 1,
        Portrait = 2,
        Body = 3,
        Appearance = 4,
        Review = 5,
    };

    CharacterCreationScreen();

    void setName(const std::string& value);
    const CharacterIdentity& draftIdentity() const { return m_identity; }
    Step activeStep() const { return m_activeStep; }
    const std::optional<EntityID>& lastCreatedEntity() const { return m_lastCreatedEntity; }

    void handleInput(const urpg::input::InputCore& input, urpg::World& world, urpg::ActorManager& actorManager);
    void draw(urpg::SpriteBatcher& batcher);

    nlohmann::json buildSnapshot() const;
    std::vector<CharacterCreationEvent> consumeEvents();

    void setCompletionHandler(std::function<void(EntityID, const CharacterIdentity&)> handler) {
        m_completionHandler = std::move(handler);
    }

private:
    void cycleStep(int delta);
    void cycleCurrentSelection(int delta);
    void toggleHighlightedAppearance();
    void finalizeCreation(urpg::World& world, urpg::ActorManager& actorManager);

    void submitText(const std::string& text, float x, float y, int32_t zOrder) const;
    void submitRect(float x, float y, float w, float h, float r, float g, float b, float a, int32_t zOrder) const;
    void submitAttributeBar(const std::string& label, float value, float x, float y, float width, int32_t zOrder) const;

    void syncClassSelection(size_t presetIndex);
    void syncNameSelection(size_t nameIndex);
    void pushEvent(CharacterCreationEventType type, std::string detail, EntityID entity = 0);

    nlohmann::json buildValidationSnapshot() const;
    nlohmann::json buildPreviewSnapshot() const;
    std::string buildAppearanceSummary() const;
    std::string stepLabel(Step step) const;
    std::string stepValue(Step step) const;
    float attributeTotal() const;
    std::string primaryAttribute() const;
    const CharacterClassPreset* activeClassPreset() const;

    const CharacterIdentityCatalog& m_catalog;
    const std::vector<CharacterClassPreset>& m_presets;
    const std::vector<std::string>& m_nameSuggestions;

    CharacterIdentity m_identity;
    Step m_activeStep = Step::Name;
    size_t m_nameIndex = 0;
    size_t m_classIndex = 0;
    size_t m_portraitIndex = 0;
    size_t m_bodyIndex = 0;
    size_t m_appearanceIndex = 0;

    std::vector<CharacterCreationEvent> m_events;
    std::optional<EntityID> m_lastCreatedEntity;
    std::function<void(EntityID, const CharacterIdentity&)> m_completionHandler;
};

} // namespace urpg::character
