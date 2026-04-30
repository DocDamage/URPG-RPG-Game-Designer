#include <catch2/catch_test_macros.hpp>
#include "editor/character/character_creator_panel.h"
#include "editor/character/character_creator_model.h"
#include "engine/core/assets/asset_action_view.h"
#include "engine/core/assets/asset_library.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/ecs/actor_manager.h"

#include <algorithm>
#include <utility>

using namespace urpg::editor;
using namespace urpg::character;

namespace {

urpg::assets::AssetPromotionManifest makePanelCharacterManifest(std::string assetId,
                                                                std::string sourcePath,
                                                                std::string promotedPath,
                                                                std::string licenseId,
                                                                std::string status) {
    return urpg::assets::deserializeAssetPromotionManifest(nlohmann::json{
        {"schemaVersion", "1.0.0"},
        {"assetId", std::move(assetId)},
        {"sourcePath", std::move(sourcePath)},
        {"promotedPath", std::move(promotedPath)},
        {"licenseId", std::move(licenseId)},
        {"status", std::move(status)},
        {"preview",
         {{"kind", "image"}, {"thumbnailPath", "resources/previews/part.thumb.png"}, {"width", 48}, {"height", 48}}},
        {"package", {{"includeInRuntime", true}, {"requiredForRelease", false}}},
        {"diagnostics", nlohmann::json::array()},
    });
}

} // namespace

TEST_CASE("CharacterCreatorPanel snapshot is empty when no model bound", "[character][editor][panel]") {
    CharacterCreatorPanel panel;
    panel.render();
    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["status"] == "disabled");
    REQUIRE(snapshot["disabled_reason"] == "No CharacterCreatorModel is bound.");
    REQUIRE(snapshot["owner"] == "editor/character");
    REQUIRE(snapshot["unlock_condition"] == "Bind CharacterCreatorModel before rendering the character creator panel.");
}

TEST_CASE("CharacterCreatorPanel snapshot reflects identity after model load", "[character][editor][panel]") {
    CharacterIdentity identity;
    identity.setName("Marcus");
    identity.setClassId("class_paladin");
    identity.setPortraitId("portrait_marcus");
    identity.addAppearanceToken("beard_short");
    identity.addAppearanceToken("armor_steel");

    CharacterCreatorModel model;
    model.loadIdentity(identity);

    CharacterCreatorPanel panel;
    panel.bindModel(&model);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["status"] == "ready");
    REQUIRE(snapshot.contains("identity_summary"));
    REQUIRE(snapshot["identity_summary"]["name"] == "Marcus");
    REQUIRE(snapshot["identity_summary"]["class_id"] == "class_paladin");
    REQUIRE(snapshot["appearance_token_count"] == 2);
    REQUIRE(snapshot.contains("preview_card"));
    REQUIRE(snapshot.contains("validation_summary"));
    REQUIRE(snapshot.contains("workflow_actions"));
}

TEST_CASE("CharacterCreatorPanel dirty flag transitions correctly", "[character][editor][panel]") {
    CharacterIdentity identity;
    identity.setName("Ayla");
    identity.setClassId("class_ranger");

    CharacterCreatorModel model;
    model.loadIdentity(identity);

    // After load, should not be dirty
    REQUIRE(model.buildSnapshot()["is_dirty"] == false);

    CharacterCreatorPanel panel;
    panel.bindModel(&model);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["is_dirty"] == false);

    // After a setter, should be dirty
    model.setName("Ayla the Brave");
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["is_dirty"] == true);

    // After reload, should be clean again
    model.loadIdentity(identity);
    panel.render();
    REQUIRE(panel.lastRenderSnapshot()["is_dirty"] == false);
}

TEST_CASE("CharacterCreatorPanel exposes workflow readiness details", "[character][editor][panel]") {
    CharacterCreatorModel model;
    model.setName("Kara");
    model.applyClassPreset("class_warrior");
    model.setBodySpriteId("sprite_warrior_body");
    model.setPortraitId("portrait_warrior_01");
    model.addAppearanceToken("armor_steel");

    CharacterCreatorPanel panel;
    panel.bindModel(&model);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["validation_summary"]["is_valid"] == true);
    REQUIRE(snapshot["workflow_actions"]["can_spawn"] == true);
    REQUIRE(snapshot["preview_card"]["primary_attribute"] == "STR");
    REQUIRE(snapshot["layered_composition_preview"]["complete"] == true);
    REQUIRE(snapshot["layered_composition_preview"]["portrait"]["layer_count"] >= 2);
    REQUIRE(snapshot["layered_composition_preview"]["field"]["surface"] == "field");
    REQUIRE(snapshot["layered_composition_preview"]["battle"]["surface"] == "battle");
}

TEST_CASE("CharacterCreatorPanel exposes created protagonist save diagnostics", "[character][editor][panel][save]") {
    urpg::World world;
    urpg::ActorManager actorManager(world);
    CharacterCreatorModel model;
    model.setName("Nova");
    model.applyClassPreset("class_ranger");
    model.setBodySpriteId("sprite_ranger_body");
    model.setPortraitId("portrait_ranger_01");
    model.addAppearanceToken("cloak_travel");

    CharacterCreatorPanel panel;
    panel.bindModel(&model);
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["save_persistence_diagnostics"]["schema_id"] ==
            "https://urpg.dev/schemas/created_protagonist_save.schema.json");
    REQUIRE(snapshot["save_persistence_diagnostics"]["save_key"] == "_created_protagonist");
    REQUIRE(snapshot["save_persistence_diagnostics"]["has_spawned_entity"] == false);
    REQUIRE(snapshot["save_persistence_diagnostics"]["can_attach_to_save"] == false);
    REQUIRE(snapshot["save_persistence_diagnostics"]["diagnostic_count"] == 1);

    const auto spawn = model.spawnCharacter(world, actorManager);
    REQUIRE(spawn.success);
    panel.render();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["save_persistence_diagnostics"]["has_spawned_entity"] == true);
    REQUIRE(snapshot["save_persistence_diagnostics"]["entity"] == spawn.entity);
    REQUIRE(snapshot["save_persistence_diagnostics"]["identity_valid"] == true);
    REQUIRE(snapshot["save_persistence_diagnostics"]["can_attach_to_save"] == true);
    REQUIRE(snapshot["save_persistence_diagnostics"]["diagnostic_count"] == 0);
}

TEST_CASE("CharacterCreatorPanel exposes promoted appearance part rows",
          "[character][editor][panel][appearance][assets]") {
    urpg::assets::AssetLibrary library;
    library.ingestPromotionManifest(makePanelCharacterManifest("asset.character.hero.portrait.hair_01",
                                                               "imports/raw/hero/hair.png",
                                                               "resources/assets/characters/hero/hair.png",
                                                               "BND-CHR",
                                                               "runtime_ready"));
    library.ingestPromotionManifest(makePanelCharacterManifest("asset.character.hero.raw_unlicensed",
                                                               "imports/raw/hero/unlicensed.png",
                                                               "resources/assets/characters/hero/unlicensed.png",
                                                               "",
                                                               "runtime_ready"));

    auto rows = urpg::assets::buildAssetActionRows(library.snapshot());
    for (auto& row : rows) {
        row["slot"] = row.value("asset_id", "").find(".portrait.") != std::string::npos ? "portrait" : "layer";
    }

    CharacterCreatorModel model;
    model.setPromotedAppearanceAssetRows(rows);
    REQUIRE(model.selectPromotedAppearancePart("asset.character.hero.portrait.hair_01"));

    CharacterCreatorPanel panel;
    panel.bindModel(&model);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["appearance_parts"].size() == 2);
    const auto portrait = std::find_if(snapshot["appearance_parts"].begin(), snapshot["appearance_parts"].end(),
                                       [](const auto& row) {
                                           return row["asset_id"] == "asset.character.hero.portrait.hair_01";
                                       });
    REQUIRE(portrait != snapshot["appearance_parts"].end());
    REQUIRE((*portrait)["enabled"] == true);
    REQUIRE((*portrait)["selected"] == true);
    REQUIRE((*portrait)["slot"] == "portrait");

    const auto unlicensed = std::find_if(snapshot["appearance_parts"].begin(), snapshot["appearance_parts"].end(),
                                         [](const auto& row) {
                                             return row["asset_id"] == "asset.character.hero.raw_unlicensed";
                                         });
    REQUIRE(unlicensed != snapshot["appearance_parts"].end());
    REQUIRE((*unlicensed)["enabled"] == false);
    REQUIRE((*unlicensed)["disabled_reason"] == "Asset is not runtime-ready or lacks license evidence.");
    REQUIRE(snapshot["identity_summary"]["portrait_asset_id"] == "asset.character.hero.portrait.hair_01");
}
