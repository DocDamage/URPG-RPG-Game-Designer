#include "editor/character/character_creator_model.h"

#include "engine/core/character/character_appearance_composition.h"
#include "engine/core/character/character_creation_rules.h"
#include "engine/core/character/character_identity_catalog.h"
#include "engine/core/character/character_identity_validator.h"
#include "engine/core/character/character_identity_system.h"
#include "engine/core/character/character_save_state.h"
#include "engine/core/ecs/actor_manager.h"

#include <algorithm>
#include <string_view>

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
    case CharacterIdentityIssueCategory::CreationRuleViolation:
        return "creation_rule_violation";
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

std::string valueOrEmpty(const nlohmann::json& row, const char* key) {
    const auto it = row.find(key);
    return it != row.end() && it->is_string() ? it->get<std::string>() : std::string{};
}

bool containsStatus(const nlohmann::json& row, std::string_view status) {
    const auto it = row.find("statuses");
    if (it == row.end() || !it->is_array()) {
        return false;
    }
    for (const auto& item : *it) {
        if (item.is_string() && item.get<std::string>() == status) {
            return true;
        }
    }
    return false;
}

bool hasDiagnostics(const nlohmann::json& row) {
    const auto it = row.find("promotion_diagnostics");
    return it != row.end() && it->is_array() && !it->empty();
}

std::string firstDiagnostic(const nlohmann::json& row) {
    const auto it = row.find("promotion_diagnostics");
    if (it != row.end() && it->is_array() && !it->empty() && (*it)[0].is_string()) {
        return (*it)[0].get<std::string>();
    }
    const auto blocked = row.find("blocked_reason");
    if (blocked != row.end() && blocked->is_string()) {
        return blocked->get<std::string>();
    }
    return {};
}

std::string disabledReasonForPromotedAsset(const nlohmann::json& row) {
    if (row.contains("management_actions")) {
        const auto diagnostic = firstDiagnostic(row);
        if (!diagnostic.empty()) {
            return diagnostic;
        }
    }
    if (containsStatus(row, "archived")) {
        return "Asset is archived and cannot be assigned to a character.";
    }
    if (containsStatus(row, "unsupported_format")) {
        return "Asset format is not supported for character appearance.";
    }
    if (containsStatus(row, "missing_file") || hasDiagnostics(row)) {
        return "Asset is not runtime-ready or lacks license evidence.";
    }
    if (!row.value("include_in_runtime", false)) {
        return "Asset is not runtime-ready or lacks license evidence.";
    }
    return {};
}

std::string labelForAssetRow(const nlohmann::json& row) {
    const auto assetId = valueOrEmpty(row, "asset_id");
    if (!assetId.empty()) {
        return assetId;
    }
    return valueOrEmpty(row, "path");
}

int intValueOrZero(const nlohmann::json& row, const char* key) {
    const auto it = row.find(key);
    return it != row.end() && it->is_number_integer() ? it->get<int>() : 0;
}

nlohmann::json defaultManagementActions(const CharacterAppearancePartRow& part) {
    const auto disabled = part.disabled_reason.empty() ? nlohmann::json(nullptr)
                                                       : nlohmann::json(part.disabled_reason);
    return {
        {"accept",
         {{"enabled", part.enabled},
          {"action", "accept_appearance_part"},
          {"disabled_reason", disabled}}},
        {"reject",
         {{"enabled", true},
          {"action", "reject_appearance_part"},
          {"disabled_reason", nlohmann::json(nullptr)}}},
        {"archive",
         {{"enabled", part.enabled},
          {"action", "archive_appearance_part"},
          {"disabled_reason", part.enabled ? nlohmann::json(nullptr) : disabled}}},
        {"assign",
         {{"enabled", part.enabled},
          {"action", "assign_appearance_part"},
          {"target_slot", part.slot},
          {"disabled_reason", disabled}}},
    };
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

void CharacterCreatorModel::setPromotedAppearanceAssetRows(const nlohmann::json& rows) {
    m_appearance_part_rows.clear();
    if (!rows.is_array()) {
        return;
    }
    for (const auto& row : rows) {
        if (!row.is_object()) {
            continue;
        }
        CharacterAppearancePartRow part;
        part.asset_id = valueOrEmpty(row, "asset_id");
        if (part.asset_id.empty()) {
            part.asset_id = valueOrEmpty(row, "path");
        }
        if (part.asset_id.empty()) {
            continue;
        }
        part.label = labelForAssetRow(row);
        part.slot = valueOrEmpty(row, "slot");
        if (part.slot.empty()) {
            part.slot = "layer";
        }
        part.preview_kind = valueOrEmpty(row, "preview_kind");
        part.source_path = valueOrEmpty(row, "source_path");
        part.normalized_path = valueOrEmpty(row, "normalized_path");
        part.preview_width = intValueOrZero(row, "preview_width");
        part.preview_height = intValueOrZero(row, "preview_height");
        part.disabled_reason = disabledReasonForPromotedAsset(row);
        part.enabled = part.disabled_reason.empty();
        const auto actions = row.find("management_actions");
        part.management_actions = actions != row.end() && actions->is_object()
                                      ? *actions
                                      : defaultManagementActions(part);
        m_appearance_part_rows.push_back(std::move(part));
    }
    std::sort(m_appearance_part_rows.begin(), m_appearance_part_rows.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.slot != rhs.slot) {
            return lhs.slot < rhs.slot;
        }
        return lhs.asset_id < rhs.asset_id;
    });
}

bool CharacterCreatorModel::selectPromotedAppearancePart(const std::string& asset_id) {
    const auto it = std::find_if(m_appearance_part_rows.begin(), m_appearance_part_rows.end(), [&](const auto& row) {
        return row.asset_id == asset_id;
    });
    if (it == m_appearance_part_rows.end() || !it->enabled) {
        return false;
    }

    if (it->slot == "portrait") {
        m_identity.setPortraitAssetId(it->asset_id);
    } else if (it->slot == "field") {
        m_identity.setFieldSpriteAssetId(it->asset_id);
    } else if (it->slot == "battle") {
        m_identity.setBattleSpriteAssetId(it->asset_id);
    } else {
        m_identity.addLayeredPartAssetId(it->asset_id);
    }
    m_dirty = true;
    return true;
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
        {"portrait_asset_id", m_identity.getPortraitAssetId()},
        {"field_sprite_asset_id", m_identity.getFieldSpriteAssetId()},
        {"battle_sprite_asset_id", m_identity.getBattleSpriteAssetId()},
        {"primary_attribute", primaryAttribute(m_identity)},
        {"attribute_total", attributeTotal(m_identity)},
        {"appearance_tokens", m_identity.getAppearanceTokens()},
        {"layered_part_asset_ids", m_identity.getLayeredPartAssetIds()}
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

nlohmann::json CharacterCreatorModel::buildAppearancePartsSnapshot() const {
    nlohmann::json rows = nlohmann::json::array();
    for (const auto& part : m_appearance_part_rows) {
        rows.push_back({
            {"asset_id", part.asset_id},
            {"label", part.label},
            {"slot", part.slot},
            {"enabled", part.enabled},
            {"disabled_reason", part.disabled_reason},
            {"preview_kind", part.preview_kind},
            {"source_path", part.source_path},
            {"normalized_path", part.normalized_path},
            {"dimensions", {{"width", part.preview_width}, {"height", part.preview_height}}},
            {"management_actions", part.management_actions},
            {"selected", part.asset_id == m_identity.getPortraitAssetId() ||
                             part.asset_id == m_identity.getFieldSpriteAssetId() ||
                             part.asset_id == m_identity.getBattleSpriteAssetId() ||
                             containsValue(m_identity.getLayeredPartAssetIds(), part.asset_id)},
        });
    }
    return rows;
}

nlohmann::json CharacterCreatorModel::buildSavePersistenceSnapshot(const nlohmann::json& validation) const {
    nlohmann::json diagnostics = nlohmann::json::array();
    const bool hasSpawnedEntity = m_last_spawned_entity.has_value();
    const bool identityValid = validation.value("is_valid", false);

    if (!hasSpawnedEntity) {
        diagnostics.push_back({
            {"code", "created_protagonist_missing_spawned_entity"},
            {"severity", "info"},
            {"message", "Spawn a validated protagonist before attaching character identity to a runtime save."},
        });
    }
    if (!identityValid) {
        diagnostics.push_back({
            {"code", "created_protagonist_identity_invalid"},
            {"severity", "error"},
            {"message", "Resolve character identity validation issues before saving the created protagonist."},
        });
    }

    return {
        {"schema_id", "https://urpg.dev/schemas/created_protagonist_save.schema.json"},
        {"save_key", urpg::character::kCreatedProtagonistSaveKey},
        {"has_spawned_entity", hasSpawnedEntity},
        {"entity", hasSpawnedEntity ? nlohmann::json(*m_last_spawned_entity) : nlohmann::json(nullptr)},
        {"identity_valid", identityValid},
        {"can_attach_to_save", hasSpawnedEntity && identityValid},
        {"diagnostic_count", diagnostics.size()},
        {"diagnostics", diagnostics},
    };
}

nlohmann::json CharacterCreatorModel::buildSnapshot() const {
    const auto validation = buildValidationSnapshot();
    return {
        {"identity", m_identity.toJson()},
        {"is_dirty", m_dirty},
        {"validation", validation},
        {"preview", buildPreviewSnapshot()},
        {"appearance_parts", buildAppearancePartsSnapshot()},
        {"appearance_composition",
         urpg::character::characterAppearanceCompositionToJson(
             urpg::character::composeCharacterAppearance(m_identity))},
        {"catalog", buildCatalogSnapshot()},
        {"save_persistence", buildSavePersistenceSnapshot(validation)},
        {"creation_rules", urpg::character::characterCreationRulesToJson(urpg::character::defaultCharacterCreationRules())},
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
