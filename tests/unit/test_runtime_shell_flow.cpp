#include "engine/core/scene/runtime_shell_flow.h"

#include <catch2/catch_test_macros.hpp>

namespace {

void press(urpg::scene::RuntimeShellFlow& flow, urpg::input::InputCore& input, urpg::input::InputAction action) {
    input.updateActionState(action, urpg::input::ActionState::Pressed);
    flow.handleInput(input);
    input.endFrame();
    input.updateActionState(action, urpg::input::ActionState::Released);
    flow.handleInput(input);
    input.endFrame();
}

} // namespace

TEST_CASE("Runtime shell completes controller-only new game save pause settings and quit journey",
          "[scene][runtime][shell][pcq601]") {
    using namespace urpg::scene;
    bool started = false, saved = false, settings_open = false, exited = false;
    RuntimeShellFlow flow({
        [&](const std::string& profile) { started = profile == "hero"; return RuntimeShellActionResult{true, started, "started", "Game started."}; },
        {},
        [&](const int slot) { saved = slot == 0; return RuntimeShellActionResult{true, saved, "saved", "Game saved."}; },
        [&] { settings_open = true; },
        [&] { settings_open = false; },
        [&] { exited = true; }
    });
    flow.setProfiles({{"hero", "Hero", true, {}}});
    flow.setActiveDevice(RuntimeInputDevice::GenericController);
    REQUIRE(flow.completeStartup(true).success);
    REQUIRE(flow.state() == RuntimeShellState::Title);
    REQUIRE(flow.snapshot().confirm_glyph == "urpg.glyph.controller.face_bottom");
    urpg::input::InputCore input;

    press(flow, input, urpg::input::InputAction::Confirm);
    REQUIRE(flow.state() == RuntimeShellState::ProfileSelect);
    press(flow, input, urpg::input::InputAction::Confirm);
    REQUIRE(started);
    REQUIRE(flow.state() == RuntimeShellState::Gameplay);
    REQUIRE(flow.snapshot().active_profile_id == "hero");

    press(flow, input, urpg::input::InputAction::Menu);
    REQUIRE(flow.state() == RuntimeShellState::Pause);
    REQUIRE(flow.snapshot().commands.front().id == "resume");
    press(flow, input, urpg::input::InputAction::MoveDown);
    press(flow, input, urpg::input::InputAction::Confirm);
    REQUIRE(saved);
    REQUIRE(flow.lastResult().code == "saved");

    press(flow, input, urpg::input::InputAction::MoveDown);
    press(flow, input, urpg::input::InputAction::Confirm);
    REQUIRE(settings_open);
    REQUIRE(flow.state() == RuntimeShellState::Settings);
    press(flow, input, urpg::input::InputAction::Cancel);
    REQUIRE_FALSE(settings_open);
    REQUIRE(flow.state() == RuntimeShellState::Pause);

    REQUIRE(flow.activate("quit_game").success);
    REQUIRE(flow.state() == RuntimeShellState::QuitConfirm);
    REQUIRE(flow.activate("confirm_quit").success);
    REQUIRE(exited);
    REQUIRE(flow.state() == RuntimeShellState::Exited);
    for (const auto& transition : flow.snapshot().transitions) {
        REQUIRE(transition.input_acknowledged);
        REQUIRE(transition.duration_ms <= 250);
    }
}

TEST_CASE("Runtime shell controller-only continue loads a valid save and reports invalid saves",
          "[scene][runtime][shell][pcq601]") {
    using namespace urpg::scene;
    int loaded_slot = -1;
    RuntimeShellFlow flow({{}, [&](const int slot) {
        loaded_slot = slot;
        return RuntimeShellActionResult{true, true, "loaded", "Save loaded."};
    }, {}, {}, {}, {}});
    flow.setProfiles({{"hero", "Hero", true, {}}});
    flow.setSaves({{2, "hero", "Chapter 2", true, {}}, {3, "hero", "Damaged", false, "Save checksum failed."}});
    REQUIRE(flow.completeStartup(true).success);
    REQUIRE(flow.activate("continue").success);
    REQUIRE(flow.state() == RuntimeShellState::SaveSelect);
    REQUIRE(flow.snapshot().commands.size() == 3);
    REQUIRE(flow.activate("save:3").code == "shell_command_disabled");
    REQUIRE(flow.lastResult().message == "Save checksum failed.");
    REQUIRE(flow.activate("save:2").success);
    REQUIRE(loaded_slot == 2);
    REQUIRE(flow.state() == RuntimeShellState::Gameplay);
    REQUIRE(flow.snapshot().active_save_slot == 2);
    REQUIRE(flow.snapshot().active_profile_id == "hero");
}

TEST_CASE("Runtime shell provides focus acknowledgement cancel recovery and bounded history",
          "[scene][runtime][shell][pcq601]") {
    using namespace urpg::scene;
    RuntimeShellFlow flow;
    REQUIRE_FALSE(flow.completeStartup(false, "Runtime bundle failed validation.").success);
    REQUIRE(flow.state() == RuntimeShellState::Error);
    REQUIRE(flow.snapshot().acknowledgement == "Runtime bundle failed validation.");
    REQUIRE(flow.activate("recover").success);
    REQUIRE(flow.state() == RuntimeShellState::Title);
    REQUIRE_FALSE(flow.activate("continue").success);
    REQUIRE(flow.lastResult().code == "shell_command_disabled");

    urpg::input::InputCore input;
    press(flow, input, urpg::input::InputAction::MoveDown);
    REQUIRE(flow.lastResult().code == "shell_focus_moved");
    REQUIRE(flow.snapshot().selected_index == 1);
    REQUIRE(flow.snapshot().acknowledgement.find("Focus moved") != std::string::npos);
    REQUIRE(flow.activate("settings").success);
    REQUIRE(flow.state() == RuntimeShellState::Settings);
    REQUIRE(flow.returnFromSettings().success);
    REQUIRE(flow.state() == RuntimeShellState::Title);
    REQUIRE(flow.snapshot().acknowledgement.find("previous focus restored") != std::string::npos);
}
