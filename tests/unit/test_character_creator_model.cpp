#include <catch2/catch_test_macros.hpp>

#include "editor/character/character_creator_model.h"
#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/actor_manager.h"

#include <utility>

using namespace urpg::editor;

namespace {

urpg::assets::AssetPromotionManifest makeCharacterManifest(std::string assetId,
                                                           std::string sourcePath,
                                                           std::string promotedPath,
                                                           std::string licenseId,
                                                           std::string status,
                                                           std::string previewKind = "image") {
    return urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", std::move(assetId)},
        {"sourcePath", std::move(sourcePath)},
        {"promotedPath", std::move(promotedPath)},
        {"licenseId", std::move(licenseId)},
        {"status", std::move(status)},
        {"preview",
         {{"kind", std::move(previewKind)}, {"thumbnailPath", "resources/previews/part.thumb.png"}, {"width", 48}, {"height", 48}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
        {"diagnostics", nlohmann::json::array()},
    });
}

} // namespace

TEST_CASE("CharacterCreatorModel class preset seeds base attributes", "[character][editor][model]") {
    CharacterCreatorModel model;

    model.applyClassPreset("class_mage");
    auto snapshot = model.buildSnapshot();

    REQUIRE(snapshot["identity"]["classId"] == "class_mage");
    REQUIRE(snapshot["identity"]["baseAttributes"]["INT"] == 14.0f);
    REQUIRE(snapshot["preview"]["primary_attribute"] == "INT");
}

TEST_CASE("CharacterCreatorModel validation blocks spawn until required fields are present", "[character][editor][model]") {
    CharacterCreatorModel model;

    auto snapshot = model.buildSnapshot();
    REQUIRE_FALSE(snapshot["validation"]["is_valid"]);
    REQUIRE_FALSE(snapshot["workflow"]["can_spawn"]);

    model.setName("Lyra");
    model.applyClassPreset("class_ranger");
    snapshot = model.buildSnapshot();

    REQUIRE(snapshot["validation"]["is_valid"]);
    REQUIRE(snapshot["workflow"]["can_spawn"]);
}

TEST_CASE("CharacterCreatorModel spawns validated character into ECS", "[character][editor][model]") {
    urpg::World world;
    urpg::ActorManager manager(world);
    CharacterCreatorModel model;

    model.setName("Lyra");
    model.applyClassPreset("class_ranger");
    model.setPortraitId("portrait_ranger_01");
    model.setBodySpriteId("sprite_ranger_body");
    model.addAppearanceToken("cloak_travel");
    model.setSpawnPosition(urpg::Fixed32::FromInt(4), urpg::Fixed32::FromInt(7));

    auto result = model.spawnCharacter(world, manager);
    REQUIRE(result.success);
    REQUIRE(result.entity != 0);
    REQUIRE(model.lastSpawnedEntity().has_value());
    REQUIRE(*model.lastSpawnedEntity() == result.entity);

    auto* actor = world.GetComponent<urpg::ActorComponent>(result.entity);
    REQUIRE(actor != nullptr);
    REQUIRE(actor->name == "Lyra");
    REQUIRE(actor->className == "class_ranger");

    auto* transform = world.GetComponent<urpg::TransformComponent>(result.entity);
    REQUIRE(transform != nullptr);
    REQUIRE(transform->position.x == urpg::Fixed32::FromInt(4));
    REQUIRE(transform->position.y == urpg::Fixed32::FromInt(7));

    auto snapshot = model.buildSnapshot();
    REQUIRE_FALSE(snapshot["is_dirty"]);
    REQUIRE(snapshot["workflow"]["has_spawned_entity"]);
}

TEST_CASE("CharacterCreatorModel rejects unknown catalog selections", "[character][editor][model]") {
    CharacterCreatorModel model;

    model.setName("Nova");
    model.setClassId("class_unknown");
    model.setPortraitId("portrait_missing");
    model.addAppearanceToken("token_missing");

    const auto snapshot = model.buildSnapshot();
    REQUIRE_FALSE(snapshot["validation"]["is_valid"]);
    REQUIRE(snapshot["validation"]["issue_count"] == 3);
}

TEST_CASE("CharacterCreatorModel selects promoted appearance parts and persists asset ids",
          "[character][editor][model][appearance][assets]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionManifest(makeCharacterManifest("asset.character.hero.portrait.hair_01",
                                                          "imports/raw/hero/hair.png",
                                                          "resources/assets/characters/hero/hair.png",
                                                          "BND-CHR",
                                                          "runtime_ready"));
    library.ingestPromotionManifest(makeCharacterManifest("asset.character.hero.field.body_01",
                                                          "imports/raw/hero/body.png",
                                                          "resources/assets/characters/hero/body.png",
                                                          "BND-CHR",
                                                          "runtime_ready"));
    library.ingestPromotionManifest(makeCharacterManifest("asset.character.hero.layer.cloak_01",
                                                          "imports/raw/hero/cloak.png",
                                                          "resources/assets/characters/hero/cloak.png",
                                                          "BND-CHR",
                                                          "runtime_ready"));
    library.ingestPromotionManifest(makeCharacterManifest("asset.character.hero.raw_unlicensed",
                                                          "imports/raw/hero/unlicensed.png",
                                                          "resources/assets/characters/hero/unlicensed.png",
                                                          "",
                                                          "runtime_ready"));
    library.ingestPromotionManifest(makeCharacterManifest("asset.character.hero.archived",
                                                          "imports/raw/hero/archived.png",
                                                          "resources/assets/characters/hero/archived.png",
                                                          "BND-CHR",
                                                          "archived"));

    auto rows = urpg::assets::buildAssetActionRows(library.snapshot());
    for (auto& row : rows) {
        const auto assetId = row.value("asset_id", "");
        if (assetId.find(".portrait.") != std::string::npos) {
            row["slot"] = "portrait";
        } else if (assetId.find(".field.") != std::string::npos) {
            row["slot"] = "field";
        } else if (assetId.find(".archived") != std::string::npos) {
            row["slot"] = "battle";
        } else {
            row["slot"] = "layer";
        }
    }

    CharacterCreatorModel model;
    model.setPromotedAppearanceAssetRows(rows);

    auto snapshot = model.buildSnapshot();
    REQUIRE(snapshot["appearance_parts"].size() == 5);
    const auto field = std::find_if(snapshot["appearance_parts"].begin(), snapshot["appearance_parts"].end(),
                                    [](const auto& row) {
                                        return row["asset_id"] == "asset.character.hero.field.body_01";
                                    });
    REQUIRE(field != snapshot["appearance_parts"].end());
    REQUIRE((*field)["enabled"] == true);

    const auto unlicensed = std::find_if(snapshot["appearance_parts"].begin(), snapshot["appearance_parts"].end(),
                                         [](const auto& row) {
                                             return row["asset_id"] == "asset.character.hero.raw_unlicensed";
                                         });
    REQUIRE(unlicensed != snapshot["appearance_parts"].end());
    REQUIRE((*unlicensed)["enabled"] == false);
    REQUIRE((*unlicensed)["disabled_reason"] == "Asset is not runtime-ready or lacks license evidence.");

    REQUIRE(model.selectPromotedAppearancePart("asset.character.hero.portrait.hair_01"));
    REQUIRE(model.selectPromotedAppearancePart("asset.character.hero.field.body_01"));
    REQUIRE(model.selectPromotedAppearancePart("asset.character.hero.layer.cloak_01"));
    REQUIRE_FALSE(model.selectPromotedAppearancePart("asset.character.hero.raw_unlicensed"));
    REQUIRE_FALSE(model.selectPromotedAppearancePart("asset.character.hero.archived"));

    snapshot = model.buildSnapshot();
    REQUIRE(snapshot["identity"]["portraitAssetId"] == "asset.character.hero.portrait.hair_01");
    REQUIRE(snapshot["identity"]["fieldSpriteAssetId"] == "asset.character.hero.field.body_01");
    REQUIRE(snapshot["identity"]["layeredPartAssetIds"][0] == "asset.character.hero.layer.cloak_01");
    REQUIRE(snapshot["preview"]["portrait_asset_id"] == "asset.character.hero.portrait.hair_01");
    REQUIRE(snapshot["appearance_composition"]["portrait"]["base_asset_id"] ==
            "asset.character.hero.portrait.hair_01");
    REQUIRE(snapshot["appearance_composition"]["field"]["base_asset_id"] == "asset.character.hero.field.body_01");

    const auto restored = urpg::character::CharacterIdentity::fromJson(snapshot["identity"]);
    REQUIRE(restored.getPortraitAssetId() == "asset.character.hero.portrait.hair_01");
    REQUIRE(restored.getFieldSpriteAssetId() == "asset.character.hero.field.body_01");
    REQUIRE(restored.getLayeredPartAssetIds().size() == 1);
    REQUIRE(restored.getLayeredPartAssetIds()[0] == "asset.character.hero.layer.cloak_01");
}

TEST_CASE("CharacterCreatorModel exposes appearance part library management rows",
          "[character][editor][model][appearance][assets][part_library]") {
    nlohmann::json rows = nlohmann::json::array({
        {
            {"asset_id", "asset.character.hero.portrait.hair_01"},
            {"label", "Hero Hair"},
            {"slot", "portrait"},
            {"source_path", ".urpg/asset-library/sources/import_001/original/img/faces/Actor1.png"},
            {"normalized_path", "resources/assets/characters/hero/hair.png"},
            {"preview_kind", "image"},
            {"preview_width", 144},
            {"preview_height", 144},
            {"include_in_runtime", true},
            {"promotion_status", "runtime_ready"},
            {"statuses", {"promoted"}},
            {"promotion_diagnostics", nlohmann::json::array()},
            {"management_actions",
             {
                 {"accept", {{"enabled", true}}},
                 {"reject", {{"enabled", true}}},
                 {"archive", {{"enabled", true}}},
                 {"assign", {{"enabled", true}, {"target_slot", "portrait"}}},
             }},
        },
        {
            {"asset_id", "asset.character.hero.battle.blocked"},
            {"label", "Blocked Battle"},
            {"slot", "battle"},
            {"source_path", ".urpg/asset-library/sources/import_001/original/img/sv_actors/Actor1.png"},
            {"normalized_path", ""},
            {"preview_kind", "image"},
            {"preview_width", 576},
            {"preview_height", 384},
            {"include_in_runtime", false},
            {"promotion_status", "blocked"},
            {"statuses", {"missing_license"}},
            {"promotion_diagnostics", {"license_evidence_missing"}},
            {"management_actions",
             {
                 {"accept", {{"enabled", false}, {"disabled_reason", "license_evidence_missing"}}},
                 {"reject", {{"enabled", true}}},
                 {"archive", {{"enabled", true}}},
                 {"assign", {{"enabled", false}, {"target_slot", "battle"}, {"disabled_reason", "license_evidence_missing"}}},
             }},
        },
    });

    CharacterCreatorModel model;
    model.setPromotedAppearanceAssetRows(rows);

    auto snapshot = model.buildSnapshot();
    REQUIRE(snapshot["appearance_parts"].size() == 2);
    const auto portrait = std::find_if(snapshot["appearance_parts"].begin(), snapshot["appearance_parts"].end(),
                                       [](const auto& row) {
                                           return row["asset_id"] == "asset.character.hero.portrait.hair_01";
                                       });
    REQUIRE(portrait != snapshot["appearance_parts"].end());
    REQUIRE((*portrait)["source_path"] ==
            ".urpg/asset-library/sources/import_001/original/img/faces/Actor1.png");
    REQUIRE((*portrait)["normalized_path"] == "resources/assets/characters/hero/hair.png");
    REQUIRE((*portrait)["dimensions"]["width"] == 144);
    REQUIRE((*portrait)["dimensions"]["height"] == 144);
    REQUIRE((*portrait)["management_actions"]["assign"]["enabled"] == true);
    REQUIRE((*portrait)["management_actions"]["assign"]["target_slot"] == "portrait");

    REQUIRE(model.selectPromotedAppearancePart("asset.character.hero.portrait.hair_01"));
    snapshot = model.buildSnapshot();
    REQUIRE(snapshot["identity"]["portraitAssetId"] == "asset.character.hero.portrait.hair_01");

    const auto blocked = std::find_if(snapshot["appearance_parts"].begin(), snapshot["appearance_parts"].end(),
                                      [](const auto& row) {
                                          return row["asset_id"] == "asset.character.hero.battle.blocked";
                                      });
    REQUIRE(blocked != snapshot["appearance_parts"].end());
    REQUIRE((*blocked)["enabled"] == false);
    REQUIRE((*blocked)["disabled_reason"] == "license_evidence_missing");
    REQUIRE((*blocked)["management_actions"]["assign"]["enabled"] == false);
    REQUIRE((*blocked)["management_actions"]["assign"]["disabled_reason"] == "license_evidence_missing");
    REQUIRE_FALSE(model.selectPromotedAppearancePart("asset.character.hero.battle.blocked"));
}
