#include "engine/core/character/character_creation_screen.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/actor_manager.h"
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
