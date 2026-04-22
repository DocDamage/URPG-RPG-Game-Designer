#pragma once

#include "engine/core/character/character_identity.h"
#include "engine/core/character/character_identity_system.h"
#include "engine/core/ecs/world.h"
#include "engine/core/math/fixed32.h"
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace urpg::editor {

/**
 * @brief Editor model that owns a CharacterIdentity being edited.
 */
class CharacterCreatorModel {
public:
    CharacterCreatorModel();

    void loadIdentity(const urpg::character::CharacterIdentity& identity);
    void resetDraft();

    const urpg::character::CharacterIdentity& getIdentity() const { return m_identity; }
    std::optional<EntityID> lastSpawnedEntity() const { return m_last_spawned_entity; }

    void setName(const std::string& value);
    void setPortraitId(const std::string& value);
    void setBodySpriteId(const std::string& value);
    void setClassId(const std::string& value);
    void setBaseAttribute(const std::string& key, float value);
    void applyClassPreset(const std::string& classId);
    void addAppearanceToken(const std::string& token);
    void removeAppearanceToken(const std::string& token);
    void setSpawnPosition(urpg::Fixed32 x, urpg::Fixed32 y, urpg::Fixed32 z = urpg::Fixed32::FromInt(0));
    void setSpawnEnemyFlag(bool is_enemy);

    urpg::CharacterSpawner::Request buildSpawnRequest() const;
    urpg::CharacterSpawner::Result spawnCharacter(urpg::World& world, class ActorManager& actor_manager);

    /** @brief Builds a snapshot JSON of the current identity plus dirty state. */
    nlohmann::json buildSnapshot() const;

private:
    nlohmann::json buildValidationSnapshot() const;
    nlohmann::json buildPreviewSnapshot() const;
    nlohmann::json buildCatalogSnapshot() const;

    urpg::character::CharacterIdentity m_identity;
    bool m_dirty = false;
    urpg::Fixed32 m_spawn_x = urpg::Fixed32::FromInt(0);
    urpg::Fixed32 m_spawn_y = urpg::Fixed32::FromInt(0);
    urpg::Fixed32 m_spawn_z = urpg::Fixed32::FromInt(0);
    bool m_spawn_is_enemy = false;
    std::optional<EntityID> m_last_spawned_entity;
    std::vector<std::string> m_known_class_ids;
    std::vector<std::string> m_known_portrait_ids;
    std::vector<std::string> m_known_body_sprite_ids;
    std::vector<std::string> m_known_appearance_tokens;
};

} // namespace urpg::editor
