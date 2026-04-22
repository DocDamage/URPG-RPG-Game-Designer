#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_route_resolver.h"
#include "engine/core/ui/menu_scene_graph.h"
#include "engine/core/input/input_core.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

using namespace urpg;
using namespace urpg::ui;
using namespace urpg::input;

TEST_CASE("MenuSceneGraph: Input Navigation", "[ui][graph][input]") {
    MenuSceneGraph graph;
    
    auto menu = std::make_shared<MenuScene>("Test");
    MenuPane pane;
    pane.id = "p1";
    pane.isActive = true;
    pane.commands = { {"c1"}, {"c2"}, {"c3"} };
    menu->addPane(pane);
    graph.registerScene(menu);
    graph.pushScene("Test");

    SECTION("MoveDown cycles forward") {
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 0);
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 1);
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 2);
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 0);
    }

    SECTION("MoveUp cycles backward") {
        graph.handleInput(InputAction::MoveUp, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 2);
    }
}

TEST_CASE("MenuSceneGraph: MoveLeft/MoveRight cycles active pane across visible panes",
          "[ui][graph][pane_focus]") {
    MenuSceneGraph graph;

    auto menu = std::make_shared<MenuScene>("PaneFocusTest");
    MenuPane paneA;
    paneA.id = "pane_a";
    paneA.isActive = true;
    paneA.isVisible = true;
    paneA.commands = {{"a_cmd"}};

    MenuPane paneB;
    paneB.id = "pane_b";
    paneB.isActive = false;
    paneB.isVisible = false; // hidden pane should be skipped
    paneB.commands = {{"b_cmd"}};

    MenuPane paneC;
    paneC.id = "pane_c";
    paneC.isActive = false;
    paneC.isVisible = true;
    paneC.commands = {{"c_cmd"}};

    menu->addPane(paneA);
    menu->addPane(paneB);
    menu->addPane(paneC);
    graph.registerScene(menu);
    graph.pushScene("PaneFocusTest");

    SECTION("MoveRight skips hidden pane and wraps") {
        graph.handleInput(InputAction::MoveRight, ActionState::Pressed);

        const auto& panes = graph.getActiveScene()->getPanes();
        REQUIRE_FALSE(panes[0].isActive);
        REQUIRE_FALSE(panes[1].isActive);
        REQUIRE(panes[2].isActive);

        graph.handleInput(InputAction::MoveRight, ActionState::Pressed);
        REQUIRE(panes[0].isActive);
        REQUIRE_FALSE(panes[1].isActive);
        REQUIRE_FALSE(panes[2].isActive);
    }

    SECTION("MoveLeft wraps backward and skips hidden pane") {
        graph.handleInput(InputAction::MoveLeft, ActionState::Pressed);

        const auto& panes = graph.getActiveScene()->getPanes();
        REQUIRE_FALSE(panes[0].isActive);
        REQUIRE_FALSE(panes[1].isActive);
        REQUIRE(panes[2].isActive);

        graph.handleInput(InputAction::MoveLeft, ActionState::Pressed);
        REQUIRE(panes[0].isActive);
        REQUIRE_FALSE(panes[1].isActive);
        REQUIRE_FALSE(panes[2].isActive);
    }
}

TEST_CASE("MenuSceneGraph: pane focus skips panes without visible+enabled commands",
          "[ui][graph][pane_focus][gate]") {
    MenuSceneGraph graph;

    auto menu = std::make_shared<MenuScene>("PaneFocusGateTest");

    MenuPane paneA;
    paneA.id = "pane_a";
    paneA.isActive = true;
    paneA.isVisible = true;
    urpg::MenuCommandMeta cmdA;
    cmdA.id = "cmd_a";
    paneA.commands = {cmdA};

    MenuPane paneB;
    paneB.id = "pane_b";
    paneB.isActive = false;
    paneB.isVisible = true;
    urpg::MenuCommandMeta blockedCmd;
    blockedCmd.id = "blocked_cmd";
    paneB.commands = {blockedCmd};

    MenuPane paneC;
    paneC.id = "pane_c";
    paneC.isActive = false;
    paneC.isVisible = true;
    urpg::MenuCommandMeta hiddenCmd;
    hiddenCmd.id = "hidden_cmd";
    paneC.commands = {hiddenCmd};

    MenuPane paneD;
    paneD.id = "pane_d";
    paneD.isActive = false;
    paneD.isVisible = true;
    urpg::MenuCommandMeta cmdD;
    cmdD.id = "cmd_d";
    paneD.commands = {cmdD};

    menu->addPane(paneA);
    menu->addPane(paneB);
    menu->addPane(paneC);
    menu->addPane(paneD);
    graph.registerScene(menu);
    graph.pushScene("PaneFocusGateTest");

    graph.setCommandEnabledEvaluator([](const urpg::MenuCommandMeta& cmd) {
        return cmd.id != "blocked_cmd";
    });
    graph.setCommandVisibleEvaluator([](const urpg::MenuCommandMeta& cmd) {
        return cmd.id != "hidden_cmd";
    });

    graph.handleInput(InputAction::MoveRight, ActionState::Pressed);
    const auto& panes = graph.getActiveScene()->getPanes();
    REQUIRE_FALSE(panes[0].isActive);
    REQUIRE_FALSE(panes[1].isActive);
    REQUIRE_FALSE(panes[2].isActive);
    REQUIRE(panes[3].isActive);

    graph.handleInput(InputAction::MoveLeft, ActionState::Pressed);
    REQUIRE(panes[0].isActive);
    REQUIRE_FALSE(panes[1].isActive);
    REQUIRE_FALSE(panes[2].isActive);
    REQUIRE_FALSE(panes[3].isActive);
}

TEST_CASE("MenuSceneGraph: active pane auto-recovers when current pane becomes non-navigable",
          "[ui][graph][pane_focus][recovery]") {
    MenuSceneGraph graph;

    auto menu = std::make_shared<MenuScene>("PaneRecoveryTest");

    MenuPane paneA;
    paneA.id = "pane_a";
    paneA.isActive = true;
    paneA.isVisible = true;
    urpg::MenuCommandMeta cmdA;
    cmdA.id = "cmd_a";
    paneA.commands = {cmdA};

    MenuPane paneB;
    paneB.id = "pane_b";
    paneB.isActive = false;
    paneB.isVisible = true;
    urpg::MenuCommandMeta cmdB;
    cmdB.id = "cmd_b";
    paneB.commands = {cmdB};

    menu->addPane(paneA);
    menu->addPane(paneB);
    graph.registerScene(menu);
    graph.pushScene("PaneRecoveryTest");

    // Simulate runtime state change that disables pane A's only command.
    graph.setCommandEnabledEvaluator([](const urpg::MenuCommandMeta& cmd) {
        return cmd.id != "cmd_a";
    });

    // Any input should trigger active-pane validation before action handling.
    graph.handleInput(InputAction::MoveDown, ActionState::Pressed);

    const auto& panes = graph.getActiveScene()->getPanes();
    REQUIRE_FALSE(panes[0].isActive);
    REQUIRE(panes[1].isActive);
}

TEST_CASE("MenuSceneGraph: registry state helper drives visibility and enabled gating",
          "[ui][graph][state_helper]") {
    MenuSceneGraph graph;
    MenuRouteResolver resolver;
    MenuCommandRegistry registry;

    int32_t optionsRouteHits = 0;
    resolver.bindRoute(urpg::MenuRouteTarget::Options, [&](const urpg::MenuCommandMeta&) {
        ++optionsRouteHits;
    });

    auto menu = std::make_shared<MenuScene>("RegistryStateHelperTest");

    MenuPane paneA;
    paneA.id = "pane_a";
    paneA.isActive = true;
    paneA.isVisible = true;
    urpg::MenuCommandMeta hiddenByRule;
    hiddenByRule.id = "hidden_by_rule";
    hiddenByRule.route = urpg::MenuRouteTarget::Options;
    hiddenByRule.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "show_hidden",
            .variable_id = "",
            .variable_threshold = 0,
            .invert = false,
        },
    };
    paneA.commands = {hiddenByRule};

    MenuPane paneB;
    paneB.id = "pane_b";
    paneB.isActive = false;
    paneB.isVisible = true;
    urpg::MenuCommandMeta disabledByRule;
    disabledByRule.id = "disabled_by_rule";
    disabledByRule.route = urpg::MenuRouteTarget::Options;
    disabledByRule.enable_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "energy",
            .variable_threshold = 3,
            .invert = false,
        },
    };
    paneB.commands = {disabledByRule};

    MenuPane paneC;
    paneC.id = "pane_c";
    paneC.isActive = false;
    paneC.isVisible = true;
    urpg::MenuCommandMeta enabledByRule;
    enabledByRule.id = "enabled_by_rule";
    enabledByRule.route = urpg::MenuRouteTarget::Options;
    enabledByRule.enable_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "",
            .variable_id = "energy",
            .variable_threshold = 1,
            .invert = false,
        },
    };
    paneC.commands = {enabledByRule};

    menu->addPane(paneA);
    menu->addPane(paneB);
    menu->addPane(paneC);
    graph.registerScene(menu);
    graph.pushScene("RegistryStateHelperTest");
    graph.setRouteResolver(&resolver);

    MenuCommandRegistry::SwitchState switches{
        {"show_hidden", false},
    };
    MenuCommandRegistry::VariableState variables{
        {"energy", 2},
    };
    graph.setCommandStateFromRegistry(registry, switches, variables);

    // pane A hidden by visibility rule, pane B disabled by enable rule, pane C is navigable.
    graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
    const auto& panes = graph.getActiveScene()->getPanes();
    REQUIRE_FALSE(panes[0].isActive);
    REQUIRE_FALSE(panes[1].isActive);
    REQUIRE(panes[2].isActive);

    graph.handleInput(InputAction::Confirm, ActionState::Pressed);
    REQUIRE(optionsRouteHits == 1);
}

TEST_CASE("MenuSceneGraph: Confirm activates selected command through route resolver",
          "[ui][graph][confirm]") {
    MenuSceneGraph graph;
    MenuRouteResolver resolver;

    int32_t optionsRouteHits = 0;
    resolver.bindRoute(urpg::MenuRouteTarget::Options, [&](const urpg::MenuCommandMeta&) {
        ++optionsRouteHits;
    });

    auto menu = std::make_shared<MenuScene>("ConfirmTest");
    MenuPane pane;
    pane.id = "p1";
    pane.isActive = true;

    urpg::MenuCommandMeta unresolvedPrimary;
    unresolvedPrimary.id = "cmd_missing_custom";
    unresolvedPrimary.route = urpg::MenuRouteTarget::Custom;
    unresolvedPrimary.custom_route_id = "missing_custom";
    unresolvedPrimary.fallback_route = urpg::MenuRouteTarget::Options;

    urpg::MenuCommandMeta unresolvedAll;
    unresolvedAll.id = "cmd_dead_end";
    unresolvedAll.route = urpg::MenuRouteTarget::Status;
    unresolvedAll.fallback_route = urpg::MenuRouteTarget::Custom;
    unresolvedAll.fallback_custom_route_id = "missing_fallback_custom";

    pane.commands = {unresolvedPrimary, unresolvedAll};
    menu->addPane(pane);
    graph.registerScene(menu);
    graph.pushScene("ConfirmTest");
    graph.setRouteResolver(&resolver);

    SECTION("Confirm resolves through fallback route") {
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 0);
        graph.handleInput(InputAction::Confirm, ActionState::Pressed);
        REQUIRE(optionsRouteHits == 1);
    }

    SECTION("Confirm on unresolved command does not trigger route callbacks") {
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 1);

        graph.handleInput(InputAction::Confirm, ActionState::Pressed);
        REQUIRE(optionsRouteHits == 0);
    }
}

TEST_CASE("MenuSceneGraph: Confirm enforces command enabled evaluator gate",
          "[ui][graph][confirm][gate]") {
    MenuSceneGraph graph;
    MenuRouteResolver resolver;

    int32_t optionsRouteHits = 0;
    resolver.bindRoute(urpg::MenuRouteTarget::Options, [&](const urpg::MenuCommandMeta&) {
        ++optionsRouteHits;
    });

    auto menu = std::make_shared<MenuScene>("ConfirmGateTest");
    MenuPane pane;
    pane.id = "gate_pane";
    pane.isActive = true;

    urpg::MenuCommandMeta blocked;
    blocked.id = "blocked_cmd";
    blocked.route = urpg::MenuRouteTarget::Options;

    urpg::MenuCommandMeta allowed;
    allowed.id = "allowed_cmd";
    allowed.route = urpg::MenuRouteTarget::Options;

    pane.commands = {blocked, allowed};
    menu->addPane(pane);
    graph.registerScene(menu);
    graph.pushScene("ConfirmGateTest");
    graph.setRouteResolver(&resolver);
    graph.setCommandEnabledEvaluator([](const urpg::MenuCommandMeta& cmd) {
        return cmd.id != "blocked_cmd";
    });

    SECTION("Blocked command does not resolve route") {
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 0);
        graph.handleInput(InputAction::Confirm, ActionState::Pressed);
        REQUIRE(optionsRouteHits == 0);
    }

    SECTION("Enabled command resolves route") {
        graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
        REQUIRE(graph.getActiveScene()->getPanes()[0].selectedCommandIndex == 1);
        graph.handleInput(InputAction::Confirm, ActionState::Pressed);
        REQUIRE(optionsRouteHits == 1);
    }
}

TEST_CASE("MenuSceneGraph: blocked confirm exposes disabled reason and callback metadata",
          "[ui][graph][confirm][blocked_reason]") {
    MenuSceneGraph graph;
    MenuRouteResolver resolver;

    int32_t optionsRouteHits = 0;
    resolver.bindRoute(urpg::MenuRouteTarget::Options, [&](const urpg::MenuCommandMeta&) {
        ++optionsRouteHits;
    });

    std::string blockedEventCommandId;
    std::string blockedEventReason;
    graph.setCommandBlockedHandler(
        [&](const urpg::MenuCommandMeta& cmd, std::string_view reason) {
            blockedEventCommandId = cmd.id;
            blockedEventReason = std::string(reason);
        }
    );

    auto menu = std::make_shared<MenuScene>("ConfirmBlockedReasonTest");
    MenuPane pane;
    pane.id = "blocked_reason_pane";
    pane.isActive = true;

    urpg::MenuCommandMeta blocked;
    blocked.id = "blocked_cmd";
    blocked.route = urpg::MenuRouteTarget::Options;

    urpg::MenuCommandMeta allowed;
    allowed.id = "allowed_cmd";
    allowed.route = urpg::MenuRouteTarget::Options;

    pane.commands = {blocked, allowed};
    menu->addPane(pane);
    graph.registerScene(menu);
    graph.pushScene("ConfirmBlockedReasonTest");
    graph.setRouteResolver(&resolver);
    graph.setCommandEnabledEvaluator([](const urpg::MenuCommandMeta& cmd) {
        return cmd.id != "blocked_cmd";
    });
    graph.setCommandDisabledReasonEvaluator([](const urpg::MenuCommandMeta& cmd) {
        if (cmd.id == "blocked_cmd") {
            return std::string("Requires 3 energy.");
        }
        return std::string();
    });

    graph.handleInput(InputAction::Confirm, ActionState::Pressed);
    REQUIRE(optionsRouteHits == 0);
    REQUIRE(graph.getLastBlockedCommandId() == "blocked_cmd");
    REQUIRE(graph.getLastBlockedReason() == "Requires 3 energy.");
    REQUIRE(blockedEventCommandId == "blocked_cmd");
    REQUIRE(blockedEventReason == "Requires 3 energy.");

    graph.handleInput(InputAction::MoveDown, ActionState::Pressed);
    graph.handleInput(InputAction::Confirm, ActionState::Pressed);
    REQUIRE(optionsRouteHits == 1);
    REQUIRE(graph.getLastBlockedCommandId().empty());
    REQUIRE(graph.getLastBlockedReason().empty());
}

TEST_CASE("MenuSceneGraph handles basic navigation and stack", "[ui][graph]") {
    MenuSceneGraph graph;
    
    auto mainMenu = std::make_shared<MenuScene>("MainMenu");
    mainMenu->addPane({"main_pane", "Main Menu", true, true, {}});
    graph.registerScene(mainMenu);

    auto settingsMenu = std::make_shared<MenuScene>("Settings");
    settingsMenu->addPane({"audio_pane", "Audio", true, false, {}});
    graph.registerScene(settingsMenu);

    SECTION("Empty stack initially") {
        REQUIRE(graph.getActiveScene() == nullptr);
        REQUIRE(graph.stackSize() == 0);
    }

    SECTION("Push and Pop Scenes") {
        graph.pushScene("MainMenu");
        REQUIRE(graph.getActiveScene()->getId() == "MainMenu");
        REQUIRE(graph.stackSize() == 1);

        graph.pushScene("Settings");
        REQUIRE(graph.getActiveScene()->getId() == "Settings");
        REQUIRE(graph.stackSize() == 2);

        graph.popScene();
        REQUIRE(graph.getActiveScene()->getId() == "MainMenu");
        REQUIRE(graph.stackSize() == 1);
    }
}

TEST_CASE("MenuSceneGraph: Cancel pops nested scenes and guards root by default",
          "[ui][graph][cancel]") {
    MenuSceneGraph graph;

    auto rootMenu = std::make_shared<MenuScene>("RootMenu");
    MenuPane rootPane;
    rootPane.id = "root_pane";
    rootPane.isActive = true;
    rootPane.commands = {{"root_cmd"}};
    rootMenu->addPane(rootPane);
    graph.registerScene(rootMenu);

    auto childMenu = std::make_shared<MenuScene>("ChildMenu");
    MenuPane childPane;
    childPane.id = "child_pane";
    childPane.isActive = true;
    childPane.commands = {{"child_cmd"}};
    childMenu->addPane(childPane);
    graph.registerScene(childMenu);

    graph.pushScene("RootMenu");
    graph.pushScene("ChildMenu");
    REQUIRE(graph.stackSize() == 2);
    REQUIRE(graph.getActiveScene()->getId() == "ChildMenu");

    graph.handleInput(InputAction::Cancel, ActionState::Pressed);
    REQUIRE(graph.stackSize() == 1);
    REQUIRE(graph.getActiveScene()->getId() == "RootMenu");

    graph.handleInput(InputAction::Cancel, ActionState::Pressed);
    REQUIRE(graph.stackSize() == 1);
    REQUIRE(graph.getActiveScene()->getId() == "RootMenu");
}

TEST_CASE("MenuSceneGraph: Cancel can pop root scene when explicitly enabled",
          "[ui][graph][cancel]") {
    MenuSceneGraph graph;

    auto rootMenu = std::make_shared<MenuScene>("RootMenu");
    MenuPane rootPane;
    rootPane.id = "root_pane";
    rootPane.isActive = true;
    rootPane.commands = {{"root_cmd"}};
    rootMenu->addPane(rootPane);
    graph.registerScene(rootMenu);

    graph.pushScene("RootMenu");
    REQUIRE(graph.stackSize() == 1);
    REQUIRE(graph.getActiveScene()->getId() == "RootMenu");

    graph.setAllowRootCancelPop(true);
    graph.handleInput(InputAction::Cancel, ActionState::Pressed);
    REQUIRE(graph.stackSize() == 0);
    REQUIRE(graph.getActiveScene() == nullptr);
}

TEST_CASE("MenuCommandRegistry handles priority and sorting", "[ui][menu]") {
    MenuCommandRegistry registry;
    
    urpg::MenuCommandMeta cmd1;
    cmd1.id = "low_prio";
    cmd1.priority = 10;
    registry.registerCommand(cmd1);

    urpg::MenuCommandMeta cmd2;
    cmd2.id = "high_prio";
    cmd2.priority = 0;
    registry.registerCommand(cmd2);

    auto list = registry.listCommands();
    REQUIRE(list.size() == 2);
    REQUIRE(list[0].id == "high_prio");
    REQUIRE(list[1].id == "low_prio");
}

TEST_CASE("MenuCommandRegistry loads from JSON schema", "[ui][menu][schema]") {
    nlohmann::json schema = R"({
        "commands": [
            { "id": "save", "label": "Save Progress", "route": "save", "priority": 100 },
            { "id": "custom_01", "label": "Special", "route": "custom", "custom_route_id": "sp_one", "fallback_route": "options", "priority": 50 }
        ]
    })"_json;

    MenuCommandRegistry registry;
    REQUIRE(registry.loadFromSchema(schema));

    const auto* saveCmd = registry.getCommand("save");
    REQUIRE(saveCmd != nullptr);
    REQUIRE(saveCmd->route == urpg::MenuRouteTarget::Save);
    REQUIRE(saveCmd->priority == 100);

    const auto* customCmd = registry.getCommand("custom_01");
    REQUIRE(customCmd != nullptr);
    REQUIRE(customCmd->route == urpg::MenuRouteTarget::Custom);
    REQUIRE(customCmd->custom_route_id == "sp_one");
    REQUIRE(customCmd->fallback_route == urpg::MenuRouteTarget::Options);

    auto list = registry.listCommands();
    REQUIRE(list[0].id == "custom_01");
    REQUIRE(list[1].id == "save");
}

TEST_CASE("MenuRouteResolver resolves native and custom targets", "[ui][menu][route]") {
    MenuRouteResolver resolver;
    bool nativeTriggered = false;
    bool customTriggered = false;

    resolver.bindRoute(urpg::MenuRouteTarget::Item, [&](const urpg::MenuCommandMeta&) {
        nativeTriggered = true;
    });

    resolver.bindCustomRoute("quest_ext", [&](const urpg::MenuCommandMeta&) {
        customTriggered = true;
    });

    urpg::MenuCommandMeta itemCmd;
    itemCmd.route = urpg::MenuRouteTarget::Item;
    REQUIRE(resolver.resolve(itemCmd));
    REQUIRE(nativeTriggered);

    urpg::MenuCommandMeta customCmd;
    customCmd.route = urpg::MenuRouteTarget::Custom;
    customCmd.custom_route_id = "quest_ext";
    REQUIRE(resolver.resolve(customCmd));
    REQUIRE(customTriggered);

    urpg::MenuCommandMeta deadCmd;
    deadCmd.route = urpg::MenuRouteTarget::Status;
    REQUIRE_FALSE(resolver.resolve(deadCmd));
}

TEST_CASE("MenuRouteResolver falls back when primary route is unavailable", "[ui][menu][route][fallback]") {
    MenuRouteResolver resolver;
    bool optionsTriggered = false;
    bool customFallbackTriggered = false;

    resolver.bindRoute(urpg::MenuRouteTarget::Options, [&](const urpg::MenuCommandMeta&) {
        optionsTriggered = true;
    });
    resolver.bindCustomRoute("backup_route", [&](const urpg::MenuCommandMeta&) {
        customFallbackTriggered = true;
    });

    urpg::MenuCommandMeta missingCustomWithNativeFallback;
    missingCustomWithNativeFallback.route = urpg::MenuRouteTarget::Custom;
    missingCustomWithNativeFallback.custom_route_id = "missing_primary_custom";
    missingCustomWithNativeFallback.fallback_route = urpg::MenuRouteTarget::Options;
    REQUIRE(resolver.resolve(missingCustomWithNativeFallback));
    REQUIRE(optionsTriggered);
    REQUIRE_FALSE(customFallbackTriggered);

    optionsTriggered = false;

    urpg::MenuCommandMeta missingNativeWithCustomFallback;
    missingNativeWithCustomFallback.route = urpg::MenuRouteTarget::Status;
    missingNativeWithCustomFallback.fallback_route = urpg::MenuRouteTarget::Custom;
    missingNativeWithCustomFallback.fallback_custom_route_id = "backup_route";
    REQUIRE(resolver.resolve(missingNativeWithCustomFallback));
    REQUIRE_FALSE(optionsTriggered);
    REQUIRE(customFallbackTriggered);

    urpg::MenuCommandMeta unresolved;
    unresolved.route = urpg::MenuRouteTarget::Custom;
    unresolved.custom_route_id = "still_missing";
    unresolved.fallback_route = urpg::MenuRouteTarget::Status;
    REQUIRE_FALSE(resolver.resolve(unresolved));
}

TEST_CASE("MenuCommandRegistry evaluates visibility and enable conditions", "[ui][menu][state]") {
    MenuCommandRegistry registry;

    urpg::MenuCommandMeta always;
    always.id = "always";
    always.priority = 30;
    registry.registerCommand(always);

    urpg::MenuCommandMeta gatedVisible;
    gatedVisible.id = "gated_visible";
    gatedVisible.priority = 10;
    gatedVisible.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "menu_unlocked",
            .variable_id = "story_progress",
            .variable_threshold = 3,
            .invert = false,
        },
    };
    registry.registerCommand(gatedVisible);

    urpg::MenuCommandMeta gatedEnable;
    gatedEnable.id = "gated_enable";
    gatedEnable.priority = 20;
    gatedEnable.enable_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "can_use_special",
            .variable_id = "mana",
            .variable_threshold = 25,
            .invert = false,
        },
    };
    registry.registerCommand(gatedEnable);

    urpg::MenuCommandMeta hiddenWhenFlagOn;
    hiddenWhenFlagOn.id = "hidden_when_flag_on";
    hiddenWhenFlagOn.priority = 40;
    hiddenWhenFlagOn.visibility_rules = {
        urpg::MenuCommandCondition{
            .switch_id = "hide_secret",
            .variable_id = "",
            .variable_threshold = 0,
            .invert = true,
        },
    };
    registry.registerCommand(hiddenWhenFlagOn);

    MenuCommandRegistry::SwitchState switches{
        {"menu_unlocked", true},
        {"can_use_special", false},
        {"hide_secret", false},
    };
    MenuCommandRegistry::VariableState variables{
        {"story_progress", 4},
        {"mana", 10},
    };

    const auto* visibleCmd = registry.getCommand("gated_visible");
    REQUIRE(visibleCmd != nullptr);
    REQUIRE(registry.isVisible(*visibleCmd, switches, variables));

    const auto* enableCmd = registry.getCommand("gated_enable");
    REQUIRE(enableCmd != nullptr);
    REQUIRE_FALSE(registry.isEnabled(*enableCmd, switches, variables));

    const auto* hiddenCmd = registry.getCommand("hidden_when_flag_on");
    REQUIRE(hiddenCmd != nullptr);
    REQUIRE(registry.isVisible(*hiddenCmd, switches, variables));

    auto visibleList = registry.listCommandsForState(switches, variables);
    REQUIRE(visibleList.size() == 4);
    REQUIRE(visibleList[0].id == "gated_visible");
    REQUIRE(visibleList[1].id == "gated_enable");
    REQUIRE(visibleList[2].id == "always");
    REQUIRE(visibleList[3].id == "hidden_when_flag_on");

    switches["menu_unlocked"] = false;
    switches["hide_secret"] = true;
    variables["story_progress"] = 1;
    variables["mana"] = 30;
    switches["can_use_special"] = true;

    REQUIRE_FALSE(registry.isVisible(*visibleCmd, switches, variables));
    REQUIRE(registry.isEnabled(*enableCmd, switches, variables));
    REQUIRE_FALSE(registry.isVisible(*hiddenCmd, switches, variables));

    visibleList = registry.listCommandsForState(switches, variables);
    REQUIRE(visibleList.size() == 2);
    REQUIRE(visibleList[0].id == "gated_enable");
    REQUIRE(visibleList[1].id == "always");
}


TEST_CASE("MenuCommandRegistry: Save to schema round-trips loaded commands", "[ui][menu][schema][parity]") {
    nlohmann::json schema = R"({
        "commands": [
            { "id": "save", "label": "Save Progress", "route": "save", "priority": 100 },
            { "id": "codex", "label": "Codex", "route": "codex", "fallback_route": "options", "priority": 50 },
            { "id": "custom_01", "label": "Special", "route": "custom", "custom_route_id": "sp_one", "fallback_route": "game_end", "priority": 25 },
            { "id": "gated", "label": "Gated", "route": "item", "priority": 10,
              "visibility_rules": [{"switch_id": "unlocked"}],
              "enable_rules": [{"variable_id": "level", "variable_threshold": 5}]
            }
        ]
    })"_json;

    MenuCommandRegistry registry;
    REQUIRE(registry.loadFromSchema(schema));

    const auto saved = registry.saveToSchema();
    REQUIRE(saved.contains("commands"));
    REQUIRE(saved["commands"].is_array());
    REQUIRE(saved["commands"].size() == 4);

    // Verify order is preserved by priority
    REQUIRE(saved["commands"][0]["id"] == "gated");
    REQUIRE(saved["commands"][0]["priority"] == 10);
    REQUIRE(saved["commands"][1]["id"] == "custom_01");
    REQUIRE(saved["commands"][1]["route"] == "custom");
    REQUIRE(saved["commands"][1]["custom_route_id"] == "sp_one");
    REQUIRE(saved["commands"][1]["fallback_route"] == "game_end");
    REQUIRE(saved["commands"][2]["id"] == "codex");
    REQUIRE(saved["commands"][2]["route"] == "codex");
    REQUIRE(saved["commands"][2]["fallback_route"] == "options");
    REQUIRE(saved["commands"][3]["id"] == "save");
    REQUIRE(saved["commands"][3]["route"] == "save");

    // Verify rules round-trip
    REQUIRE(saved["commands"][0].contains("visibility_rules"));
    REQUIRE(saved["commands"][0]["visibility_rules"].size() == 1);
    REQUIRE(saved["commands"][0]["visibility_rules"][0]["switch_id"] == "unlocked");
    REQUIRE(saved["commands"][0].contains("enable_rules"));
    REQUIRE(saved["commands"][0]["enable_rules"].size() == 1);
    REQUIRE(saved["commands"][0]["enable_rules"][0]["variable_id"] == "level");
    REQUIRE(saved["commands"][0]["enable_rules"][0]["variable_threshold"] == 5);

    // Verify full round-trip by reloading
    MenuCommandRegistry reloaded;
    REQUIRE(reloaded.loadFromSchema(saved));
    REQUIRE(reloaded.getCommand("codex") != nullptr);
    REQUIRE(reloaded.getCommand("codex")->route == urpg::MenuRouteTarget::Codex);
    REQUIRE(reloaded.getCommand("custom_01")->custom_route_id == "sp_one");
    REQUIRE(reloaded.getCommand("gated")->visibility_rules.size() == 1);
    REQUIRE(reloaded.getCommand("gated")->enable_rules.size() == 1);
}

TEST_CASE("MenuRouteResolver: Introspection exposes bound routes", "[ui][menu][route][parity]") {
    MenuRouteResolver resolver;

    REQUIRE_FALSE(resolver.isRouteBound(urpg::MenuRouteTarget::Item));
    REQUIRE_FALSE(resolver.isRouteBound(urpg::MenuRouteTarget::Options));
    REQUIRE_FALSE(resolver.isCustomRouteBound("quest_ext"));
    REQUIRE(resolver.listBoundNativeRoutes().empty());
    REQUIRE(resolver.listBoundCustomRoutes().empty());

    resolver.bindRoute(urpg::MenuRouteTarget::Item, [](const urpg::MenuCommandMeta&) {});
    resolver.bindRoute(urpg::MenuRouteTarget::Options, [](const urpg::MenuCommandMeta&) {});
    resolver.bindCustomRoute("quest_ext", [](const urpg::MenuCommandMeta&) {});
    resolver.bindCustomRoute("mods", [](const urpg::MenuCommandMeta&) {});

    REQUIRE(resolver.isRouteBound(urpg::MenuRouteTarget::Item));
    REQUIRE(resolver.isRouteBound(urpg::MenuRouteTarget::Options));
    REQUIRE_FALSE(resolver.isRouteBound(urpg::MenuRouteTarget::Save));
    REQUIRE(resolver.isCustomRouteBound("quest_ext"));
    REQUIRE(resolver.isCustomRouteBound("mods"));
    REQUIRE_FALSE(resolver.isCustomRouteBound("missing"));

    auto native = resolver.listBoundNativeRoutes();
    REQUIRE(native.size() == 2);
    auto custom = resolver.listBoundCustomRoutes();
    REQUIRE(custom.size() == 2);
}
