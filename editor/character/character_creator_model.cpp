#include "editor/character/character_creator_model.h"

#include "engine/core/character/character_identity_system.h"
#include "engine/core/ecs/actor_manager.h"

#include <algorithm>
#include <unordered_map>

namespace urpg::editor {

namespace {

using AttributeMap = std::unordered_map<std::string, float>;

const std::unordered_map<std::string, AttributeMap>& classPresets() {
    static const std::unordered_map<std::string, AttributeMap> kPresets = {
        {"class_warrior", {{"STR", 14.0f}, {"VIT", 12.0f}, {"INT", 6.0f}, {"AGI", 8.0f}}},
        {"class_mage", {{"STR", 6.0f}, {"VIT", 8.0f}, {"INT", 14.0f}, {"AGI", 10.0f}}},
        {"class_ranger", {{"STR", 9.0f}, {"VIT", 9.0f}, {"INT", 8.0f}, {"AGI", 14.0f}}},
        {"class_rogue", {{"STR", 8.0f}, {"VIT", 8.0f}, {"INT", 9.0f}, {"AGI", 15.0f}}}
    };
    return kPresets;
}

bool containsValue(const std::vector<std::string>& values, const std::string& value) {
    return std::find(values.begin(), values.end(), value) != values.end();
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
    m_known_class_ids = {"class_warrior", "class_mage", "class_ranger", "class_rogue"};
    m_known_portrait_ids = {"portrait_warrior_01", "portrait_mage_01", "portrait_ranger_01", "portrait_rogue_01"};
    m_known_body_sprite_ids = {"sprite_warrior_body", "sprite_mage_body", "sprite_ranger_body", "sprite_rogue_body"};
    m_known_appearance_tokens = {"hair_short", "hair_long", "beard_short", "armor_steel", "cloak_travel", "hat_wizard"};
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
    m_identity.setClassId(classId);

    const auto preset_it = classPresets().find(classId);
    if (preset_it != classPresets().end()) {
        m_identity.setBaseAttributes(preset_it->second);
    }

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
    nlohmann::json issues = nlohmann::json::array();

    if (m_identity.getName().empty()) {
        issues.push_back({{"field", "name"}, {"code", "missing_name"}, {"message", "Character name is required."}});
    }
    if (m_identity.getClassId().empty()) {
        issues.push_back({{"field", "classId"}, {"code", "missing_class"}, {"message", "Character class is required."}});
    } else if (!containsValue(m_known_class_ids, m_identity.getClassId())) {
        issues.push_back({{"field", "classId"}, {"code", "unknown_class"}, {"message", "Character class is not in the current class catalog."}});
    }
    if (!m_identity.getPortraitId().empty() && !containsValue(m_known_portrait_ids, m_identity.getPortraitId())) {
        issues.push_back({{"field", "portraitId"}, {"code", "unknown_portrait"}, {"message", "Portrait id is not in the current portrait catalog."}});
    }
    if (!m_identity.getBodySpriteId().empty() && !containsValue(m_known_body_sprite_ids, m_identity.getBodySpriteId())) {
        issues.push_back({{"field", "bodySpriteId"}, {"code", "unknown_body_sprite"}, {"message", "Body sprite id is not in the current body-sprite catalog."}});
    }

    std::vector<std::string> seen_tokens;
    for (const auto& token : m_identity.getAppearanceTokens()) {
        if (!containsValue(m_known_appearance_tokens, token)) {
            issues.push_back({{"field", "appearanceTokens"}, {"code", "unknown_appearance_token"}, {"message", "Appearance token '" + token + "' is not in the current appearance catalog."}});
        }
        if (containsValue(seen_tokens, token)) {
            issues.push_back({{"field", "appearanceTokens"}, {"code", "duplicate_appearance_token"}, {"message", "Appearance token '" + token + "' is duplicated."}});
        } else {
            seen_tokens.push_back(token);
        }
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
