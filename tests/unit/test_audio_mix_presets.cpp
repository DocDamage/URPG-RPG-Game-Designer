#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include "engine/core/audio/audio_mix_presets.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/audio_mix_validator.h"
#include <nlohmann/json.hpp>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>

using namespace urpg::audio;
using Catch::Matchers::Equals;
using nlohmann::json;

TEST_CASE("AudioMixPresetBank default presets exist", "[audio][mix]") {
    AudioMixPresetBank bank;

    auto names = bank.listPresets();
    REQUIRE(names.size() == 3);

    REQUIRE(bank.loadPreset("Default").has_value());
    REQUIRE(bank.loadPreset("Battle").has_value());
    REQUIRE(bank.loadPreset("Cinematic").has_value());
    REQUIRE_FALSE(bank.loadPreset("NonExistent").has_value());

    auto defaultPreset = bank.loadPreset("Default");
    REQUIRE(defaultPreset->categoryVolumes.at(AudioCategory::BGM) == 1.0f);
    REQUIRE(defaultPreset->duckBGMOnSE == false);
    REQUIRE(defaultPreset->duckAmount == 0.0f);

    auto battlePreset = bank.loadPreset("Battle");
    REQUIRE(battlePreset->categoryVolumes.at(AudioCategory::BGM) == 0.8f);
    REQUIRE(battlePreset->categoryVolumes.at(AudioCategory::SE) == 1.2f);
    REQUIRE(battlePreset->duckBGMOnSE == true);
    REQUIRE(battlePreset->duckAmount == 0.5f);

    auto cinematicPreset = bank.loadPreset("Cinematic");
    REQUIRE(cinematicPreset->categoryVolumes.at(AudioCategory::BGS) == 0.0f);
    REQUIRE(cinematicPreset->categoryVolumes.at(AudioCategory::SE) == 0.6f);
}

TEST_CASE("AudioMixPresetBank save and load round-trip", "[audio][mix]") {
    AudioMixPresetBank bank;

    MixPreset custom;
    custom.name = "Custom";
    custom.categoryVolumes[AudioCategory::BGM] = 0.5f;
    custom.categoryVolumes[AudioCategory::SE] = 1.5f;
    custom.duckBGMOnSE = true;
    custom.duckAmount = 0.3f;
    bank.savePreset(custom);

    auto loaded = bank.loadPreset("Custom");
    REQUIRE(loaded.has_value());
    REQUIRE(loaded->name == "Custom");
    REQUIRE(loaded->categoryVolumes.at(AudioCategory::BGM) == 0.5f);
    REQUIRE(loaded->categoryVolumes.at(AudioCategory::SE) == 1.5f);
    REQUIRE(loaded->duckBGMOnSE == true);
    REQUIRE(loaded->duckAmount == 0.3f);
}

TEST_CASE("AudioMixPresetBank applyPreset changes AudioCore category volumes", "[audio][mix]") {
    AudioCore core;
    AudioMixPresetBank bank;

    REQUIRE(bank.applyPreset(core, "Battle"));
    REQUIRE(core.getCategoryVolume(AudioCategory::BGM) == 0.8f);
    REQUIRE(core.getCategoryVolume(AudioCategory::SE) == 1.2f);

    REQUIRE(bank.applyPreset(core, "Default"));
    REQUIRE(core.getCategoryVolume(AudioCategory::BGM) == 1.0f);
    REQUIRE(core.getCategoryVolume(AudioCategory::BGS) == 1.0f);
    REQUIRE(core.getCategoryVolume(AudioCategory::SE) == 1.0f);
    REQUIRE(core.getCategoryVolume(AudioCategory::ME) == 1.0f);
    REQUIRE(core.getCategoryVolume(AudioCategory::System) == 1.0f);

    REQUIRE_FALSE(bank.applyPreset(core, "Missing"));
}

TEST_CASE("AudioMixPresetBank removePreset deletes preset", "[audio][mix]") {
    AudioMixPresetBank bank;

    bank.removePreset("Battle");
    REQUIRE_FALSE(bank.loadPreset("Battle").has_value());

    auto names = bank.listPresets();
    REQUIRE(names.size() == 2);
}

TEST_CASE("AudioMixPresetBank listPresets returns names", "[audio][mix]") {
    AudioMixPresetBank bank;

    auto names = bank.listPresets();
    REQUIRE(names.size() == 3);
    REQUIRE(std::find(names.begin(), names.end(), "Default") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "Battle") != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "Cinematic") != names.end());
}

TEST_CASE("AudioMixPresetBank toJson and fromJson round-trip", "[audio][mix]") {
    AudioMixPresetBank bank;
    auto json = bank.toJson();

    REQUIRE(json["version"] == "1.0.0");
    REQUIRE(json["presets"].is_array());
    REQUIRE(json["presets"].size() == 3);

    AudioMixPresetBank restored;
    restored.fromJson(json);

    auto names = restored.listPresets();
    REQUIRE(names.size() == 3);
    REQUIRE(restored.loadPreset("Battle").has_value());
}

TEST_CASE("AudioMixPresetBank fromJson throws on invalid version", "[audio][mix]") {
    AudioMixPresetBank bank;
    nlohmann::json badJson;
    badJson["version"] = "2.0.0";
    badJson["presets"] = nlohmann::json::array();

    REQUIRE_THROWS(bank.fromJson(badJson));
}

TEST_CASE("AudioMixValidator: Default bank has no issues", "[audio][mix][validation]") {
    AudioMixPresetBank bank;
    AudioMixValidator validator;
    auto issues = validator.validate(bank);
    REQUIRE(issues.empty());
}

TEST_CASE("AudioMixValidator: Missing default preset is an error", "[audio][mix][validation]") {
    AudioMixPresetBank bank;
    bank.removePreset("Default");
    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    REQUIRE(issues.size() == 1);
    REQUIRE(issues[0].severity == AudioMixIssueSeverity::Error);
    REQUIRE(issues[0].category == AudioMixIssueCategory::MissingDefaultPreset);
}

TEST_CASE("AudioMixValidator: Conflicting duck rules are warnings", "[audio][mix][validation]") {
    AudioMixPresetBank bank;

    MixPreset badDuckEnabled;
    badDuckEnabled.name = "BadDuckEnabled";
    badDuckEnabled.duckBGMOnSE = true;
    badDuckEnabled.duckAmount = 0.0f;
    badDuckEnabled.categoryVolumes[AudioCategory::BGM] = 1.0f;
    bank.savePreset(badDuckEnabled);

    MixPreset badDuckDisabled;
    badDuckDisabled.name = "BadDuckDisabled";
    badDuckDisabled.duckBGMOnSE = false;
    badDuckDisabled.duckAmount = 0.5f;
    badDuckDisabled.categoryVolumes[AudioCategory::BGM] = 1.0f;
    bank.savePreset(badDuckDisabled);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    REQUIRE(issues.size() == 2);
    for (const auto& issue : issues) {
        REQUIRE(issue.severity == AudioMixIssueSeverity::Warning);
        REQUIRE(issue.category == AudioMixIssueCategory::ConflictingDuckRules);
    }
}

TEST_CASE("AudioMixValidator: Volume out of range is an error", "[audio][mix][validation]") {
    AudioMixPresetBank bank;

    MixPreset badVol;
    badVol.name = "BadVolume";
    badVol.categoryVolumes[AudioCategory::BGM] = -0.5f;
    badVol.categoryVolumes[AudioCategory::SE] = 2.5f;
    bank.savePreset(badVol);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto rangeIssues = std::count_if(issues.begin(), issues.end(),
        [](const AudioMixIssue& i) { return i.category == AudioMixIssueCategory::VolumeOutOfRange; });
    REQUIRE(rangeIssues == 2);
}

TEST_CASE("AudioMixValidator: Empty category volumes is a warning", "[audio][mix][validation]") {
    AudioMixPresetBank bank;

    MixPreset emptyCats;
    emptyCats.name = "EmptyCats";
    bank.savePreset(emptyCats);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto emptyIssues = std::count_if(issues.begin(), issues.end(),
        [](const AudioMixIssue& i) { return i.category == AudioMixIssueCategory::EmptyCategoryVolumes; });
    REQUIRE(emptyIssues == 1);
}

TEST_CASE("AudioMixValidator: CI governance script validates artifacts", "[audio][mix][validation][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_audio_governance.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_audio_gov_out.json";

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
