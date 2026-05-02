#include "editor/audio/audio_mix_panel.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/audio_mix_backend_smoke.h"
#include "engine/core/audio/audio_mix_presets.h"
#include "engine/core/audio/audio_mix_validator.h"
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_vector.hpp>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
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

    const auto conflictingDuckRuleIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& issue) {
        return issue.category == AudioMixIssueCategory::ConflictingDuckRules;
    });

    REQUIRE(conflictingDuckRuleIssues == 2);
    for (const auto& issue : issues) {
        if (issue.category == AudioMixIssueCategory::ConflictingDuckRules) {
            REQUIRE(issue.severity == AudioMixIssueSeverity::Warning);
        }
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

    auto rangeIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::VolumeOutOfRange;
    });
    REQUIRE(rangeIssues == 2);
}

TEST_CASE("AudioMixValidator: Empty category volumes is a warning", "[audio][mix][validation]") {
    AudioMixPresetBank bank;

    MixPreset emptyCats;
    emptyCats.name = "EmptyCats";
    bank.savePreset(emptyCats);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto emptyIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::EmptyCategoryVolumes;
    });
    REQUIRE(emptyIssues == 1);
}

TEST_CASE("AudioMixValidator: CI governance script validates artifacts",
          "[audio][mix][validation][project_audit_cli]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto scriptPath = repoRoot / "tools" / "ci" / "check_audio_governance.ps1";
    const auto outputPath = std::filesystem::temp_directory_path() / "urpg_audio_gov_out.json";

    const std::string command =
        "powershell -ExecutionPolicy Bypass -File \"" + scriptPath.string() + "\" > \"" + outputPath.string() + "\"";

    REQUIRE(std::system(command.c_str()) == 0);

    std::ifstream resultFile(outputPath);
    REQUIRE(resultFile.is_open());

    std::string jsonStr((std::istreambuf_iterator<char>(resultFile)), std::istreambuf_iterator<char>());
    resultFile.close();

    auto result = json::parse(jsonStr);
    REQUIRE(result["passed"].get<bool>() == true);
    REQUIRE(result["errors"].is_array());
    REQUIRE(result["errors"].empty());
}

TEST_CASE("AudioMixValidator: unknown category name from JSON produces a warning", "[audio][mix][validation]") {
    nlohmann::json j;
    j["version"] = "1.0.0";
    j["presets"] = nlohmann::json::array();

    nlohmann::json p;
    p["name"] = "Default";
    p["duckBGMOnSE"] = false;
    p["duckAmount"] = 0.0f;
    p["categoryVolumes"] = nlohmann::json::object();
    p["categoryVolumes"]["BGM"] = 1.0f;
    p["categoryVolumes"]["UNKNOWN_CATEGORY"] = 0.5f;
    j["presets"].push_back(p);

    AudioMixPresetBank bank;
    bank.fromJson(j);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto unknownIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::UnknownCategory;
    });
    REQUIRE(unknownIssues == 1);

    const auto it = std::find_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::UnknownCategory;
    });
    REQUIRE(it != issues.end());
    REQUIRE(it->severity == AudioMixIssueSeverity::Warning);
    REQUIRE(it->presetName == "Default");
    REQUIRE(it->message.find("UNKNOWN_CATEGORY") != std::string::npos);
}

TEST_CASE("AudioMixValidator: multiple unknown category names produce one warning each", "[audio][mix][validation]") {
    nlohmann::json j;
    j["version"] = "1.0.0";
    j["presets"] = nlohmann::json::array();

    nlohmann::json p;
    p["name"] = "Default";
    p["duckBGMOnSE"] = false;
    p["duckAmount"] = 0.0f;
    p["categoryVolumes"] = nlohmann::json::object();
    p["categoryVolumes"]["BGM"] = 1.0f;
    p["categoryVolumes"]["BADCAT_A"] = 0.4f;
    p["categoryVolumes"]["BADCAT_B"] = 0.6f;
    j["presets"].push_back(p);

    AudioMixPresetBank bank;
    bank.fromJson(j);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto unknownIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::UnknownCategory;
    });
    REQUIRE(unknownIssues == 2);
}

TEST_CASE("AudioMixValidator: cross-preset duck conflict produces a warning", "[audio][mix][validation]") {
    AudioMixPresetBank bank;
    // Default bank already contains Battle (duckBGMOnSE=true, duckAmount=0.5).
    // Adding a second preset with duckBGMOnSE=true and a different duckAmount triggers the conflict rule.
    MixPreset conflicting;
    conflicting.name = "ConflictingDuck";
    conflicting.duckBGMOnSE = true;
    conflicting.duckAmount = 0.3f;
    conflicting.categoryVolumes[AudioCategory::BGM] = 1.0f;
    bank.savePreset(conflicting);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto conflictIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::CrossPresetDuckConflict;
    });
    REQUIRE(conflictIssues == 1);

    const auto it = std::find_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::CrossPresetDuckConflict;
    });
    REQUIRE(it->severity == AudioMixIssueSeverity::Warning);
    REQUIRE(it->message.find("Battle") != std::string::npos);
    REQUIRE(it->message.find("ConflictingDuck") != std::string::npos);
}

TEST_CASE("AudioMixValidator: two presets with same duck amount do not trigger cross-preset conflict",
          "[audio][mix][validation]") {
    AudioMixPresetBank bank;
    // Battle: duckBGMOnSE=true, duckAmount=0.5
    MixPreset sameDuck;
    sameDuck.name = "SameDuck";
    sameDuck.duckBGMOnSE = true;
    sameDuck.duckAmount = 0.5f; // Same as Battle — no conflict
    sameDuck.categoryVolumes[AudioCategory::BGM] = 1.0f;
    bank.savePreset(sameDuck);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    auto conflictIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::CrossPresetDuckConflict;
    });
    REQUIRE(conflictIssues == 0);
}

TEST_CASE("AudioMixValidator: negative fixture cases load and produce expected validator issues",
          "[audio][mix][validation]") {
    const auto repoRoot = std::filesystem::path(__FILE__).parent_path().parent_path().parent_path();
    const auto fixturePath = repoRoot / "content" / "fixtures" / "audio_mix_presets_fixture.json";
    REQUIRE(std::filesystem::exists(fixturePath));

    std::ifstream f(fixturePath);
    REQUIRE(f.is_open());
    const auto fixtureJson = nlohmann::json::parse(f);

    AudioMixPresetBank bank;
    bank.fromJson(fixtureJson);

    // Verify negative presets were loaded
    REQUIRE(bank.loadPreset("NegativeUnknownCategory").has_value());
    REQUIRE(bank.loadPreset("NegativeDuckConflict").has_value());

    // The NegativeUnknownCategory preset should have its unknown name tracked
    const auto negUnknown = bank.loadPreset("NegativeUnknownCategory");
    REQUIRE_FALSE(negUnknown->unknownCategoryNames.empty());
    const bool hasUnknownCat =
        std::find(negUnknown->unknownCategoryNames.begin(), negUnknown->unknownCategoryNames.end(),
                  "UNKNOWN_CATEGORY") != negUnknown->unknownCategoryNames.end();
    REQUIRE(hasUnknownCat);

    AudioMixValidator validator;
    auto issues = validator.validate(bank);

    // Should have at least one UnknownCategory warning
    auto unknownIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::UnknownCategory;
    });
    REQUIRE(unknownIssues >= 1);

    // Should have a CrossPresetDuckConflict warning (Battle @ 0.5 vs NegativeDuckConflict @ 0.3)
    auto conflictIssues = std::count_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::CrossPresetDuckConflict;
    });
    REQUIRE(conflictIssues == 1);
    const auto conflictIt = std::find_if(issues.begin(), issues.end(), [](const AudioMixIssue& i) {
        return i.category == AudioMixIssueCategory::CrossPresetDuckConflict;
    });
    REQUIRE(conflictIt->message.find("NegativeDuckConflict") != std::string::npos);
}

// ─── S28-T03: Audio presets connected to live runtime backend ─────────────────

TEST_CASE("applyPreset forwards duck parameters to AudioCore", "[audio][mix][runtime][s28t03]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    // Battle preset has duckBGMOnSE=true and duckAmount=0.5 (canonical fixture).
    REQUIRE(bank.applyPreset(core, "Battle"));
    REQUIRE(core.getDuckBGMOnSE() == true);
    REQUIRE(core.getDuckAmount() == Catch::Approx(0.5f));

    // Default preset should disable ducking.
    REQUIRE(bank.applyPreset(core, "Default"));
    REQUIRE(core.getDuckBGMOnSE() == false);
}

TEST_CASE("AudioMixPanel selectPreset applies preset to bound AudioCore", "[audio][mix][runtime][s28t03]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    urpg::editor::AudioMixPanel panel;
    panel.bindBank(&bank);
    panel.bindCore(&core);

    REQUIRE(panel.selectPreset("Battle"));
    REQUIRE(core.getDuckBGMOnSE() == true);

    REQUIRE(panel.selectPreset("Default"));
    REQUIRE(core.getDuckBGMOnSE() == false);
}

TEST_CASE("AudioMixPanel selectPreset returns false for unknown preset", "[audio][mix][runtime][s28t03]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    urpg::editor::AudioMixPanel panel;
    panel.bindBank(&bank);
    panel.bindCore(&core);

    REQUIRE_FALSE(panel.selectPreset("NonExistent"));
    // Core duck state should remain at default.
    REQUIRE(core.getDuckBGMOnSE() == false);
}

TEST_CASE("AudioMixPanel render snapshot includes liveCore duck state when core is bound",
          "[audio][mix][runtime][s28t03]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    urpg::editor::AudioMixPanel panel;
    panel.bindBank(&bank);
    panel.bindCore(&core);
    panel.selectPreset("Battle");
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.contains("liveCore"));
    REQUIRE(snapshot["liveCore"]["duckBGMOnSE"] == true);
    REQUIRE(snapshot["liveCore"]["duckAmount"].get<float>() == Catch::Approx(0.5f));
}

TEST_CASE("AudioMixPanel render snapshot has no liveCore key when no core bound", "[audio][mix][runtime][s28t03]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();

    urpg::editor::AudioMixPanel panel;
    panel.bindBank(&bank);
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE_FALSE(snapshot.contains("liveCore"));
    REQUIRE(snapshot["bankBound"] == true);
    REQUIRE(snapshot["coreBound"] == false);
    REQUIRE(snapshot["actions"]["selectPreset"] == true);
    REQUIRE(snapshot["actions"]["applyToLiveCore"] == false);
    REQUIRE(snapshot["statusMessages"].size() == 1);
    REQUIRE(snapshot["statusMessages"][0] ==
            "No live AudioCore is bound; preset selection will not affect runtime audio.");
}

TEST_CASE("AudioMixPanel render snapshot explains missing preset bank and core", "[audio][mix][runtime][s28t03]") {
    urpg::editor::AudioMixPanel panel;
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["bankBound"] == false);
    REQUIRE(snapshot["coreBound"] == false);
    REQUIRE(snapshot["presetCount"] == 0);
    REQUIRE(snapshot["actions"]["selectPreset"] == false);
    REQUIRE(snapshot["actions"]["applyToLiveCore"] == false);
    REQUIRE(snapshot["statusMessages"].size() == 2);
    REQUIRE(snapshot["statusMessages"][0] == "No audio mix preset bank is bound.");
    REQUIRE(snapshot["statusMessages"][1] ==
            "No live AudioCore is bound; preset selection will not affect runtime audio.");
}

TEST_CASE("Audio mix backend smoke applies preset and records live backend diagnostics",
          "[audio][mix][backend_smoke]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    AudioMixBackendSmokeRequest request;
    request.presetName = "Battle";
    const auto result = runAudioMixBackendSmoke(bank, core, request);
    const auto jsonResult = audioMixBackendSmokeResultToJson(result);

    REQUIRE(result.presetFound);
    REQUIRE(result.presetApplied);
    REQUIRE(result.probeAttempted);
    REQUIRE(result.probeAccepted);
    REQUIRE(result.duckBGMOnSE);
    REQUIRE(result.duckAmount == Catch::Approx(0.5f));
    REQUIRE(result.categoryVolumes.at(AudioCategory::BGM) == Catch::Approx(0.8f));
    REQUIRE(result.categoryVolumes.at(AudioCategory::SE) == Catch::Approx(1.2f));
    REQUIRE(jsonResult["probeAccepted"] == true);
    REQUIRE(jsonResult["deviceOpenBefore"].is_boolean());
    REQUIRE(jsonResult["deviceOpenAfter"].is_boolean());
    REQUIRE(jsonResult["diagnostics"].is_array());

    const auto hasSyntheticProbeDiagnostic = std::any_of(result.diagnostics.begin(), result.diagnostics.end(),
                                                         [](const AudioBackendDiagnostic& diagnostic) {
                                                             return diagnostic.code == "audio.synthetic_asset";
                                                         });
    REQUIRE(hasSyntheticProbeDiagnostic);
}

TEST_CASE("Audio mix backend smoke reports missing presets without playback",
          "[audio][mix][backend_smoke]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    AudioMixBackendSmokeRequest request;
    request.presetName = "MissingPreset";
    const auto result = runAudioMixBackendSmoke(bank, core, request);

    REQUIRE_FALSE(result.presetFound);
    REQUIRE_FALSE(result.presetApplied);
    REQUIRE_FALSE(result.probeAttempted);
    REQUIRE_FALSE(result.probeAccepted);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0].code == "audio.mix_preset_missing");
    REQUIRE(result.diagnostics[0].asset_id == "MissingPreset");
}

TEST_CASE("AudioMixPanel exposes backend smoke action and snapshot",
          "[audio][mix][backend_smoke]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();
    AudioCore core;

    urpg::editor::AudioMixPanel panel;
    panel.bindBank(&bank);
    panel.bindCore(&core);

    REQUIRE(panel.runBackendSmoke("Battle"));
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["actions"]["runBackendSmoke"] == true);
    REQUIRE(snapshot["backendSmoke"]["presetName"] == "Battle");
    REQUIRE(snapshot["backendSmoke"]["presetApplied"] == true);
    REQUIRE(snapshot["backendSmoke"]["probeAttempted"] == true);
    REQUIRE(snapshot["backendSmoke"]["probeAccepted"] == true);
    REQUIRE(snapshot["backendSmoke"]["duckBGMOnSE"] == true);
    REQUIRE(snapshot["liveCore"]["duckAmount"].get<float>() == Catch::Approx(0.5f));
}

TEST_CASE("Audio backend matrix evidence reports backend device state and muted fallback",
          "[audio][mix][backend_matrix][task5]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();

    const auto fixtures = defaultAudioBackendMatrixFixtures();
    REQUIRE(fixtures.size() == 6);

    const auto results = evaluateAudioBackendMatrix(bank, fixtures);
    REQUIRE(results.size() == fixtures.size());

    const auto findResult = [&](const std::string& id) -> const AudioBackendMatrixResult& {
        const auto it = std::find_if(results.begin(), results.end(), [&](const AudioBackendMatrixResult& result) {
            return result.fixtureId == id;
        });
        REQUIRE(it != results.end());
        return *it;
    };

    const auto& nullBackend = findResult("null_backend");
    REQUIRE(nullBackend.backendId == "null");
    REQUIRE(nullBackend.deviceState == "not_applicable");
    REQUIRE(nullBackend.presetApplied);
    REQUIRE_FALSE(nullBackend.playbackActive);
    REQUIRE(nullBackend.fallbackPolicy == "silent_noop");

    const auto& sdlAvailable = findResult("sdl_backend_available");
    REQUIRE(sdlAvailable.backendId == "sdl");
    REQUIRE(sdlAvailable.deviceState == "available");
    REQUIRE(sdlAvailable.presetApplied);
    REQUIRE(sdlAvailable.playbackActive);
    REQUIRE(sdlAvailable.fallbackPolicy == "normal_playback");

    const auto& sdlUnavailable = findResult("sdl_backend_unavailable");
    REQUIRE(sdlUnavailable.backendId == "sdl");
    REQUIRE(sdlUnavailable.deviceState == "backend_unavailable");
    REQUIRE(sdlUnavailable.presetApplied);
    REQUIRE_FALSE(sdlUnavailable.playbackActive);
    REQUIRE(sdlUnavailable.lastPlaybackDiagnostic.code == "audio.backend_unavailable");

    const auto& missingDevice = findResult("missing_output_device");
    REQUIRE(missingDevice.deviceState == "missing_output_device");
    REQUIRE_FALSE(missingDevice.playbackActive);
    REQUIRE(missingDevice.fallbackPolicy == "muted_release_fallback");
    REQUIRE(missingDevice.releaseSafe);

    const auto& stereo = findResult("stereo_output");
    REQUIRE(stereo.deviceState == "stereo_output");
    REQUIRE(stereo.channelCount == 2);
    REQUIRE(stereo.playbackActive);

    const auto& muted = findResult("muted_release_fallback");
    REQUIRE_FALSE(muted.playbackActive);
    REQUIRE(muted.fallbackPolicy == "muted_release_fallback");
    REQUIRE(muted.lastPlaybackDiagnostic.code == "audio.muted_release_fallback");
}

TEST_CASE("AudioMixPanel exposes backend matrix evidence in snapshot",
          "[audio][mix][backend_matrix][task5]") {
    AudioMixPresetBank bank;
    bank.loadDefaults();

    urpg::editor::AudioMixPanel panel;
    panel.bindBank(&bank);
    panel.setBackendMatrixEvidence(evaluateAudioBackendMatrix(bank, defaultAudioBackendMatrixFixtures()));
    panel.render();

    const auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot["backendMatrix"]["fixtureCount"] == 6);
    REQUIRE(snapshot["backendMatrix"]["releaseSafeMutedFallbackCount"] == 2);
    REQUIRE(snapshot["backendMatrix"]["results"].is_array());
    REQUIRE(snapshot["backendMatrix"]["results"].size() == 6);
    REQUIRE(snapshot["backendMatrix"]["results"][0].contains("backendId"));
    REQUIRE(snapshot["backendMatrix"]["results"][0].contains("deviceState"));
    REQUIRE(snapshot["backendMatrix"]["results"][0].contains("fallbackPolicy"));
    REQUIRE(snapshot["backendMatrix"]["results"][0].contains("lastPlaybackDiagnostic"));
}
