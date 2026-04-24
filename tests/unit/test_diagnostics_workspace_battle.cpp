#include "editor/diagnostics/diagnostics_workspace.h"
#include "editor/diagnostics/diagnostics_facade.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/input/input_core.h"
#include "engine/core/message/message_core.h"
#include "engine/core/scene/battle_scene.h"
#include "engine/core/ui/menu_command_registry.h"
#include "engine/core/ui/menu_scene_graph.h"

#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/plugin_manager.h"

#include <catch2/catch_test_macros.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>

namespace {

urpg::scene::BattleParticipant* findParticipant(std::vector<urpg::scene::BattleParticipant>& participants, bool is_enemy) {
    for (auto& participant : participants) {
        if (participant.isEnemy == is_enemy) {
            return &participant;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("DiagnosticsWorkspace - Battle tab exports live scene diagnostics preview payload",
          "[editor][diagnostics][integration][battle_preview]") {
    auto& data_manager = urpg::compat::DataManager::instance();
    REQUIRE(data_manager.loadDatabase());
    data_manager.setupNewGame();

    urpg::scene::BattleScene battle({"2"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0, 0}, nullptr);
    battle.addEnemy("2", "Goblin", 50, 0, {100, 100}, nullptr);
    battle.setPhase(urpg::scene::BattlePhase::ACTION);
    battle.flowController().noteEscapeFailure();

    auto& participants = const_cast<std::vector<urpg::scene::BattleParticipant>&>(battle.getParticipants());
    auto* hero = findParticipant(participants, false);
    auto* goblin = findParticipant(participants, true);
    REQUIRE(hero != nullptr);
    REQUIRE(goblin != nullptr);

    urpg::scene::BattleScene::BattleAction attack{};
    attack.subject = hero;
    attack.target = goblin;
    attack.command = "attack";
    battle.addActionToQueue(attack);

    const auto preview = battle.buildDiagnosticsPreview();
    REQUIRE(preview.has_value());

    const auto expected_physical_damage = urpg::battle::BattleRuleResolver::resolveDamage(preview->physical_preview);
    const auto expected_magical_damage = urpg::battle::BattleRuleResolver::resolveDamage(preview->magical_preview);
    const auto expected_escape_ratio_now = urpg::battle::BattleRuleResolver::resolveEscapeRatio(
        preview->party_agi, preview->troop_agi, battle.flowController().escapeFailures());
    const auto expected_escape_ratio_next = urpg::battle::BattleRuleResolver::resolveEscapeRatio(
        preview->party_agi, preview->troop_agi, battle.flowController().escapeFailures() + 1);

    urpg::editor::DiagnosticsWorkspace workspace;
    workspace.bindBattleRuntime(battle);
    workspace.setActiveTab(urpg::editor::DiagnosticsTab::Battle);
    workspace.update();

    const auto exported = nlohmann::json::parse(workspace.exportAsJson());
    REQUIRE(exported["active_tab"] == "battle");
    REQUIRE(exported["active_tab_detail"]["tab"] == "battle");
    REQUIRE(exported["active_tab_detail"]["battle_summary"]["phase"] == "action");
    REQUIRE(exported["active_tab_detail"]["battle_summary"]["total_actions"] == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"].size() == 1);
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["subject_id"] == "1");
    REQUIRE(exported["active_tab_detail"]["visible_rows"][0]["target_id"] == "2");
    REQUIRE(exported["active_tab_detail"]["preview"]["physical_damage"] == expected_physical_damage);
    REQUIRE(exported["active_tab_detail"]["preview"]["magical_damage"] == expected_magical_damage);
    REQUIRE(exported["active_tab_detail"]["preview"]["escape_ratio_now"] == expected_escape_ratio_now);
    REQUIRE(exported["active_tab_detail"]["preview"]["escape_ratio_next_fail"] == expected_escape_ratio_next);
}

