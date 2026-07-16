#include "editor/ui/editor_command_palette.h"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <set>

TEST_CASE("Golden-loop editor commands are searchable with shortcuts and requirements", "[editor][command_palette]") {
    auto palette = urpg::editor::buildGoldenLoopCommandPalette();
    REQUIRE(palette.commands().size() == 7);

    const std::set<std::string> required = {"project.create", "map.open", "event.create", "project.save",
                                            "playtest.current_map", "project.health", "export.validate"};
    for (const auto& command : palette.commands()) {
        REQUIRE(required.contains(command.id));
        REQUIRE_FALSE(command.label.empty());
        REQUIRE_FALSE(command.shortcut.empty());
        REQUIRE_FALSE(command.requirement.empty());
        REQUIRE_FALSE(command.help.empty());
    }

    const auto play = palette.search("play");
    REQUIRE_FALSE(play.empty());
    REQUIRE(play.front().command.id == "playtest.current_map");
    REQUIRE(play.front().command.shortcut == "F6");
    REQUIRE(play.front().command.requirement.find("blocking diagnostics") != std::string::npos);

    const auto dialogue = palette.search("dialogue");
    REQUIRE_FALSE(dialogue.empty());
    REQUIRE(dialogue.front().command.id == "event.create");
    REQUIRE(palette.search("not-a-command").empty());
    REQUIRE(palette.shortcutDiscovery().size() == 7);
}

TEST_CASE("Editor command palette maintains deterministic enabled recent actions", "[editor][command_palette][recent]") {
    auto palette = urpg::editor::buildGoldenLoopCommandPalette();
    REQUIRE(palette.recordAction("project.save"));
    REQUIRE(palette.recordAction("playtest.current_map"));
    REQUIRE(palette.recordAction("project.save"));
    REQUIRE_FALSE(palette.recordAction("missing.command"));

    const auto recent = palette.recentActions();
    REQUIRE(recent.size() == 2);
    REQUIRE(recent[0].id == "project.save");
    REQUIRE(recent[1].id == "playtest.current_map");

    const auto search = palette.search("project");
    const auto saved = std::find_if(search.begin(), search.end(), [](const auto& row) {
        return row.command.id == "project.save";
    });
    REQUIRE(saved != search.end());
    REQUIRE(saved->recent);
}
