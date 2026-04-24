#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/diagnostics/diagnostics_facade.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/input/input_core.h"
#include "engine/core/message/message_core.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

TEST_CASE("DiagnosticsWorkspace - Audio runtime actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][audio_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);

    urpg::audio::AudioCore firstAudioCore;
    firstAudioCore.playSound("first_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(firstAudioCore);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "audio");
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "first_audio");

    workspace.clearAudioRuntime();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["live_rows"].empty());

    urpg::audio::AudioCore secondAudioCore;
    secondAudioCore.playSound("second_audio_a", urpg::audio::AudioCategory::SE);
    secondAudioCore.playSound("second_audio_b", urpg::audio::AudioCategory::BGS);
    workspace.bindAudioRuntime(secondAudioCore);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "second_audio_a");
    REQUIRE(exported["active_tab_detail"]["live_rows"][1]["assetId"] == "second_audio_b");
}

TEST_CASE("DiagnosticsWorkspace - Audio row navigation is exposed at workspace level",
          "[editor][diagnostics][integration][audio_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);

    urpg::audio::AudioCore audioCore;
    audioCore.playSound("audio_a", urpg::audio::AudioCategory::SE);
    audioCore.playSound("audio_b", urpg::audio::AudioCategory::BGS);
    workspace.bindAudioRuntime(audioCore);

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_handle"].is_null());
    REQUIRE(exported["active_tab_detail"]["selected_row"].is_null());
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_row"] == false);

    REQUIRE(workspace.selectNextAudioRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["assetId"] == "audio_a");
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_row"] == false);

    REQUIRE(workspace.selectNextAudioRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["assetId"] == "audio_b");
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == false);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_row"] == true);

    REQUIRE(workspace.selectPreviousAudioRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["assetId"] == "audio_a");
}

TEST_CASE("DiagnosticsWorkspace - Event authority actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][event_authority_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventAuthority);

    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_alpha\",\"block_id\":\"blk_alpha\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"alpha\"}\n"
        "{\"ts\":\"2026-03-04T00:00:03Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_beta\",\"block_id\":\"blk_beta\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"beta\"}"
    );
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "event_authority");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 2);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);

    workspace.clearEventAuthorityDiagnostics();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 0);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"].empty());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);

    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:04Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_gamma\",\"block_id\":\"blk_gamma\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"new_warning\",\"message\":\"gamma\"}"
    );
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"][0]["event_id"] == "evt_gamma");
}

TEST_CASE("DiagnosticsWorkspace - Activating a snapshot-backed tab refreshes exported detail without manual render",
          "[editor][diagnostics][integration][tab_switch_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    urpg::audio::AudioCore audioCore;
    audioCore.playSound("tab_switch_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(audioCore);

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "compat");

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "audio");
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "tab_switch_audio");
}

TEST_CASE("DiagnosticsWorkspace - Revealing a hidden snapshot-backed tab refreshes exported detail without manual render",
          "[editor][diagnostics][integration][visible_switch_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Audio);
    workspace.setVisible(false);

    urpg::audio::AudioCore audioCore;
    audioCore.playSound("visible_switch_audio", urpg::audio::AudioCategory::SE);
    workspace.bindAudioRuntime(audioCore);

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "audio");
    REQUIRE(exported["visible"] == false);

    workspace.setVisible(true);
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["visible"] == true);
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["live_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["live_rows"][0]["assetId"] == "visible_switch_audio");
}


TEST_CASE("DiagnosticsWorkspace - Event authority workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][event_authority_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::EventAuthority);

    workspace.ingestEventAuthorityDiagnosticsJsonl(
        "{\"ts\":\"2026-03-04T00:00:02Z\",\"level\":\"warn\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_alpha\",\"block_id\":\"blk_alpha\",\"mode\":\"compat\",\"operation\":\"edit_urpg_ast\",\"error_code\":\"read_only_derived_view\",\"message\":\"alpha\"}\n"
        "{\"ts\":\"2026-03-04T00:00:03Z\",\"level\":\"error\",\"subsystem\":\"event_authority\",\"event\":\"edit_rejected\",\"event_id\":\"evt_beta\",\"block_id\":\"blk_beta\",\"mode\":\"mixed\",\"operation\":\"edit_raw_command_list\",\"error_code\":\"invalid_for_mode\",\"message\":\"beta\"}"
    );

    REQUIRE(workspace.setEventAuthorityEventIdFilter("evt_beta"));
    REQUIRE(workspace.selectEventAuthorityRow(0));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["event_id_filter"] == "evt_beta");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(exported["active_tab_detail"]["selected_row"]["event_id"] == "evt_beta");

    REQUIRE(workspace.setEventAuthorityEventIdFilter(""));
    REQUIRE(workspace.selectNextEventAuthorityRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["event_id"] == "evt_alpha");
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == true);

    REQUIRE(workspace.selectNextEventAuthorityRow());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_row"]["event_id"] == "evt_beta");
    REQUIRE(exported["active_tab_detail"]["can_select_next_row"] == false);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_row"] == true);

    REQUIRE(workspace.setEventAuthorityLevelFilter("error"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["level_filter"] == "error");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 1);
    REQUIRE(exported["active_tab_detail"]["visible_row_entries"][0]["event_id"] == "evt_beta");

    REQUIRE(workspace.clearEventAuthorityFilters());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["event_id_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["level_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["mode_filter"] == "");
    REQUIRE(exported["active_tab_detail"]["visible_rows"] == 2);
}


