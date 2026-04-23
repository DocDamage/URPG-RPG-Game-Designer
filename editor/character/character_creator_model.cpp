#include "editor/character/character_creator_model.h"

#include "engine/core/character/character_identity_catalog.h"
#include "engine/core/character/character_identity_validator.h"
#include "engine/core/character/character_identity_system.h"
#include "engine/core/ecs/actor_manager.h"

#include <algorithm>

namespace urpg::editor {

namespace {

bool containsValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
}

std::string issueCode(urpg::character::CharacterIdentityIssueCategory category) {
    using urpg::character::CharacterIdentityIssueCategory;

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
    }

    return "unknown_issue";
}

float attributeTotal(const urpg::character::CharacterIdentity& identity) {
    float total = 0.0f;
    for (const auto& [key, value] : identity.getBaseAttributes()) {
        (void)key;
        total += value;
    }
    return total;
}

std::string primaryAttribute(const urpg::character::CharacterIdentity& identity) {
    std::string best_key;
    float best_value = -1.0f;
    for (const auto& [key, value] : identity.getBaseAttributes()) {
        if (value > best_value) {
            best_key = key;
            best_value = value;
        }
    }
    return best_key;
}

} // namespace

CharacterCreatorModel::CharacterCreatorModel() {
    const auto& catalog = urpg::character::defaultCharacterIdentityCatalog();
    m_known_class_ids = catalog.classIds;
    m_known_portrait_ids = catalog.portraitIds;
    m_known_body_sprite_ids = catalog.bodySpriteIds;
    m_known_appearance_tokens = catalog.appearanceTokens;
}

void CharacterCreatorModel::loadIdentity(const urpg::character::CharacterIdentity& identity) {
    m_identity = identity;
    m_dirty = false;
    m_last_spawned_entity.reset();
}

void CharacterCreatorModel::resetDraft() {
    m_identity = urpg::character::CharacterIdentity{};
    m_spawn_x = urpg::Fixed32::FromInt(0);
    m_spawn_y = urpg::Fixed32::FromInt(0);
    m_spawn_z = urpg::Fixed32::FromInt(0);
    m_spawn_is_enemy = false;
    m_last_spawned_entity.reset();
    m_dirty = false;
}

void CharacterCreatorModel::setName(const std::string& value) {
    m_identity.setName(value);
    m_dirty = true;
}

void CharacterCreatorModel::setPortraitId(const std::string& value) {
    m_identity.setPortraitId(value);
    m_dirty = true;
}

void CharacterCreatorModel::setBodySpriteId(const std::string& value) {
    m_identity.setBodySpriteId(value);
    m_dirty = true;
}

void CharacterCreatorModel::setClassId(const std::string& value) {
    m_identity.setClassId(value);
    m_dirty = true;
}

void CharacterCreatorModel::setBaseAttribute(const std::string& key, float value) {
    m_identity.setAttribute(key, value);
    m_dirty = true;
}

void CharacterCreatorModel::applyClassPreset(const std::string& classId) {
    urpg::character::applyCharacterClassPreset(m_identity, classId, true, true);
    m_dirty = true;
}

void CharacterCreatorModel::addAppearanceToken(const std::string& token) {
    if (containsValue(m_identity.getAppearanceTokens(), token)) {
        return;
    }
    m_identity.addAppearanceToken(token);
    m_dirty = true;
}

void CharacterCreatorModel::removeAppearanceToken(const std::string& token) {
    m_identity.removeAppearanceToken(token);
    m_dirty = true;
}

void CharacterCreatorModel::setSpawnPosition(urpg::Fixed32 x, urpg::Fixed32 y, urpg::Fixed32 z) {
    m_spawn_x = x;
    m_spawn_y = y;
    m_spawn_z = z;
    m_dirty = true;
}

void CharacterCreatorModel::setSpawnEnemyFlag(bool is_enemy) {
    m_spawn_is_enemy = is_enemy;
    m_dirty = true;
}

urpg::CharacterSpawner::Request CharacterCreatorModel::buildSpawnRequest() const {
    urpg::CharacterSpawner::Request request;
    request.identity = m_identity;
    request.x = m_spawn_x;
    request.y = m_spawn_y;
    request.z = m_spawn_z;
    request.isEnemy = m_spawn_is_enemy;
    return request;
}

urpg::CharacterSpawner::Result CharacterCreatorModel::spawnCharacter(urpg::World& world,
                                                                     ActorManager& actor_manager) {
    const auto validation = buildValidationSnapshot();
    if (!validation.value("is_valid", false)) {
        return {};
    }

    auto result = urpg::CharacterSpawner::spawn(world, actor_manager, buildSpawnRequest());
    if (result.success) {
        m_last_spawned_entity = result.entity;
        m_dirty = false;
    }
    return result;
}

nlohmann::json CharacterCreatorModel::buildValidationSnapshot() const {
    urpg::character::CharacterIdentityCatalog catalog;
    catalog.classIds = m_known_class_ids;
    catalog.portraitIds = m_known_portrait_ids;
    catalog.bodySpriteIds = m_known_body_sprite_ids;
    catalog.appearanceTokens = m_known_appearance_tokens;

    const urpg::character::CharacterIdentityValidator validator;
    const auto validationIssues = validator.validate(m_identity, catalog);

    nlohmann::json issues = nlohmann::json::array();
    for (const auto& issue : validationIssues) {
        issues.push_back({
            {"field", issue.field},
            {"code", issueCode(issue.category)},
            {"message", issue.message},
        });
    }

    return {
        {"is_valid", issues.empty()},
        {"issue_count", issues.size()},
        {"issues", issues}
    };
}

nlohmann::json CharacterCreatorModel::buildPreviewSnapshot() const {
    return {
        {"display_name", m_identity.getDisplayName()},
        {"class_id", m_identity.getClassId()},
        {"portrait_id", m_identity.getPortraitId()},
        {"body_sprite_id", m_identity.getBodySpriteId()},
        {"primary_attribute", primaryAttribute(m_identity)},
        {"attribute_total", attributeTotal(m_identity)},
        {"appearance_tokens", m_identity.getAppearanceTokens()}
    };
}

nlohmann::json CharacterCreatorModel::buildCatalogSnapshot() const {
    return {
        {"class_ids", m_known_class_ids},
        {"portrait_ids", m_known_portrait_ids},
        {"body_sprite_ids", m_known_body_sprite_ids},
        {"appearance_tokens", m_known_appearance_tokens}
    };
}

nlohmann::json CharacterCreatorModel::buildSnapshot() const {
    const auto validation = buildValidationSnapshot();
    return {
        {"identity", m_identity.toJson()},
        {"is_dirty", m_dirty},
        {"validation", validation},
        {"preview", buildPreviewSnapshot()},
        {"catalog", buildCatalogSnapshot()},
        {"spawn_config", {
            {"x", m_spawn_x.ToFloat()},
            {"y", m_spawn_y.ToFloat()},
            {"z", m_spawn_z.ToFloat()},
            {"is_enemy", m_spawn_is_enemy}
        }},
        {"workflow", {
            {"can_spawn", validation.value("is_valid", false)},
            {"can_export", true},
            {"has_spawned_entity", m_last_spawned_entity.has_value()}
        }},
        {"last_spawned_entity", m_last_spawned_entity.has_value() ? nlohmann::json(*m_last_spawned_entity) : nlohmann::json(nullptr)}
    };
}

} // namespace urpg::editor
