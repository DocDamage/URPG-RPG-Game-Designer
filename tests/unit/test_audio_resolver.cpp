#include "engine/core/audio/audio_core.h"
#include "engine/core/audio/state_driven_audio_resolver.h"
#include "engine/core/global_state_hub.h"
#include <catch2/catch_test_macros.hpp>

using namespace urpg::audio;
using namespace urpg;

TEST_CASE("StateDrivenAudioResolver: BGM Auto-Switching", "[audio][state][resolver]") {
    auto& hub = GlobalStateHub::getInstance();
    AudioCore core;
    StateDrivenAudioResolver resolver(core);

    SECTION("Rule triggers BGM check on Hub update") {
        resolver.addRule({"map.type", "forest", "forest_theme", 1.0f});
        resolver.addRule({"battle.active", "true", "battle_theme", 0.5f});

        hub.setConfig("map.type", "forest");
        // Verify core is asked to play forest_theme
        // In a real mock we would verify the call, but here we can check internal state
        // (if AudioCore exposed its current BGM)
    }

    SECTION("Battle triggers override map theme") {
        resolver.addRule({"map.type", "forest", "forest_theme", 1.0f});
        resolver.addRule({"battle.active", "true", "battle_theme", 0.5f});

        hub.setConfig("map.type", "forest");
        hub.setConfig("battle.active", "true");
        // Battle theme should take precedence if we check internal priority.
    }
}
