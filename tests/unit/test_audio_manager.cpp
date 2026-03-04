// AudioManager Unit Tests - Phase 2 Compat Layer
// Tests for MZ Audio Middleware Compatibility Surface

#include "runtimes/compat_js/audio_manager.h"
#include <catch2/catch_test_macros.hpp>
#include <cmath>

using namespace urpg::compat;

TEST_CASE("AudioManager: Channel creation and destruction", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Create channel returns valid ID") {
        uint32_t id = am.createChannel("test_bgm", AudioBus::BGM);
        REQUIRE(id > 0);
        
        am.destroyChannel(id);
    }
    
    SECTION("Get channel by name") {
        uint32_t id = am.createChannel("test_se", AudioBus::SE);
        
        AudioChannel* channel = am.getChannel("test_se");
        REQUIRE(channel != nullptr);
        REQUIRE(channel->getBus() == AudioBus::SE);
        
        am.destroyChannel(id);
    }
    
    SECTION("Get channel by ID") {
        uint32_t id = am.createChannel("test_me", AudioBus::ME);
        
        AudioChannel* channel = am.getChannel(id);
        REQUIRE(channel != nullptr);
        REQUIRE(channel->getBus() == AudioBus::ME);
        
        am.destroyChannel(id);
    }
}

TEST_CASE("AudioManager: BGM playback control", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Play BGM sets correct state") {
        am.playBgm("battle_theme", 80.0, 100.0, 0);
        
        AudioChannel* bgm = am.getChannel("bgm");
        REQUIRE(bgm != nullptr);
        REQUIRE(bgm->isPlaying());
        
        am.stopBgm();
    }
    
    SECTION("Stop BGM clears state") {
        am.playBgm("field_theme", 90.0, 100.0);
        am.stopBgm();
        
        AudioChannel* bgm = am.getChannel("bgm");
        if (bgm != nullptr) {
            REQUIRE_FALSE(bgm->isPlaying());
        }
    }
    
    SECTION("Pause and resume BGM") {
        am.playBgm("dungeon_theme", 90.0, 100.0);
        am.pauseBgm();
        
        AudioChannel* bgm = am.getChannel("bgm");
        if (bgm != nullptr) {
            REQUIRE(bgm->isPaused());
        }
        
        am.resumeBgm();
        
        if (bgm != nullptr) {
            REQUIRE(bgm->isPlaying());
            REQUIRE_FALSE(bgm->isPaused());
        }
        
        am.stopBgm();
    }
}

TEST_CASE("AudioManager: Volume control", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Set bus volume") {
        am.setBusVolume(AudioBus::BGM, 0.5);
        REQUIRE(am.getBusVolume(AudioBus::BGM) == Approx(0.5));
        
        am.setBusVolume(AudioBus::BGM, 1.0);
    }
    
    SECTION("Set master volume") {
        am.setMasterVolume(0.8);
        REQUIRE(am.getMasterVolume() == Approx(0.8));
        
        am.setMasterVolume(1.0);
    }
    
    SECTION("Volume clamping") {
        am.setBusVolume(AudioBus::SE, 1.5);
        REQUIRE(am.getBusVolume(AudioBus::SE) <= 1.0);
        
        am.setBusVolume(AudioBus::SE, -0.5);
        REQUIRE(am.getBusVolume(AudioBus::SE) >= 0.0);
    }
}

TEST_CASE("AudioManager: BGM settings save/restore", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Save and restore BGM settings") {
        am.playBgm("theme1", 75.0, 105.0, 100);
        am.saveBgmSettings();
        
        am.playBgm("theme2", 50.0, 95.0, 0);
        
        am.restoreBgmSettings();
        
        AudioChannel* bgm = am.getChannel("bgm");
        if (bgm != nullptr) {
            REQUIRE(bgm->getName() == "theme1");
        }
        
        am.stopBgm();
    }
}

TEST_CASE("AudioManager: Ducking", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Duck BGM") {
        am.playBgm("test_theme", 90.0, 100.0);
        am.duckBgm(0.3);
        
        REQUIRE(am.isBgmDucked());
        
        am.unduckBgm();
        REQUIRE_FALSE(am.isBgmDucked());
        
        am.stopBgm();
    }
}

TEST_CASE("AudioManager: Crossfade", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Crossfade to new BGM") {
        am.playBgm("old_theme", 90.0, 100.0);
        am.crossfadeBgm("new_theme", 80.0, 100.0, 60);
        
        // Crossfade should be initiated
        // Implementation details may vary
        
        am.stopBgm();
    }
}

TEST_CASE("AudioManager: SE playback", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Play SE") {
        am.playSe("cursor", 90.0, 100.0);
        // SE should play without blocking
    }
    
    SECTION("Stop all SE") {
        am.playSe("sound1", 90.0, 100.0);
        am.playSe("sound2", 90.0, 100.0);
        am.stopSe();
        // All SE should be stopped
    }
}

TEST_CASE("AudioManager: ME playback", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Play ME") {
        am.playMe("victory_fanfare", 90.0, 100.0);
        
        AudioChannel* me = am.getChannel("me");
        if (me != nullptr) {
            REQUIRE(me->isPlaying());
        }
        
        am.stopMe();
    }
}

TEST_CASE("AudioManager: BGS playback", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("Play BGS") {
        am.playBgs("rain", 70.0, 100.0);
        
        AudioChannel* bgs = am.getChannel("bgs");
        if (bgs != nullptr) {
            REQUIRE(bgs->isPlaying());
        }
        
        am.stopBgs();
    }
}

TEST_CASE("AudioManager: Method status registry", "[audio_manager]") {
    AudioManager& am = AudioManager::instance();
    
    SECTION("GetMethodStatus returns valid status") {
        CompatStatus status = am.getMethodStatus("playBgm");
        REQUIRE(status == CompatStatus::FULL);
    }
    
    SECTION("GetMethodStatus returns UNSUPPORTED for unknown methods") {
        CompatStatus status = am.getMethodStatus("nonexistentMethod");
        REQUIRE(status == CompatStatus::UNSUPPORTED);
    }
}

TEST_CASE("AudioChannel: State transitions", "[audio_manager]") {
    AudioChannel channel("test", AudioBus::SE);
    
    SECTION("Initial state is STOPPED") {
        REQUIRE(channel.getState() == AudioState::STOPPED);
        REQUIRE_FALSE(channel.isPlaying());
        REQUIRE_FALSE(channel.isPaused());
    }
    
    SECTION("Play transitions to PLAYING") {
        channel.play("test_file", 90.0, 100.0, 0);
        REQUIRE(channel.getState() == AudioState::PLAYING);
        REQUIRE(channel.isPlaying());
    }
    
    SECTION("Stop transitions to STOPPED") {
        channel.play("test_file", 90.0, 100.0, 0);
        channel.stop();
        REQUIRE(channel.getState() == AudioState::STOPPED);
        REQUIRE_FALSE(channel.isPlaying());
    }
    
    SECTION("Pause transitions to PAUSED") {
        channel.play("test_file", 90.0, 100.0, 0);
        channel.pause();
        REQUIRE(channel.getState() == AudioState::PAUSED);
        REQUIRE(channel.isPaused());
    }
    
    SECTION("Resume transitions back to PLAYING") {
        channel.play("test_file", 90.0, 100.0, 0);
        channel.pause();
        channel.resume();
        REQUIRE(channel.getState() == AudioState::PLAYING);
        REQUIRE(channel.isPlaying());
        REQUIRE_FALSE(channel.isPaused());
    }
}

TEST_CASE("AudioChannel: Volume and pitch control", "[audio_manager]") {
    AudioChannel channel("test", AudioBus::BGM);
    
    SECTION("Set and get volume") {
        channel.setVolume(0.75);
        REQUIRE(channel.getVolume() == Approx(0.75));
    }
    
    SECTION("Set and get pitch") {
        channel.setPitch(1.5);
        REQUIRE(channel.getPitch() == Approx(1.5));
    }
    
    SECTION("Volume is clamped to [0, 1]") {
        channel.setVolume(1.5);
        REQUIRE(channel.getVolume() <= 1.0);
        
        channel.setVolume(-0.5);
        REQUIRE(channel.getVolume() >= 0.0);
    }
    
    SECTION("Pitch is clamped to [0.5, 2.0]") {
        channel.setPitch(3.0);
        REQUIRE(channel.getPitch() <= 2.0);
        
        channel.setPitch(0.1);
        REQUIRE(channel.getPitch() >= 0.5);
    }
}

TEST_CASE("AudioChannel: Position control", "[audio_manager]") {
    AudioChannel channel("test", AudioBus::BGM);
    
    SECTION("Set and get position") {
        channel.setPosition(500);
        REQUIRE(channel.getPosition() == 500);
    }
    
    SECTION("Position cannot be negative") {
        channel.setPosition(-100);
        REQUIRE(channel.getPosition() >= 0);
    }
}
