#include "engine/gameplay/combat/combat_calc.h"
#include "engine/core/scene/combat_formula.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("Physical damage formula baseline", "[combat]") {
    urpg::CombatCalc calc;

    urpg::ActorStats attacker;
    attacker.level = 10;
    attacker.atk = urpg::Fixed32::FromInt(100);
    attacker.def = urpg::Fixed32::FromInt(0);

    urpg::ActorStats defender;
    defender.level = 10;
    defender.atk = urpg::Fixed32::FromInt(0);
    defender.def = urpg::Fixed32::FromInt(50);

    const auto result = calc.PhysicalDamage(attacker, defender, 0);

    REQUIRE(result.damage == 50);
    REQUIRE_FALSE(result.critical);
}

namespace {

urpg::scene::BattleParticipant makeFormulaParticipant(const std::string& id, bool is_enemy) {
    urpg::scene::BattleParticipant participant;
    participant.id = id;
    participant.name = is_enemy ? "Target" : "Subject";
    participant.hp = 100;
    participant.maxHp = 100;
    participant.mp = 25;
    participant.maxMp = 25;
    participant.isEnemy = is_enemy;
    return participant;
}

} // namespace

TEST_CASE("CombatFormula evaluates the supported arithmetic subset", "[combat][battle]") {
    auto subject = makeFormulaParticipant("-1", false);
    auto target = makeFormulaParticipant("-1", true);
    const urpg::combat::CombatFormula::Context ctx{&subject, &target, nullptr, nullptr};

    const auto result = urpg::combat::CombatFormula::evaluateFormula("4 * a.atk - 2 * b.def + 3", ctx);

    REQUIRE_FALSE(result.usedFallback);
    REQUIRE(result.reason.empty());
    REQUIRE(result.normalizedFormula == "4 * 10 - 2 * 10 + 3");
    REQUIRE(result.value == 23);
    REQUIRE(urpg::combat::CombatFormula::parseFormula("4 * a.atk - 2 * b.def + 3", ctx) == 23);
}

TEST_CASE("CombatFormula flags unsupported symbols instead of silently pretending they parsed", "[combat][battle]") {
    auto subject = makeFormulaParticipant("-1", false);
    auto target = makeFormulaParticipant("-1", true);
    const urpg::combat::CombatFormula::Context ctx{&subject, &target, nullptr, nullptr};

    const auto result = urpg::combat::CombatFormula::evaluateFormula("a.customStat * 2", ctx);

    REQUIRE(result.usedFallback);
    REQUIRE(result.reason == "unsupported_formula_symbol:a.customStat");
    REQUIRE(result.value == urpg::combat::CombatFormula::evaluateDamage(ctx));
    REQUIRE(result.normalizedFormula.empty());
}

TEST_CASE("CombatFormula resolves common MZ battler value symbols", "[combat][battle]") {
    urpg::compat::DataManager::setDataDirectory("");
    auto& data = urpg::compat::DataManager::instance();
    REQUIRE(data.loadDatabase());
    data.setupNewGame();
    auto* actor = data.getActor(1);
    REQUIRE(actor != nullptr);
    actor->level = 7;

    auto subject = makeFormulaParticipant("1", false);
    subject.hp = 44;
    subject.maxHp = 100;
    subject.mp = 9;
    subject.maxMp = 30;
    auto target = makeFormulaParticipant("1", true);
    target.hp = 12;
    target.maxHp = 30;
    target.mp = 3;
    target.maxMp = 0;
    const urpg::combat::CombatFormula::Context ctx{&subject, &target, nullptr, nullptr};

    const auto result =
        urpg::combat::CombatFormula::evaluateFormula("a.level + a.hp + a.mhp + a.mp + a.mmp + b.hp + b.mhp", ctx);

    REQUIRE_FALSE(result.usedFallback);
    REQUIRE(result.value == 7 + 44 + 100 + 9 + 30 + 12 + 30);
}

TEST_CASE("CombatFormula flags malformed expressions instead of silently degrading", "[combat][battle]") {
    auto subject = makeFormulaParticipant("-1", false);
    auto target = makeFormulaParticipant("-1", true);
    const urpg::combat::CombatFormula::Context ctx{&subject, &target, nullptr, nullptr};

    const auto result = urpg::combat::CombatFormula::evaluateFormula("a.atk +", ctx);

    REQUIRE(result.usedFallback);
    REQUIRE(result.reason == "malformed_formula_expression");
    REQUIRE(result.value == urpg::combat::CombatFormula::evaluateDamage(ctx));
}
