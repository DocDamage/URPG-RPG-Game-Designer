#include <catch2/catch_test_macros.hpp>

#include "engine/core/character/character_creation_screen.h"
#include "engine/core/ecs/actor_components.h"
#include "engine/core/ecs/actor_manager.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/sprite_batcher.h"

#include <algorithm>
#include <type_traits>

namespace {

using urpg::character::CharacterCreationScreen;

template <typename StoredCommand>
const urpg::TextRenderData* textPayload(const StoredCommand& command) {
    if constexpr (requires { command.template tryGet<urpg::TextRenderData>(); }) {
        return command.template tryGet<urpg::TextRenderData>();
    }
    return nullptr;
}

template <typename StoredCommand>
const urpg::RectRenderData* rectPayload(const StoredCommand& command) {
    if constexpr (requires { command.template tryGet<urpg::RectRenderData>(); }) {
        return command.template tryGet<urpg::RectRenderData>();
    }
    return nullptr;
}

bool frameContainsText(const std::vector<urpg::FrameRenderCommand>& commands, const std::string& expected) {
    return std::any_of(commands.begin(), commands.end(), [&](const auto& command) {
        if (command.type != urpg::RenderCmdType::Text) {
            return false;
        }
        const auto* text = textPayload(command);
        return text != nullptr && text->text.find(expected) != std::string::npos;
    });
}

size_t rectCount(const std::vector<urpg::FrameRenderCommand>& commands) {
    return static_cast<size_t>(std::count_if(commands.begin(), commands.end(), [](const auto& command) {
        return command.type == urpg::RenderCmdType::Rect && rectPayload(command) != nullptr;
    }));
}

urpg::input::InputCore pressed(urpg::input::InputAction action) {
    urpg::input::InputCore input;
    input.updateActionState(action, urpg::input::ActionState::Pressed);
    return input;
}

} // namespace

TEST_CASE("CharacterCreationScreen cycles runtime selections and updates preview snapshot",
          "[character][runtime][screen]") {
    CharacterCreationScreen screen;
    urpg::World world;
    urpg::ActorManager actorManager(world);

    REQUIRE(screen.buildSnapshot()["workflow"]["active_step"] == "Name");

    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    REQUIRE(screen.activeStep() == CharacterCreationScreen::Step::Class);

    const auto beforeClass = screen.buildSnapshot()["identity"]["classId"].get<std::string>();
    screen.handleInput(pressed(urpg::input::InputAction::MoveRight), world, actorManager);

    const auto afterSnapshot = screen.buildSnapshot();
    const auto afterClass = afterSnapshot["identity"]["classId"].get<std::string>();
    REQUIRE(afterClass != beforeClass);
    REQUIRE(afterSnapshot["preview"]["primary_attribute"].get<std::string>().empty() == false);

    const auto events = screen.consumeEvents();
    REQUIRE(events.empty() == false);
    REQUIRE(std::any_of(events.begin(), events.end(), [](const auto& event) {
        return event.type == urpg::character::CharacterCreationEventType::StepChanged;
    }));
    REQUIRE(std::any_of(events.begin(), events.end(), [](const auto& event) {
        return event.type == urpg::character::CharacterCreationEventType::SelectionChanged;
    }));
}

TEST_CASE("CharacterCreationScreen final review spawns actor and raises completion event",
          "[character][runtime][screen]") {
    CharacterCreationScreen screen;
    urpg::World world;
    urpg::ActorManager actorManager(world);

    screen.setName("Nova");
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    screen.handleInput(pressed(urpg::input::InputAction::MoveDown), world, actorManager);
    REQUIRE(screen.activeStep() == CharacterCreationScreen::Step::Review);

    urpg::EntityID callbackEntity = 0;
    screen.setCompletionHandler([&](urpg::EntityID entity, const urpg::character::CharacterIdentity& identity) {
        callbackEntity = entity;
        REQUIRE(identity.getName() == "Nova");
    });

    screen.handleInput(pressed(urpg::input::InputAction::Confirm), world, actorManager);

    REQUIRE(screen.lastCreatedEntity().has_value());
    REQUIRE(*screen.lastCreatedEntity() != 0);
    REQUIRE(callbackEntity == *screen.lastCreatedEntity());

    const auto* actor = world.GetComponent<urpg::ActorComponent>(*screen.lastCreatedEntity());
    REQUIRE(actor != nullptr);
    REQUIRE(actor->name == "Nova");
    REQUIRE(actor->className.empty() == false);

    const auto events = screen.consumeEvents();
    REQUIRE(std::any_of(events.begin(), events.end(), [&](const auto& event) {
        return event.type == urpg::character::CharacterCreationEventType::CreationCompleted &&
               event.entity == *screen.lastCreatedEntity();
    }));
}

TEST_CASE("CharacterCreationScreen draws a runtime preview surface into RenderLayer",
          "[character][runtime][screen][render]") {
    CharacterCreationScreen screen;
    urpg::World world;
    urpg::ActorManager actorManager(world);
    urpg::SpriteBatcher batcher;
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    screen.setName("Lyra");
    screen.draw(batcher);

    const auto& commands = layer.getFrameCommands();
    REQUIRE(commands.empty() == false);
    REQUIRE(frameContainsText(commands, "Create Character"));
    REQUIRE(frameContainsText(commands, "Preview Card"));
    REQUIRE(frameContainsText(commands, "Name: Lyra"));
    REQUIRE(frameContainsText(commands, "Class:"));
    REQUIRE(rectCount(commands) >= 6);

    layer.flush();
    (void)world;
    (void)actorManager;
}
