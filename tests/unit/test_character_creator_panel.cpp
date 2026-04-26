#include <catch2/catch_test_macros.hpp>
#include "editor/character/character_creator_panel.h"
#include "editor/character/character_creator_model.h"

using namespace urpg::editor;
using namespace urpg::character;

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
}
