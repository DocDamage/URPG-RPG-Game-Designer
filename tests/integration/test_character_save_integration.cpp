#include "engine/core/character/character_creation_screen.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/actor_manager.h"
#include "engine/core/progression/stat_allocation.h"
#include "engine/core/save/save_runtime.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>

namespace {

urpg::input::InputCore pressed(urpg::input::InputAction action) {
    urpg::input::InputCore input;
    input.updateActionState(action, urpg::input::ActionState::Pressed);
    return input;
}

} // namespace

TEST_CASE("Integration: runtime-created protagonist survives save and load", "[integration][character][save]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_character_save_integration";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    urpg::character::CharacterCreationScreen screen;
    urpg::World world;
    urpg::ActorManager actorManager(world);

    screen.setName("Nova");
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::Confirm), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    REQUIRE(screen.activeStep() == urpg::character::CharacterCreationScreen::Step::Review);

    urpg::character::CharacterIdentity completedIdentity;
    urpg::EntityID completedEntity = 0;
    screen.setCompletionHandler([&](urpg::EntityID entity, const urpg::character::CharacterIdentity& identity) {
        completedEntity = entity;
        completedIdentity = identity;
    });

    screen.handleInput(pressed(urpg::input::InputAction::Confirm), world, actorManager);
    REQUIRE(completedEntity != 0);
    REQUIRE(completedIdentity.getName() == "Nova");

    const auto* actor = world.GetComponent<urpg::ActorComponent>(completedEntity);
    REQUIRE(actor != nullptr);
    REQUIRE(actor->name == "Nova");

    nlohmann::json payload;
    payload["_urpg_format_version"] = "1.0";
    payload["map_id"] = 1;
    payload["player"] = {{"gold", 25}};

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = base / "slot_001.json";

    std::vector<std::string> saveDiagnostics;
    REQUIRE(urpg::RuntimeSaveLoader::SaveCreatedProtagonist(request, payload.dump(), completedEntity, completedIdentity,
                                                            &saveDiagnostics));
    REQUIRE(saveDiagnostics.empty());

    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE(result.created_protagonist.has_value());
    REQUIRE(result.created_protagonist->valid);
    REQUIRE(result.created_protagonist->entity == completedEntity);
    REQUIRE(result.created_protagonist->identity.getName() == "Nova");
    REQUIRE(result.created_protagonist->identity.getClassId() == completedIdentity.getClassId());
    REQUIRE(result.created_protagonist->identity.getPortraitId() == completedIdentity.getPortraitId());
    REQUIRE(result.created_protagonist->identity.getBodySpriteId() == completedIdentity.getBodySpriteId());
    REQUIRE(result.created_protagonist->identity.getBaseAttributes() == completedIdentity.getBaseAttributes());
    REQUIRE(result.created_protagonist->identity.getAppearanceTokens() == completedIdentity.getAppearanceTokens());
    REQUIRE(result.diagnostics.empty());

    const auto loadedPayload = nlohmann::json::parse(result.payload);
    REQUIRE(loadedPayload[urpg::character::kCreatedProtagonistSaveKey]["identity"]["name"] == "Nova");
    REQUIRE(loadedPayload[urpg::character::kCreatedProtagonistSaveKey]["identity"]["schemaVersion"] == "1.0.0");
    REQUIRE(loadedPayload["player"]["gold"] == 25);

    std::filesystem::remove_all(base);
}

TEST_CASE("Integration: applied stat allocation survives runtime save and load",
          "[integration][progression][save][stat_allocation]") {
    const auto base = std::filesystem::temp_directory_path() / "urpg_stat_allocation_save_integration";
    std::filesystem::remove_all(base);
    std::filesystem::create_directories(base);

    urpg::progression::StatAllocationDocument document;
    document.addPool({"actor_nova_pool",
                      "actor.nova",
                      "class.ranger",
                      2,
                      4,
                      {{"hp", 1, 8, 160}, {"atk", 1, 2, 30}, {"agi", 1, 1, 24}}});

    urpg::progression::ActorStatBlock stats;
    stats.hp = 120;
    stats.atk = 12;
    stats.agi = 15;

    urpg::progression::StatAllocationRequest allocation_request;
    allocation_request.pool_id = "actor_nova_pool";
    allocation_request.points_by_stat = {{"hp", 1}, {"atk", 1}, {"agi", 2}};

    const auto allocation = document.commit("actor_nova_pool", stats, allocation_request);
    REQUIRE(allocation.valid);
    REQUIRE(allocation.remaining_points == 0);
    REQUIRE(allocation.after.hp == 128);
    REQUIRE(allocation.after.atk == 14);
    REQUIRE(allocation.after.agi == 17);

    nlohmann::json payload;
    payload["_urpg_format_version"] = "1.0";
    payload["map_id"] = 2;

    urpg::RuntimeSaveLoadRequest save_request;
    save_request.primary_save_path = base / "slot_002.json";

    std::vector<std::string> diagnostics;
    REQUIRE(urpg::RuntimeSaveLoader::SaveStatAllocation(save_request, payload.dump(), allocation, &diagnostics));
    REQUIRE(diagnostics.empty());

    const auto result = urpg::RuntimeSaveLoader::Load(save_request);
    REQUIRE(result.ok);
    REQUIRE(result.stat_allocations.size() == 1);
    REQUIRE(result.stat_allocations[0].actor_id == "actor.nova");
    REQUIRE(result.stat_allocations[0].class_id == "class.ranger");
    REQUIRE(result.stat_allocations[0].points_by_stat.at("agi") == 2);
    REQUIRE(result.stat_allocations[0].after.hp == 128);
    REQUIRE(result.stat_allocations[0].after.atk == 14);
    REQUIRE(result.stat_allocations[0].after.agi == 17);

    const auto loaded_payload = nlohmann::json::parse(result.payload);
    REQUIRE(loaded_payload[urpg::progression::kStatAllocationsSaveKey][0]["remainingPoints"] == 0);
    REQUIRE(loaded_payload["map_id"] == 2);

    std::filesystem::remove_all(base);
}
