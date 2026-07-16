#include "engine/core/project/project_reference_change_plan.h"

#include <catch2/catch_test_macros.hpp>

using urpg::project::ProjectReferenceChangeKind;
using urpg::project::ProjectReferenceChangeRequest;
using urpg::project::ProjectReferenceDocument;
using urpg::project::ProjectReferenceEdge;
using urpg::project::ProjectReferenceIndex;
using urpg::project::previewProjectReferenceChange;

namespace {

ProjectReferenceIndex fixtureIndex() {
    ProjectReferenceIndex index;
    REQUIRE(index.rebuild({ProjectReferenceDocument{
                              "content/maps/start.p2d.json",
                              {ProjectReferenceEdge{"event", "start/welcome", "dialogue", "intro", "start_dialogue",
                                                    {}, "event:welcome.command:0", false},
                               ProjectReferenceEdge{"map", "start", "asset", "asset.grass", "tile_palette_asset",
                                                    {}, "tile_palette:grass", true}}},
                          ProjectReferenceDocument{
                              "content/dialogues/intro.json",
                              {ProjectReferenceEdge{"dialogue_node", "intro/start", "asset", "voice.guide",
                                                    "node_voice", {}, "node:start", true}}}})
                .success);
    return index;
}

} // namespace

TEST_CASE("Reference change preview reports updates package impact and inverse", "[project][reference_change]") {
    const auto index = fixtureIndex();
    const auto plan = previewProjectReferenceChange(
        index, ProjectReferenceChangeRequest{"rename.voice", ProjectReferenceChangeKind::Rename, "asset",
                                             "voice.guide", "voice.guide.remastered", {}});
    REQUIRE(plan.success);
    REQUIRE(plan.applicable);
    REQUIRE(plan.updates.size() == 1);
    REQUIRE(plan.updates[0].before.local_id == "node:start");
    REQUIRE(plan.updates[0].after.target_id == "voice.guide.remastered");
    REQUIRE(plan.package_impact.size() == 1);
    REQUIRE(plan.inverse_request.source_id == "voice.guide.remastered");
    REQUIRE(plan.inverse_request.replacement_id == "voice.guide");
}

TEST_CASE("Reference delete preview blocks indexed inbound uses", "[project][reference_change]") {
    const auto plan = previewProjectReferenceChange(
        fixtureIndex(), ProjectReferenceChangeRequest{"delete.dialogue", ProjectReferenceChangeKind::Delete,
                                                      "dialogue", "intro", {}, {}});
    REQUIRE(plan.success);
    REQUIRE_FALSE(plan.applicable);
    REQUIRE(plan.code == "project_reference_delete_blocked");
    REQUIRE(plan.blocked_references.size() == 1);
    REQUIRE(plan.blocked_references[0].document_path == "content/maps/start.p2d.json");
}

TEST_CASE("Reference move preview preserves stable ID and records inverse document", "[project][reference_change]") {
    const auto plan = previewProjectReferenceChange(
        fixtureIndex(), ProjectReferenceChangeRequest{"move.dialogue", ProjectReferenceChangeKind::Move,
                                                      "dialogue_node", "intro/start", {},
                                                      "content/dialogues/chapter1/intro.json"});
    REQUIRE(plan.success);
    REQUIRE(plan.applicable);
    REQUIRE(plan.updates.size() == 1);
    REQUIRE(plan.updates[0].after.source_id == "intro/start");
    REQUIRE(plan.updates[0].after.document_path == "content/dialogues/chapter1/intro.json");
    REQUIRE(plan.inverse_request.destination_document == "content/dialogues/intro.json");
}
