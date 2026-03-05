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

    am.playBgm("battle_theme", 80.0, 100.0, 0);
    REQUIRE(am.isBgmPlaying());
    REQUIRE_FALSE(am.isBgmPaused());

    am.pauseBgm();
    REQUIRE(am.isBgmPaused());

    am.resumeBgm();
    REQUIRE(am.isBgmPlaying());
    REQUIRE_FALSE(am.isBgmPaused());

    am.saveBgmSettings();
    am.playBgm("other_theme", 50.0, 90.0, 0);
    am.restoreBgmSettings();
    REQUIRE(am.isBgmPlaying());

    am.stopBgm();
    REQUIRE_FALSE(am.isBgmPlaying());
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
    REQUIRE(AudioManager::getMethodStatus("crossfadeBgm") == CompatStatus::PARTIAL);
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
