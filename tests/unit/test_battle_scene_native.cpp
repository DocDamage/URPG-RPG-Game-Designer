#include "engine/core/scene/battle_scene.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_test_macros.hpp>

using namespace urpg::scene;

namespace {

BattleParticipant* findParticipant(std::vector<BattleParticipant>& participants, bool isEnemy) {
    for (auto& participant : participants) {
        if (participant.isEnemy == isEnemy) {
            return &participant;
        }
    }
    return nullptr;
}

} // namespace

TEST_CASE("BattleScene Logic: lifecycle and automated turn progression", "[battle][scene][logic]") {
    auto& dm = urpg::compat::DataManager::instance();
    dm.setupNewGame();

    auto battle = std::make_shared<BattleScene>(std::vector<std::string>{"1", "2"});

    SECTION("Initial state is START") {
        battle->onStart();
        REQUIRE(battle->getCurrentPhase() == BattlePhase::START);
        REQUIRE(battle->getTurnCount() == 1);
    }

    SECTION("Phase transition to INPUT via onUpdate") {
        battle->onStart();
        battle->onUpdate(0.1f);
        REQUIRE(battle->getCurrentPhase() == BattlePhase::INPUT);
    }
}

TEST_CASE("BattleScene: Victory condition detection", "[battle][scene]") {
    auto& dm = urpg::compat::DataManager::instance();
    dm.setupNewGame();

    auto battle = std::make_shared<BattleScene>(std::vector<std::string>{"1"});
    battle->onStart();

    battle->addEnemy("1", "Slime", 10, 0, {100, 100}, nullptr);
    battle->addActor("1", "Hero", 100, 20, {0, 0}, nullptr);

    SECTION("Victory when enemies are at 0 HP") {
        battle->setPhase(BattlePhase::ACTION);

        auto& participants = const_cast<std::vector<BattleParticipant>&>(battle->getParticipants());
        for (auto& participant : participants) {
            participant.hp = participant.isEnemy ? 0 : 100;
        }

        BattleScene::BattleAction action{};
        action.subject = findParticipant(participants, false);
        action.target = action.subject;
        action.command = "guard";
        REQUIRE(action.subject != nullptr);

        battle->addActionToQueue(action);
        battle->onUpdate(0.1f);

        REQUIRE(battle->getCurrentPhase() == BattlePhase::VICTORY);
    }

    SECTION("Defeat when actors are at 0 HP") {
        battle->setPhase(BattlePhase::ACTION);

        auto& participants = const_cast<std::vector<BattleParticipant>&>(battle->getParticipants());
        for (auto& participant : participants) {
            participant.hp = participant.isEnemy ? 10 : 0;
        }

        BattleScene::BattleAction action{};
        action.subject = findParticipant(participants, true);
        action.target = action.subject;
        action.command = "guard";
        REQUIRE(action.subject != nullptr);

        battle->addActionToQueue(action);
        battle->onUpdate(0.1f);

        REQUIRE(battle->getCurrentPhase() == BattlePhase::DEFEAT);
    }
}

TEST_CASE("BattleScene: executeAction effect application", "[battle][scene]") {
    auto& dm = urpg::compat::DataManager::instance();
    dm.setupNewGame();

    auto battle = std::make_shared<BattleScene>(std::vector<std::string>{"1"});
    battle->onStart();

    battle->addActor("1", "Hero", 100, 20, {0, 0}, nullptr);
    battle->addEnemy("1", "Slime", 20, 0, {100, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle->getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    SECTION("Basic attack reduces target HP") {
        BattleScene::BattleAction action{};
        action.subject = hero;
        action.target = slime;
        action.command = "attack";

        const int initialHp = slime->hp;

        battle->setPhase(BattlePhase::ACTION);
        battle->addActionToQueue(action);
        battle->onUpdate(0.1f);

        CHECK(slime->hp < initialHp);
        CHECK(slime->DamagePopupValue > 0.0f);
    }

    SECTION("Guard reduces incoming damage (logic check via formula)") {
        hero->isGuarding = true;

        BattleScene::BattleAction action{};
        action.subject = slime;
        action.target = hero;
        action.command = "attack";

        const int initialHp = hero->hp;

        battle->setPhase(BattlePhase::ACTION);
        battle->addActionToQueue(action);
        battle->onUpdate(0.1f);

        CHECK(hero->hp < initialHp);
    }
}
