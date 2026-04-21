#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include "engine/core/audio/audio_mix_presets.h"
#include "engine/core/audio/audio_core.h"

using namespace urpg::audio;
using Catch::Matchers::Equals;

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
