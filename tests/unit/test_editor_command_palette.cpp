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

TEST_CASE("Editor command palette maintains an incremental candidate search index",
          "[editor][command_palette][search][pcq701][perf][primary_routes]") {
    urpg::editor::EditorCommandPalette palette;
    for (int index = 0; index < 256; ++index) {
        const auto suffix = std::to_string(1000 + index);
        REQUIRE(palette.registerCommand({"generated.command." + suffix,
                                         "Generated Command " + suffix,
                                         "Generated",
                                         {"route-token-" + suffix},
                                         {},
                                         "Generated route is available.",
                                         "Exercise deterministic indexed command search."}));
    }
    REQUIRE(palette.searchIndexRevision() == 256);
    REQUIRE_FALSE(palette.registerCommand({"generated.command.1000", "Duplicate", "Generated", {}, {}, {}, "Help"}));
    REQUIRE(palette.searchIndexRevision() == 256);

    const auto result = palette.search("route-token-1137");
    REQUIRE(result.size() == 1);
    REQUIRE(result.front().command.id == "generated.command.1137");
    REQUIRE(palette.lastSearchCandidateCount() == 1);

    const auto empty = palette.search({}, 5);
    REQUIRE(empty.size() == 5);
    REQUIRE(palette.lastSearchCandidateCount() == 256);
    REQUIRE(palette.search("route-token-1137", 0).empty());
    REQUIRE(palette.lastSearchCandidateCount() == 0);

    REQUIRE(palette.recordAction("generated.command.1255"));
    REQUIRE(urpg::editor::EditorCommandSearchJob::kDefaultMaximumCandidatesPerSlice == 64);
    auto job = palette.beginSearch("generated", 20);
    REQUIRE_FALSE(job.complete());
    REQUIRE(job.candidateCount() == 256);
    REQUIRE_FALSE(job.advance(0));
    REQUIRE(job.processedCount() == 0);
    REQUIRE_FALSE(job.advance(64));
    REQUIRE(job.processedCount() == 64);
    REQUIRE_FALSE(job.advance(64));
    REQUIRE(job.processedCount() == 128);
    REQUIRE_FALSE(job.advance(64));
    REQUIRE(job.processedCount() == 192);
    REQUIRE(job.advance(64));
    REQUIRE(job.complete());
    REQUIRE(job.processedCount() == 256);
    REQUIRE(job.results().size() == 20);
    REQUIRE(job.results().front().command.id == "generated.command.1255");
    const auto synchronous = palette.search("generated", 20);
    REQUIRE(std::equal(job.results().begin(), job.results().end(), synchronous.begin(), synchronous.end(),
                       [](const auto& left, const auto& right) {
                           return left.command.id == right.command.id && left.score == right.score &&
                                  left.recent == right.recent;
                       }));
}
