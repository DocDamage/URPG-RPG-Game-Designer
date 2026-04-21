#include "engine/core/character/character_identity.h"
#include "engine/core/character/character_identity_component.h"
#include "engine/core/character/character_identity_system.h"
#include "engine/core/ecs/world.h"
#include "engine/core/ecs/actor_manager.h"
#include "engine/core/ecs/actor_components.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg;
using namespace urpg::character;

TEST_CASE("CharacterIdentityComponent attaches to entity", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Test Hero");
    identity.setClassId("CLS_WARRIOR");
    identity.setBodySpriteId("spr_hero_01");

    EntityID id = manager.CreateActorFromIdentity(identity);
    REQUIRE(id != 0);

    auto* comp = world.GetComponent<CharacterIdentityComponent>(id);
    REQUIRE(comp != nullptr);
    REQUIRE(comp->identity.getName() == "Test Hero");
    REQUIRE(comp->dirty == true);
}

TEST_CASE("CharacterIdentitySystem syncs name and class to ActorComponent", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Mage");
    identity.setClassId("CLS_MAGE");

    EntityID id = manager.CreateActorFromIdentity(identity);
    REQUIRE(id != 0);

    CharacterIdentitySystem system;
    system.update(world);

    auto* actor = world.GetComponent<ActorComponent>(id);
    REQUIRE(actor != nullptr);
    REQUIRE(actor->name == "Mage");
    REQUIRE(actor->className == "CLS_MAGE");

    auto* comp = world.GetComponent<CharacterIdentityComponent>(id);
    REQUIRE(comp != nullptr);
    REQUIRE(comp->dirty == false);
}

TEST_CASE("CharacterIdentitySystem syncs body sprite to VisualComponent", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Rogue");
    identity.setBodySpriteId("spr_rogue_01");

    EntityID id = manager.CreateActorFromIdentity(identity);

    CharacterIdentitySystem system;
    system.update(world);

    auto* visual = world.GetComponent<VisualComponent>(id);
    REQUIRE(visual != nullptr);
    REQUIRE(visual->assetPath == "spr_rogue_01");
}

TEST_CASE("CharacterIdentitySystem does not overwrite empty identity fields", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Knight");
    // classId intentionally empty

    EntityID id = manager.CreateActorFromIdentity(identity);

    // Pre-set actor className to verify it's NOT overwritten by empty identity value
    auto* actor = world.GetComponent<ActorComponent>(id);
    REQUIRE(actor != nullptr);
    actor->className = "PRESET_CLASS";

    CharacterIdentitySystem system;
    system.update(world);

    REQUIRE(actor->name == "Knight");
    REQUIRE(actor->className == "PRESET_CLASS");
}

TEST_CASE("CharacterIdentitySystem only processes dirty components", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Paladin");
    identity.setClassId("CLS_PALADIN");

    EntityID id = manager.CreateActorFromIdentity(identity);

    CharacterIdentitySystem system;
    system.update(world);

    // After first sync, dirty is false
    auto* comp = world.GetComponent<CharacterIdentityComponent>(id);
    REQUIRE(comp->dirty == false);

    // Change identity and mark dirty
    comp->identity.setName("Dark Knight");
    comp->dirty = true;

    system.update(world);

    auto* actor = world.GetComponent<ActorComponent>(id);
    REQUIRE(actor->name == "Dark Knight");
    REQUIRE(comp->dirty == false);
}

TEST_CASE("CharacterSpawner spawns actor with position", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Archer");
    identity.setBodySpriteId("spr_archer_01");

    CharacterSpawner::Request request;
    request.identity = identity;
    request.x = Fixed32::FromInt(10);
    request.y = Fixed32::FromInt(20);
    request.isEnemy = false;

    auto result = CharacterSpawner::spawn(world, manager, request);
    REQUIRE(result.success == true);
    REQUIRE(result.entity != 0);

    auto* actor = world.GetComponent<ActorComponent>(result.entity);
    REQUIRE(actor != nullptr);
    REQUIRE(actor->name == "Archer");

    auto* transform = world.GetComponent<TransformComponent>(result.entity);
    REQUIRE(transform != nullptr);
    REQUIRE(transform->position.x == Fixed32::FromInt(10));
    REQUIRE(transform->position.y == Fixed32::FromInt(20));

    auto* visual = world.GetComponent<VisualComponent>(result.entity);
    REQUIRE(visual != nullptr);
    REQUIRE(visual->assetPath == "spr_archer_01");
}

TEST_CASE("CharacterSpawner spawns enemy actor", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity identity;
    identity.setName("Goblin");

    CharacterSpawner::Request request;
    request.identity = identity;
    request.isEnemy = true;

    auto result = CharacterSpawner::spawn(world, manager, request);
    REQUIRE(result.success == true);

    auto* actor = world.GetComponent<ActorComponent>(result.entity);
    REQUIRE(actor != nullptr);
    REQUIRE(actor->isEnemy == true);
}

TEST_CASE("CharacterIdentity round-trip through ECS preserves data", "[character][identity][ecs]") {
    World world;
    ActorManager manager(world);

    CharacterIdentity original;
    original.setName("Sorcerer");
    original.setPortraitId("port_sorcerer");
    original.setBodySpriteId("spr_sorcerer");
    original.setClassId("CLS_SORCERER");
    original.setAttribute("int", 18.0f);
    original.setAttribute("wis", 14.0f);
    original.addAppearanceToken("hat_wizard");
    original.addAppearanceToken("robe_blue");

    EntityID id = manager.CreateActorFromIdentity(original);

    auto* comp = world.GetComponent<CharacterIdentityComponent>(id);
    REQUIRE(comp != nullptr);

    // Verify identity data is preserved in the component
    REQUIRE(comp->identity.getName() == "Sorcerer");
    REQUIRE(comp->identity.getPortraitId() == "port_sorcerer");
    REQUIRE(comp->identity.getBodySpriteId() == "spr_sorcerer");
    REQUIRE(comp->identity.getClassId() == "CLS_SORCERER");
    REQUIRE(comp->identity.getAttribute("int") == 18.0f);
    REQUIRE(comp->identity.getAttribute("wis") == 14.0f);
    auto tokens = comp->identity.getAppearanceTokens();
    REQUIRE(tokens.size() == 2);
    REQUIRE(tokens[0] == "hat_wizard");
    REQUIRE(tokens[1] == "robe_blue");
}
