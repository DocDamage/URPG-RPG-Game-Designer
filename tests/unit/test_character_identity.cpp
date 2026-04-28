#include <catch2/catch_test_macros.hpp>
#include "engine/core/character/character_creation_rules.h"
#include "engine/core/character/character_identity_validator.h"
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include "engine/core/character/character_identity.h"
#include <nlohmann/json.hpp>

using namespace urpg::character;
using nlohmann::json;

TEST_CASE("CharacterIdentity round-trip serialization", "[character][identity]") {
    CharacterIdentity original;
    original.setName("Elena");
    original.setPortraitId("portrait_elena_01");
    original.setBodySpriteId("sprite_elena_body");
    original.setClassId("class_mage");
    original.setSpeciesId("elf");
    original.setOriginId("capital");
    original.setBackgroundId("scholar");
    original.setAttribute("STR", 8.0f);
    original.setAttribute("INT", 15.0f);
    original.addAppearanceToken("hair_brown");
    original.addAppearanceToken("eyes_green");

    auto json = original.toJson();
    REQUIRE(json["schemaVersion"] == "1.0.0");
    REQUIRE(json["name"] == "Elena");
    REQUIRE(json["classId"] == "class_mage");
    REQUIRE(json["speciesId"] == "elf");
    REQUIRE(json["originId"] == "capital");
    REQUIRE(json["backgroundId"] == "scholar");
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
    REQUIRE(restored.getSpeciesId() == "elf");
    REQUIRE(restored.getOriginId() == "capital");
    REQUIRE(restored.getBackgroundId() == "scholar");
    REQUIRE(restored.getAttribute("STR") == 8.0f);
    REQUIRE(restored.getAttribute("INT") == 15.0f);
    REQUIRE(restored.getAppearanceTokens().size() == 2);
    REQUIRE(restored.getAppearanceTokens()[0] == "hair_brown");
    REQUIRE(restored.getAppearanceTokens()[1] == "eyes_green");
}

TEST_CASE("CharacterCreationRules enforce origins species appearances and point budget",
          "[character][identity][rules]") {
    CharacterIdentity identity;
    identity.setName("Ka");
    identity.setClassId("class_mage");
    identity.setSpeciesId("dwarf");
    identity.setOriginId("frontier");
    identity.setBackgroundId("soldier");
    identity.addAppearanceToken("armor_steel");
    identity.setAttribute("STR", 19.0f);
    identity.setAttribute("VIT", 18.0f);
    identity.setAttribute("INT", 18.0f);
    identity.setAttribute("AGI", 18.0f);

    const auto issues = validateCharacterCreationRules(identity);
    REQUIRE_FALSE(issues.empty());
    REQUIRE(std::find_if(issues.begin(), issues.end(), [](const CharacterCreationRuleIssue& issue) {
                return issue.code == "class_species_restricted";
            }) != issues.end());
    REQUIRE(std::find_if(issues.begin(), issues.end(), [](const CharacterCreationRuleIssue& issue) {
                return issue.code == "class_origin_restricted";
            }) != issues.end());
    REQUIRE(std::find_if(issues.begin(), issues.end(), [](const CharacterCreationRuleIssue& issue) {
                return issue.code == "appearance_forbidden";
            }) != issues.end());
    REQUIRE(std::find_if(issues.begin(), issues.end(), [](const CharacterCreationRuleIssue& issue) {
                return issue.code == "attribute_budget_exceeded";
            }) != issues.end());
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

TEST_CASE("CharacterIdentityValidator reports bounded catalog issues", "[character][identity][validation]") {
    CharacterIdentity identity;
    identity.setName("Nova");
    identity.setClassId("class_unknown");
    identity.setPortraitId("portrait_missing");
    identity.setBodySpriteId("sprite_missing");
    identity.addAppearanceToken("token_missing");
    identity.addAppearanceToken("token_missing");

    CharacterIdentityCatalog catalog;
    catalog.classIds = {"class_warrior", "class_mage"};
    catalog.portraitIds = {"portrait_warrior_01"};
    catalog.bodySpriteIds = {"sprite_warrior_body"};
    catalog.appearanceTokens = {"armor_steel"};

    CharacterIdentityValidator validator;
    const auto issues = validator.validate(identity, catalog);

    REQUIRE(issues.size() == 6);
    REQUIRE(issues[0].category == CharacterIdentityIssueCategory::UnknownClass);
    REQUIRE(issues[1].category == CharacterIdentityIssueCategory::UnknownPortrait);
    REQUIRE(issues[2].category == CharacterIdentityIssueCategory::UnknownBodySprite);
    REQUIRE(issues[3].category == CharacterIdentityIssueCategory::UnknownAppearanceToken);
    REQUIRE(issues[4].category == CharacterIdentityIssueCategory::UnknownAppearanceToken);
    REQUIRE(issues[5].category == CharacterIdentityIssueCategory::DuplicateAppearanceToken);
}

TEST_CASE("CharacterIdentityValidator: CI governance script validates artifacts",
          "[character][identity][validation][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_character_governance.ps1";
    const auto uniqueSuffix =
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    const auto outputPath =
        std::filesystem::temp_directory_path() / ("urpg_character_gov_out_" + uniqueSuffix + ".json");

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                         std::istreambuf_iterator<char>());
    resultFile.close();

    const auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}
