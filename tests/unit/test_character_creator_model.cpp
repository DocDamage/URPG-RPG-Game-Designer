#include <catch2/catch_test_macros.hpp>

#include "editor/character/character_creator_model.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/actor_manager.h"

using namespace urpg::editor;

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
