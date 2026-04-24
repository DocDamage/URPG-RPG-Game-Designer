#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace {

using urpg::Object;
using urpg::Value;
using urpg::compat::BattleAction;
using urpg::compat::BattleActionType;
using urpg::compat::BattleManager;
using urpg::compat::BattlePhase;
using urpg::compat::BattleResult;
using urpg::compat::BattleSubject;
using urpg::compat::BattleSubjectType;
using urpg::compat::PluginManager;

std::filesystem::path sourceRootFromMacro() {
#ifdef URPG_SOURCE_DIR
    std::string sourceRoot = URPG_SOURCE_DIR;
    if (sourceRoot.size() >= 2 &&
        sourceRoot.front() == '"' &&
        sourceRoot.back() == '"') {
        sourceRoot = sourceRoot.substr(1, sourceRoot.size() - 2);
    }
    return std::filesystem::path(sourceRoot);
#else
    return {};
#endif
}

std::filesystem::path fixtureDir() {
    const auto sourceRoot = sourceRootFromMacro();
    if (!sourceRoot.empty()) {
        return sourceRoot / "tests" / "compat" / "fixtures" / "plugins";
    }
    return std::filesystem::path("tests") / "compat" / "fixtures" / "plugins";
}

std::filesystem::path fixturePath(const std::string& pluginName) {
    return fixtureDir() / (pluginName + ".json");
}

std::filesystem::path uniqueTempFixturePath(std::string_view stem) {
    const auto ticks = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    return std::filesystem::temp_directory_path() /
           (std::string(stem) + "_" + std::to_string(ticks) + ".json");
}

void writeTextFile(const std::filesystem::path& path, std::string_view contents) {
    std::ofstream out(path, std::ios::binary);
    REQUIRE(out.is_open());
    out << contents;
}

} // namespace

TEST_CASE("Compat fixtures: curated battle-flow scenarios survive plugin reload",
                "[compat][fixtures]") {
        PluginManager& pm = PluginManager::instance();
        pm.unloadAllPlugins();
        pm.clearFailureDiagnostics();

        REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
        REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

        const auto reloadFixture = uniqueTempFixturePath("urpg_curated_battle_flow_reload_fixture");
        writeTextFile(
            reloadFixture,
            R"({
        "name": "CuratedBattleFlowReloadFixture",
        "parameters": {
        "defaultRoute": "action",
        "defaultMotion": "slash"
        },
        "commands": [
        {
            "name": "engage",
            "script": [
            {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "battle_flow_boot"}], "store": "boot", "expect": "non_nil"},
            {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
            {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultMotion"}}},
            {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "escape"},
                "then": [
                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "routeResult", "expect": "non_nil"},
                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "supportResult", "expect": "non_nil"},
                {"op": "set", "key": "routeToken", "value": "escape:attempt"}
                ],
                "else": [
                {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "routeResult", "expect": "non_nil"},
                {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "supportResult", "expect": "non_nil"},
                {"op": "set", "key": "routeToken", "value": "action:resolve"}
                ]
            },
            {"op": "returnObject"}
            ]
        }
        ]
    })"
        );

        REQUIRE(pm.loadPlugin(reloadFixture.string()));

        auto verifyActionRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
            REQUIRE(std::get<std::string>(object.at("route").v) == "action");
            REQUIRE(std::get<std::string>(object.at("routeToken").v) == "action:resolve");
                REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "battle_hud");
            REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("hudSlots").v) == 4);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("supportResult").v).at("profile").v) == "character_motion");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("supportResult").v).at("argCount").v) == 1);
                REQUIRE(std::get<bool>(std::get<Object>(object.at("supportResult").v).at("supportsScale").v));

            BattleManager battle;
            int actionStartCount = 0;
            int actionEndCount = 0;
            int damageCount = 0;
            int healCount = 0;
            battle.registerHook(BattleManager::HookPoint::ON_ACTION_START, "battle_anchor", [&actionStartCount](const std::vector<Value>&) {
                ++actionStartCount;
                return Value::Nil();
            });
            battle.registerHook(BattleManager::HookPoint::ON_ACTION_END, "battle_anchor", [&actionEndCount](const std::vector<Value>&) {
                ++actionEndCount;
                return Value::Nil();
            });
            battle.registerHook(BattleManager::HookPoint::ON_DAMAGE, "battle_anchor", [&damageCount](const std::vector<Value>&) {
                ++damageCount;
                return Value::Nil();
            });
            battle.registerHook(BattleManager::HookPoint::ON_HEAL, "battle_anchor", [&healCount](const std::vector<Value>&) {
                ++healCount;
                return Value::Nil();
            });

            battle.setup(11, true, false);
            battle.startBattle();
            REQUIRE(battle.getPhase() == BattlePhase::INPUT);
            battle.startTurn();
            REQUIRE(battle.getPhase() == BattlePhase::TURN);

            BattleSubject actor;
            actor.type = BattleSubjectType::ACTOR;
            actor.index = 0;
            actor.hp = 50;
            actor.mhp = 100;
            actor.actionSpeed = 18;

            BattleSubject enemy;
            enemy.type = BattleSubjectType::ENEMY;
            enemy.index = 0;
            enemy.hp = 90;
            enemy.mhp = 90;

            battle.queueAction(&actor, BattleActionType::WAIT);
            BattleAction* action = battle.getNextAction();
            REQUIRE(action != nullptr);
            REQUIRE(action->type == BattleActionType::WAIT);
            battle.processAction(action);
            REQUIRE(actionStartCount == 1);
            REQUIRE(actionEndCount == 1);

            battle.applyDamage(&enemy, 15);
            battle.applyHeal(&actor, 20);
            REQUIRE(enemy.hp == 75);
            REQUIRE(actor.hp == 70);
            REQUIRE(damageCount == 1);
            REQUIRE(healCount == 1);

            battle.endTurn();
            REQUIRE(battle.getTurnCount() == 1);
            REQUIRE(battle.getPhase() == BattlePhase::INPUT);
        };

        auto verifyEscapeRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
            REQUIRE(std::holds_alternative<Object>(value.v));
            const auto& object = std::get<Object>(value.v);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
            REQUIRE(std::get<std::string>(object.at("route").v) == "escape");
            REQUIRE(std::get<std::string>(object.at("routeToken").v) == "escape:attempt");
                REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("routeResult").v).at("profile").v) == "character_motion");
                REQUIRE(std::get<int64_t>(std::get<Object>(object.at("routeResult").v).at("argCount").v) == 1);
            REQUIRE(std::get<std::string>(std::get<Object>(object.at("supportResult").v).at("profile").v) == "battle_hud");

            BattleManager battle;
            int escapeHookCount = 0;
            battle.registerHook(BattleManager::HookPoint::ON_ESCAPE, "battle_anchor", [&escapeHookCount](const std::vector<Value>&) {
                ++escapeHookCount;
                return Value::Nil();
            });

            battle.setup(17, true, false);
            battle.startBattle();
            REQUIRE(battle.getPhase() == BattlePhase::INPUT);
            bool escaped = false;
            int attempts = 0;
            while (attempts < 6 && battle.canEscape()) {
                ++attempts;
                escaped = battle.processEscape();
                if (escaped) {
                    break;
                }
            }

            REQUIRE(escaped);
            REQUIRE(attempts >= 1);
            REQUIRE(escapeHookCount == attempts + 1);
            REQUIRE(battle.getResult() == BattleResult::ESCAPE);
            REQUIRE(battle.getPhase() == BattlePhase::NONE);
        };

        Value beforeActionBoot;
        beforeActionBoot.v = std::string("before_reload_battle_action");
        const Value beforeAction =
            pm.executeCommand("CuratedBattleFlowReloadFixture", "engage", {beforeActionBoot});
        verifyActionRoute(beforeAction, "before_reload_battle_action", "slash");

        REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
        REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
        REQUIRE(pm.reloadPlugin("CuratedBattleFlowReloadFixture"));

        REQUIRE(pm.hasCommand("CuratedBattleFlowReloadFixture", "engage"));
        REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
        REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

        Value afterActionBoot;
        afterActionBoot.v = std::string("after_reload_battle_action");
        const Value afterAction =
            pm.executeCommandByName("CuratedBattleFlowReloadFixture_engage", {afterActionBoot});
        verifyActionRoute(afterAction, "after_reload_battle_action", "slash");

        Value escapeBoot;
        escapeBoot.v = std::string("after_reload_battle_escape");
        Value escapeRoute;
        escapeRoute.v = std::string("escape");
        Value retreatMotion;
        retreatMotion.v = std::string("retreat");
        const Value afterEscape =
            pm.executeCommand("CuratedBattleFlowReloadFixture", "engage", {escapeBoot, escapeRoute, retreatMotion});
        verifyEscapeRoute(afterEscape, "after_reload_battle_escape", "retreat");

        REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

        pm.clearFailureDiagnostics();
        pm.unloadAllPlugins();

        std::error_code ec;
        std::filesystem::remove(reloadFixture, ec);
    }

TEST_CASE("Compat fixtures: curated battle outcome-status scenarios survive plugin reload",
            "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_battle_outcome_reload_fixture");
    writeTextFile(
        reloadFixture,
        R"({
    "name": "CuratedBattleOutcomeReloadFixture",
    "parameters": {
    "defaultRoute": "victory",
    "defaultMotion": "triumph"
    },
    "commands": [
    {
        "name": "resolve",
        "script": [
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "battle_outcome_boot"}], "store": "boot", "expect": "non_nil"},
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}} ,
        {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultMotion"}}},
        {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "hud", "expect": "non_nil"},
        {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "motion", "expect": "non_nil"},
        {"op": "set", "key": "routeToken", "value": {"from": "concat", "parts": ["outcome:", {"from": "local", "name": "route"}]}} ,
        {"op": "returnObject"}
        ]
    }
    ]
})"
    );

    REQUIRE(pm.loadPlugin(reloadFixture.string()));

    auto verifyVictoryRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == "victory");
        REQUIRE(std::get<std::string>(object.at("routeToken").v) == "outcome:victory");
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("hud").v).at("profile").v) == "battle_hud");
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("motion").v).at("profile").v) == "character_motion");

        BattleManager battle;
        int stateAddedCount = 0;
        int stateRemovedCount = 0;
        int victoryCount = 0;
        std::vector<std::string> damageOrder;

        battle.registerHook(BattleManager::HookPoint::ON_STATE_ADDED, "battle_outcome_anchor", [&stateAddedCount](const std::vector<Value>&) {
            ++stateAddedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_STATE_REMOVED, "battle_outcome_anchor", [&stateRemovedCount](const std::vector<Value>&) {
            ++stateRemovedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_VICTORY, "battle_outcome_anchor", [&victoryCount](const std::vector<Value>&) {
            ++victoryCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_DAMAGE, "battle_outcome_anchor", [&damageOrder](const std::vector<Value>& args) {
            damageOrder.push_back(std::to_string(std::get<int64_t>(args[0].v)) + ":" + std::to_string(std::get<int64_t>(args[1].v)));
            return Value::Nil();
        });

        BattleSubject actor;
        actor.type = BattleSubjectType::ACTOR;
        actor.index = 0;
        actor.hp = 40;
        actor.mhp = 50;

        BattleSubject enemy;
        enemy.type = BattleSubjectType::ENEMY;
        enemy.index = 0;
        enemy.hp = 12;
        enemy.mhp = 12;

        battle.setup(19, true, false);
        battle.addActorSubject(actor);
        battle.addEnemySubject(enemy);
        BattleSubject* seededActor = battle.getActor(0);
        BattleSubject* seededEnemy = battle.getEnemy(0);
        REQUIRE(seededActor != nullptr);
        REQUIRE(seededEnemy != nullptr);
        REQUIRE(battle.addState(seededActor, 21, 1, 6, 0));
        REQUIRE(battle.addState(seededEnemy, 31, 1, -12, 0));
        REQUIRE(battle.addBuff(seededActor, 6, 2, 1));

        battle.startBattle();
        battle.startTurn();
        battle.endTurn();

        REQUIRE(seededActor->hp == 46);
        REQUIRE(seededEnemy->hp == 0);
        REQUIRE_FALSE(battle.hasState(seededActor, 21));
        REQUIRE_FALSE(battle.hasState(seededEnemy, 31));
        REQUIRE(battle.getModifierStage(seededActor, 6) == 1);
        REQUIRE(stateAddedCount == 2);
        REQUIRE(stateRemovedCount == 2);
        REQUIRE(damageOrder.size() == 1);
        REQUIRE(damageOrder[0] == "1:0");
        REQUIRE(victoryCount == 1);
        REQUIRE(battle.getResult() == BattleResult::WIN);
        REQUIRE(battle.getPhase() == BattlePhase::NONE);
    };

    auto verifyDefeatRoute = [&](const Value& value, const std::string& expectedBoot, const std::string& expectedMotion) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == "defeat");
        REQUIRE(std::get<std::string>(object.at("routeToken").v) == "outcome:defeat");
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("hud").v).at("profile").v) == "battle_hud");
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("motion").v).at("profile").v) == "character_motion");

        BattleManager battle;
        int stateAddedCount = 0;
        int stateRemovedCount = 0;
        int defeatCount = 0;
        int actorDeathCount = 0;

        battle.registerHook(BattleManager::HookPoint::ON_STATE_ADDED, "battle_outcome_anchor", [&stateAddedCount](const std::vector<Value>&) {
            ++stateAddedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_STATE_REMOVED, "battle_outcome_anchor", [&stateRemovedCount](const std::vector<Value>&) {
            ++stateRemovedCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_DEFEAT, "battle_outcome_anchor", [&defeatCount](const std::vector<Value>&) {
            ++defeatCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_ACTOR_DEATH, "battle_outcome_anchor", [&actorDeathCount](const std::vector<Value>&) {
            ++actorDeathCount;
            return Value::Nil();
        });

        BattleSubject actor;
        actor.type = BattleSubjectType::ACTOR;
        actor.index = 0;
        actor.hp = 9;
        actor.mhp = 20;

        battle.setup(23, true, true);
        battle.addActorSubject(actor);
        BattleSubject* seededActor = battle.getActor(0);
        REQUIRE(seededActor != nullptr);
        REQUIRE(battle.addState(seededActor, 41, 1, -12, 0));
        REQUIRE(battle.addDebuff(seededActor, 2, 1, 1));

        battle.startBattle();
        battle.startTurn();
        battle.endTurn();

        REQUIRE(seededActor->hp == 0);
        REQUIRE_FALSE(battle.hasState(seededActor, 41));
        REQUIRE(battle.getModifierStage(seededActor, 2) == 0);
        REQUIRE(stateAddedCount == 1);
        REQUIRE(stateRemovedCount == 1);
        REQUIRE(actorDeathCount == 1);
        REQUIRE(defeatCount == 1);
        REQUIRE(battle.getResult() == BattleResult::DEFEAT);
        REQUIRE(battle.getPhase() == BattlePhase::NONE);
    };

    Value beforeReloadBoot;
    beforeReloadBoot.v = std::string("before_reload_battle_victory");
    const Value beforeReloadVictory =
        pm.executeCommand("CuratedBattleOutcomeReloadFixture", "resolve", {beforeReloadBoot});
    verifyVictoryRoute(beforeReloadVictory, "before_reload_battle_victory", "triumph");

    REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
    REQUIRE(pm.reloadPlugin("CuratedBattleOutcomeReloadFixture"));

    REQUIRE(pm.hasCommand("CuratedBattleOutcomeReloadFixture", "resolve"));
    REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
    REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

    Value afterReloadVictoryBoot;
    afterReloadVictoryBoot.v = std::string("after_reload_battle_victory");
    const Value afterReloadVictory =
        pm.executeCommandByName("CuratedBattleOutcomeReloadFixture_resolve", {afterReloadVictoryBoot});
    verifyVictoryRoute(afterReloadVictory, "after_reload_battle_victory", "triumph");

    Value defeatBoot;
    defeatBoot.v = std::string("after_reload_battle_defeat");
    Value defeatRoute;
    defeatRoute.v = std::string("defeat");
    Value defeatMotion;
    defeatMotion.v = std::string("collapse");
    const Value afterReloadDefeat =
        pm.executeCommand("CuratedBattleOutcomeReloadFixture", "resolve", {defeatBoot, defeatRoute, defeatMotion});
    verifyDefeatRoute(afterReloadDefeat, "after_reload_battle_defeat", "collapse");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(reloadFixture, ec);
}

TEST_CASE("Compat fixtures: curated battle tactical-routing scenarios survive plugin reload",
          "[compat][fixtures]") {
    PluginManager& pm = PluginManager::instance();
    pm.unloadAllPlugins();
    pm.clearFailureDiagnostics();

    REQUIRE(pm.loadPlugin(fixturePath("VisuStella_CoreEngine_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_BattleHud_MZ").string()));
    REQUIRE(pm.loadPlugin(fixturePath("MOG_CharacterMotion_MZ").string()));

    const auto reloadFixture = uniqueTempFixturePath("urpg_curated_battle_tactical_reload_fixture");
    writeTextFile(
        reloadFixture,
        R"({
    "name": "CuratedBattleTacticalReloadFixture",
    "parameters": {
    "defaultRoute": "tactical",
    "defaultMotion": "feint"
    },
    "commands": [
    {
        "name": "execute",
        "script": [
        {"op": "invoke", "plugin": "VisuStella_CoreEngine_MZ", "command": "boot", "args": [{"from": "arg", "index": 0, "default": "battle_tactical_boot"}], "store": "boot", "expect": "non_nil"},
        {"op": "set", "key": "route", "value": {"from": "coalesce", "values": [{"from": "arg", "index": 1}, {"from": "param", "name": "defaultRoute"}]}},
        {"op": "set", "key": "motionName", "value": {"from": "arg", "index": 2, "default": {"from": "param", "name": "defaultMotion"}}},
        {"op": "invoke", "plugin": "MOG_BattleHud_MZ", "command": "showHud", "store": "hud", "expect": "non_nil"},
        {"op": "invoke", "plugin": "MOG_CharacterMotion_MZ", "command": "startMotion", "args": [{"from": "local", "name": "motionName"}], "store": "motion", "expect": "non_nil"},
        {"op": "if", "condition": {"from": "equals", "left": {"from": "local", "name": "route"}, "right": "forced"},
            "then": [{"op": "set", "key": "routeToken", "value": "forced:queue"}],
            "else": [{"op": "set", "key": "routeToken", "value": "tactical:queue"}]
        },
        {"op": "returnObject"}
        ]
    }
    ]
})"
    );

    REQUIRE(pm.loadPlugin(reloadFixture.string()));

    auto verifyTacticalRoute = [&](const Value& value,
                                   const std::string& expectedBoot,
                                   const std::string& expectedRoute,
                                   const std::string& expectedToken,
                                   const std::string& expectedMotion) {
        REQUIRE(std::holds_alternative<Object>(value.v));
        const auto& object = std::get<Object>(value.v);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("boot").v).at("firstArg").v) == expectedBoot);
        REQUIRE(std::get<std::string>(object.at("route").v) == expectedRoute);
        REQUIRE(std::get<std::string>(object.at("routeToken").v) == expectedToken);
        REQUIRE(std::get<std::string>(object.at("motionName").v) == expectedMotion);
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("hud").v).at("profile").v) == "battle_hud");
        REQUIRE(std::get<std::string>(std::get<Object>(object.at("motion").v).at("profile").v) == "character_motion");

        BattleManager battle;
        int actionStartCount = 0;
        int actionEndCount = 0;
        int damageCount = 0;
        battle.registerHook(BattleManager::HookPoint::ON_ACTION_START, "battle_tactical_anchor", [&actionStartCount](const std::vector<Value>&) {
            ++actionStartCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_ACTION_END, "battle_tactical_anchor", [&actionEndCount](const std::vector<Value>&) {
            ++actionEndCount;
            return Value::Nil();
        });
        battle.registerHook(BattleManager::HookPoint::ON_DAMAGE, "battle_tactical_anchor", [&damageCount](const std::vector<Value>&) {
            ++damageCount;
            return Value::Nil();
        });

        BattleSubject actor;
        actor.type = BattleSubjectType::ACTOR;
        actor.index = 0;
        actor.hp = 120;
        actor.mhp = 120;
        actor.actionSpeed = 16;

        BattleSubject enemy;
        enemy.type = BattleSubjectType::ENEMY;
        enemy.index = 0;
        enemy.hp = 100;
        enemy.mhp = 100;
        enemy.actionSpeed = 8;

        battle.setup(31, true, false);
        battle.addActorSubject(actor);
        battle.addEnemySubject(enemy);
        BattleSubject* seededActor = battle.getActor(0);
        BattleSubject* seededEnemy = battle.getEnemy(0);
        REQUIRE(seededActor != nullptr);
        REQUIRE(seededEnemy != nullptr);
        REQUIRE(battle.addBuff(seededActor, 2, 2, 2));
        REQUIRE(battle.addDebuff(seededEnemy, 3, 2, 1));

        battle.startBattle();
        battle.startTurn();
        REQUIRE(battle.getPhase() == BattlePhase::TURN);

        battle.setActorAction(0, BattleActionType::ATTACK, -1);
        battle.forceAction(0, BattleSubjectType::ACTOR, BattleActionType::ATTACK, -1);
        battle.sortActionsBySpeed();

        int processedActions = 0;
        while (BattleAction* action = battle.getNextAction()) {
            battle.processAction(action);
            ++processedActions;
        }
        REQUIRE(processedActions == 2);
        REQUIRE(actionStartCount == 2);
        REQUIRE(actionEndCount == 2);
        REQUIRE(damageCount == 2);
        REQUIRE(seededEnemy->hp < 100);
        REQUIRE(battle.getModifierStage(seededActor, 2) == 2);
        REQUIRE(battle.getModifierStage(seededEnemy, 3) == -1);

        REQUIRE(battle.calculateExp() == 10);
        REQUIRE(battle.calculateGold() == 5);

        auto computeDrops = []() {
            BattleManager dropsBattle;
            dropsBattle.setup(37, true, false);
            for (int i = 0; i < 3; ++i) {
                BattleSubject enemyDrop;
                enemyDrop.type = BattleSubjectType::ENEMY;
                enemyDrop.index = i;
                enemyDrop.hp = 30;
                enemyDrop.mhp = 30;
                dropsBattle.addEnemySubject(enemyDrop);
            }
            return dropsBattle.calculateDrops();
        };
        REQUIRE(computeDrops() == computeDrops());

        battle.endTurn();
        REQUIRE(battle.getTurnCount() == 1);
        REQUIRE(battle.getPhase() == BattlePhase::INPUT);
    };

    Value beforeReloadBoot;
    beforeReloadBoot.v = std::string("before_reload_battle_tactical");
    const Value beforeReload =
        pm.executeCommand("CuratedBattleTacticalReloadFixture", "execute", {beforeReloadBoot});
    verifyTacticalRoute(beforeReload,
                        "before_reload_battle_tactical",
                        "tactical",
                        "tactical:queue",
                        "feint");

    REQUIRE(pm.reloadPlugin("VisuStella_CoreEngine_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_BattleHud_MZ"));
    REQUIRE(pm.reloadPlugin("MOG_CharacterMotion_MZ"));
    REQUIRE(pm.reloadPlugin("CuratedBattleTacticalReloadFixture"));

    REQUIRE(pm.hasCommand("CuratedBattleTacticalReloadFixture", "execute"));
    REQUIRE(pm.hasCommand("MOG_BattleHud_MZ", "showHud"));
    REQUIRE(pm.hasCommand("MOG_CharacterMotion_MZ", "startMotion"));

    Value afterReloadBoot;
    afterReloadBoot.v = std::string("after_reload_battle_tactical");
    Value forcedRoute;
    forcedRoute.v = std::string("forced");
    Value forcedMotion;
    forcedMotion.v = std::string("parry");
    const Value afterReload =
        pm.executeCommandByName("CuratedBattleTacticalReloadFixture_execute",
                                {afterReloadBoot, forcedRoute, forcedMotion});
    verifyTacticalRoute(afterReload,
                        "after_reload_battle_tactical",
                        "forced",
                        "forced:queue",
                        "parry");

    REQUIRE(pm.exportFailureDiagnosticsJsonl().empty());

    pm.clearFailureDiagnostics();
    pm.unloadAllPlugins();

    std::error_code ec;
    std::filesystem::remove(reloadFixture, ec);
}
