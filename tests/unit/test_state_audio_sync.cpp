#include <catch2/catch_test_macros.hpp>
#include "engine/core/audio/audio_core.h"
#include "engine/core/global_state_hub.h"

using namespace urpg::audio;
using namespace urpg;

TEST_CASE("AudioCore: GlobalStateHub Sync", "[audio][state][sync]") {
    auto& hub = GlobalStateHub::getInstance();
    hub.resetAll();
    
    // Set some initial config
    hub.setConfig("audio.bgm_volume", "0.4");
    hub.setConfig("audio.se_volume", "0.75");

    AudioCore audio;

    SECTION("Initial volumes are synced from Hub on construction") {
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 0.4f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::SE) == 0.75f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::System) == 1.0f); // default
    }

    SECTION("Live updates via Hub subscription") {
        hub.setConfig("audio.bgm_volume", "0.1");
        hub.setConfig("audio.system_volume", "0.5");
        
        // Notifications are synchronous in this implementation
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 0.1f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::System) == 0.5f);
    }

    SECTION("Irrelevant state changes do not affect audio") {
        hub.setSwitch("game.started", true);
        hub.setVariable("player.gold", 100);
        
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 0.4f);
    }
}
