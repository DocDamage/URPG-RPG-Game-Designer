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

TEST_CASE("DiagnosticsWorkspace - Migration wizard state clears cleanly",
          "[editor][diagnostics][integration][wizard_clear]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.migrationWizardPanel().onProjectUpdateRequested({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });

    const auto populatedSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::MigrationWizard);
    REQUIRE(populatedSummary.item_count == 1);
    REQUIRE(populatedSummary.has_data);

    workspace.clearMigrationWizardRuntime();

    const auto clearedSummary = workspace.tabSummary(urpg::editor::DiagnosticsTab::MigrationWizard);
    REQUIRE(clearedSummary.item_count == 0);
    REQUIRE_FALSE(clearedSummary.has_data);
    REQUIRE(workspace.migrationWizardPanel().getModel()->getReport().summary_logs.empty());
    REQUIRE(workspace.migrationWizardPanel().getModel()->getReport().subsystem_results.empty());
    REQUIRE_FALSE(workspace.migrationWizardPanel().getModel()->getReport().is_complete);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard export carries selected subsystem detail",
          "[editor][diagnostics][integration][wizard_export]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.bindMigrationWizardRuntime({
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });

    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "migration_wizard");
    REQUIRE(exported["active_tab_detail"]["tab"] == "migration_wizard");
    REQUIRE(exported["active_tab_detail"]["summary"]["item_count"] == 2);
    REQUIRE(exported["active_tab_detail"].contains("total_files_processed"));
    REQUIRE(exported["active_tab_detail"].contains("warning_count"));
    REQUIRE(exported["active_tab_detail"].contains("error_count"));
    REQUIRE(exported["active_tab_detail"]["total_files_processed"] == 2);
    REQUIRE(exported["active_tab_detail"]["warning_count"] == 2);
    REQUIRE(exported["active_tab_detail"]["error_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["summary_logs"].is_array());
    REQUIRE(exported["active_tab_detail"]["summary_logs"].size() == 3);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].is_array());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_display_name"] == "Message");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_processed_count"] == 1);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_completed"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("Message migration") != std::string::npos);
    REQUIRE(exported["active_tab_detail"]["can_rerun_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_clear_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_next_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["exported_report_json"].is_string());
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard export carries aggregate counts from loaded reports",
          "[editor][diagnostics][integration][wizard_export_counts]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto temp_path =
        (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_export_counts.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 2},
        {"warning_count", 3},
        {"error_count", 1},
        {"is_complete", true},
        {"summary_logs", {
            "Message migration: 1 dialogue sequence(s), 2 diagnostic(s).",
            "Menu migration: 1 scene panel(s), 1 command(s).",
            "Migration wizard complete."
        }},
        {"subsystem_results", {
            {
                {"subsystem_id", "message"},
                {"display_name", "Message"},
                {"processed_count", 1},
                {"warning_count", 2},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Message migration: 1 dialogue sequence(s), 2 diagnostic(s)."}
            },
            {
                {"subsystem_id", "menu"},
                {"display_name", "Menu"},
                {"processed_count", 1},
                {"warning_count", 1},
                {"error_count", 1},
                {"completed", true},
                {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}
            }
        }},
        {"selected_subsystem_id", "message"}
    };

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"].contains("total_files_processed"));
    REQUIRE(exported["active_tab_detail"].contains("warning_count"));
    REQUIRE(exported["active_tab_detail"].contains("error_count"));
    REQUIRE(exported["active_tab_detail"]["total_files_processed"] == 2);
    REQUIRE(exported["active_tab_detail"]["warning_count"] == 3);
    REQUIRE(exported["active_tab_detail"]["error_count"] == 1);

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard workflow actions are exposed at workspace level",
          "[editor][diagnostics][integration][wizard_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["can_select_next_subsystem"] == true);

    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
    workspace.render();

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["can_select_previous_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_next_subsystem"] == false);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(workspace.rerunMigrationWizardSubsystem("menu", project_data));
    workspace.render();

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);

    const auto exported_report_json = workspace.exportMigrationWizardReportJson();
    REQUIRE_FALSE(exported_report_json.empty());
    const auto parsed_report = nlohmann::json::parse(exported_report_json);
    REQUIRE(parsed_report["selected_subsystem_id"] == "menu");

    REQUIRE(workspace.clearMigrationWizardSubsystemResult("menu"));
    workspace.render();

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 1);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard export carries a rendered workflow body",
          "[editor][diagnostics][integration][wizard_rendered_workflow]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["workflow_sections"].is_array());
    REQUIRE(exported["active_tab_detail"]["workflow_sections"].size() >= 5);
    REQUIRE(exported["active_tab_detail"]["workflow_sections"][0] == "overview");
    REQUIRE(exported["active_tab_detail"]["workflow_sections"][1] == "actions");
    REQUIRE(exported["active_tab_detail"]["workflow_sections"][2] == "subsystems");
    REQUIRE(exported["active_tab_detail"]["workflow_sections"][3] == "report_io");
    REQUIRE(exported["active_tab_detail"]["workflow_sections"][4] == "bound_runtime");

    REQUIRE(exported["active_tab_detail"]["primary_actions"]["run_migration"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["rerun_selected_subsystem"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["clear_selected_subsystem"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_subsystem"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_subsystem"]["enabled"] == false);

    REQUIRE(exported["active_tab_detail"]["subsystem_cards"].is_array());
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][0]["subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][0]["is_selected"] == true);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][1]["subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][1]["is_selected"] == false);

    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["can_rerun"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["can_clear"] == true);

    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["report_io"]["load"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["report_io"]["exported_report_json"].get<std::string>().empty() == false);

    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == true);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard empty export still carries rendered workflow shell",
          "[editor][diagnostics][integration][wizard_rendered_workflow_empty]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "migration_wizard");
    REQUIRE(exported["active_tab_detail"]["workflow_sections"].is_array());
    REQUIRE(exported["active_tab_detail"]["workflow_sections"].size() >= 5);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["run_migration"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["rerun_selected_subsystem"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["clear_selected_subsystem"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"].empty());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"].is_null());
    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["report_io"]["load"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == false);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard selected-subsystem actions are exposed at workspace level",
          "[editor][diagnostics][integration][wizard_selected_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["can_rerun_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_clear_selected_subsystem"] == true);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(workspace.rerunSelectedMigrationWizardSubsystem(project_data));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);

    REQUIRE(workspace.clearSelectedMigrationWizardSubsystemResult());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");

    REQUIRE(workspace.clearSelectedMigrationWizardSubsystemResult());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["can_rerun_selected_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["can_clear_selected_subsystem"] == false);

    REQUIRE_FALSE(workspace.clearSelectedMigrationWizardSubsystemResult());
    REQUIRE_FALSE(workspace.rerunSelectedMigrationWizardSubsystem(project_data));
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard bound-runtime rerun actions are exposed at workspace level",
          "[editor][diagnostics][integration][wizard_bound_runtime]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == true);

    REQUIRE(workspace.clearMigrationWizardSubsystemResult("menu"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 1);

    REQUIRE(workspace.rerunBoundMigrationWizard());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");

    REQUIRE(workspace.selectMigrationWizardSubsystemResult("menu"));
    REQUIRE(workspace.rerunBoundSelectedMigrationWizardSubsystem());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");

    workspace.clearMigrationWizardRuntime();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == false);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_selected_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == false);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard actions keep exported snapshot current without manual render",
          "[editor][diagnostics][integration][wizard_snapshot]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(workspace.rerunMigrationWizardSubsystem("menu", project_data));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);

    REQUIRE(workspace.clearMigrationWizardSubsystemResult("menu"));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");

    workspace.clearMigrationWizardRuntime();
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard rendered workflow updates across workspace actions",
          "[editor][diagnostics][integration][wizard_rendered_workflow_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_subsystem"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_subsystem"]["enabled"] == false);

    project_data["scenes"].push_back({{"symbol", "equip"}, {"name", "Equip"}});
    REQUIRE(workspace.rerunSelectedMigrationWizardSubsystem(project_data));
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][1]["summary_line"].get<std::string>().find("2 command(s)") != std::string::npos);

    REQUIRE(workspace.clearSelectedMigrationWizardSubsystemResult());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_subsystem"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_subsystem"]["enabled"] == false);
}

TEST_CASE("DiagnosticsWorkspace - Clearing the last migration wizard subsystem clears exported active-tab detail",
          "[editor][diagnostics][integration][wizard_clear_last]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.bindMigrationWizardRuntime({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["is_complete"] == true);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");

    REQUIRE(workspace.clearMigrationWizardSubsystemResult("menu"));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["is_complete"] == false);
    REQUIRE(exported["active_tab_detail"]["summary_logs"].empty());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["can_clear_selected_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["can_save_report"] == false);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard failed load clears exported snapshot state",
          "[editor][diagnostics][integration][wizard_file_failure]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    workspace.bindMigrationWizardRuntime({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 1);

    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_bad_load.json").string();
    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << "not valid json";
    }

    REQUIRE_FALSE(workspace.loadMigrationWizardReportFromFile(temp_path));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["summary_logs"].empty());
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"].is_null());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == false);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_selected_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["report_io"]["load"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == false);
    REQUIRE_FALSE(workspace.rerunBoundMigrationWizard());
    REQUIRE_FALSE(workspace.rerunBoundSelectedMigrationWizardSubsystem());

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard save/load round-trip preserves exported workflow state",
          "[editor][diagnostics][integration][wizard_file_roundtrip]") {
    urpg::editor::DiagnosticsWorkspace workspace;

    nlohmann::json project_data = {
        {"messages", {
            {
                {"id", "page_1"},
                {"speaker", "Guide"},
                {"text", "Welcome to URPG."}
            }
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    workspace.bindMigrationWizardRuntime(project_data);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();
    REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());

    const auto original_report_json = workspace.exportMigrationWizardReportJson();
    const auto temp_path = (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_roundtrip.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(workspace.saveMigrationWizardReportToFile(temp_path));
    REQUIRE(std::filesystem::exists(temp_path));

    workspace.clearMigrationWizardRuntime();
    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == false);
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].empty());
    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["report_io"]["load"]["enabled"] == true);

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["subsystem_results"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["can_save_report"] == true);
    REQUIRE(exported["active_tab_detail"]["can_load_report"] == true);
    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["report_io"]["load"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "menu");

    const auto reloaded_report_json = workspace.exportMigrationWizardReportJson();
    REQUIRE(reloaded_report_json == original_report_json);

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard loaded report exports rendered cards without bound runtime",
          "[editor][diagnostics][integration][wizard_rendered_workflow_loaded_report]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto temp_path =
        (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_loaded_workflow.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 2},
        {"warning_count", 1},
        {"error_count", 0},
        {"is_complete", true},
        {"summary_logs", {
            "Message migration: 1 dialogue sequence(s), 1 diagnostic(s).",
            "Menu migration: 1 scene panel(s), 1 command(s).",
            "Migration wizard complete."
        }},
        {"subsystem_results", {
            {
                {"subsystem_id", "message"},
                {"display_name", "Message"},
                {"processed_count", 1},
                {"warning_count", 1},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Message migration: 1 dialogue sequence(s), 1 diagnostic(s)."}
            },
            {
                {"subsystem_id", "menu"},
                {"display_name", "Menu"},
                {"processed_count", 1},
                {"warning_count", 0},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}
            }
        }},
        {"selected_subsystem_id", "menu"}
    };

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_data"] == true);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][0]["subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][0]["is_selected"] == false);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][1]["subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"][1]["is_selected"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["report_io"]["load"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == false);

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard load repairs orphaned selected subsystem ids in exported detail",
          "[editor][diagnostics][integration][wizard_file_selection_repair]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto temp_path =
        (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_orphan_selection.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 1},
        {"warning_count", 0},
        {"error_count", 0},
        {"is_complete", true},
        {"summary_logs", {"Menu migration: 1 scene panel(s), 1 command(s).", "Migration wizard complete."}},
        {"subsystem_results", {
            {
                {"subsystem_id", "menu"},
                {"display_name", "Menu"},
                {"processed_count", 1},
                {"warning_count", 0},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}
            }
        }},
        {"selected_subsystem_id", "missing"}
    };

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_display_name"] == "Menu");
    REQUIRE(exported["active_tab_detail"]["can_rerun_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_clear_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "menu");
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["can_rerun"] == true);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["can_clear"] == true);

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard file load clears previously bound runtime affordances",
          "[editor][diagnostics][integration][wizard_file_bound_runtime]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);

    workspace.bindMigrationWizardRuntime({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == true);

    const auto temp_path =
        (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_load_clears_binding.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 1},
        {"warning_count", 0},
        {"error_count", 0},
        {"is_complete", true},
        {"summary_logs", {"Menu migration: 1 scene panel(s), 1 command(s).", "Migration wizard complete."}},
        {"subsystem_results", {
            {
                {"subsystem_id", "menu"},
                {"display_name", "Menu"},
                {"processed_count", 1},
                {"warning_count", 0},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}
            }
        }},
        {"selected_subsystem_id", "menu"}
    };

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == false);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_selected_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == false);
    REQUIRE_FALSE(workspace.rerunBoundMigrationWizard());
    REQUIRE_FALSE(workspace.rerunBoundSelectedMigrationWizardSubsystem());

    std::filesystem::remove(temp_path);
}

TEST_CASE("DiagnosticsWorkspace - Migration wizard save preserves bound runtime affordances",
          "[editor][diagnostics][integration][wizard_file_save_binding]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.bindMigrationWizardRuntime({
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    });
    workspace.render();

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_selected_subsystem"] == true);

    const auto temp_path =
        (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_save_preserves_binding.json").string();
    std::filesystem::remove(temp_path);

    REQUIRE(workspace.saveMigrationWizardReportToFile(temp_path));

    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_migration"] == true);
    REQUIRE(exported["active_tab_detail"]["can_rerun_bound_selected_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["has_bound_project_data"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_migration"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == true);
    REQUIRE(workspace.rerunBoundMigrationWizard());

    std::filesystem::remove(temp_path);
}


TEST_CASE("DiagnosticsWorkspace - Migration wizard issue-navigation actions are exposed at workspace level",
          "[editor][diagnostics][integration][migration_wizard_issue_actions]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.setVisible(true);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);

    const std::string temp_path =
        (std::filesystem::temp_directory_path() / "urpg_workspace_migration_wizard_issue_navigation.json").string();
    std::filesystem::remove(temp_path);

    nlohmann::json report_json = {
        {"total_files_processed", 3},
        {"warning_count", 1},
        {"error_count", 1},
        {"is_complete", true},
        {"summary_logs", {
            "Message migration: 1 dialogue sequence(s), 1 diagnostic(s).",
            "Menu migration: 1 scene panel(s), 1 command(s).",
            "Battle migration: 1 troop(s), 3 action(s).",
            "Migration wizard complete."
        }},
        {"subsystem_results", {
            {
                {"subsystem_id", "message"},
                {"display_name", "Message"},
                {"processed_count", 1},
                {"warning_count", 1},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Message migration: 1 dialogue sequence(s), 1 diagnostic(s)."}
            },
            {
                {"subsystem_id", "menu"},
                {"display_name", "Menu"},
                {"processed_count", 1},
                {"warning_count", 0},
                {"error_count", 0},
                {"completed", true},
                {"summary_line", "Menu migration: 1 scene panel(s), 1 command(s)."}
            },
            {
                {"subsystem_id", "battle"},
                {"display_name", "Battle"},
                {"processed_count", 1},
                {"warning_count", 0},
                {"error_count", 1},
                {"completed", true},
                {"summary_line", "Battle migration: 1 troop(s), 3 action(s)."}
            }
        }},
        {"selected_subsystem_id", "message"}
    };

    {
        std::ofstream ofs(temp_path);
        REQUIRE(ofs);
        ofs << report_json.dump();
    }

    REQUIRE(workspace.loadMigrationWizardReportFromFile(temp_path));

    auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["can_select_next_issue_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_issue_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_issue_subsystem"]["visible"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_issue_subsystem"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_issue_subsystem"]["visible"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_issue_subsystem"]["enabled"] == false);

    REQUIRE(workspace.selectNextMigrationWizardIssueSubsystemResult());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "battle");
    REQUIRE(exported["active_tab_detail"]["can_select_next_issue_subsystem"] == false);
    REQUIRE(exported["active_tab_detail"]["can_select_previous_issue_subsystem"] == true);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_issue_subsystem"]["enabled"] == false);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_issue_subsystem"]["enabled"] == true);

    REQUIRE(workspace.selectPreviousMigrationWizardIssueSubsystemResult());
    exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_id"] == "message");

    std::filesystem::remove(temp_path);
}


