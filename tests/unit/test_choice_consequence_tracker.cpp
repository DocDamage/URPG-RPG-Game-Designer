#include "engine/core/narrative/choice_consequence_tracker.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("choice consequence tracker links choices to state changes", "[narrative][choice][ffs10]") {
    urpg::narrative::ChoiceConsequenceTracker tracker;
    tracker.registerConsequence({"choice_help", "guide_affinity", 5});
    tracker.registerConsequence({"choice_help", "intro_flags", 1});

    std::map<std::string, int> state;
    tracker.applyChoice("choice_help", state);

    REQUIRE(state["guide_affinity"] == 5);
    REQUIRE(state["intro_flags"] == 1);
    REQUIRE(tracker.consequencesFor("choice_help").size() == 2);
}
