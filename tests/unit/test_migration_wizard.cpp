#include <catch2/catch_test_macros.hpp>
#include "editor/diagnostics/migration_wizard_panel.h"
#include "editor/diagnostics/migration_wizard_model.h"
#include <nlohmann/json.hpp>

using namespace urpg::editor;

TEST_CASE("MigrationWizardModel: Batch Orchestration", "[editor][diagnostics][wizard]") {
    MigrationWizardModel model;
    
    nlohmann::json project_data = {
        {"messages", {
            {"pages", {
                {
                    {"id", "speaker_a"},
                    {"route", "speaker"},
                    {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                    {"text", {"Hello there."}}
                }
            }}
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }},
        {"troops", {
            {{"id", 1}, {"name", "Slime x2"}, {"members", {}}}
        }}
    };

    SECTION("Initial state") {
        auto report = model.getReport();
        REQUIRE(report.total_files_processed == 0);
        REQUIRE_FALSE(report.is_complete);
    }

    SECTION("Run full migration collects counts from subsystems") {
        model.runFullMigration(project_data);
        auto report = model.getReport();
        
        REQUIRE(report.is_complete);
        REQUIRE(report.total_files_processed == 3); // messages, scenes/menus, troops
        REQUIRE(report.subsystem_results.size() == 3);
        REQUIRE(report.subsystem_results[0].subsystem_id == "message");
        REQUIRE(report.subsystem_results[0].processed_count == 1);
        REQUIRE(report.subsystem_results[0].summary_line.find("Message migration") != std::string::npos);
        REQUIRE(report.subsystem_results[1].subsystem_id == "menu");
        REQUIRE(report.subsystem_results[1].processed_count == 1);
        REQUIRE(report.subsystem_results[1].summary_line.find("Menu migration") != std::string::npos);
        REQUIRE(report.subsystem_results[2].subsystem_id == "battle");
        REQUIRE(report.subsystem_results[2].processed_count == 1);
        REQUIRE(report.subsystem_results[2].summary_line.find("Battle migration") != std::string::npos);
        REQUIRE(report.summary_logs.size() == 4);
        REQUIRE(report.summary_logs[0].find("Message migration") != std::string::npos);
        REQUIRE(report.summary_logs[1].find("Menu migration") != std::string::npos);
        REQUIRE(report.summary_logs[2].find("Battle migration") != std::string::npos);
        REQUIRE(report.summary_logs[3] == "Migration wizard complete.");
        REQUIRE(model.selectedSubsystemId().has_value());
        REQUIRE(*model.selectedSubsystemId() == "message");
    }
}

TEST_CASE("MigrationWizardPanel: Visible render records migration snapshot", "[editor][diagnostics][wizard][panel]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

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

TEST_CASE("MigrationWizardPanel: Clear resets report and rendered snapshot", "[editor][diagnostics][wizard][panel][clear]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

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

TEST_CASE("MigrationWizardModel: subsystem selection follows current results", "[editor][diagnostics][wizard][selection]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {
        {"messages", {
            {"pages", {
                {
                    {"id", "speaker_a"},
                    {"route", "speaker"},
                    {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                    {"text", {"Hello there."}}
                }
            }}
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

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

TEST_CASE("MigrationWizardPanel: render snapshot carries selected subsystem", "[editor][diagnostics][wizard][panel][selection]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {
        {"messages", {
            {"pages", {
                {
                    {"id", "speaker_a"},
                    {"route", "speaker"},
                    {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                    {"text", {"Hello there."}}
                }
            }}
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

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

TEST_CASE("MigrationWizardModel: rerunSubsystem updates existing result", "[editor][diagnostics][wizard][rerun]") {
    MigrationWizardModel model;

    nlohmann::json project_data = {
        {"messages", {
            {"pages", {
                {
                    {"id", "speaker_a"},
                    {"route", "speaker"},
                    {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                    {"text", {"Hello there."}}
                }
            }}
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

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

    nlohmann::json project_data = {
        {"messages", {
            {"pages", {
                {
                    {"id", "speaker_a"},
                    {"route", "speaker"},
                    {"speaker", {{"actor_id", 1}, {"name", "Alyx"}}},
                    {"text", {"Hello there."}}
                }
            }}
        }},
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

    model.runFullMigration(project_data);
    REQUIRE(model.getReport().subsystem_results.size() == 2);

    nlohmann::json battle_data = {
        {"troops", {
            {{"id", 1}, {"name", "Slime x2"}, {"members", {}}}
        }}
    };
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

TEST_CASE("MigrationWizardPanel: rerun action reflected in render snapshot", "[editor][diagnostics][wizard][panel][rerun]") {
    MigrationWizardPanel panel;

    nlohmann::json project_data = {
        {"scenes", {
            {{"symbol", "item"}, {"name", "Items"}}
        }}
    };

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
