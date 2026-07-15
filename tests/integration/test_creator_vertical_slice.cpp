#include "editor/ui/editor_context_action.h"
#include "engine/core/gameplay/gameplay_runtime_facade.h"
#include "engine/core/platform/process_runner.h"
#include "engine/core/project/contextual_creator_project.h"
#include "engine/core/project/project_creation_service.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <set>

namespace {

nlohmann::json loadAcceptance() {
    std::ifstream input(std::filesystem::path(URPG_SOURCE_DIR) / "content" / "examples" / "creator_vertical_slice" /
                        "acceptance.json", std::ios::binary);
    return nlohmann::json::parse(input);
}

void writeReport(const nlohmann::json& report) {
    const auto path = std::filesystem::path(URPG_BINARY_DIR) / "creator_vertical_slice_report.json";
    std::ofstream output(path, std::ios::binary);
    output << report.dump(2) << '\n';
    REQUIRE(output.good());
}

std::vector<std::string> buildGovernedPackage(const std::filesystem::path& exampleRoot,
                                               const std::filesystem::path& authoredProject,
                                               const std::filesystem::path& packageRoot) {
    std::filesystem::create_directories(packageRoot / "content" / "maps");
    const std::vector<std::pair<std::filesystem::path, std::filesystem::path>> files = {
        {exampleRoot / "project.json", packageRoot / "project.json"},
        {exampleRoot / "content" / "attachments.json", packageRoot / "content" / "attachments.json"},
        {exampleRoot / "content" / "maps" / "willow_village.json", packageRoot / "content" / "maps" / "willow_village.json"},
        {exampleRoot / "content" / "maps" / "moonwell_shrine.json", packageRoot / "content" / "maps" / "moonwell_shrine.json"},
        {authoredProject / urpg::project::ContextualCreatorProject::kRelativePath,
         packageRoot / urpg::project::ContextualCreatorProject::kRelativePath},
    };
    for (const auto& [source, destination] : files) {
        REQUIRE(std::filesystem::is_regular_file(source));
        std::filesystem::create_directories(destination.parent_path());
        std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
    }
    std::vector<std::string> inventory;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(packageRoot)) {
        if (entry.is_regular_file()) inventory.push_back(std::filesystem::relative(entry.path(), packageRoot).generic_string());
    }
    std::sort(inventory.begin(), inventory.end());
    return inventory;
}

} // namespace

TEST_CASE("creator vertical slice proves the governed creator scenario contract", "[integration][creator vertical slice]") {
    const auto acceptance = loadAcceptance();
    REQUIRE(acceptance.value("schema", "") == "urpg.creator_vertical_slice.v1");
    REQUIRE(acceptance.value("maximum_playthrough_minutes", 0) <= 15);
    REQUIRE(acceptance["project"]["maps"].size() == 2);
    REQUIRE(acceptance["project"]["npcs"].size() == 2);
    REQUIRE(acceptance["project"]["abilities"].size() == 2);
    REQUIRE_FALSE(acceptance["asset_policy"].value("allow_raw_external", true));
    REQUIRE_FALSE(acceptance["asset_policy"].value("allow_absolute_paths", true));
    REQUIRE(std::filesystem::is_regular_file(std::filesystem::path(URPG_SOURCE_DIR) /
                                              acceptance["asset_policy"]["governance_fixture"].get<std::string>()));
    const auto exampleRoot = std::filesystem::path(URPG_SOURCE_DIR) / "content" / "examples" / "creator_vertical_slice";
    REQUIRE(std::filesystem::is_regular_file(exampleRoot / "project.json"));
    REQUIRE(std::filesystem::is_regular_file(exampleRoot / "content" / "attachments.json"));
    for (const auto& mapId : acceptance["project"]["maps"]) {
        REQUIRE(std::filesystem::is_regular_file(exampleRoot / "content" / "maps" / (mapId.get<std::string>() + ".json")));
    }

    const auto root = std::filesystem::temp_directory_path() /
                      ("urpg_creator_vertical_slice_" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    const auto projectRoot = root / "LanternOfTheWillow";
    urpg::project::ProjectCreationRequest request;
    request.project_id = acceptance["id"].get<std::string>();
    request.project_name = acceptance["title"].get<std::string>();
    request.template_id = acceptance["template_id"].get<std::string>();
    request.destination = projectRoot;
    request.starter_map = acceptance["project"]["maps"][0].get<std::string>();
    const auto created = urpg::project::ProjectCreationService{}.createProject(request);
    REQUIRE(created.success);

    urpg::project::ContextualCreatorProject authored;
    REQUIRE(authored.open(projectRoot).success);
    urpg::events::EventDocument events;
    events.addMap({"willow_village", 8, 8});
    urpg::events::EventPage elderPage;
    elderPage.id = "elder_page";
    elderPage.commands.push_back({"elder_message", urpg::events::EventCommandKind::Message, "elder_mira",
                                  "Restore the Moonwell Lantern."});
    events.addEvent({"elder_mira_intro", "willow_village", 2, 2, {elderPage}});
    authored.setEventDocument(std::move(events));
    urpg::dialogue::DialogueGraph dialogue;
    REQUIRE(dialogue.addNode({"elder_start", "elder_mira", "Elder Mira", "dialogue.elder.start",
                              "Will you restore the lantern?", true, {}}));
    authored.setDialogue("elder_mira_intro", std::move(dialogue));
    urpg::character::CharacterIdentity hero;
    hero.setName("Willow");
    hero.setClassId("class_guardian");
    authored.setCharacter("willow_hero", std::move(hero));
    urpg::database::RpgDatabase database;
    database.upsertActor({"willow_hero", "Willow", "class_guardian", 120, 12});
    database.upsertItem({"moonwell_lantern", "Moonwell Lantern", 0, {"quest"}});
    authored.setDatabase(std::move(database));
    REQUIRE(authored.save().success);
    urpg::project::ContextualCreatorProject reopened;
    REQUIRE(reopened.open(projectRoot).success);
    REQUIRE(reopened.snapshot().value("is_valid", false));

    urpg::platform::ProcessCommand runtimeCommand;
    runtimeCommand.executable = URPG_RUNTIME_PATH;
    runtimeCommand.arguments = {"--headless", "--frames", "1", "--project-root", projectRoot.generic_string()};
    runtimeCommand.workingDirectory = projectRoot;
    runtimeCommand.timeout = std::chrono::seconds(20);
    const auto runtimeLaunch = urpg::platform::runProcess(runtimeCommand);
    INFO(runtimeLaunch.stderrText);
    REQUIRE_FALSE(runtimeLaunch.timedOut);
    REQUIRE(runtimeLaunch.error.empty());
    REQUIRE(runtimeLaunch.exitCode == 0);

    urpg::map::GridPartDocument village("willow_village", 8, 8);
    urpg::map::PlacedPartInstance savePoint;
    savePoint.instance_id = "village_well";
    savePoint.part_id = "save_point";
    savePoint.category = urpg::map::GridPartCategory::SavePoint;
    savePoint.grid_x = 2;
    savePoint.grid_y = 2;
    REQUIRE(village.placePart(savePoint));
    urpg::level::PathfindingGraph paths(8, 8);
    urpg::gameplay::GameplayRuntimeFacade facade;
    facade.bindMap(&village, &paths);
    urpg::npc::NpcRuntimeState elder;
    elder.npcId = "elder_mira";
    elder.mapId = "willow_village";
    REQUIRE(facade.registerNpc(elder));
    REQUIRE(facade.queryMapAt(2, 2).success);
    REQUIRE(facade.moveNpc("elder_mira", {1, 0}).success);
    REQUIRE(facade.invokeEvent({"restore_moonwell_lantern"}).success);
    REQUIRE(facade.bindInput("confirm", "interact").success);
    REQUIRE(facade.addResource("moonwell_lantern", 1).success);
    REQUIRE(facade.resourceCount("moonwell_lantern") == 1);

    urpg::editor::EditorContextActionStack routes;
    for (const auto& route : acceptance["required_creator_routes"]) {
        INFO("creator route: " << route.get<std::string>());
        urpg::editor::EditorContextAction action;
        action.route = route.get<std::string>();
        action.objectKind = "vertical_slice";
        action.objectId = acceptance["id"].get<std::string>();
        action.projectRoot = projectRoot;
        const auto result = routes.open(action);
        REQUIRE(result.success);
        REQUIRE(routes.returnToPrevious().success);
    }

    const auto packageInventory = buildGovernedPackage(exampleRoot, projectRoot, root / "LanternOfTheWillow.package");
    REQUIRE(packageInventory == std::vector<std::string>{"content/attachments.json", "content/contextual_authoring.json",
                                                          "content/maps/moonwell_shrine.json", "content/maps/willow_village.json",
                                                          "project.json"});
    for (const auto& path : packageInventory) {
        REQUIRE(path.find("://") == std::string::npos);
        REQUIRE(path.find(".urpg/recovery") == std::string::npos);
        REQUIRE(path.find(".urpg/playtest") == std::string::npos);
        REQUIRE(path.find("asset_catalog.db") == std::string::npos);
    }

    const auto completedSteps = acceptance["required_runtime_steps"];
    const nlohmann::json report = {
        {"schema", "urpg.creator_vertical_slice_report.v1"},
        {"id", acceptance["id"]},
        {"status", "passed"},
        {"completed_steps", completedSteps},
        {"package_inventory", packageInventory},
    };
    writeReport(report);
    std::filesystem::remove_all(root);
}
