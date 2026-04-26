#include "editor/diagnostics/migration_wizard_model.h"
#include "editor/diagnostics/migration_wizard_panel.h"
#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

using namespace urpg::editor;

TEST_CASE("MigrationWizardModel: Batch Orchestration", "[editor][diagnostics][wizard]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {
        {"messages",
         {{"pages",
           {{{"id", "speaker_a"},
             {"route", "speaker"},
             {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
             {"text", {"Hello there."}}}}}}},
        {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}},
        {"troops", {{{"id", 1}, {"name", "Slime x2"}, {"members", {}}}}},
        {"save",
         {{"_urpg_format_version", "mz_compat_1"},
          {"meta", {{"slotId", 4}, {"mapName", "Archivist Vault"}, {"playtimeSeconds", 222}}}}}};

    SECTION("Initial state") {
        auto report = model.getReport();
        REQUIRE(report.total_files_processed == 0);
        REQUIRE_FALSE(report.is_complete);
    }

    SECTION("Run full migration collects counts from subsystems") {
        model.runFullMigration(project_data);
        auto report = model.getReport();

        REQUIRE(report.is_complete);
        REQUIRE(report.total_files_processed == 4); // messages, scenes/menus, troops, save metadata
        REQUIRE(report.subsystem_results.size() == 4);
        REQUIRE(report.subsystem_results[0].subsystem_id == "message");
        REQUIRE(report.subsystem_results[0].processed_count == 1);
        REQUIRE(report.subsystem_results[0].summary_line.find("Message migration") != std::string::npos);
        REQUIRE(report.subsystem_results[1].subsystem_id == "menu");
        REQUIRE(report.subsystem_results[1].processed_count == 1);
        REQUIRE(report.subsystem_results[1].summary_line.find("Menu migration") != std::string::npos);
        REQUIRE(report.subsystem_results[2].subsystem_id == "battle");
        REQUIRE(report.subsystem_results[2].processed_count == 1);
        REQUIRE(report.subsystem_results[2].summary_line.find("Battle migration") != std::string::npos);
        REQUIRE(report.subsystem_results[3].subsystem_id == "save");
        REQUIRE(report.subsystem_results[3].processed_count == 1);
        REQUIRE(report.subsystem_results[3].summary_line.find("Save migration") != std::string::npos);
        REQUIRE(report.summary_logs.size() == 5);
        REQUIRE(report.summary_logs[0].find("Message migration") != std::string::npos);
        REQUIRE(report.summary_logs[1].find("Menu migration") != std::string::npos);
        REQUIRE(report.summary_logs[2].find("Battle migration") != std::string::npos);
        REQUIRE(report.summary_logs[3].find("Save migration") != std::string::npos);
        REQUIRE(report.summary_logs[4] == "Migration wizard complete.");
        REQUIRE(model.selectedSubsystemId().has_value());
        REQUIRE(*model.selectedSubsystemId() == "message");
    }
}

TEST_CASE("MigrationWizardModel: save migration warnings propagate from unmapped compat save fields",
          "[editor][diagnostics][wizard][save][warnings]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {
        {"save",
         {{"_urpg_format_version", "mz_compat_1"},
          {"meta", {{"slotId", 9}, {"mapName", "Signal Spire"}, {"customBadge", "nightmare"}}},
          {"pluginHeader", {{"uiTab", "system"}, {"theme", "violet"}}}}}};

    model.runFullMigration(project_data);
    const auto& report = model.getReport();

    REQUIRE(report.is_complete);
    REQUIRE(report.total_files_processed == 1);
    REQUIRE(report.warning_count > 0);
    REQUIRE(report.error_count == 0);
    REQUIRE(report.subsystem_results.size() == 1);
    REQUIRE(report.subsystem_results[0].subsystem_id == "save");
    REQUIRE(report.subsystem_results[0].warning_count > 0);
    REQUIRE(report.subsystem_results[0].summary_line.find("Save migration") != std::string::npos);
}

TEST_CASE("MigrationWizardPanel: Visible render records migration snapshot", "[editor][diagnostics][wizard][panel]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);

    panel.render();

    REQUIRE(panel.hasRenderedFrame());
    REQUIRE(panel.lastRenderSnapshot().total_files_processed == 1);
    REQUIRE(panel.lastRenderSnapshot().warning_count == 0);
    REQUIRE(panel.lastRenderSnapshot().error_count == 0);
    REQUIRE(panel.lastRenderSnapshot().is_complete);
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().summary_log_count == 2);
    REQUIRE(panel.lastRenderSnapshot().headline == "Migration wizard complete.");
    REQUIRE(panel.lastRenderSnapshot().summary_logs.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().summary_logs[0].find("Menu migration") != std::string::npos);
    REQUIRE(panel.lastRenderSnapshot().summary_logs[1] == "Migration wizard complete.");
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results[0].subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().subsystem_results[0].processed_count == 1);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_display_name == "Menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_processed_count == 1);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_warning_count == 0);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_error_count == 0);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_completed);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_summary_line.find("Menu migration") != std::string::npos);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_selected_subsystem);
}

TEST_CASE("MigrationWizardModel: battle migration warnings propagate from unsupported troop phase/page data",
          "[editor][diagnostics][wizard][battle][warnings]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {
        {"troops",
         {{{"id", 7},
           {"name", "Page-heavy troop"},
           {"members", {{{"enemyId", 1}, {"x", 120}, {"y", 180}, {"hidden", false}}}},
           {"pages",
            {{{"conditions", {{"turnEnding", true}}},
              {"list", {{{"code", 101}, {"parameters", {"Hello!"}}}, {{"code", 999}, {"parameters", {}}}}}}}}}}}};

    model.runFullMigration(project_data);
    const auto& report = model.getReport();

    REQUIRE(report.is_complete);
    REQUIRE(report.total_files_processed == 1);
    REQUIRE(report.warning_count > 0);
    REQUIRE(report.error_count == 0);
    REQUIRE(report.subsystem_results.size() == 1);
    REQUIRE(report.subsystem_results[0].subsystem_id == "battle");
    REQUIRE(report.subsystem_results[0].warning_count > 0);
    REQUIRE(report.subsystem_results[0].summary_line.find("Battle migration") != std::string::npos);
}

TEST_CASE("MigrationWizardPanel: render snapshot exposes a rendered workflow body",
          "[editor][diagnostics][wizard][panel][workflow]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}},
                                   {"troops", {{{"id", 1}, {"name", "Slime x2"}, {"members", {}}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.workflow_sections.size() >= 5);
    REQUIRE(snapshot.workflow_sections[0] == "overview");
    REQUIRE(snapshot.workflow_sections[1] == "actions");
    REQUIRE(snapshot.workflow_sections[2] == "subsystems");
    REQUIRE(snapshot.workflow_sections[3] == "report_io");
    REQUIRE(snapshot.workflow_sections[4] == "bound_runtime");

    REQUIRE(snapshot.primary_actions.run_migration.visible);
    REQUIRE(snapshot.primary_actions.run_migration.enabled);
    REQUIRE(snapshot.primary_actions.rerun_selected_subsystem.visible);
    REQUIRE(snapshot.primary_actions.rerun_selected_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.clear_selected_subsystem.visible);
    REQUIRE(snapshot.primary_actions.clear_selected_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.next_subsystem.visible);
    REQUIRE(snapshot.primary_actions.next_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.previous_subsystem.visible);
    REQUIRE_FALSE(snapshot.primary_actions.previous_subsystem.enabled);

    REQUIRE(snapshot.subsystem_cards.size() == 3);
    REQUIRE(snapshot.subsystem_cards[0].subsystem_id == "message");
    REQUIRE(snapshot.subsystem_cards[0].is_selected);
    REQUIRE(snapshot.subsystem_cards[1].subsystem_id == "menu");
    REQUIRE_FALSE(snapshot.subsystem_cards[1].is_selected);
    REQUIRE(snapshot.subsystem_cards[2].subsystem_id == "battle");

    REQUIRE(snapshot.selected_subsystem_card.has_value());
    REQUIRE(snapshot.selected_subsystem_card->subsystem_id == "message");
    REQUIRE(snapshot.selected_subsystem_card->display_name == "Message");
    REQUIRE(snapshot.selected_subsystem_card->can_rerun);
    REQUIRE(snapshot.selected_subsystem_card->can_clear);

    REQUIRE(snapshot.report_io.save.visible);
    REQUIRE(snapshot.report_io.save.enabled);
    REQUIRE(snapshot.report_io.load.visible);
    REQUIRE(snapshot.report_io.load.enabled);
    REQUIRE_FALSE(snapshot.report_io.exported_report_json.empty());

    REQUIRE(snapshot.bound_runtime_actions.has_bound_project_data);
    REQUIRE(snapshot.bound_runtime_actions.rerun_migration.visible);
    REQUIRE(snapshot.bound_runtime_actions.rerun_migration.enabled);
    REQUIRE(snapshot.bound_runtime_actions.rerun_selected_subsystem.visible);
    REQUIRE(snapshot.bound_runtime_actions.rerun_selected_subsystem.enabled);
}

TEST_CASE("MigrationWizardPanel: Clear resets report and rendered snapshot",
          "[editor][diagnostics][wizard][panel][clear]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.getModel()->getReport().total_files_processed == 1);
    REQUIRE(panel.hasRenderedFrame());

    panel.clear();

    const auto& report = panel.getModel()->getReport();
    REQUIRE(report.total_files_processed == 0);
    REQUIRE(report.warning_count == 0);
    REQUIRE(report.error_count == 0);
    REQUIRE_FALSE(report.is_complete);
    REQUIRE(report.summary_logs.empty());
    REQUIRE(report.subsystem_results.empty());

    REQUIRE_FALSE(panel.hasRenderedFrame());
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().summary_logs.empty());
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.empty());
}

TEST_CASE("MigrationWizardPanel: empty render still exposes workflow shell with disabled actions",
          "[editor][diagnostics][wizard][panel][workflow][empty]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);

    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.workflow_sections.size() >= 5);
    REQUIRE(snapshot.workflow_sections[0] == "overview");
    REQUIRE(snapshot.workflow_sections[1] == "actions");
    REQUIRE(snapshot.workflow_sections[2] == "subsystems");
    REQUIRE(snapshot.workflow_sections[3] == "report_io");
    REQUIRE(snapshot.workflow_sections[4] == "bound_runtime");

    REQUIRE(snapshot.primary_actions.run_migration.visible);
    REQUIRE_FALSE(snapshot.primary_actions.run_migration.enabled);
    REQUIRE(snapshot.primary_actions.rerun_selected_subsystem.visible);
    REQUIRE_FALSE(snapshot.primary_actions.rerun_selected_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.clear_selected_subsystem.visible);
    REQUIRE_FALSE(snapshot.primary_actions.clear_selected_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.next_subsystem.visible);
    REQUIRE_FALSE(snapshot.primary_actions.next_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.previous_subsystem.visible);
    REQUIRE_FALSE(snapshot.primary_actions.previous_subsystem.enabled);

    REQUIRE(snapshot.subsystem_cards.empty());
    REQUIRE_FALSE(snapshot.selected_subsystem_card.has_value());

    REQUIRE(snapshot.report_io.save.visible);
    REQUIRE_FALSE(snapshot.report_io.save.enabled);
    REQUIRE(snapshot.report_io.load.visible);
    REQUIRE(snapshot.report_io.load.enabled);

    REQUIRE_FALSE(snapshot.bound_runtime_actions.has_bound_project_data);
    REQUIRE(snapshot.bound_runtime_actions.rerun_migration.visible);
    REQUIRE_FALSE(snapshot.bound_runtime_actions.rerun_migration.enabled);
    REQUIRE(snapshot.bound_runtime_actions.rerun_selected_subsystem.visible);
    REQUIRE_FALSE(snapshot.bound_runtime_actions.rerun_selected_subsystem.enabled);
}

TEST_CASE("MigrationWizardModel: subsystem selection follows current results",
          "[editor][diagnostics][wizard][selection]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);

    REQUIRE(model.selectSubsystemResult("message"));
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");
    REQUIRE(model.selectedSubsystemResult().has_value());
    REQUIRE(model.selectedSubsystemResult()->display_name == "Message");

    REQUIRE_FALSE(model.selectSubsystemResult("missing"));
    REQUIRE_FALSE(model.selectedSubsystemId().has_value());
    REQUIRE_FALSE(model.selectedSubsystemResult().has_value());

    REQUIRE(model.selectSubsystemResult("menu"));
    model.clear();
    REQUIRE_FALSE(model.selectedSubsystemId().has_value());
    REQUIRE_FALSE(model.selectedSubsystemResult().has_value());
}

TEST_CASE("MigrationWizardModel: selection can move to adjacent subsystem results",
          "[editor][diagnostics][wizard][selection][navigation]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}},
                                   {"troops", {{{"id", 1}, {"name", "Slime x2"}, {"members", {}}}}}};

    model.runFullMigration(project_data);

    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");

    REQUIRE(model.selectNextSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "menu");

    REQUIRE(model.selectNextSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "battle");

    REQUIRE_FALSE(model.selectNextSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "battle");

    REQUIRE(model.selectPreviousSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "menu");

    REQUIRE(model.selectPreviousSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");

    REQUIRE_FALSE(model.selectPreviousSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");
}

TEST_CASE("MigrationWizardPanel: render snapshot carries selected subsystem",
          "[editor][diagnostics][wizard][panel][selection]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    REQUIRE(panel.selectSubsystemResult("menu"));
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_display_name == "Menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_processed_count == 1);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_warning_count == 0);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_error_count == 0);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_completed);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_summary_line.find("Menu migration") != std::string::npos);
}

TEST_CASE("MigrationWizardPanel: render snapshot exposes selection navigation state",
          "[editor][diagnostics][wizard][panel][selection][navigation]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE_FALSE(panel.lastRenderSnapshot().can_select_previous_subsystem);
    REQUIRE(panel.lastRenderSnapshot().can_select_next_subsystem);

    REQUIRE(panel.selectNextSubsystemResult());
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().can_select_previous_subsystem);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_select_next_subsystem);
}

TEST_CASE("MigrationWizardPanel: rendered workflow body tracks selection and action state",
          "[editor][diagnostics][wizard][panel][workflow][actions]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.selectNextSubsystemResult());
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.selected_subsystem_card.has_value());
    REQUIRE(snapshot.selected_subsystem_card->subsystem_id == "menu");
    REQUIRE(snapshot.primary_actions.previous_subsystem.enabled);
    REQUIRE_FALSE(snapshot.primary_actions.next_subsystem.enabled);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(panel.rerunSelectedSubsystem(project_data));
    panel.render();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.selected_subsystem_card.has_value());
    REQUIRE(snapshot.selected_subsystem_card->subsystem_id == "menu");
    REQUIRE(snapshot.selected_subsystem_card->summary_line.find("2 command(s)") != std::string::npos);
    REQUIRE(snapshot.subsystem_cards.size() == 2);
    REQUIRE(snapshot.subsystem_cards[1].summary_line.find("2 command(s)") != std::string::npos);

    REQUIRE(panel.clearSelectedSubsystemResult());
    panel.render();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.selected_subsystem_card.has_value());
    REQUIRE(snapshot.selected_subsystem_card->subsystem_id == "message");
    REQUIRE_FALSE(snapshot.primary_actions.previous_subsystem.enabled);
    REQUIRE_FALSE(snapshot.primary_actions.next_subsystem.enabled);
}

TEST_CASE("MigrationWizardModel: rerunSubsystem updates existing result", "[editor][diagnostics][wizard][rerun]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);
    REQUIRE(model.getReport().subsystem_results.size() == 2);
    REQUIRE(model.getReport().subsystem_results[1].subsystem_id == "menu");
    REQUIRE(model.getReport().subsystem_results[1].processed_count == 1);
    REQUIRE(model.getReport().total_files_processed == 2);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(model.rerunSubsystem("menu", project_data));
    auto report = model.getReport();
    REQUIRE(report.subsystem_results.size() == 2);
    REQUIRE(report.subsystem_results[1].processed_count == 1);
    REQUIRE(report.subsystem_results[1].summary_line.find("2 command(s)") != std::string::npos);
    REQUIRE(report.total_files_processed == 2);
    REQUIRE(report.summary_logs.size() == 3);
    REQUIRE(report.summary_logs[0].find("Message migration") != std::string::npos);
    REQUIRE(report.summary_logs[1].find("Menu migration") != std::string::npos);
    REQUIRE(report.summary_logs[2] == "Migration wizard complete.");
}

TEST_CASE("MigrationWizardModel: rerunSubsystem on missing subsystem adds it", "[editor][diagnostics][wizard][rerun]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);
    REQUIRE(model.getReport().subsystem_results.size() == 2);

    nlohmann::json battle_data = {{"troops", {{{"id", 1}, {"name", "Slime x2"}, {"members", {}}}}}};
    REQUIRE(model.rerunSubsystem("battle", battle_data));
    auto report = model.getReport();
    REQUIRE(report.subsystem_results.size() == 3);
    REQUIRE(report.subsystem_results[2].subsystem_id == "battle");
    REQUIRE(report.subsystem_results[2].processed_count == 1);
    REQUIRE(report.total_files_processed == 3);
    REQUIRE(report.is_complete);
    REQUIRE(report.summary_logs.size() == 4);
    REQUIRE(report.summary_logs[2].find("Battle migration") != std::string::npos);
    REQUIRE(report.summary_logs[3] == "Migration wizard complete.");
}

TEST_CASE("MigrationWizardModel: selected-subsystem actions operate on the current selection",
          "[editor][diagnostics][wizard][selected_actions]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");
    REQUIRE(model.selectSubsystemResult("menu"));

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(model.rerunSelectedSubsystem(project_data));
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "menu");
    REQUIRE(model.selectedSubsystemResult().has_value());
    REQUIRE(model.selectedSubsystemResult()->summary_line.find("2 command(s)") != std::string::npos);

    REQUIRE(model.clearSelectedSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");

    REQUIRE(model.clearSelectedSubsystemResult());
    REQUIRE_FALSE(model.selectedSubsystemId().has_value());
    REQUIRE_FALSE(model.clearSelectedSubsystemResult());
    REQUIRE_FALSE(model.rerunSelectedSubsystem(project_data));
}

TEST_CASE("MigrationWizardPanel: rerun action reflected in render snapshot",
          "[editor][diagnostics][wizard][panel][rerun]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().can_rerun_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results[0].processed_count == 1);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(panel.rerunSubsystem("menu", project_data));
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().subsystem_results[0].processed_count == 1);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results[0].summary_line.find("2 command(s)") != std::string::npos);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_selected_subsystem);
}

TEST_CASE("MigrationWizardPanel: selected-subsystem actions follow the current selection",
          "[editor][diagnostics][wizard][panel][selected_actions]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    REQUIRE(panel.selectSubsystemResult("menu"));
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().can_rerun_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().can_clear_selected_subsystem);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(panel.rerunSelectedSubsystem(project_data));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_summary_line.find("2 command(s)") != std::string::npos);

    REQUIRE(panel.clearSelectedSubsystemResult());
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "message");

    REQUIRE(panel.clearSelectedSubsystemResult());
    panel.render();
    REQUIRE_FALSE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_selected_subsystem);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_clear_selected_subsystem);
}

TEST_CASE("MigrationWizardPanel: bound runtime enables no-arg rerun actions",
          "[editor][diagnostics][wizard][panel][bound_runtime]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);

    REQUIRE(panel.clearSubsystemResult("menu"));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 1);

    REQUIRE(panel.rerunBoundProject());
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "message");

    REQUIRE(panel.selectSubsystemResult("menu"));
    REQUIRE(panel.rerunBoundSelectedSubsystem());
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "menu");
}

TEST_CASE("MigrationWizardModel: clearSubsystemResult removes result and updates counts",
          "[editor][diagnostics][wizard][clear]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}},
                                   {"troops", {{{"id", 1}, {"name", "Slime x2"}, {"members", {}}}}}};

    model.runFullMigration(project_data);
    auto report = model.getReport();
    REQUIRE(report.subsystem_results.size() == 3);
    REQUIRE(report.total_files_processed == 3);
    REQUIRE(model.selectedSubsystemId().has_value());

    REQUIRE(model.clearSubsystemResult("menu"));
    report = model.getReport();
    REQUIRE(report.subsystem_results.size() == 2);
    REQUIRE(report.total_files_processed == 2);
    REQUIRE(report.summary_logs.size() == 3);
    REQUIRE(report.summary_logs[0].find("Message migration") != std::string::npos);
    REQUIRE(report.summary_logs[1].find("Battle migration") != std::string::npos);
    REQUIRE(report.summary_logs[2] == "Migration wizard complete.");

    REQUIRE_FALSE(model.clearSubsystemResult("menu"));
}

TEST_CASE("MigrationWizardModel: clearSubsystemResult updates selection when selected is cleared",
          "[editor][diagnostics][wizard][clear][selection]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");

    REQUIRE(model.clearSubsystemResult("message"));
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "menu");

    REQUIRE(model.clearSubsystemResult("menu"));
    REQUIRE_FALSE(model.selectedSubsystemId().has_value());
}

TEST_CASE("MigrationWizardModel: clearing the last subsystem result resets the wizard to empty state",
          "[editor][diagnostics][wizard][clear][last_result]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);
    REQUIRE(model.getReport().total_files_processed == 1);
    REQUIRE(model.getReport().is_complete);
    REQUIRE(model.getReport().subsystem_results.size() == 1);

    REQUIRE(model.clearSubsystemResult("menu"));

    const auto& report = model.getReport();
    REQUIRE(report.total_files_processed == 0);
    REQUIRE(report.warning_count == 0);
    REQUIRE(report.error_count == 0);
    REQUIRE_FALSE(report.is_complete);
    REQUIRE(report.summary_logs.empty());
    REQUIRE(report.subsystem_results.empty());
    REQUIRE_FALSE(model.selectedSubsystemId().has_value());
}

TEST_CASE("MigrationWizardModel: getReportJson returns structured report", "[editor][diagnostics][wizard][export]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    model.runFullMigration(project_data);
    const auto json_str = model.getReportJson();
    const auto parsed = nlohmann::json::parse(json_str);

    REQUIRE(parsed["total_files_processed"] == 1);
    REQUIRE(parsed["warning_count"] == 0);
    REQUIRE(parsed["error_count"] == 0);
    REQUIRE(parsed["is_complete"] == true);
    REQUIRE(parsed["summary_logs"].is_array());
    REQUIRE(parsed["subsystem_results"].is_array());
    REQUIRE(parsed["subsystem_results"].size() == 1);
    REQUIRE(parsed["subsystem_results"][0]["subsystem_id"] == "menu");
    REQUIRE(parsed["subsystem_results"][0]["display_name"] == "Menu");
    REQUIRE(parsed["subsystem_results"][0]["completed"] == true);
    REQUIRE(parsed["subsystem_results"][0]["summary_line"].get<std::string>().find("Menu migration") !=
            std::string::npos);
}

TEST_CASE("MigrationWizardPanel: clear action reflected in render snapshot",
          "[editor][diagnostics][wizard][panel][clear]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 2);
    REQUIRE(panel.lastRenderSnapshot().can_clear_selected_subsystem);

    REQUIRE(panel.clearSubsystemResult("message"));
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 1);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results[0].subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().can_clear_selected_subsystem);
}

TEST_CASE("MigrationWizardPanel: clearing the last subsystem result yields an empty snapshot",
          "[editor][diagnostics][wizard][panel][clear][last_result]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().can_clear_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 1);

    REQUIRE(panel.clearSubsystemResult("menu"));
    panel.render();

    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().is_complete);
    REQUIRE(panel.lastRenderSnapshot().summary_logs.empty());
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.empty());
    REQUIRE_FALSE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_clear_selected_subsystem);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_save_report);
    REQUIRE(panel.lastRenderSnapshot().can_load_report);
}

TEST_CASE("MigrationWizardPanel: exportReportJson and snapshot export field",
          "[editor][diagnostics][wizard][panel][export]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};

    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    const auto direct_export = panel.exportReportJson();
    const auto snapshot_export = panel.lastRenderSnapshot().exported_report_json;
    REQUIRE_FALSE(direct_export.empty());
    REQUIRE(direct_export == snapshot_export);

    const auto parsed = nlohmann::json::parse(snapshot_export);
    REQUIRE(parsed["total_files_processed"] == 1);
    REQUIRE(parsed["subsystem_results"].size() == 1);
    REQUIRE(parsed["subsystem_results"][0]["subsystem_id"] == "menu");
}

TEST_CASE("MigrationWizardModel: saveReportToFile writes getReportJson to disk",
          "[editor][diagnostics][wizard][file]") {
    MigrationWizardModel model;
    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};
    model.runFullMigration(project_data);
    const auto original_json = model.getReportJson();

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_save_test.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(model.saveReportToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    std::string file_content;
    {
        std::ifstream ifs(temp_path);
        REQUIRE(ifs);
        file_content = std::string((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    }
    REQUIRE(file_content == original_json);

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardModel: loadReportFromFile restores report state", "[editor][diagnostics][wizard][file]") {
    MigrationWizardModel model;
    nlohmann::json project_data = {{"messages",
                                    {{"pages",
                                      {{{"id", "speaker_a"},
                                        {"route", "speaker"},
                                        {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                                        {"text", {"Hello there."}}}}}}},
                                   {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};
    model.runFullMigration(project_data);
    model.selectSubsystemResult("menu");

    const auto original_report = model.getReport();
    const auto original_selected = model.selectedSubsystemId();

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_load_test.json").string();
    std::filesystem::remove(temp_path);
    REQUIRE(model.saveReportToFile(temp_path));

    MigrationWizardModel loaded_model;
    REQUIRE(loaded_model.loadReportFromFile(temp_path));
    const auto loaded_report = loaded_model.getReport();
    REQUIRE(loaded_report.total_files_processed == original_report.total_files_processed);
    REQUIRE(loaded_report.warning_count == original_report.warning_count);
    REQUIRE(loaded_report.error_count == original_report.error_count);
    REQUIRE(loaded_report.is_complete == original_report.is_complete);
    REQUIRE(loaded_report.summary_logs.size() == original_report.summary_logs.size());
    REQUIRE(loaded_report.subsystem_results.size() == original_report.subsystem_results.size());
    for (size_t i = 0; i < loaded_report.subsystem_results.size(); ++i) {
        REQUIRE(loaded_report.subsystem_results[i].subsystem_id == original_report.subsystem_results[i].subsystem_id);
        REQUIRE(loaded_report.subsystem_results[i].display_name == original_report.subsystem_results[i].display_name);
        REQUIRE(loaded_report.subsystem_results[i].processed_count ==
                original_report.subsystem_results[i].processed_count);
        REQUIRE(loaded_report.subsystem_results[i].warning_count == original_report.subsystem_results[i].warning_count);
        REQUIRE(loaded_report.subsystem_results[i].error_count == original_report.subsystem_results[i].error_count);
        REQUIRE(loaded_report.subsystem_results[i].completed == original_report.subsystem_results[i].completed);
        REQUIRE(loaded_report.subsystem_results[i].summary_line == original_report.subsystem_results[i].summary_line);
    }
    REQUIRE(loaded_model.selectedSubsystemId() == original_selected);

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardModel: loadReportFromFile repairs orphaned selected subsystem ids",
          "[editor][diagnostics][wizard][file][selection]") {
    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_orphan_selection_test.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 1},
        {"warning_count", 0},
        {"error_count", 0},
        {"is_complete", true},
        {"summary_logs", {"Menu migration: 1 scene panel(s), 1 command(s).", "Migration wizard complete."}},
        {"subsystem_results",
         {{{"subsystem_id", "menu"},
           {"display_name", "Menu"},
           {"processed_count", 1},
           {"warning_count", 0},
           {"error_count", 0},
           {"completed", true},
           {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}}}},
        {"selected_subsystem_id", "missing"}};

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    MigrationWizardModel model;
    REQUIRE(model.loadReportFromFile(temp_path));
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "menu");
    REQUIRE(model.selectedSubsystemResult().has_value());
    REQUIRE(model.selectedSubsystemResult()->display_name == "Menu");

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardModel: loadReportFromFile with bad file returns false and clears model",
          "[editor][diagnostics][wizard][file]") {
    MigrationWizardModel model;
    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};
    model.runFullMigration(project_data);
    REQUIRE(model.getReport().total_files_processed > 0);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_bad_test.json").string();
    {
        std::ofstream ofs(temp_path);
        ofs << "this is not json";
    }

    REQUIRE_FALSE(model.loadReportFromFile(temp_path));
    REQUIRE(model.getReport().total_files_processed == 0);
    REQUIRE_FALSE(model.getReport().is_complete);
    REQUIRE(model.getReport().subsystem_results.empty());
    REQUIRE_FALSE(model.selectedSubsystemId().has_value());

    std::filesystem::remove(temp_path);

    const std::string nonexistent =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_does_not_exist.json").string();
    std::filesystem::remove(nonexistent);
    REQUIRE_FALSE(model.loadReportFromFile(nonexistent));
    REQUIRE(model.getReport().total_files_processed == 0);
}

TEST_CASE("MigrationWizardPanel: failed load clears previously bound runtime affordances",
          "[editor][diagnostics][wizard][panel][file][failure][bound_runtime]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);
    panel.onProjectUpdateRequested({{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}});
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_panel_bad_load_clears_binding.json").string();
    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << "not valid json";
    }

    REQUIRE_FALSE(panel.loadReportFromFile(temp_path));
    panel.render();

    REQUIRE_FALSE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().report_io.load.enabled);
    REQUIRE_FALSE(panel.lastRenderSnapshot().report_io.save.enabled);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.has_bound_project_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_migration.enabled);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_selected_subsystem.enabled);
    REQUIRE_FALSE(panel.rerunBoundProject());
    REQUIRE_FALSE(panel.rerunBoundSelectedSubsystem());
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.empty());

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardPanel: save/load report forwards to model and snapshot exposes flags",
          "[editor][diagnostics][wizard][panel][file]") {
    MigrationWizardPanel panel;
    nlohmann::json project_data = {{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}};
    panel.onProjectUpdateRequested(project_data);
    panel.setVisible(true);
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().can_save_report);
    REQUIRE(panel.lastRenderSnapshot().can_load_report);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_panel_test.json").string();
    std::filesystem::remove(temp_path);
    REQUIRE(panel.saveReportToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    panel.clear();
    panel.render();
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_save_report);
    REQUIRE(panel.lastRenderSnapshot().can_load_report);
    REQUIRE_FALSE(panel.lastRenderSnapshot().report_io.save.enabled);
    REQUIRE(panel.lastRenderSnapshot().report_io.load.enabled);

    REQUIRE(panel.loadReportFromFile(temp_path));
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().has_data);
    REQUIRE(panel.lastRenderSnapshot().can_save_report);
    REQUIRE(panel.lastRenderSnapshot().report_io.save.enabled);
    REQUIRE(panel.lastRenderSnapshot().report_io.load.enabled);
    REQUIRE(panel.lastRenderSnapshot().subsystem_results.size() == 1);

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardPanel: saving a report preserves bound runtime affordances",
          "[editor][diagnostics][wizard][panel][file][save_binding]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);
    panel.onProjectUpdateRequested({{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}});
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_panel_save_preserves_binding.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(panel.saveReportToFile(temp_path));
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().bound_runtime_actions.has_bound_project_data);
    REQUIRE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_migration.enabled);
    REQUIRE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_selected_subsystem.enabled);
    REQUIRE(panel.rerunBoundProject());

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardPanel: load report repairs orphaned selected subsystem ids in snapshot",
          "[editor][diagnostics][wizard][panel][file][selection]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_panel_orphan_selection_test.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 1},
        {"warning_count", 0},
        {"error_count", 0},
        {"is_complete", true},
        {"summary_logs", {"Menu migration: 1 scene panel(s), 1 command(s).", "Migration wizard complete."}},
        {"subsystem_results",
         {{{"subsystem_id", "menu"},
           {"display_name", "Menu"},
           {"processed_count", 1},
           {"warning_count", 0},
           {"error_count", 0},
           {"completed", true},
           {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}}}},
        {"selected_subsystem_id", "missing"}};

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(panel.loadReportFromFile(temp_path));
    panel.render();

    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_id.has_value());
    REQUIRE(*panel.lastRenderSnapshot().selected_subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_display_name == "Menu");
    REQUIRE(panel.lastRenderSnapshot().can_rerun_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().can_clear_selected_subsystem);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_card.has_value());
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_card->subsystem_id == "menu");
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_card->can_rerun);
    REQUIRE(panel.lastRenderSnapshot().selected_subsystem_card->can_clear);
    REQUIRE_FALSE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.has_bound_project_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_migration.enabled);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_selected_subsystem.enabled);

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardPanel: loading a saved report renders workflow cards without bound runtime",
          "[editor][diagnostics][wizard][panel][file][workflow]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_panel_loaded_workflow.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {{"total_files_processed", 2},
                                  {"warning_count", 1},
                                  {"error_count", 0},
                                  {"is_complete", true},
                                  {"summary_logs",
                                   {"Message migration: 1 dialogue sequence(s), 1 diagnostic(s).",
                                    "Menu migration: 1 scene panel(s), 1 command(s).", "Migration wizard complete."}},
                                  {"subsystem_results",
                                   {{{"subsystem_id", "message"},
                                     {"display_name", "Message"},
                                     {"processed_count", 1},
                                     {"warning_count", 1},
                                     {"error_count", 0},
                                     {"completed", true},
                                     {"summary_line", "Message migration: 1 dialogue sequence(s), 1 diagnostic(s)."}},
                                    {{"subsystem_id", "menu"},
                                     {"display_name", "Menu"},
                                     {"processed_count", 1},
                                     {"warning_count", 0},
                                     {"error_count", 0},
                                     {"completed", true},
                                     {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}}}},
                                  {"selected_subsystem_id", "menu"}};

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(panel.loadReportFromFile(temp_path));
    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.has_data);
    REQUIRE(snapshot.subsystem_cards.size() == 2);
    REQUIRE(snapshot.subsystem_cards[0].subsystem_id == "message");
    REQUIRE_FALSE(snapshot.subsystem_cards[0].is_selected);
    REQUIRE(snapshot.subsystem_cards[1].subsystem_id == "menu");
    REQUIRE(snapshot.subsystem_cards[1].is_selected);
    REQUIRE(snapshot.selected_subsystem_card.has_value());
    REQUIRE(snapshot.selected_subsystem_card->subsystem_id == "menu");
    REQUIRE(snapshot.selected_subsystem_card->summary_line.find("Menu migration") != std::string::npos);
    REQUIRE(snapshot.report_io.save.enabled);
    REQUIRE(snapshot.report_io.load.enabled);
    REQUIRE_FALSE(snapshot.bound_runtime_actions.has_bound_project_data);
    REQUIRE_FALSE(snapshot.bound_runtime_actions.rerun_migration.enabled);
    REQUIRE_FALSE(snapshot.bound_runtime_actions.rerun_selected_subsystem.enabled);

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardPanel: loading a report clears any previously bound runtime",
          "[editor][diagnostics][wizard][panel][file][bound_runtime]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);

    panel.onProjectUpdateRequested({{"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}});
    panel.render();
    REQUIRE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE(panel.lastRenderSnapshot().can_rerun_bound_migration);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_panel_load_clears_binding.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 1},
        {"warning_count", 0},
        {"error_count", 0},
        {"is_complete", true},
        {"summary_logs", {"Menu migration: 1 scene panel(s), 1 command(s).", "Migration wizard complete."}},
        {"subsystem_results",
         {{{"subsystem_id", "menu"},
           {"display_name", "Menu"},
           {"processed_count", 1},
           {"warning_count", 0},
           {"error_count", 0},
           {"completed", true},
           {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}}}},
        {"selected_subsystem_id", "menu"}};

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(panel.loadReportFromFile(temp_path));
    panel.render();

    REQUIRE_FALSE(panel.lastRenderSnapshot().has_bound_project_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_bound_migration);
    REQUIRE_FALSE(panel.lastRenderSnapshot().can_rerun_bound_selected_subsystem);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.has_bound_project_data);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_migration.enabled);
    REQUIRE_FALSE(panel.lastRenderSnapshot().bound_runtime_actions.rerun_selected_subsystem.enabled);
    REQUIRE_FALSE(panel.rerunBoundProject());
    REQUIRE_FALSE(panel.rerunBoundSelectedSubsystem());

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardModel: issue navigation skips clean subsystem results",
          "[editor][diagnostics][wizard][selection][issues]") {
    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_issue_navigation_model.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {{"total_files_processed", 3},
                                  {"warning_count", 2},
                                  {"error_count", 1},
                                  {"is_complete", true},
                                  {"summary_logs",
                                   {"Message migration: 1 dialogue sequence(s), 2 diagnostic(s).",
                                    "Menu migration: 1 scene panel(s), 1 command(s).",
                                    "Battle migration: 1 troop(s), 3 action(s).", "Migration wizard complete."}},
                                  {"subsystem_results",
                                   {{{"subsystem_id", "message"},
                                     {"display_name", "Message"},
                                     {"processed_count", 1},
                                     {"warning_count", 2},
                                     {"error_count", 0},
                                     {"completed", true},
                                     {"summary_line", "Message migration: 1 dialogue sequence(s), 2 diagnostic(s)."}},
                                    {{"subsystem_id", "menu"},
                                     {"display_name", "Menu"},
                                     {"processed_count", 1},
                                     {"warning_count", 0},
                                     {"error_count", 0},
                                     {"completed", true},
                                     {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}},
                                    {{"subsystem_id", "battle"},
                                     {"display_name", "Battle"},
                                     {"processed_count", 1},
                                     {"warning_count", 0},
                                     {"error_count", 1},
                                     {"completed", true},
                                     {"summary_line", "Battle migration: 1 troop(s), 3 action(s)."}}}},
                                  {"selected_subsystem_id", "message"}};

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    MigrationWizardModel model;
    REQUIRE(model.loadReportFromFile(temp_path));
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");
    REQUIRE(model.canSelectNextIssueSubsystemResult());
    REQUIRE_FALSE(model.canSelectPreviousIssueSubsystemResult());

    REQUIRE(model.selectNextIssueSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "battle");
    REQUIRE_FALSE(model.canSelectNextIssueSubsystemResult());
    REQUIRE(model.canSelectPreviousIssueSubsystemResult());

    REQUIRE(model.selectPreviousIssueSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");
    REQUIRE_FALSE(model.selectPreviousIssueSubsystemResult());

    REQUIRE(model.selectSubsystemResult("menu"));
    REQUIRE(model.canSelectNextIssueSubsystemResult());
    REQUIRE(model.canSelectPreviousIssueSubsystemResult());
    REQUIRE(model.selectNextIssueSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "battle");
    REQUIRE(model.selectPreviousIssueSubsystemResult());
    REQUIRE(model.selectedSubsystemId().has_value());
    REQUIRE(*model.selectedSubsystemId() == "message");

    std::filesystem::remove(temp_path);
}

TEST_CASE("MigrationWizardPanel: render snapshot exposes issue-focused navigation actions",
          "[editor][diagnostics][wizard][panel][workflow][issues]") {
    MigrationWizardPanel panel;
    panel.setVisible(true);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_migration_wizard_issue_navigation_panel.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {{"total_files_processed", 3},
                                  {"warning_count", 1},
                                  {"error_count", 1},
                                  {"is_complete", true},
                                  {"summary_logs",
                                   {"Message migration: 1 dialogue sequence(s), 1 diagnostic(s).",
                                    "Menu migration: 1 scene panel(s), 1 command(s).",
                                    "Battle migration: 1 troop(s), 3 action(s).", "Migration wizard complete."}},
                                  {"subsystem_results",
                                   {{{"subsystem_id", "message"},
                                     {"display_name", "Message"},
                                     {"processed_count", 1},
                                     {"warning_count", 1},
                                     {"error_count", 0},
                                     {"completed", true},
                                     {"summary_line", "Message migration: 1 dialogue sequence(s), 1 diagnostic(s)."}},
                                    {{"subsystem_id", "menu"},
                                     {"display_name", "Menu"},
                                     {"processed_count", 1},
                                     {"warning_count", 0},
                                     {"error_count", 0},
                                     {"completed", true},
                                     {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}},
                                    {{"subsystem_id", "battle"},
                                     {"display_name", "Battle"},
                                     {"processed_count", 1},
                                     {"warning_count", 0},
                                     {"error_count", 1},
                                     {"completed", true},
                                     {"summary_line", "Battle migration: 1 troop(s), 3 action(s)."}}}},
                                  {"selected_subsystem_id", "message"}};

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(panel.loadReportFromFile(temp_path));
    panel.render();

    auto snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.can_select_next_issue_subsystem);
    REQUIRE_FALSE(snapshot.can_select_previous_issue_subsystem);
    REQUIRE(snapshot.primary_actions.next_issue_subsystem.visible);
    REQUIRE(snapshot.primary_actions.next_issue_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.previous_issue_subsystem.visible);
    REQUIRE_FALSE(snapshot.primary_actions.previous_issue_subsystem.enabled);

    REQUIRE(panel.selectNextIssueSubsystemResult());
    panel.render();

    snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.selected_subsystem_id.has_value());
    REQUIRE(*snapshot.selected_subsystem_id == "battle");
    REQUIRE_FALSE(snapshot.can_select_next_issue_subsystem);
    REQUIRE(snapshot.can_select_previous_issue_subsystem);
    REQUIRE_FALSE(snapshot.primary_actions.next_issue_subsystem.enabled);
    REQUIRE(snapshot.primary_actions.previous_issue_subsystem.enabled);

    std::filesystem::remove(temp_path);
}
