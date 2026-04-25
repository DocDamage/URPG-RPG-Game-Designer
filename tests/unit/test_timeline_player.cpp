#include "engine/core/timeline/timeline_document.h"
#include "engine/core/timeline/timeline_player.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>

TEST_CASE("Timeline playback emits deterministic command stream for the same seed", "[timeline][replay][ffs11]") {
    urpg::timeline::TimelineDocument document;
    document.addActor("hero");
    document.addCommand(urpg::timeline::TimelineCommand{"later", urpg::timeline::TimelineCommandKind::Audio, 8, 0, {}, "se_intro"});
    document.addCommand(urpg::timeline::TimelineCommand{"move", urpg::timeline::TimelineCommandKind::Movement, 2, 4, "hero", "x:3,y:4"});
    document.addCommand(urpg::timeline::TimelineCommand{"message", urpg::timeline::TimelineCommandKind::Message, 2, 0, {}, "Ready"});

    const auto first = urpg::timeline::TimelinePlayer::play(document, 1337);
    const auto second = urpg::timeline::TimelinePlayer::play(document, 1337);

    REQUIRE(first == second);
    REQUIRE(first.size() == 3);
    REQUIRE(first[0].command_id == "message");
    REQUIRE(first[1].command_id == "move");
    REQUIRE(first[2].command_id == "later");
}

TEST_CASE("Timeline validation reports missing actors and non-positive waits", "[timeline][replay][ffs11]") {
    urpg::timeline::TimelineDocument document;
    document.addCommand(urpg::timeline::TimelineCommand{"bad_move", urpg::timeline::TimelineCommandKind::Movement, 1, 1, "missing_actor", {}});
    document.addCommand(urpg::timeline::TimelineCommand{"bad_wait", urpg::timeline::TimelineCommandKind::Wait, 2, 0, {}, {}});

    const auto diagnostics = document.validate();

    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "missing_actor";
    }));
    REQUIRE(std::any_of(diagnostics.begin(), diagnostics.end(), [](const auto& diagnostic) {
        return diagnostic.code == "non_positive_wait";
    }));
}
