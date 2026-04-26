#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/audio_mixer.h"

using namespace urpg::audio;

TEST_CASE("AudioMixer routes spatial gain and pan into AudioCore snapshots", "[audio][spatial]") {
    AudioCore core;
    AudioMixer mixer;
    mixer.bindCore(&core);

    const AudioHandle handle = mixer.playSE("missing_spatial_test_asset", 100.0f, 0.0f);
    REQUIRE(handle > 0);

    mixer.update(0.0f, 0.0f);

    const auto source = mixer.getSource(handle);
    REQUIRE(source.has_value());
    REQUIRE(source->lastGain == Catch::Approx(0.01f));
    REQUIRE(source->lastPan == Catch::Approx(1.0f));

    const auto snapshots = core.activeSources();
    REQUIRE(snapshots.size() == 1);
    REQUIRE(snapshots.front().handle == handle);
    REQUIRE(snapshots.front().spatialGain == Catch::Approx(0.01f));
    REQUIRE(snapshots.front().pan == Catch::Approx(1.0f));
}

TEST_CASE("AudioMixer updates moving source pan and gain each frame", "[audio][spatial]") {
    AudioCore core;
    AudioMixer mixer;
    mixer.bindCore(&core);

    const AudioHandle handle = mixer.playSE("missing_spatial_move_asset", -50.0f, 0.0f);
    REQUIRE(handle > 0);

    mixer.update(0.0f, 0.0f);
    auto leftSource = mixer.getSource(handle);
    REQUIRE(leftSource.has_value());
    REQUIRE(leftSource->lastPan == Catch::Approx(-0.5f));

    REQUIRE(mixer.moveSource(handle, 0.0f, 0.0f));
    mixer.update(0.0f, 0.0f);
    auto centeredSource = mixer.getSource(handle);
    REQUIRE(centeredSource.has_value());
    REQUIRE(centeredSource->lastPan == Catch::Approx(0.0f));
    REQUIRE(centeredSource->lastGain == Catch::Approx(1.0f));

    const auto snapshots = core.activeSources();
    REQUIRE(snapshots.size() == 1);
    REQUIRE(snapshots.front().pan == Catch::Approx(0.0f));
    REQUIRE(snapshots.front().spatialGain == Catch::Approx(1.0f));
}

TEST_CASE("AudioMixer reports missing core and missing source handles", "[audio][spatial]") {
    AudioMixer mixer;

    const AudioHandle missingCoreHandle = mixer.playSE("unbound_asset", 0.0f, 0.0f);
    REQUIRE(missingCoreHandle == 0);
    REQUIRE_FALSE(mixer.diagnostics().empty());
    REQUIRE(mixer.diagnostics().back().code == "audio_mixer.core_missing");

    mixer.clearDiagnostics();
    REQUIRE_FALSE(mixer.moveSource(999, 1.0f, 1.0f));
    REQUIRE_FALSE(mixer.diagnostics().empty());
    REQUIRE(mixer.diagnostics().back().code == "audio_mixer.source_missing");
}

TEST_CASE("AudioMixer routes BGM through bound AudioCore", "[audio][spatial]") {
    AudioCore core;
    AudioMixer mixer;
    mixer.bindCore(&core);

    mixer.setBGM("spatial_bgm", 0.5f);
    REQUIRE(core.currentBGM() == "spatial_bgm");
}
