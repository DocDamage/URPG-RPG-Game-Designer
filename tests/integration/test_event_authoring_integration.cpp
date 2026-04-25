#include "engine/core/events/event_dependency_graph.h"
#include "engine/core/save/save_runtime.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <algorithm>
#include <filesystem>

TEST_CASE("Integration: event authoring save-field dependencies survive save load path", "[integration][events][save]") {
    using namespace urpg::events;

    EventDocument document;
    document.addMap(MapDefinition{"town", 10, 10});
    document.setKnownSaveFields({"party.gold", "quest.flags.intro"});
    document.addEvent(EventDefinition{
        "evt_reward",
        "town",
        2,
        3,
        {EventPage{
            "reward",
            0,
            EventTrigger::ActionButton,
            {},
            {EventCommand{"reward_gold", EventCommandKind::Gold, {}, {}, 100, {"party.gold"}, {"party.gold", "quest.flags.intro"}}}
        }}
    });

    REQUIRE(document.validate().empty());
    const auto graph = EventDependencyGraph::build(document);
    REQUIRE(graph.edges().size() == 3);

    nlohmann::json save_payload;
    save_payload["party"]["gold"] = 50;
    save_payload["quest"]["flags"]["intro"] = false;
    save_payload["_event_authoring_dependencies"] = document.toJson();

    const auto base = std::filesystem::temp_directory_path() / "urpg_event_authoring_save_integration";
    std::filesystem::create_directories(base);

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = base / "save.json";

    REQUIRE(urpg::RuntimeSaveLoader::Save(request, save_payload.dump()));
    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);

    const auto loaded_payload = nlohmann::json::parse(result.payload);
    const auto loaded_document = EventDocument::fromJson(loaded_payload["_event_authoring_dependencies"]);
    const auto loaded_graph = EventDependencyGraph::build(loaded_document);

    REQUIRE(loaded_graph.edges().size() == graph.edges().size());
    REQUIRE(std::any_of(loaded_graph.edges().begin(), loaded_graph.edges().end(), [](const auto& edge) {
        return edge.target_type == "save_field" && edge.target_id == "quest.flags.intro" &&
               edge.access == EventDependencyAccess::Write;
    }));

    std::filesystem::remove_all(base);
}
