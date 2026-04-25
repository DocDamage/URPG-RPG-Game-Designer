#include "engine/core/replay/replay_player.h"
#include "engine/core/replay/replay_recorder.h"
#include "engine/core/save/save_runtime.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

TEST_CASE("Integration: replay artifacts survive save load and compare deterministically", "[integration][replay][save][ffs11]") {
    urpg::replay::ReplayRecorder recorder(99, "2.0.0");
    recorder.addLabel("golden");
    recorder.recordInput(0, "start", {}, {{"map", "town"}});
    recorder.recordInput(3, "confirm", {{"button", "ok"}}, {{"map", "town"}, {"dialogue", 1}});
    const auto artifact = recorder.finish("intro_replay");

    nlohmann::json save_payload;
    save_payload["_replay_gallery"] = {artifact.toJson()};

    const auto base = std::filesystem::temp_directory_path() / "urpg_replay_save_integration";
    std::filesystem::create_directories(base);

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = base / "save.json";

    REQUIRE(urpg::RuntimeSaveLoader::Save(request, save_payload.dump()));
    const auto result = urpg::RuntimeSaveLoader::Load(request);
    REQUIRE(result.ok);

    const auto loaded_payload = nlohmann::json::parse(result.payload);
    const auto loaded = urpg::replay::ReplayArtifact::fromJson(loaded_payload["_replay_gallery"][0]);
    const auto comparison = urpg::replay::ReplayPlayer::compare(artifact, loaded);

    REQUIRE(comparison.matches);
    REQUIRE(comparison.first_mismatched_tick == -1);

    std::filesystem::remove_all(base);
}
