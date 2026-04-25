#include "engine/core/tutorial/tutorial_lesson.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("tutorial lesson tracks completion and resets", "[tutorial][onboarding][ffs08]") {
    urpg::tutorial::TutorialLesson lesson("first_steps", {"open_map", "talk_to_guide"});

    REQUIRE_FALSE(lesson.isComplete());
    REQUIRE(lesson.completeStep("open_map"));
    REQUIRE_FALSE(lesson.isComplete());
    REQUIRE(lesson.completeStep("talk_to_guide"));
    REQUIRE(lesson.isComplete());

    lesson.reset();
    REQUIRE_FALSE(lesson.isComplete());
}

TEST_CASE("tutorial completion persists through save load", "[tutorial][onboarding][ffs08]") {
    urpg::tutorial::TutorialLesson lesson("first_steps", {"open_map", "talk_to_guide"});
    REQUIRE(lesson.completeStep("open_map"));
    REQUIRE(lesson.completeStep("talk_to_guide"));

    const auto restored = urpg::tutorial::TutorialLesson::deserialize(lesson.serialize());

    REQUIRE(restored.id() == "first_steps");
    REQUIRE(restored.isComplete());
}
