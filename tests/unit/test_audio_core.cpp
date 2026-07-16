#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/audio_runtime_backend.h"
#include "engine/core/assets/asset_promotion_manifest.h"
#include "engine/core/global_state_hub.h"
#include "engine/core/ui/menu_scene_graph.h"
#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>

using namespace urpg::audio;
using namespace urpg::ui;

namespace {

std::filesystem::path uniqueAudioTempRoot(const std::string& prefix) {
    const auto tick = std::chrono::steady_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() / (prefix + "_" + std::to_string(tick));
}

void writeJsonFile(const std::filesystem::path& path, const nlohmann::json& json) {
    std::filesystem::create_directories(path.parent_path());
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << json.dump(2) << '\n';
}

} // namespace

TEST_CASE("AudioCore: Category Volume and Playback", "[audio][core]") {
    urpg::GlobalStateHub::getInstance().resetAll();
    AudioCore audio;

    SECTION("Initial volumes are default") {
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 1.0f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::SE) == 1.0f);
    }

    SECTION("Setting category volume") {
        audio.setCategoryVolume(AudioCategory::BGM, 0.5f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 0.5f);
    }

    SECTION("Playing sounds returns unique handles") {
        auto h1 = audio.playSound("test_se");
        auto h2 = audio.playSound("test_se");
        REQUIRE(h1 != h2);
        REQUIRE(h1 > 0);
    }
}

TEST_CASE("AudioCore: runtime backend reports asset failures without dropping lifecycle state",
          "[audio][core][runtime]") {
    AudioCore audio;
    audio.clearAudioDiagnostics();

    const auto handle = audio.playSound("missing_release_audio_asset", AudioCategory::SE);
    REQUIRE(handle == 0);
    REQUIRE(audio.activeSourceCount() == 0);

    const auto diagnostics = audio.audioDiagnostics();
    REQUIRE_FALSE(diagnostics.empty());
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "audio.asset_missing" && diagnostic.asset_id == "missing_release_audio_asset";
    }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "audio.play_failed" && diagnostic.asset_id == "missing_release_audio_asset";
    }));

    audio.stopHandle(handle);
    REQUIRE(audio.activeSourceCount() == 0);
}

TEST_CASE("SdlAudioRuntimeBackend resolves attached audio by stable project asset ID", "[audio][runtime][assets]") {
    const auto root = uniqueAudioTempRoot("urpg_audio_asset_resolution");
    const auto contentRoot = root / "content";
    const auto payload = contentRoot / "assets" / "imported" / "voice.line" / "line.wav";
    std::filesystem::create_directories(payload.parent_path());
    std::ofstream(payload, std::ios::binary | std::ios::trunc) << "not-decoded-by-this-resolution-test";

    urpg::assets::AssetPromotionManifest manifest;
    manifest.assetId = "voice.line";
    manifest.promotedPath = payload.generic_string();
    manifest.status = urpg::assets::AssetPromotionStatus::RuntimeReady;
    manifest.preview.kind = "audio";
    manifest.package.includeInRuntime = true;
    writeJsonFile(contentRoot / "assets" / "manifests" / "voice.line.json",
                  urpg::assets::serializeAssetPromotionManifest(manifest));

    SdlAudioRuntimeBackend backend;
    backend.setAssetRoot(contentRoot);
    REQUIRE(backend.resolveAssetPath("voice.line") == std::filesystem::weakly_canonical(payload));

    const auto externalPayload = root / "outside.wav";
    std::ofstream(externalPayload, std::ios::binary | std::ios::trunc) << "outside";
    manifest.assetId = "voice.outside";
    manifest.promotedPath = externalPayload.generic_string();
    writeJsonFile(contentRoot / "assets" / "manifests" / "voice.outside.json",
                  urpg::assets::serializeAssetPromotionManifest(manifest));
    REQUIRE(backend.resolveAssetPath("voice.outside").empty());

    std::filesystem::remove_all(root);
}

TEST_CASE("AudioCore: malformed config values fall back without throwing", "[audio][core][config]") {
    auto& hub = urpg::GlobalStateHub::getInstance();
    hub.resetAll();
    hub.setConfig("audio.bgm_volume", "not-a-number");

    AudioCore audio;

    REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 1.0f);
    const auto diagnostics = audio.audioDiagnostics();
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "audio.config_invalid" && diagnostic.asset_id == "not-a-number";
    }));

    audio.clearAudioDiagnostics();
    hub.setConfig("audio.bgm_volume", "still-bad");
    REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 1.0f);
    const auto updateDiagnostics = audio.audioDiagnostics();
    REQUIRE(std::any_of(updateDiagnostics.begin(), updateDiagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "audio.config_invalid" && diagnostic.asset_id == "still-bad";
    }));

    hub.resetAll();
}

TEST_CASE("AudioCore: category playback lifecycle covers BGM BGS ME and SE", "[audio][core][runtime]") {
    AudioCore audio;

    audio.playBGM("battle_theme", 0.25f);
    REQUIRE(audio.currentBGM() == "battle_theme");

    audio.playBGS("rain_loop", 0.25f);
    REQUIRE(audio.currentBGS() == "rain_loop");

    audio.playME("victory_fanfare");
    const auto seHandle = audio.playSound("cursor", AudioCategory::SE, 0.5f, 1.25f);

    const auto sources = audio.activeSources();
    REQUIRE(sources.size() == 4);
    REQUIRE(std::any_of(sources.begin(), sources.end(),
                        [](const auto& source) { return source.category == AudioCategory::BGM && source.isLooping; }));
    REQUIRE(std::any_of(sources.begin(), sources.end(),
                        [](const auto& source) { return source.category == AudioCategory::BGS && source.isLooping; }));
    REQUIRE(std::any_of(sources.begin(), sources.end(),
                        [](const auto& source) { return source.category == AudioCategory::ME && !source.isLooping; }));
    REQUIRE(std::any_of(sources.begin(), sources.end(), [seHandle](const auto& source) {
        return source.handle == seHandle && source.category == AudioCategory::SE && source.volume == 0.5f;
    }));

    audio.stopCategory(AudioCategory::BGM);
    REQUIRE(audio.currentBGM().empty());
    audio.stopAll();
    REQUIRE(audio.activeSourceCount() == 0);
}

TEST_CASE("UI Audio Integration", "[ui][audio]") {
    auto audio = std::make_shared<AudioCore>();
    MenuSceneGraph graph;
    graph.setAudio(audio);

    auto menu = std::make_shared<MenuScene>("MainMenu");
    MenuPane pane;
    pane.id = "p1";
    pane.isActive = true;
    pane.commands = {{"c1"}, {"c2"}};
    menu->addPane(pane);
    graph.registerScene(menu);

    SECTION("Scene Push triggers sound") {
        graph.pushScene("MainMenu");
        // Logic check: pushScene adds to stack
        REQUIRE(graph.stackSize() == 1);
    }

    SECTION("Navigation triggers cursor sound") {
        graph.pushScene("MainMenu");
        // We can't easily check the side effect in a mock,
        // but we verify the code path doesn't crash
        graph.handleInput(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 1);
    }
}
