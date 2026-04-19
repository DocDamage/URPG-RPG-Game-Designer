#include "runtimes/compat_js/audio_manager.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace urpg::compat;

TEST_CASE("AudioManager: channel lifecycle", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();

    uint32_t id = am.createChannel("test_bgm", AudioBus::BGM);
    REQUIRE(id > 0);

    AudioChannel* byName = am.getChannel("test_bgm");
    REQUIRE(byName != nullptr);
    REQUIRE(byName->getBus() == AudioBus::BGM);

    AudioChannel* byId = am.getChannel(id);
    REQUIRE(byId != nullptr);
    REQUIRE(byId->getBus() == AudioBus::BGM);

    am.destroyChannel(id);
    REQUIRE(am.getChannel(id) == nullptr);
}

TEST_CASE("AudioManager: BGM controls", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();

    am.playBgm("battle_theme", 80.0, 100.0, 42);
    REQUIRE(am.isBgmPlaying());
    REQUIRE_FALSE(am.isBgmPaused());
    REQUIRE(am.getCurrentBgm().name == "battle_theme");

    am.pauseBgm();
    REQUIRE(am.isBgmPaused());

    am.resumeBgm();
    REQUIRE(am.isBgmPlaying());
    REQUIRE_FALSE(am.isBgmPaused());

    am.saveBgmSettings();
    am.playBgm("other_theme", 50.0, 90.0, 0);
    am.restoreBgmSettings();
    REQUIRE(am.isBgmPlaying());
    AudioInfo restored = am.getCurrentBgm();
    REQUIRE(restored.name == "battle_theme");
    REQUIRE(restored.volume == Catch::Approx(80.0));
    REQUIRE(restored.pitch == Catch::Approx(100.0));
    REQUIRE(restored.pos == 42);

    am.stopBgm();
    REQUIRE_FALSE(am.isBgmPlaying());
}

TEST_CASE("AudioManager: BGM position advances while playing and pauses cleanly", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopBgm();

    am.playBgm("position_theme", 90.0, 100.0, 10);
    REQUIRE(am.getCurrentBgm().pos == 10);

    am.update();
    REQUIRE(am.getCurrentBgm().pos == 11);

    am.update();
    REQUIRE(am.getCurrentBgm().pos == 12);

    am.pauseBgm();
    am.update();
    REQUIRE(am.getCurrentBgm().pos == 12);

    am.resumeBgm();
    am.update();
    REQUIRE(am.getCurrentBgm().pos == 13);

    am.stopBgm();
}

TEST_CASE("AudioManager: deterministic crossfade progression", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();

    am.playBgm("theme_a", 100.0, 100.0, 0);
    am.crossfadeBgm("theme_b", 60.0, 100.0, 4);

    am.update();
    AudioInfo frame1 = am.getCurrentBgm();
    REQUIRE(frame1.name == "theme_a");
    REQUIRE(frame1.volume == Catch::Approx(50.0));

    am.update();
    AudioInfo frame2 = am.getCurrentBgm();
    REQUIRE(frame2.name == "theme_b");
    REQUIRE(frame2.volume == Catch::Approx(0.0));

    am.update();
    AudioInfo frame3 = am.getCurrentBgm();
    REQUIRE(frame3.name == "theme_b");
    REQUIRE(frame3.volume == Catch::Approx(30.0));

    am.update();
    AudioInfo frame4 = am.getCurrentBgm();
    REQUIRE(frame4.name == "theme_b");
    REQUIRE(frame4.volume == Catch::Approx(60.0));
    am.stopBgm();

    am.playBgs("rain_a", 80.0, 100.0, 0);
    am.crossfadeBgs("rain_b", 40.0, 100.0, 2);
    am.update();

    AudioChannel* bgs = am.getChannel("bgs");
    REQUIRE(bgs != nullptr);
    REQUIRE(bgs->getFilename() == "rain_b");
    REQUIRE(bgs->getVolume() == Catch::Approx(0.0));

    am.update();
    REQUIRE(bgs->getFilename() == "rain_b");
    REQUIRE(bgs->getVolume() == Catch::Approx(0.4));
    am.stopBgs();
}

TEST_CASE("AudioManager: buses and ducking", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopBgm();
    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);
    am.setBusVolume(AudioBus::SE, 1.0);

    am.setMasterVolume(0.8);
    REQUIRE(am.getMasterVolume() == Catch::Approx(0.8));

    am.setBusVolume(AudioBus::SE, 1.5);
    REQUIRE(am.getBusVolume(AudioBus::SE) == Catch::Approx(1.0));

    am.setBusVolume(AudioBus::SE, -0.5);
    REQUIRE(am.getBusVolume(AudioBus::SE) == Catch::Approx(0.0));

    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);

    am.playBgm("duck_test", 90.0, 100.0);
    AudioChannel* bgm = am.getChannel("bgm");
    REQUIRE(bgm != nullptr);
    REQUIRE(bgm->getVolume() == Catch::Approx(0.9));

    am.duckBgm(30.0, 3);
    REQUIRE(am.isBgmDucked());
    REQUIRE(bgm->getVolume() == Catch::Approx(0.9));

    am.update();
    REQUIRE(bgm->getVolume() == Catch::Approx(0.7));

    am.update();
    REQUIRE(bgm->getVolume() == Catch::Approx(0.5));

    am.update();
    REQUIRE(bgm->getVolume() == Catch::Approx(0.3));

    am.unduckBgm(2);
    am.update();
    REQUIRE(bgm->getVolume() == Catch::Approx(0.6));

    am.update();
    REQUIRE(bgm->getVolume() == Catch::Approx(0.9));
    REQUIRE_FALSE(am.isBgmDucked());

    am.stopBgm();
}

TEST_CASE("AudioManager: duck and unduck preserve deterministic BGM state", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    struct AudioStateCleanupGuard {
        AudioManager& am;
        ~AudioStateCleanupGuard() {
            am.stopBgm();
            am.setMasterVolume(1.0);
            am.setBusVolume(AudioBus::BGM, 1.0);
        }
    } cleanup{am};

    am.stopBgm();
    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);
    am.playBgm("phase2_theme", 80.0, 100.0, 4);
    REQUIRE(am.getCurrentBgm().name == "phase2_theme");
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(80.0));

    am.setMasterVolume(0.5);
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(40.0));

    am.setBusVolume(AudioBus::BGM, 0.25);
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(10.0));

    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(80.0));

    am.duckBgm(30.0, 3);
    REQUIRE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().name == "phase2_theme");
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(80.0));

    am.update();
    REQUIRE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(63.3333333333));

    am.update();
    REQUIRE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(46.6666666667));

    am.update();
    REQUIRE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(30.0));

    am.unduckBgm(2);
    am.update();
    REQUIRE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(55.0));

    am.setMasterVolume(0.75);
    am.setBusVolume(AudioBus::BGM, 0.5);
    am.update();
    REQUIRE_FALSE(am.isBgmDucked());
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(30.0));
}

TEST_CASE("AudioManager: master and bus volumes affect active playback", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopBgm();
    am.stopBgs();
    am.stopSe();
    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);
    am.setBusVolume(AudioBus::BGS, 1.0);
    am.setBusVolume(AudioBus::SE, 1.0);

    am.playBgm("mix_theme", 80.0, 100.0);
    AudioChannel* bgm = am.getChannel("bgm");
    REQUIRE(bgm != nullptr);
    REQUIRE(bgm->getVolume() == Catch::Approx(0.8));

    am.setMasterVolume(0.5);
    REQUIRE(bgm->getVolume() == Catch::Approx(0.4));

    am.setBusVolume(AudioBus::BGM, 0.25);
    REQUIRE(bgm->getVolume() == Catch::Approx(0.1));

    am.playBgs("mix_rain", 60.0, 100.0);
    AudioChannel* bgs = am.getChannel("bgs");
    REQUIRE(bgs != nullptr);
    REQUIRE(bgs->getVolume() == Catch::Approx(0.3));

    am.setBusVolume(AudioBus::BGS, 0.5);
    REQUIRE(bgs->getVolume() == Catch::Approx(0.15));

    const uint32_t markerId = am.createChannel("se_mix_probe", AudioBus::SE);
    am.destroyChannel(markerId);
    const auto expectedSeName = "se_" + std::to_string(markerId + 1);

    am.playSe("mix_cursor", 50.0, 100.0);
    AudioChannel* se = am.getChannel(expectedSeName);
    REQUIRE(se != nullptr);
    REQUIRE(se->getVolume() == Catch::Approx(0.25));

    am.setBusVolume(AudioBus::SE, 0.2);
    REQUIRE(se->getVolume() == Catch::Approx(0.05));

    am.stopSe();
    am.stopBgs();
    am.stopBgm();
    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);
    am.setBusVolume(AudioBus::BGS, 1.0);
    am.setBusVolume(AudioBus::SE, 1.0);
}

TEST_CASE("AudioManager: SE/ME/BGS controls", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();

    am.playSe("cursor", 90.0, 100.0);
    am.playSe("ok", 90.0, 100.0);
    am.stopSe();

    am.playMe("victory", 90.0, 100.0);
    AudioChannel* me = am.getChannel("me");
    REQUIRE(me != nullptr);
    REQUIRE(me->isPlaying());
    am.stopMe();

    am.playBgs("rain", 70.0, 100.0);
    AudioChannel* bgs = am.getChannel("bgs");
    REQUIRE(bgs != nullptr);
    REQUIRE(bgs->isPlaying());
    am.stopBgs();
}

TEST_CASE("AudioManager: SE channels are reclaimed after playback completion", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopSe();

    const uint32_t markerId = am.createChannel("se_marker_probe", AudioBus::SE);
    am.destroyChannel(markerId);

    const auto expectedSeName = "se_" + std::to_string(markerId + 1);
    am.playSe("cursor_cleanup", 90.0, 100.0);

    AudioChannel* seChannel = am.getChannel(expectedSeName);
    REQUIRE(seChannel != nullptr);
    REQUIRE(seChannel->isPlaying());

    am.update();

    REQUIRE(am.getChannel(expectedSeName) == nullptr);
}

TEST_CASE("AudioManager: SE channel growth is bounded across play/update cycles", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopSe();

    const uint32_t markerId = am.createChannel("se_growth_probe", AudioBus::SE);
    am.destroyChannel(markerId);

    const uint32_t baseNextId = markerId + 1;

    // Play multiple SEs across several cycles; after each update all SE channels
    // should be reclaimed because SE completes in one frame (P1-03 fix).
    for (int cycle = 0; cycle < 4; ++cycle) {
        am.playSe("cycle_se", 90.0, 100.0);
        am.playSe("cycle_se", 90.0, 100.0);
        am.playSe("cycle_se", 90.0, 100.0);
        am.update();
    }

    // No SE channels from the cycles should remain
    for (uint32_t i = 0; i < 12; ++i) {
        const auto seName = "se_" + std::to_string(baseNextId + i);
        REQUIRE(am.getChannel(seName) == nullptr);
    }
}

TEST_CASE("AudioManager: method status registry", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    (void)am;

    REQUIRE(AudioManager::getMethodStatus("playBgm") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("crossfadeBgm") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("crossfadeBgs") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("duckBgm") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("getCurrentBgm") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("setMasterVolume") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("createChannel") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodDeviation("duckBgm").find("live mixer") != std::string::npos);
    REQUIRE(AudioManager::getMethodDeviation("getCurrentBgm").find("deterministic harness") != std::string::npos);
    REQUIRE(AudioManager::getMethodDeviation("playBgm").find("deterministic harness playback state") != std::string::npos);
    REQUIRE(AudioManager::getMethodDeviation("setMasterVolume").find("mix scaling") != std::string::npos);
    REQUIRE(AudioManager::getMethodDeviation("createChannel").find("harness channels") != std::string::npos);
    REQUIRE(AudioManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
}

TEST_CASE("AudioManager: QuickJS API routes into live audio state", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.stopBgm();
    am.stopBgs();
    am.stopMe();
    am.stopSe();
    am.setMasterVolume(1.0);
    am.setBusVolume(AudioBus::BGM, 1.0);
    am.setBusVolume(AudioBus::BGS, 1.0);

    QuickJSContext ctx;
    QuickJSConfig config;
    REQUIRE(ctx.initialize(config));

    AudioManager::registerAPI(ctx);

    urpg::Value playName;
    playName.v = std::string("js_theme");

    auto playResult = ctx.callMethod("AudioManager",
                                     "playBgm",
                                     {playName, urpg::Value::Int(80), urpg::Value::Int(100), urpg::Value::Int(7)});
    REQUIRE(playResult.success);
    REQUIRE(am.isBgmPlaying());
    REQUIRE(am.getCurrentBgm().name == "js_theme");
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(80.0));
    REQUIRE(am.getCurrentBgm().pos == 7);

    auto playingResult = ctx.callMethod("AudioManager", "isBgmPlaying", {});
    REQUIRE(playingResult.success);
    REQUIRE(std::get<int64_t>(playingResult.value.v) == 1);

    auto currentBgmResult = ctx.callMethod("AudioManager", "getCurrentBgm", {});
    REQUIRE(currentBgmResult.success);
    REQUIRE(std::holds_alternative<urpg::Object>(currentBgmResult.value.v));
    const auto& bgmObj = std::get<urpg::Object>(currentBgmResult.value.v);
    REQUIRE(std::get<std::string>(bgmObj.at("name").v) == "js_theme");
    REQUIRE(std::get<double>(bgmObj.at("volume").v) == Catch::Approx(80.0));
    REQUIRE(std::get<int64_t>(bgmObj.at("pos").v) == 7);

    auto pauseResult = ctx.callMethod("AudioManager", "pauseBgm", {});
    REQUIRE(pauseResult.success);
    REQUIRE(am.isBgmPaused());

    auto resumeResult = ctx.callMethod("AudioManager", "resumeBgm", {});
    REQUIRE(resumeResult.success);
    REQUIRE(am.isBgmPlaying());

    urpg::Value bgsName;
    bgsName.v = std::string("js_rain");
    auto playBgsResult = ctx.callMethod("AudioManager",
                                        "playBgs",
                                        {bgsName, urpg::Value::Int(70), urpg::Value::Int(100), urpg::Value::Int(2)});
    REQUIRE(playBgsResult.success);
    AudioChannel* bgs = am.getChannel("bgs");
    REQUIRE(bgs != nullptr);
    REQUIRE(bgs->getFilename() == "js_rain");
    REQUIRE(bgs->getPosition() == 2);

    auto stopBgsResult = ctx.callMethod("AudioManager", "stopBgs", {});
    REQUIRE(stopBgsResult.success);
    REQUIRE_FALSE(bgs->isPlaying());

    urpg::Value meName;
    meName.v = std::string("js_fanfare");
    auto playMeResult = ctx.callMethod("AudioManager", "playMe", {meName, urpg::Value::Int(65), urpg::Value::Int(100)});
    REQUIRE(playMeResult.success);
    AudioChannel* me = am.getChannel("me");
    REQUIRE(me != nullptr);
    REQUIRE(me->isPlaying());

    auto stopMeResult = ctx.callMethod("AudioManager", "stopMe", {});
    REQUIRE(stopMeResult.success);
    REQUIRE_FALSE(me->isPlaying());

    auto setMasterResult = ctx.callMethod("AudioManager", "setMasterVolume", {urpg::Value::Int(50)});
    REQUIRE(setMasterResult.success);
    auto getMasterResult = ctx.callMethod("AudioManager", "getMasterVolume", {});
    REQUIRE(getMasterResult.success);
    REQUIRE(std::get<double>(getMasterResult.value.v) == Catch::Approx(0.5));

    auto setBusResult = ctx.callMethod("AudioManager", "setBusVolume", {urpg::Value::Int(static_cast<int64_t>(AudioBus::BGM)), urpg::Value::Int(25)});
    REQUIRE(setBusResult.success);
    auto getBusResult = ctx.callMethod("AudioManager", "getBusVolume", {urpg::Value::Int(static_cast<int64_t>(AudioBus::BGM))});
    REQUIRE(getBusResult.success);
    REQUIRE(std::get<double>(getBusResult.value.v) == Catch::Approx(0.25));

    auto duckResult = ctx.callMethod("AudioManager", "duckBgm", {urpg::Value::Int(30), urpg::Value::Int(2)});
    REQUIRE(duckResult.success);
    auto isDuckedResult = ctx.callMethod("AudioManager", "isBgmDucked", {});
    REQUIRE(isDuckedResult.success);
    REQUIRE(std::get<int64_t>(isDuckedResult.value.v) == 1);

    auto unduckResult = ctx.callMethod("AudioManager", "unduckBgm", {urpg::Value::Int(1)});
    REQUIRE(unduckResult.success);

    urpg::Value seName;
    seName.v = std::string("js_cursor");
    auto playSeResult = ctx.callMethod("AudioManager", "playSe", {seName, urpg::Value::Int(90), urpg::Value::Int(100)});
    REQUIRE(playSeResult.success);

    auto stopResult = ctx.callMethod("AudioManager", "stopBgm", {});
    REQUIRE(stopResult.success);
    REQUIRE_FALSE(am.isBgmPlaying());

    auto stopSeResult = ctx.callMethod("AudioManager", "stopSe", {});
    REQUIRE(stopSeResult.success);
    am.stopBgm();
    am.stopBgs();
    am.stopMe();
    am.stopSe();
}

TEST_CASE("AudioChannel: state and controls", "[audio_manager]") {
    AudioChannel channel("test", AudioBus::SE);

    REQUIRE(channel.getState() == AudioState::STOPPED);
    REQUIRE_FALSE(channel.isPlaying());
    REQUIRE_FALSE(channel.isPaused());

    channel.play("sfx", 90.0, 100.0, 0);
    REQUIRE(channel.getState() == AudioState::PLAYING);
    REQUIRE(channel.isPlaying());
    REQUIRE(channel.getFilename() == "sfx");

    channel.pause();
    REQUIRE(channel.getState() == AudioState::PAUSED);
    REQUIRE(channel.isPaused());

    channel.resume();
    REQUIRE(channel.getState() == AudioState::PLAYING);
    REQUIRE(channel.isPlaying());

    channel.setVolume(1.5);
    REQUIRE(channel.getVolume() == Catch::Approx(1.0));

    channel.setPitch(0.1);
    REQUIRE(channel.getPitch() == Catch::Approx(0.5));

    channel.setPosition(-10);
    REQUIRE(channel.getPosition() == 0);

    channel.stop();
    REQUIRE(channel.getState() == AudioState::STOPPED);
}
