#include <catch2/catch_test_macros.hpp>
#include "engine/core/audio/audio_ai_bridge.h"
#include "engine/core/animation/animation_ai_bridge.h"

TEST_CASE("Audio Bridge Regex Validation", "[ai][audio]") {
    SECTION("Standard command parsing") {
        std::string response = "I will play some music for you. [ACTION: PLAY_BGM, ASSET: Town_Theme, VOL: 0.8, FADE: 2.0]";
        auto cmds = urpg::ai::AudioKnowledgeBridge::parseAudioCommands(response);
        
        REQUIRE(cmds.size() == 1);
        CHECK(cmds[0].action == "PLAY_BGM");
        CHECK(cmds[0].assetId == "Town_Theme");
        CHECK(cmds[0].volume == 0.8f);
        CHECK(cmds[0].fadeTime == 2.0f);
    }

    SECTION("Command with extra whitespace and commentary") {
        std::string response = "Entering the dungeon... [ ACTION: CROSSFADE , ASSET: Dark_Cave , VOL: 0.5 , FADE: 5.0 ] Good luck!";
        auto cmds = urpg::ai::AudioKnowledgeBridge::parseAudioCommands(response);
        
        REQUIRE(cmds.size() == 1);
        CHECK(cmds[0].action == "CROSSFADE");
        CHECK(cmds[0].assetId == "Dark_Cave");
        CHECK(cmds[0].volume == 0.5f);
        CHECK(cmds[0].fadeTime == 5.0f);
    }

    SECTION("Multiple commands") {
        std::string response = "[ACTION: STOP, ASSET: None, VOL: 0, FADE: 1.0] [ACTION: PLAY_SE, ASSET: Door_Open, VOL: 1.0, FADE: 0]";
        auto cmds = urpg::ai::AudioKnowledgeBridge::parseAudioCommands(response);
        
        REQUIRE(cmds.size() == 2);
        CHECK(cmds[0].action == "STOP");
        CHECK(cmds[1].action == "PLAY_SE");
        CHECK(cmds[1].assetId == "Door_Open");
    }
}

TEST_CASE("Animation Bridge Regex Validation", "[ai][animation]") {
    SECTION("Typical keyframe sequence") {
        std::string response = "The hero jumps! [KEYFRAME: 0.5, POS: 0:10:0][KEYFRAME: 1.0, POS: 0:0:0]";
        auto keyframes = urpg::ai::AnimationKnowledgeBridge::parseKeyframes(response);
        
        REQUIRE(keyframes.size() == 2);
        CHECK(keyframes[0].time.ToFloat() == 0.5f);
        CHECK(keyframes[1].time.ToFloat() == 1.0f);
    }
}
