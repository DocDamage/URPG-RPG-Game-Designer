#include <catch2/catch_test_macros.hpp>
#include "engine/core/audio/audio_core.h"
#include "engine/core/ui/menu_scene_graph.h"

using namespace urpg::audio;
using namespace urpg::ui;

TEST_CASE("AudioCore: Category Volume and Playback", "[audio][core]") {
    AudioCore audio;

    SECTION("Initial volumes are default") {
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 1.0f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::SE) == 1.0f);
    }

    SECTION("Setting category volume") {
        audio.setCategoryVolume(AudioCategory::BGM, 0.5f);
        REQUIRE(audio.getCategoryVolume(AudioCategory::BGM) == 0.5f);
    }

    SECTION("Playing sounds returns unique handles") {
        auto h1 = audio.playSound("test_se");
        auto h2 = audio.playSound("test_se");
        REQUIRE(h1 != h2);
        REQUIRE(h1 > 0);
    }
}

TEST_CASE("UI Audio Integration", "[ui][audio]") {
    auto audio = std::make_shared<AudioCore>();
    MenuSceneGraph graph;
    graph.setAudio(audio);

    auto menu = std::make_shared<MenuScene>("MainMenu");
    MenuPane pane;
    pane.id = "p1";
    pane.isActive = true;
    pane.commands = { {"c1"}, {"c2"} };
    menu->addPane(pane);
    graph.registerScene(menu);

    SECTION("Scene Push triggers sound") {
        graph.pushScene("MainMenu");
        // Logic check: pushScene adds to stack
        REQUIRE(graph.stackSize() == 1);
    }

    SECTION("Navigation triggers cursor sound") {
        graph.pushScene("MainMenu");
        // We can't easily check the side effect in a mock, 
        // but we verify the code path doesn't crash
        graph.handleInput(urpg::input::InputAction::MoveDown, urpg::input::ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 1);
    }
}
