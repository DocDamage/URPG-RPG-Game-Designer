#include <catch2/catch_test_macros.hpp>
#include "testing/headless_play_mode.h"
#include <memory>

using namespace urpg::testing;

TEST_CASE("HeadlessPlayMode executes scripted sequence", "[testing]") {
    HeadlessPlayMode playMode;
    auto session = std::make_unique<HeadlessSession>();

    bool actionExecuted = false;
    bool waitCondition = false;

    session->addStep({"Step 1", [&]() { 
        actionExecuted = true; 
        return true; 
    }, "Immediate execution step"});

    session->addStep({"Step 2", [&]() { 
        return waitCondition; 
    }, "Wait step"});

    playMode.startSession(std::move(session));

    SECTION("Executes immediate steps") {
        playMode.update(0.16f);
        REQUIRE(actionExecuted);
    }

    SECTION("Waits until conditions are met") {
        playMode.update(0.16f); // After Step 1
        
        playMode.update(0.16f); // Stuck on Step 2
        REQUIRE_FALSE(playMode.isFinished());

        waitCondition = true;
        playMode.update(0.16f); // Finishes Step 2
        REQUIRE(playMode.isFinished());
    }
}
