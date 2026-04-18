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

    am.setMasterVolume(0.8);
    REQUIRE(am.getMasterVolume() == Catch::Approx(0.8));

    am.setBusVolume(AudioBus::SE, 1.5);
    REQUIRE(am.getBusVolume(AudioBus::SE) == Catch::Approx(1.0));

    am.setBusVolume(AudioBus::SE, -0.5);
    REQUIRE(am.getBusVolume(AudioBus::SE) == Catch::Approx(0.0));

    am.playBgm("duck_test", 90.0, 100.0);
    am.duckBgm(30.0);
    REQUIRE(am.isBgmDucked());

    am.unduckBgm();
    REQUIRE_FALSE(am.isBgmDucked());

    am.stopBgm();
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

TEST_CASE("AudioManager: method status registry", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    (void)am;

    REQUIRE(AudioManager::getMethodStatus("playBgm") == CompatStatus::FULL);
    REQUIRE(AudioManager::getMethodStatus("crossfadeBgm") == CompatStatus::FULL);
    REQUIRE(AudioManager::getMethodStatus("crossfadeBgs") == CompatStatus::FULL);
    REQUIRE(AudioManager::getMethodStatus("duckBgm") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodStatus("getCurrentBgm") == CompatStatus::PARTIAL);
    REQUIRE(AudioManager::getMethodDeviation("duckBgm").find("smooth") != std::string::npos);
    REQUIRE(AudioManager::getMethodStatus("nonexistentMethod") == CompatStatus::UNSUPPORTED);
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

TEST_CASE("AudioManager: BGM position increments on update", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.playBgm("position_test", 100.0, 100.0, 0);
    REQUIRE(am.getCurrentBgm().pos == 0);

    for (int i = 0; i < 10; ++i) {
        am.update();
    }

    REQUIRE(am.getCurrentBgm().pos == 10);
    am.stopBgm();
}

TEST_CASE("AudioManager: duckBgm smoothly interpolates volume over frames", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.playBgm("duck_smooth", 100.0, 100.0, 0);
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(100.0));

    am.duckBgm(50.0, 10);
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(100.0));

    for (int i = 0; i < 5; ++i) {
        am.update();
    }
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(75.0));

    for (int i = 0; i < 5; ++i) {
        am.update();
    }
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(50.0));

    am.stopBgm();
}

TEST_CASE("AudioManager: unduckBgm restores volume over frames", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.playBgm("unduck_test", 100.0, 100.0, 0);
    am.duckBgm(50.0, 10);
    for (int i = 0; i < 10; ++i) {
        am.update();
    }
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(50.0));
    REQUIRE(am.isBgmDucked());

    am.unduckBgm(10);
    for (int i = 0; i < 5; ++i) {
        am.update();
    }
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(75.0));

    for (int i = 0; i < 5; ++i) {
        am.update();
    }
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(100.0));
    REQUIRE_FALSE(am.isBgmDucked());

    am.stopBgm();
}

TEST_CASE("AudioManager: instant duck with duration 0 sets volume immediately", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    am.playBgm("instant_duck", 100.0, 100.0, 0);
    am.duckBgm(25.0, 0);
    REQUIRE(am.getCurrentBgm().volume == Catch::Approx(25.0));
    am.stopBgm();
}
