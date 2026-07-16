#include "engine/core/presentation/exploration_feedback.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/scene/map_scene.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Exploration feedback covers the complete sensory interaction vocabulary",
          "[presentation][exploration][pcq602]") {
    using namespace urpg::presentation;
    ExplorationFeedbackDirector director;
    const std::vector<ExplorationFeedbackRequest> requests = {
        {"prompt", ExplorationFeedbackKind::InteractionPrompt, "Talk", "guide", ""},
        {"ack", ExplorationFeedbackKind::InteractionAcknowledged, "Interact", "guide", ""},
        {"pickup", ExplorationFeedbackKind::Pickup, "Potion acquired", "item.potion", ""},
        {"reward", ExplorationFeedbackKind::Reward, "100 gold", "quest.one", ""},
        {"door", ExplorationFeedbackKind::Door, "Door opened", "door.one", "urpg.sound.wood_door"},
        {"transfer", ExplorationFeedbackKind::Transfer, "Entering town", "town", ""},
        {"quest", ExplorationFeedbackKind::QuestUpdate, "Quest updated", "quest.one", ""},
        {"dialogue", ExplorationFeedbackKind::DialogueLine, "Welcome to town.", "guide", ""},
        {"transition", ExplorationFeedbackKind::ScreenTransition, "Travel", "town", ""}
    };
    for (const auto& request : requests) REQUIRE(director.submit(request).success);
    REQUIRE_FALSE(director.submit(requests.front()).success);
    while (director.snapshot().active || !director.snapshot().queued.empty()) director.advance(4000);
    const auto snapshot = director.snapshot();
    REQUIRE(snapshot.timeline.size() == requests.size());
    for (const auto& cue : snapshot.timeline) {
        REQUIRE(cue.completed);
        REQUIRE(cue.elapsed_ms == cue.duration_ms);
        REQUIRE(cue.duration_ms > 0);
        REQUIRE(cue.duration_ms <= 4000);
        REQUIRE(cue.icon.starts_with("urpg.icon."));
        REQUIRE(cue.non_color_cue);
    }
    REQUIRE(snapshot.timeline.front().kind == ExplorationFeedbackKind::Transfer);
    REQUIRE(snapshot.timeline[1].kind == ExplorationFeedbackKind::ScreenTransition);
}

TEST_CASE("Exploration feedback honors audio reduced-motion and dialogue cadence settings",
          "[presentation][exploration][pcq602]") {
    using namespace urpg::presentation;
    ExplorationFeedbackDirector director;
    director.setSettings({false, true, true, 2.0F});
    REQUIRE(director.submit({"reward", ExplorationFeedbackKind::Reward, "Treasure found", "chest", ""}).success);
    auto active = director.snapshot().active;
    REQUIRE(active);
    REQUIRE(active->sound.empty());
    REQUIRE(active->particle_motif.empty());
    REQUIRE(active->camera_cue.empty());
    director.advance(4000);
    REQUIRE(director.submit({"line", ExplorationFeedbackKind::DialogueLine,
                             "A deliberately longer line used to verify bounded readable cadence.", "guide", ""}).success);
    active = director.snapshot().active;
    REQUIRE(active->duration_ms >= 400);
    REQUIRE(active->duration_ms <= 4000);
    REQUIRE(active->sound.empty());
}

TEST_CASE("MapScene emits and renders integrated interaction dialogue and transfer feedback",
          "[scene][map][exploration_feedback][pcq602]") {
    using namespace urpg::presentation;
    urpg::scene::MapScene map("town", 8, 8);
    REQUIRE(map.notifyExplorationFeedback({"quest", ExplorationFeedbackKind::QuestUpdate,
                                           "Find the guide", "quest.guide", ""}).success);
    map.onUpdate(0.016F);
    REQUIRE(map.explorationFeedback().activeCue());
    bool rendered = false;
    for (const auto& command : urpg::RenderLayer::getInstance().getFrameCommands()) {
        if (command.type != urpg::RenderCmdType::Text) continue;
        const auto* text = command.tryGet<urpg::TextRenderData>();
        rendered = rendered || (text != nullptr && text->text.find("Find the guide") != std::string::npos);
    }
    REQUIRE(rendered);
    map.startDialogue({{"hello", "Welcome.", {}, true, {}, 0, {}}});
    REQUIRE(map.explorationFeedback().snapshot().queued.size() == 1);
}
