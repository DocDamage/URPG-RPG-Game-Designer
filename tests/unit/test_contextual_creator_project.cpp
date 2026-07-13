#include "engine/core/project/contextual_creator_project.h"
#include "engine/core/project/project_creation_service.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>

TEST_CASE("contextual creator project saves and reopens native authoring data", "[project][contextual_authoring]") {
    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_contextual_project_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto destination = root / "CreatorProject";
    urpg::project::ProjectCreationRequest request;
    request.template_id = "jrpg";
    request.project_id = "contextual_project";
    request.project_name = "Contextual Project";
    request.destination = destination;
    auto created = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE(created.success);

    urpg::project::ContextualCreatorProject project;
    REQUIRE(project.open(destination).success);
    urpg::events::EventDocument events;
    events.addMap({"map_intro", 16, 12});
    urpg::events::EventPage page;
    page.id = "page_1";
    page.commands.push_back({"greet", urpg::events::EventCommandKind::Message, "elder", "Hello"});
    events.addEvent({"elder", "map_intro", 4, 6, {page}});
    project.setEventDocument(events);
    urpg::dialogue::DialogueGraph dialogue;
    REQUIRE(dialogue.addNode({"start", "elder", "Elder", "dialogue.elder.start", "Hello", true, {}}));
    project.setDialogue("elder", dialogue);
    urpg::character::CharacterIdentity hero;
    hero.setName("Willow");
    hero.setClassId("class_guardian");
    project.setCharacter("willow_hero", hero);
    urpg::database::RpgDatabase database;
    database.upsertActor({"willow_hero", "Willow", "class_guardian", 120, 12});
    database.upsertItem({"moonwell_lantern", "Moonwell Lantern", 0, {"quest"}});
    project.setDatabase(database);
    REQUIRE(project.save().success);

    urpg::project::ContextualCreatorProject reopened;
    REQUIRE(reopened.open(destination).success);
    REQUIRE(reopened.snapshot()["is_valid"] == true);
    REQUIRE(reopened.eventDocument().events().size() == 1);
    REQUIRE(reopened.dialogues().at("elder").findNode("start") != nullptr);
    REQUIRE(reopened.characters().at("willow_hero").getDisplayName() == "Willow");
    REQUIRE(reopened.database().items().contains("moonwell_lantern"));
    std::filesystem::remove_all(root);
}
