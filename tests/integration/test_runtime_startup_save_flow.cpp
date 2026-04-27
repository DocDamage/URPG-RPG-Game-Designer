#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/save/save_serialization_hub.h"
#include "engine/core/scene/runtime_title_scene.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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

    void writeSaveFile(const std::string& filename, const std::vector<uint8_t>& payload) const {
        std::filesystem::create_directories(saves());
        std::ofstream out(saves() / filename, std::ios::binary);
        out.write(reinterpret_cast<const char*>(payload.data()), static_cast<std::streamsize>(payload.size()));
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
                                          }, {}});
    title.setContinueAvailability(state.hasLoadableSave(), state.continueDisabledReason());
    const auto commandResult = title.activateCommand(urpg::scene::RuntimeTitleCommandId::Continue);
    REQUIRE(commandResult.success);
    REQUIRE(commandResult.code == "continue_loaded");
    REQUIRE(continueCalled);
}

TEST_CASE("Runtime startup Continue loads valid URSV payloads through checksum path",
          "[integration][runtime][save][checksum]") {
    const TempProject project;
    const auto binary = urpg::save::SaveSerializationHub::jsonToBinary(R"({"state":"binary-primary"})",
                                                                       urpg::save::SaveSerializationHub::CompressionLevel::None);
    project.writeSaveFile("slot_5.ursv", binary);
    project.writeSaveFile("slot_5_meta.json", metadataJson(5, "2026-04-26T12:00:00Z", "BinaryMap"));

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE(state.hasLoadableSave());
    REQUIRE(state.newestLoadableSave() != nullptr);
    REQUIRE(state.newestLoadableSave()->slot_id == 5);

    const auto result = urpg::continueNewestRuntimeSave(state);
    REQUIRE(result.ok);
    REQUIRE(result.slot_id == 5);
    REQUIRE_FALSE(result.loaded_from_recovery);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::None);
    REQUIRE(result.active_meta.map_display_name == "BinaryMap");
    REQUIRE(result.diagnostics.empty());
}

TEST_CASE("Runtime startup Continue recovers from corrupt URSV primary through autosave",
          "[integration][runtime][save][recovery][checksum][autosave]") {
    const TempProject project;
    auto binary = urpg::save::SaveSerializationHub::jsonToBinary(R"({"state":"broken-primary"})",
                                                                 urpg::save::SaveSerializationHub::CompressionLevel::None);
    binary[12] = static_cast<uint8_t>(binary[12] ^ 0x7F);
    project.writeSaveFile("slot_6.ursv", binary);
    project.writeSaveFile("slot_6_meta.json", metadataJson(6, "2026-04-26T13:00:00Z", "CorruptPrimaryMap"));
    project.writeSaveFile("autosave.json", R"({"state":"autosave-fallback"})");

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE(state.hasLoadableSave());
    REQUIRE(state.newestLoadableSave() != nullptr);
    REQUIRE(state.newestLoadableSave()->slot_id == 6);

    const auto result = urpg::continueNewestRuntimeSave(state);
    REQUIRE(result.ok);
    REQUIRE(result.slot_id == 6);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level1Autosave);
    REQUIRE_FALSE(result.boot_safe_mode);
    REQUIRE(result.diagnostics.size() == 1);
    REQUIRE(result.diagnostics[0] == "primary_payload_ursv_checksum_failed");
}

TEST_CASE("Runtime startup Continue recovers corrupt primary and corrupt autosave through safe skeleton",
          "[integration][runtime][save][recovery][checksum]") {
    const TempProject project;
    auto binary = urpg::save::SaveSerializationHub::jsonToBinary(R"({"state":"broken-primary"})",
                                                                 urpg::save::SaveSerializationHub::CompressionLevel::None);
    binary.back() = static_cast<uint8_t>(binary.back() ^ 0x7F);
    project.writeSaveFile("slot_7.ursv", binary);
    project.writeSaveFile("slot_7_meta.json", metadataJson(7, "2026-04-26T14:00:00Z", "SkeletonMap"));
    project.writeSaveFile("autosave.json", R"({"state":)");

    const auto state = urpg::discoverRuntimeSaves(project.root());

    REQUIRE(state.hasLoadableSave());
    REQUIRE(state.newestLoadableSave() != nullptr);
    REQUIRE(state.newestLoadableSave()->slot_id == 7);

    const auto result = urpg::continueNewestRuntimeSave(state);
    REQUIRE(result.ok);
    REQUIRE(result.slot_id == 7);
    REQUIRE(result.loaded_from_recovery);
    REQUIRE(result.boot_safe_mode);
    REQUIRE(result.recovery_tier == urpg::SaveRecoveryTier::Level3SafeSkeleton);
    REQUIRE(result.active_meta.flags.corrupted);
    REQUIRE(result.active_meta.map_display_name == "SkeletonMap");
    REQUIRE(result.diagnostics.size() == 2);
    REQUIRE(result.diagnostics[0] == "primary_payload_ursv_checksum_failed");
    REQUIRE(result.diagnostics[1] == "autosave_payload_json_parse_failed");
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
