#include <catch2/catch_test_macros.hpp>
#include "engine/core/character/character_identity.h"

using namespace urpg::character;

TEST_CASE("CharacterIdentity round-trip serialization", "[character][identity]") {
    CharacterIdentity original;
    original.setName("Elena");
    original.setPortraitId("portrait_elena_01");
    original.setBodySpriteId("sprite_elena_body");
    original.setClassId("class_mage");
    original.setAttribute("STR", 8.0f);
    original.setAttribute("INT", 15.0f);
    original.addAppearanceToken("hair_brown");
    original.addAppearanceToken("eyes_green");

    auto json = original.toJson();
    REQUIRE(json["schemaVersion"] == "1.0.0");
    REQUIRE(json["name"] == "Elena");
    REQUIRE(json["classId"] == "class_mage");
    REQUIRE(json["portraitId"] == "portrait_elena_01");
    REQUIRE(json["bodySpriteId"] == "sprite_elena_body");
    REQUIRE(json["baseAttributes"]["STR"] == 8.0f);
    REQUIRE(json["baseAttributes"]["INT"] == 15.0f);
    REQUIRE(json["appearanceTokens"].size() == 2);
    REQUIRE(json["appearanceTokens"][0] == "hair_brown");
    REQUIRE(json["appearanceTokens"][1] == "eyes_green");

    auto restored = CharacterIdentity::fromJson(json);
    REQUIRE(restored.getName() == "Elena");
    REQUIRE(restored.getPortraitId() == "portrait_elena_01");
    REQUIRE(restored.getBodySpriteId() == "sprite_elena_body");
    REQUIRE(restored.getClassId() == "class_mage");
    REQUIRE(restored.getAttribute("STR") == 8.0f);
    REQUIRE(restored.getAttribute("INT") == 15.0f);
    REQUIRE(restored.getAppearanceTokens().size() == 2);
    REQUIRE(restored.getAppearanceTokens()[0] == "hair_brown");
    REQUIRE(restored.getAppearanceTokens()[1] == "eyes_green");
}

TEST_CASE("CharacterIdentity default attribute returns 0", "[character][identity]") {
    CharacterIdentity identity;
    REQUIRE(identity.getAttribute("NON_EXISTENT") == 0.0f);
}

TEST_CASE("CharacterIdentity schema version validation", "[character][identity]") {
    nlohmann::json valid;
    valid["schemaVersion"] = "1.0.0";
    valid["name"] = "Test";
    valid["classId"] = "class_warrior";

    auto identity = CharacterIdentity::fromJson(valid);
    REQUIRE(identity.getName() == "Test");
}

TEST_CASE("CharacterIdentity invalid version throws", "[character][identity]") {
    nlohmann::json missingVersion;
    missingVersion["name"] = "Test";
    missingVersion["classId"] = "class_warrior";
    REQUIRE_THROWS_AS(CharacterIdentity::fromJson(missingVersion), std::invalid_argument);

    nlohmann::json badVersion;
    badVersion["schemaVersion"] = "2.0.0";
    badVersion["name"] = "Test";
    badVersion["classId"] = "class_warrior";
    REQUIRE_THROWS_AS(CharacterIdentity::fromJson(badVersion), std::invalid_argument);
}
