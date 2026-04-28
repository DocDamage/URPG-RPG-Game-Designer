#include "engine/core/achievement/achievement_registry.h"
#include "engine/core/achievement/achievement_validator.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

using namespace urpg::achievement;
using nlohmann::json;

TEST_CASE("AchievementRegistry register and retrieve", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_001";
    def.title = "First Blood";
    def.description = "Defeat your first enemy.";
    def.secret = false;
    def.unlockCondition = "kill_count_1";
    def.iconId = "icon_sword";

    registry.registerAchievement(def);

    auto retrieved = registry.getAchievement("ach_001");
    REQUIRE(retrieved.has_value());
    REQUIRE(retrieved->id == "ach_001");
    REQUIRE(retrieved->title == "First Blood");

    auto all = registry.getAllAchievements();
    REQUIRE(all.size() == 1);
    REQUIRE(all[0].id == "ach_001");
}

TEST_CASE("AchievementRegistry progress increment unlocks at target", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_010";
    def.title = "Decimator";
    def.description = "Defeat ten enemies.";
    def.secret = false;
    def.unlockCondition = "kill_count_10";
    def.iconId = "icon_axe";

    registry.registerAchievement(def);

    REQUIRE_FALSE(registry.reportProgress("ach_010", 5));
    auto progress = registry.getProgress("ach_010");
    REQUIRE(progress.has_value());
    REQUIRE(progress->current == 5);
    REQUIRE_FALSE(progress->unlocked);

    REQUIRE(registry.reportProgress("ach_010", 5));
    progress = registry.getProgress("ach_010");
    REQUIRE(progress->current == 10);
    REQUIRE(progress->unlocked);
    REQUIRE(progress->unlockTime.has_value());
    REQUIRE(progress->unlockTime.value() == "deterministic_timestamp");
}

TEST_CASE("AchievementRegistry double-unlock returns false second time", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_001";
    def.title = "First Step";
    def.description = "Take a step.";
    def.secret = false;
    def.unlockCondition = "step_count_1";
    def.iconId = "icon_boot";

    registry.registerAchievement(def);

    REQUIRE(registry.reportProgress("ach_001", 1));
    REQUIRE_FALSE(registry.reportProgress("ach_001", 1));
}

TEST_CASE("AchievementRegistry reset clears progress", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_005";
    def.title = "Walker";
    def.description = "Walk five steps.";
    def.secret = false;
    def.unlockCondition = "step_count_5";
    def.iconId = "icon_walk";

    registry.registerAchievement(def);
    registry.reportProgress("ach_005", 5);

    auto progress = registry.getProgress("ach_005");
    REQUIRE(progress->unlocked);

    registry.resetProgress("ach_005");
    progress = registry.getProgress("ach_005");
    REQUIRE(progress->current == 0);
    REQUIRE_FALSE(progress->unlocked);
    REQUIRE_FALSE(progress->unlockTime.has_value());
}

TEST_CASE("AchievementRegistry save/load round-trip", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_003";
    def.title = "Collector";
    def.description = "Collect three items.";
    def.secret = false;
    def.unlockCondition = "collect_count_3";
    def.iconId = "icon_bag";

    registry.registerAchievement(def);
    registry.reportProgress("ach_003", 2);

    auto json = registry.saveToJson();
    REQUIRE(json["version"] == "1.0.0");
    REQUIRE(json["progress"].is_array());
    REQUIRE(json["progress"].size() == 1);

    AchievementRegistry loadedRegistry;
    loadedRegistry.registerAchievement(def);
    loadedRegistry.loadFromJson(json);

    auto progress = loadedRegistry.getProgress("ach_003");
    REQUIRE(progress.has_value());
    REQUIRE(progress->current == 2);
    REQUIRE(progress->target == 3);
    REQUIRE_FALSE(progress->unlocked);
}

TEST_CASE("AchievementRegistry unknown id in load is ignored", "[achievement]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_known";
    def.title = "Known";
    def.description = "A known achievement.";
    def.secret = false;
    def.unlockCondition = "count_1";
    def.iconId = "icon_known";

    registry.registerAchievement(def);

    nlohmann::json json = {
        {"version", "1.0.0"},
        {"progress", nlohmann::json::array({
            {
                {"id", "ach_known"},
                {"current", 1},
                {"target", 1},
                {"unlocked", true},
                {"unlockTime", "deterministic_timestamp"}
            },
            {
                {"id", "ach_unknown"},
                {"current", 5},
                {"target", 10},
                {"unlocked", false}
            }
        })}
    };

    registry.loadFromJson(json);

    auto progress = registry.getProgress("ach_known");
    REQUIRE(progress.has_value());
    REQUIRE(progress->unlocked);

    auto unknown = registry.getProgress("ach_unknown");
    REQUIRE_FALSE(unknown.has_value());
}

TEST_CASE("AchievementRegistry exports vendor-neutral trophy payload", "[achievement][export]") {
    AchievementRegistry registry;

    AchievementDef first;
    first.id = "ach_001";
    first.title = "First Blood";
    first.description = "Defeat one enemy.";
    first.secret = false;
    first.unlockCondition = "kill_count_1";
    first.iconId = "icon_sword";

    AchievementDef secret;
    secret.id = "ach_secret";
    secret.title = "Hidden Door";
    secret.description = "Find the hidden door.";
    secret.secret = true;
    secret.unlockCondition = "door_find_3";
    secret.iconId = "icon_door";

    registry.registerAchievement(first);
    registry.registerAchievement(secret);
    REQUIRE(registry.reportProgress("ach_001", 1));
    registry.reportProgress("ach_secret", 2);

    const auto payload = registry.exportTrophyPayload("urpg-neutral");

    REQUIRE(payload["version"] == "1.0.0");
    REQUIRE(payload["platform"] == "urpg-neutral");
    REQUIRE(payload["backendIntegration"] == "not_configured");
    REQUIRE(payload["summary"]["total"] == 2);
    REQUIRE(payload["summary"]["unlocked"] == 1);
    REQUIRE(payload["summary"]["secret"] == 1);
    REQUIRE(payload["trophies"].is_array());
    REQUIRE(payload["trophies"].size() == 2);

    REQUIRE(payload["trophies"][0]["id"] == "ach_001");
    REQUIRE(payload["trophies"][0]["target"] == 1);
    REQUIRE(payload["trophies"][0]["progress"] == 1);
    REQUIRE(payload["trophies"][0]["unlocked"] == true);
    REQUIRE(payload["trophies"][0]["unlockTime"] == "deterministic_timestamp");

    REQUIRE(payload["trophies"][1]["id"] == "ach_secret");
    REQUIRE(payload["trophies"][1]["secret"] == true);
    REQUIRE(payload["trophies"][1]["target"] == 3);
    REQUIRE(payload["trophies"][1]["progress"] == 2);
    REQUIRE(payload["trophies"][1]["unlocked"] == false);
    REQUIRE_FALSE(payload["trophies"][1].contains("unlockTime"));
}

TEST_CASE("AchievementRegistry syncs unlocks to configured platform backend",
          "[achievement][platform]") {
    AchievementRegistry registry;

    auto backend = std::make_shared<MemoryAchievementPlatformBackend>("steam");
    registry.addPlatformBackend(backend);

    AchievementDef def;
    def.id = "ach_platform";
    def.title = "Platform";
    def.description = "Submit to backend.";
    def.secret = false;
    def.unlockCondition = "count_1";
    def.iconId = "icon_platform";

    registry.registerAchievement(def);
    REQUIRE(registry.reportProgress("ach_platform", 1));

    const auto snapshot = backend->snapshot();
    REQUIRE(snapshot["platform"] == "steam");
    REQUIRE(snapshot["submittedCount"] == 1);
    REQUIRE(snapshot["updates"][0]["achievementId"] == "ach_platform");
    REQUIRE(snapshot["updates"][0]["unlocked"] == true);
    REQUIRE(registry.exportTrophyPayload("steam")["backendIntegration"] == "configured");
}

TEST_CASE("AchievementRegistry trophy payload defaults to neutral platform", "[achievement][export]") {
    AchievementRegistry registry;

    AchievementDef def;
    def.id = "ach_001";
    def.title = "First Blood";
    def.description = "Defeat one enemy.";
    def.secret = false;
    def.unlockCondition = "kill_count_1";
    def.iconId = "icon_sword";

    registry.registerAchievement(def);

    REQUIRE(registry.exportTrophyPayload("")["platform"] == "urpg-neutral");
}

TEST_CASE("AchievementValidator: Valid definitions produce no issues", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"ach_001", "First Blood", "Defeat your first enemy.", false, "kill_count_1", "icon_sword", 0},
        {"ach_010", "Decimator", "Defeat ten enemies.", false, "kill_count_10", "icon_axe", 0}
    };

    auto issues = validator.validate(defs);
    REQUIRE(issues.empty());
}

TEST_CASE("AchievementValidator: Empty id is an error", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"", "No ID", "Missing id.", false, "count_1", "icon", 0}
    };

    auto issues = validator.validate(defs);
    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].severity == AchievementIssueSeverity::Error);
    REQUIRE(issues[0].category == AchievementIssueCategory::EmptyId);
}

TEST_CASE("AchievementValidator: Duplicate id is an error", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"ach_dup", "First", "First desc.", false, "count_1", "icon1", 0},
        {"ach_dup", "Second", "Second desc.", false, "count_2", "icon2", 0}
    };

    auto issues = validator.validate(defs);

    auto dupIssues = std::count_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::DuplicateId; });
    REQUIRE(dupIssues == 1);
    REQUIRE(std::find_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::EmptyTitle; }) == issues.end());
}

TEST_CASE("AchievementValidator: Empty title is an error", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"ach_001", "", "No title.", false, "count_1", "icon", 0}
    };

    auto issues = validator.validate(defs);

    auto titleIssues = std::count_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::EmptyTitle; });
    REQUIRE(titleIssues == 1);
}

TEST_CASE("AchievementValidator: Empty unlock condition is an error", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"ach_001", "Bad Condition", "No condition.", false, "", "icon", 0}
    };

    auto issues = validator.validate(defs);

    auto condIssues = std::count_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::EmptyUnlockCondition; });
    REQUIRE(condIssues == 1);
}

TEST_CASE("AchievementValidator: Zero target is a warning", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"ach_001", "Zero Target", "Condition with explicit target 0.", false, "kill_count_0", "icon", 0}
    };

    auto issues = validator.validate(defs);

    auto targetIssues = std::count_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::ZeroTarget; });
    REQUIRE(targetIssues == 1);
    REQUIRE(std::find_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::ZeroTarget; })->severity == AchievementIssueSeverity::Warning);
}

TEST_CASE("AchievementValidator: Empty iconId is a warning", "[achievement][validation]") {
    AchievementValidator validator;
    std::vector<AchievementDef> defs = {
        {"ach_001", "No Icon", "Missing icon.", false, "count_1", "", 0}
    };

    auto issues = validator.validate(defs);

    auto iconIssues = std::count_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::EmptyIconId; });
    REQUIRE(iconIssues == 1);
    REQUIRE(std::find_if(issues.begin(), issues.end(),
        [](const AchievementIssue& i) { return i.category == AchievementIssueCategory::EmptyIconId; })->severity == AchievementIssueSeverity::Warning);
}

TEST_CASE("AchievementValidator: CI governance script validates artifacts", "[achievement][validation][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_achievement_governance.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_achievement_gov_out.json";

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() +
        "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)),
                         std::istreambuf_iterator<char>());
    resultFile.close();

    auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}
