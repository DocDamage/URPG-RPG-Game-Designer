#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/scene/runtime_title_scene.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace {

class TempProject {
  public:
    TempProject() {
        const auto unique = std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
        root_ = std::filesystem::temp_directory_path() / ("urpg_runtime_startup_save_" + unique);
        std::filesystem::create_directories(root_);
    }

    ~TempProject() {
        std::error_code ec;
        std::filesystem::remove_all(root_, ec);
    }

    const std::filesystem::path& root() const { return root_; }
    std::filesystem::path saves() const { return root_ / "saves"; }

    void writeSaveFile(const std::string& filename, const std::string& payload) const {
        std::filesystem::create_directories(saves());
        std::ofstream out(saves() / filename, std::ios::binary);
        out << payload;
    }

  private:
    std::filesystem::path root_;
};

std::string metadataJson(int slot_id, const std::string& timestamp, const std::string& map_name) {
    return std::string("{") + "\"_slot_id\":" + std::to_string(slot_id) + "," +
           "\"_slot_category\":\"manual\"," + "\"_retention_class\":\"manual\"," +
           "\"_save_version\":\"1.0\"," + "\"_timestamp\":\"" + timestamp + "\"," +
           "\"_map_display_name\":\"" + map_name + "\"}";
}

} // namespace

TEST_CASE("Runtime startup save discovery disables Continue when no saves exist",
          "[integration][runtime][save]") {
    const TempProject project;

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE(state.save_root == project.saves());
    REQUIRE(state.entries.empty());
    REQUIRE_FALSE(state.hasLoadableSave());
    REQUIRE(state.continueDisabledReason() == "No loadable save data found");

    urpg::scene::RuntimeTitleScene title;
    title.setContinueAvailability(state.hasLoadableSave(), state.continueDisabledReason());
    const auto* continueCommand = title.findCommand(urpg::scene::RuntimeTitleCommandId::Continue);
    REQUIRE(continueCommand != nullptr);
    REQUIRE_FALSE(continueCommand->enabled);
    REQUIRE(continueCommand->disabled_reason == "No loadable save data found");
}

TEST_CASE("Runtime startup save discovery enables Continue for newest valid primary save",
          "[integration][runtime][save]") {
    const TempProject project;
    project.writeSaveFile("slot_1.json", R"({"player":"older"})");
    project.writeSaveFile("slot_1_meta.json", metadataJson(1, "2026-04-25T10:00:00Z", "OlderMap"));
    project.writeSaveFile("slot_2.json", R"({"player":"newer"})");
    project.writeSaveFile("slot_2_meta.json", metadataJson(2, "2026-04-26T10:00:00Z", "NewestMap"));

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE(state.hasLoadableSave());
    REQUIRE(state.entries.size() == 2);
    REQUIRE(state.newestLoadableSave() != nullptr);
    REQUIRE(state.newestLoadableSave()->slot_id == 2);
    REQUIRE(state.newestLoadableSave()->expected_recovery_tier == urpg::SaveRecoveryTier::None);

    const auto result = urpg::continueNewestRuntimeSave(state);
    REQUIRE(result.ok);
    REQUIRE(result.slot_id == 2);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::None);
    REQUIRE(result.active_meta.map_display_name == "NewestMap");

    bool continueCalled = false;
    urpg::scene::RuntimeTitleScene title({{}, {}, [&continueCalled] {
                                              continueCalled = true;
                                              return urpg::scene::RuntimeTitleCommandResult{
                                                  true, true, "continue_loaded", "loaded"};
                                          }});
    title.setContinueAvailability(state.hasLoadableSave(), state.continueDisabledReason());
    const auto commandResult = title.activateCommand(urpg::scene::RuntimeTitleCommandId::Continue);
    REQUIRE(commandResult.success);
    REQUIRE(commandResult.code == "continue_loaded");
    REQUIRE(continueCalled);
}

TEST_CASE("Runtime startup Continue can recover from metadata and variables when primary save is missing",
          "[integration][runtime][save]") {
    const TempProject project;
    project.writeSaveFile("slot_3_meta.json", metadataJson(3, "2026-04-26T11:00:00Z", "RecoveredMap"));
    project.writeSaveFile("slot_3_vars.json", R"({"gold":42})");

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE(state.hasLoadableSave());
    REQUIRE(state.newestLoadableSave() != nullptr);
    REQUIRE(state.newestLoadableSave()->slot_id == 3);
    REQUIRE(state.newestLoadableSave()->requires_recovery);
    REQUIRE(state.newestLoadableSave()->expected_recovery_tier == urpg::SaveRecoveryTier::Level2MetadataVariables);

    const auto result = urpg::continueNewestRuntimeSave(state);
    REQUIRE(result.ok);
    REQUIRE(result.slot_id == 3);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level2MetadataVariables);
    REQUIRE(result.active_meta.flags.corrupted);
    REQUIRE(result.active_meta.map_display_name == "RecoveredMap");
}

TEST_CASE("Runtime startup save discovery rejects corrupt metadata-only slots without crashing",
          "[integration][runtime][save]") {
    const TempProject project;
    project.writeSaveFile("slot_4_meta.json", R"({"_slot_id":4,)");

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE_FALSE(state.hasLoadableSave());
    REQUIRE(state.entries.size() == 1);
    REQUIRE(state.entries[0].slot_id == 4);
    REQUIRE_FALSE(state.entries[0].loadable);
    REQUIRE(state.entries[0].diagnostic.find("metadata_parse_failed") != std::string::npos);
    REQUIRE(state.entries[0].diagnostic.find("slot_not_loadable") != std::string::npos);

    const auto result = urpg::continueNewestRuntimeSave(state);
    REQUIRE_FALSE(result.ok);
    REQUIRE(result.error == "no_loadable_save");
}
