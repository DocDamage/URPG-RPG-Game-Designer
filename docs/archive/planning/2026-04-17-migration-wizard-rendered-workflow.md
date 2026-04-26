# Migration Wizard Rendered Workflow Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the migration wizard panel's snapshot-only scaffold with a rendered workflow body that exposes overview, subsystem list/detail, action affordances, report I/O state, and bound-runtime rerun controls through the existing diagnostics workspace actions.

**Architecture:** Keep migration-specific rendering logic inside `editor/diagnostics/migration_wizard_panel.h` by expanding its render snapshot into explicit workflow sections. Reuse the existing workspace action methods in `editor/diagnostics/diagnostics_workspace.h` and `editor/diagnostics/diagnostics_workspace.cpp`, and extend exports/tests to assert workflow rendering rather than only raw counters.

**Tech Stack:** C++17, Catch2, nlohmann::json, existing editor diagnostics panel patterns

---

### Task 1: Add failing rendered-workflow tests for the panel

**Files:**
- Modify: `C:/dev/URPG Maker/tests/unit/test_migration_wizard.cpp`
- Test: `C:/dev/URPG Maker/tests/unit/test_migration_wizard.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST_CASE("MigrationWizardPanel: render snapshot exposes structured workflow sections",
          "[editor][diagnostics][wizard][panel][workflow]") {
    MigrationWizardPanel panel;
    panel.onProjectUpdateRequested({
        {"messages", {{{"id", "page_1"}, {"speaker", "Guide"}, {"text", "Welcome to URPG."}}}},
        {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}},
        {"troops", {{{"id", 1}, {"name", "Slime x2"}, {"members", nlohmann::json::array()}}}}
    });
    panel.setVisible(true);

    panel.render();

    const auto& snapshot = panel.lastRenderSnapshot();
    REQUIRE(snapshot.workflow_sections.size() >= 4);
    REQUIRE(snapshot.primary_actions.run_migration.visible == true);
    REQUIRE(snapshot.primary_actions.run_migration.enabled == true);
    REQUIRE(snapshot.primary_actions.next_subsystem.enabled == true);
    REQUIRE(snapshot.selected_subsystem_card.has_value());
    REQUIRE(snapshot.subsystem_cards.size() == 3);
    REQUIRE(snapshot.report_io.save_action.enabled == true);
    REQUIRE(snapshot.bound_runtime_actions.rerun_migration.enabled == true);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target test_migration_wizard && ctest --test-dir build --output-on-failure -R test_migration_wizard`
Expected: FAIL with compile errors or assertions because `RenderSnapshot` does not yet expose workflow sections/cards/action groups.

- [ ] **Step 3: Write minimal implementation**

```cpp
struct WorkflowActionState {
    std::string id;
    std::string label;
    bool visible = false;
    bool enabled = false;
};
```

Add grouped workflow fields to `MigrationWizardPanel::RenderSnapshot` and populate them from the existing model/bound-runtime state in `render()`.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build --target test_migration_wizard && ctest --test-dir build --output-on-failure -R test_migration_wizard`
Expected: PASS for the new workflow render test and existing migration wizard tests.

- [ ] **Step 5: Commit**

```bash
git add tests/unit/test_migration_wizard.cpp editor/diagnostics/migration_wizard_panel.h
git commit -m "feat: render migration wizard workflow state"
```

### Task 2: Add failing workspace export tests for the rendered workflow body

**Files:**
- Modify: `C:/dev/URPG Maker/tests/unit/test_diagnostics_workspace.cpp`
- Test: `C:/dev/URPG Maker/tests/unit/test_diagnostics_workspace.cpp`

- [ ] **Step 1: Write the failing test**

```cpp
TEST_CASE("DiagnosticsWorkspace - Migration wizard export carries rendered workflow body",
          "[editor][diagnostics][integration][wizard_rendered_workflow]") {
    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindMigrationWizardRuntime({
        {"messages", {{{"id", "page_1"}, {"speaker", "Guide"}, {"text", "Welcome to URPG."}}}},
        {"scenes", {{{"symbol", "item"}, {"name", "Items"}}}}
    });
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::MigrationWizard);
    workspace.render();

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab_detail"]["workflow_sections"].is_array());
    REQUIRE(exported["active_tab_detail"]["workflow_sections"].size() >= 4);
    REQUIRE(exported["active_tab_detail"]["primary_actions"]["run_migration"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["subsystem_cards"].size() == 2);
    REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "message");
    REQUIRE(exported["active_tab_detail"]["report_io"]["save"]["enabled"] == true);
    REQUIRE(exported["active_tab_detail"]["bound_runtime_actions"]["rerun_selected_subsystem"]["enabled"] == true);
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `cmake --build build --target test_diagnostics_workspace && ctest --test-dir build --output-on-failure -R test_diagnostics_workspace`
Expected: FAIL because the workspace export currently only emits flat migration wizard fields.

- [ ] **Step 3: Write minimal implementation**

```cpp
activeTabDetail["workflow_sections"] = snapshot.workflow_sections;
activeTabDetail["primary_actions"] = MigrationWizardActionGroupJson(snapshot.primary_actions);
activeTabDetail["subsystem_cards"] = MigrationWizardSubsystemCardsJson(snapshot.subsystem_cards);
```

Extend `DiagnosticsWorkspace::exportAsJson()` to serialize the new rendered workflow state from the panel snapshot without changing the existing action methods.

- [ ] **Step 4: Run test to verify it passes**

Run: `cmake --build build --target test_diagnostics_workspace && ctest --test-dir build --output-on-failure -R test_diagnostics_workspace`
Expected: PASS for the new rendered workflow export test and existing workspace migration wizard tests.

- [ ] **Step 5: Commit**

```bash
git add tests/unit/test_diagnostics_workspace.cpp editor/diagnostics/diagnostics_workspace.cpp
git commit -m "feat: export migration wizard rendered workflow"
```

### Task 3: Verify end-to-end migration wizard workflow actions against the rendered body

**Files:**
- Modify: `C:/dev/URPG Maker/tests/unit/test_migration_wizard.cpp`
- Modify: `C:/dev/URPG Maker/tests/unit/test_diagnostics_workspace.cpp`
- Modify: `C:/dev/URPG Maker/editor/diagnostics/migration_wizard_panel.h`
- Modify: `C:/dev/URPG Maker/editor/diagnostics/diagnostics_workspace.cpp`

- [ ] **Step 1: Write the failing tests**

```cpp
REQUIRE(workspace.selectNextMigrationWizardSubsystemResult());
auto exported = nlohmann::json::parse(workspace.exportAsJson());
REQUIRE(exported["active_tab_detail"]["selected_subsystem_card"]["subsystem_id"] == "menu");
REQUIRE(exported["active_tab_detail"]["primary_actions"]["previous_subsystem"]["enabled"] == true);
REQUIRE(exported["active_tab_detail"]["primary_actions"]["next_subsystem"]["enabled"] == false);
```

Add assertions that rerun, clear, load/save, and bound-runtime actions update the rendered action groups and selected subsystem card immediately.

- [ ] **Step 2: Run tests to verify they fail**

Run: `ctest --test-dir build --output-on-failure -R "test_migration_wizard|test_diagnostics_workspace"`
Expected: FAIL until the rendered workflow state is recomputed for each action.

- [ ] **Step 3: Write minimal implementation**

```cpp
bool DiagnosticsWorkspace::selectNextMigrationWizardSubsystemResult() {
    const bool changed = migration_wizard_panel_.selectNextSubsystemResult();
    if (changed) {
        refreshMigrationWizardSnapshotIfActive();
    }
    return changed;
}
```

Keep the existing workspace action wiring, but make sure every wizard action leaves `MigrationWizardPanel::lastRenderSnapshot()` and exported workflow state current.

- [ ] **Step 4: Run tests to verify they pass**

Run: `ctest --test-dir build --output-on-failure -R "test_migration_wizard|test_diagnostics_workspace"`
Expected: PASS with the new rendered workflow assertions and all existing migration wizard regressions still green.

- [ ] **Step 5: Commit**

```bash
git add tests/unit/test_migration_wizard.cpp tests/unit/test_diagnostics_workspace.cpp editor/diagnostics/migration_wizard_panel.h editor/diagnostics/diagnostics_workspace.cpp
git commit -m "test: cover migration wizard rendered workflow actions"
```

### Task 4: Final verification

**Files:**
- Test: `C:/dev/URPG Maker/tests/unit/test_migration_wizard.cpp`
- Test: `C:/dev/URPG Maker/tests/unit/test_diagnostics_workspace.cpp`

- [ ] **Step 1: Run targeted test binaries**

Run: `ctest --test-dir build --output-on-failure -R "test_migration_wizard|test_diagnostics_workspace"`
Expected: PASS

- [ ] **Step 2: Run focused migration wizard cases if the suite target names differ**

Run: `build\\tests\\unit\\test_migration_wizard.exe --reporter compact`
Expected: PASS

Run: `build\\tests\\unit\\test_diagnostics_workspace.exe --reporter compact`
Expected: PASS

- [ ] **Step 3: Review diff for scope**

Run: `git diff -- editor/diagnostics/migration_wizard_panel.h editor/diagnostics/diagnostics_workspace.cpp tests/unit/test_migration_wizard.cpp tests/unit/test_diagnostics_workspace.cpp docs/superpowers/plans/2026-04-17-migration-wizard-rendered-workflow.md`
Expected: Only migration wizard workflow rendering, export wiring, tests, and the plan doc changed.

- [ ] **Step 4: Commit**

```bash
git add docs/superpowers/plans/2026-04-17-migration-wizard-rendered-workflow.md
git commit -m "docs: add migration wizard rendered workflow plan"
```
