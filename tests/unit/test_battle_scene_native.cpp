#include "engine/core/scene/battle_scene.h"
#include "engine/core/battle/battle_core.h"
#include "engine/core/sprite_batcher.h"
#include "runtimes/compat_js/battle_manager.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>
#include <algorithm>
#include <sstream>
#include <iostream>

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

BattleParticipant* findParticipantById(std::vector<BattleParticipant>& participants, const std::string& id, bool isEnemy) {
    for (auto& participant : participants) {
        if (participant.isEnemy == isEnemy && participant.id == id) {
            return &participant;
        }
    }
    return nullptr;
}

std::vector<std::vector<int32_t>> makeActorParams(int32_t mhp,
                                                  int32_t mmp,
                                                  int32_t atk,
                                                  int32_t def,
                                                  int32_t mat,
                                                  int32_t mdf,
                                                  int32_t agi,
                                                  int32_t luk) {
    return {
        {mhp, mhp},
        {mmp, mmp},
        {atk, atk},
        {def, def},
        {mat, mat},
        {mdf, mdf},
        {agi, agi},
        {luk, luk},
    };
}

urpg::battle::BattleDamageContext buildAttackContext(const BattleParticipant& subject, const BattleParticipant& target) {
    auto& dm = urpg::compat::DataManager::instance();
    urpg::battle::BattleDamageContext context;
    context.target.hp = target.hp;
    context.target.guarding = target.isGuarding;

    const int subjectId = std::stoi(subject.id);
    const int targetId = std::stoi(target.id);
    if (subject.isEnemy) {
        const auto* enemy = dm.getEnemy(subjectId);
        context.subject.atk = enemy ? enemy->atk : 1;
        context.subject.mat = enemy ? enemy->mat : 1;
        context.subject.agi = enemy ? enemy->agi : 1;
    } else {
        context.subject.atk = dm.getActorParam(subjectId, 2, 1);
        context.subject.mat = dm.getActorParam(subjectId, 4, 1);
        context.subject.agi = dm.getActorParam(subjectId, 6, 1);
    }

    if (target.isEnemy) {
        const auto* enemy = dm.getEnemy(targetId);
        context.target.def = enemy ? enemy->def : 1;
        context.target.mdf = enemy ? enemy->mdf : 1;
    } else {
        context.target.def = dm.getActorParam(targetId, 3, 1);
        context.target.mdf = dm.getActorParam(targetId, 5, 1);
    }

    return context;
}

float colorChannel(uint32_t color, int shift) {
    return static_cast<float>((color >> shift) & 0xFFu) / 255.0f;
}

bool batchContainsColor(const urpg::SpriteBatcher& batcher, uint32_t color) {
    const float expectedR = colorChannel(color, 24);
    const float expectedG = colorChannel(color, 16);
    const float expectedB = colorChannel(color, 8);
    const float expectedA = colorChannel(color, 0);

    for (const auto& drawData : batcher.getBatches()) {
        if (drawData.vertices.empty()) {
            continue;
        }

        const auto& vertex = drawData.vertices.front();
        if (vertex.color[0] == Catch::Approx(expectedR) &&
            vertex.color[1] == Catch::Approx(expectedG) &&
            vertex.color[2] == Catch::Approx(expectedB) &&
            vertex.color[3] == Catch::Approx(expectedA)) {
            return true;
        }

        for (const auto& candidate : drawData.vertices) {
            if (candidate.color[0] == Catch::Approx(expectedR) &&
                candidate.color[1] == Catch::Approx(expectedG) &&
                candidate.color[2] == Catch::Approx(expectedB) &&
                candidate.color[3] == Catch::Approx(expectedA)) {
                return true;
            }
        }
    }

    return false;
}

} // namespace

TEST_CASE("BattleScene Logic: lifecycle and automated turn progression", "[battle][scene][logic]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
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

TEST_CASE("BattleScene: missing battleback emits a named diagnostic during startup", "[battle][scene][assets]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    auto battle = std::make_shared<BattleScene>(std::vector<std::string>{"1"});

    std::ostringstream captured;
    auto* originalBuffer = std::cerr.rdbuf(captured.rdbuf());
    battle->onStart();
    std::cerr.rdbuf(originalBuffer);

    REQUIRE(captured.str().find("MISSING_BATTLEBACK") != std::string::npos);
    REQUIRE(captured.str().find("img/battlebacks1/Grassland.png") != std::string::npos);
}

TEST_CASE("BattleScene: compat-selected battleback path is attempted before fallback", "[battle][scene][assets]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    urpg::compat::BattleManager::instance().changeBattleBackground("ruins_night");
    auto battle = std::make_shared<BattleScene>(std::vector<std::string>{"1"});

    std::ostringstream captured;
    auto* originalBuffer = std::cerr.rdbuf(captured.rdbuf());
    battle->onStart();
    std::cerr.rdbuf(originalBuffer);

    REQUIRE(captured.str().find("MISSING_BATTLEBACK") != std::string::npos);
    REQUIRE(captured.str().find("img/battlebacks1/ruins_night.png") != std::string::npos);
    REQUIRE(captured.str().find("img/battlebacks1/Grassland.png") != std::string::npos);

    urpg::compat::BattleManager::instance().changeBattleBackground("");
}

TEST_CASE("BattleScene routes live phase progression through Battle Core flow state", "[battle][scene][logic]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();

    REQUIRE(battle.flowController().phase() == urpg::battle::BattleFlowPhase::Start);

    battle.onUpdate(0.1f);

    REQUIRE(battle.flowController().phase() == urpg::battle::BattleFlowPhase::Input);
    REQUIRE(battle.getCurrentPhase() == BattlePhase::INPUT);
}

TEST_CASE("BattleScene: Victory condition detection", "[battle][scene]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
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
    REQUIRE(dm.loadDatabase());
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

    SECTION("Basic attack uses native battle core damage rules") {
        BattleScene::BattleAction action{};
        action.subject = hero;
        action.target = slime;
        action.command = "attack";

        const int initialHp = slime->hp;
        const auto expectedDamage = urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*hero, *slime));

        battle->setPhase(BattlePhase::ACTION);
        battle->addActionToQueue(action);
        battle->onUpdate(0.1f);

        CHECK(slime->hp == initialHp - expectedDamage);
        CHECK(slime->DamagePopupValue > 0.0f);
    }

    SECTION("Guarded targets use native battle core mitigation") {
        hero->isGuarding = true;

        BattleScene::BattleAction action{};
        action.subject = slime;
        action.target = hero;
        action.command = "attack";

        const int initialHp = hero->hp;
        const auto expectedDamage = urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*slime, *hero));

        battle->setPhase(BattlePhase::ACTION);
        battle->addActionToQueue(action);
        battle->onUpdate(0.1f);

        CHECK(hero->hp == initialHp - expectedDamage);
    }
}

TEST_CASE("BattleScene drains queued actions through native battle action ordering", "[battle][scene]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    BattleScene battle({"2"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0, 0}, nullptr);
    battle.addEnemy("2", "Goblin", 50, 0, {100, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* goblin = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(goblin != nullptr);

    BattleScene::BattleAction heroAttack{};
    heroAttack.subject = hero;
    heroAttack.target = goblin;
    heroAttack.command = "attack";

    BattleScene::BattleAction goblinAttack{};
    goblinAttack.subject = goblin;
    goblinAttack.target = hero;
    goblinAttack.command = "attack";

    const int initialHeroHp = hero->hp;
    const int initialGoblinHp = goblin->hp;
    const auto expectedHeroDamage = urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*goblin, *hero));
    const auto expectedGoblinDamage = urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*hero, *goblin));

    battle.addActionToQueue(heroAttack);
    battle.addActionToQueue(goblinAttack);

    REQUIRE(battle.nativeActionQueue().size() == 2);
    const auto ordered = battle.nativeActionQueue().snapshotOrdered();
    REQUIRE(ordered.size() == 2);

    battle.setPhase(BattlePhase::ACTION);
    battle.onUpdate(0.1f);

    REQUIRE(battle.nativeActionQueue().size() == 1);
    if (ordered[0].subject_id == goblin->id) {
        REQUIRE(hero->hp == initialHeroHp - expectedHeroDamage);
        REQUIRE(goblin->hp == initialGoblinHp);
    } else {
        REQUIRE(goblin->hp == initialGoblinHp - expectedGoblinDamage);
        REQUIRE(hero->hp == initialHeroHp);
    }

    battle.onUpdate(1.0f);

    REQUIRE(battle.nativeActionQueue().empty());
    REQUIRE(goblin->hp == initialGoblinHp - expectedGoblinDamage);
}

TEST_CASE("BattleScene builds diagnostics preview from the next ordered queued action", "[battle][scene][diagnostics]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    auto* actorData = dm.getActor(1);
    REQUIRE(actorData != nullptr);
    actorData->params = makeActorParams(120, 40, 18, 12, 29, 11, 30, 9);

    auto* skillData = dm.getSkill(2);
    REQUIRE(skillData != nullptr);
    skillData->damage.type = 2;
    skillData->damage.power = 33;
    skillData->damage.variance = 15;
    skillData->damage.canCrit = true;

    BattleScene battle({"1", "2"});
    battle.onStart();
    battle.addActor("1", "Hero", 120, 40, {0, 0}, nullptr);
    battle.addEnemy("1", "Slime", 30, 0, {100, 100}, nullptr);
    battle.addEnemy("2", "Goblin", 50, 0, {140, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipantById(participants, "1", false);
    BattleParticipant* slime = findParticipantById(participants, "1", true);
    BattleParticipant* goblin = findParticipantById(participants, "2", true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);
    REQUIRE(goblin != nullptr);

    slime->isGuarding = true;

    BattleScene::BattleAction slowerEnemyAttack{};
    slowerEnemyAttack.subject = goblin;
    slowerEnemyAttack.target = hero;
    slowerEnemyAttack.command = "attack";

    BattleScene::BattleAction fasterHeroSkill{};
    fasterHeroSkill.subject = hero;
    fasterHeroSkill.target = slime;
    fasterHeroSkill.command = "skill";
    fasterHeroSkill.isSkill = true;
    fasterHeroSkill.skillId = 2;

    battle.addActionToQueue(slowerEnemyAttack);
    battle.addActionToQueue(fasterHeroSkill);

    const auto preview = battle.buildDiagnosticsPreview();
    REQUIRE(preview.has_value());

    CHECK(preview->physical_preview.power == 33);
    CHECK(preview->physical_preview.variance_percent == 15);
    CHECK(preview->physical_preview.critical);
    CHECK_FALSE(preview->physical_preview.magical);
    CHECK(preview->physical_preview.subject.atk == dm.getActorParam(1, 2, 1));
    CHECK(preview->physical_preview.subject.agi == dm.getActorParam(1, 6, 1));
    CHECK(preview->physical_preview.target.def == dm.getEnemy(1)->def);
    CHECK(preview->physical_preview.target.guarding);

    CHECK(preview->magical_preview.power == 33);
    CHECK(preview->magical_preview.variance_percent == 15);
    CHECK(preview->magical_preview.critical);
    CHECK(preview->magical_preview.magical);
    CHECK(preview->magical_preview.subject.mat == dm.getActorParam(1, 4, 1));
    CHECK(preview->magical_preview.target.mdf == dm.getEnemy(1)->mdf);
    CHECK(preview->magical_preview.target.hp == slime->hp);

    CHECK(preview->party_agi == dm.getActorParam(1, 6, 1));
    CHECK(preview->troop_agi == dm.getEnemy(1)->agi + dm.getEnemy(2)->agi);
}

TEST_CASE("BattleScene routes actor skills through participant ability runtimes", "[battle][scene][ability]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    auto* actorData = dm.getActor(1);
    auto* skillData = dm.getSkill(2);
    REQUIRE(actorData != nullptr);
    REQUIRE(skillData != nullptr);

    const auto originalSkills = actorData->skills;
    const int originalCost = skillData->mpCost;

    actorData->skills = {2};
    skillData->mpCost = 7;

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 120, 20, {0, 0}, nullptr);
    battle.addEnemy("1", "Slime", 40, 0, {100, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipantById(participants, "1", false);
    BattleParticipant* slime = findParticipantById(participants, "1", true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);
    REQUIRE(hero->abilitySystem.getAbilities().size() == 1);

    SECTION("Granted skill activates through the participant ASC and spends MP once") {
        BattleScene::BattleAction action{};
        action.subject = hero;
        action.target = slime;
        action.command = "skill";
        action.isSkill = true;
        action.skillId = 2;

        const int initialMp = hero->mp;
        const int initialHp = slime->hp;

        battle.setPhase(BattlePhase::ACTION);
        battle.addActionToQueue(action);
        battle.onUpdate(0.1f);

        CHECK(hero->mp == initialMp - skillData->mpCost);
        CHECK(slime->hp < initialHp);
        REQUIRE_FALSE(hero->abilitySystem.getAbilityExecutionHistory().empty());
        const auto& lastRecord = hero->abilitySystem.getAbilityExecutionHistory().back();
        CHECK(lastRecord.ability_id == "skill.2");
        CHECK(lastRecord.outcome == "executed");
        CHECK(lastRecord.mp_before == Catch::Approx(static_cast<float>(initialMp)));
        CHECK(lastRecord.mp_after == Catch::Approx(static_cast<float>(initialMp - skillData->mpCost)));
    }

    SECTION("Granted skill is blocked when the participant ASC lacks enough MP") {
        hero->mp = 3;

        BattleScene::BattleAction action{};
        action.subject = hero;
        action.target = slime;
        action.command = "skill";
        action.isSkill = true;
        action.skillId = 2;

        const int initialHp = slime->hp;

        battle.setPhase(BattlePhase::ACTION);
        battle.addActionToQueue(action);
        battle.onUpdate(0.1f);

        CHECK(hero->mp == 3);
        CHECK(slime->hp == initialHp);
        REQUIRE_FALSE(hero->abilitySystem.getAbilityExecutionHistory().empty());
        const auto& lastRecord = hero->abilitySystem.getAbilityExecutionHistory().back();
        CHECK(lastRecord.ability_id == "skill.2");
        CHECK(lastRecord.outcome == "blocked");
        CHECK(lastRecord.reason == "insufficient_mp");
    }

    actorData->skills = originalSkills;
    skillData->mpCost = originalCost;
}

TEST_CASE("BattleScene applies target-aware skill effects after ability activation", "[battle][scene][ability][targeting]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    auto* actorData = dm.getActor(1);
    auto* skillData = dm.getSkill(2);
    REQUIRE(actorData != nullptr);
    REQUIRE(skillData != nullptr);

    const auto originalSkills = actorData->skills;
    const int originalCost = skillData->mpCost;
    const int originalDamageType = skillData->damage.type;
    const int originalDamagePower = skillData->damage.power;
    const auto originalEffects = skillData->effects;

    actorData->skills = {2};
    skillData->mpCost = 4;
    skillData->damage.type = 0;
    skillData->damage.power = 0;
    skillData->effects.clear();
    skillData->effects.push_back({11, 0, 0.0, 12.0}); // Recover HP
    skillData->effects.push_back({21, 9, 0.0, 0.0});  // Add state

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 120, 20, {0, 0}, nullptr);
    battle.addEnemy("1", "Slime", 40, 0, {100, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipantById(participants, "1", false);
    BattleParticipant* slime = findParticipantById(participants, "1", true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    slime->hp = 10;
    slime->states.clear();

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "skill";
    action.isSkill = true;
    action.skillId = 2;

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    CHECK(hero->mp == 16);
    CHECK(slime->hp == 12);
    CHECK(slime->hp > 10);
    CHECK(std::find(slime->states.begin(), slime->states.end(), 9) != slime->states.end());
    REQUIRE_FALSE(hero->abilitySystem.getAbilityExecutionHistory().empty());
    CHECK(hero->abilitySystem.getAbilityExecutionHistory().back().outcome == "executed");

    actorData->skills = originalSkills;
    skillData->mpCost = originalCost;
    skillData->damage.type = originalDamageType;
    skillData->damage.power = originalDamagePower;
    skillData->effects = originalEffects;
}

TEST_CASE("BattleScene applies buff and debuff effect codes to live battle stats", "[battle][scene][ability][modifiers]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    auto* actorData = dm.getActor(1);
    auto* skillData = dm.getSkill(2);
    REQUIRE(actorData != nullptr);
    REQUIRE(skillData != nullptr);

    const auto originalSkills = actorData->skills;
    const int originalCost = skillData->mpCost;
    const int originalDamageType = skillData->damage.type;
    const int originalDamagePower = skillData->damage.power;
    const auto originalEffects = skillData->effects;

    actorData->skills = {2};
    skillData->mpCost = 4;
    skillData->damage.type = 0;
    skillData->damage.power = 0;
    skillData->effects.clear();
    skillData->effects.push_back({31, 2, 2.0, 1.0}); // +1 ATK buff for 2 turns
    skillData->effects.push_back({32, 3, 2.0, 1.0}); // -1 DEF debuff for 2 turns

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 120, 20, {0, 0}, nullptr);
    battle.addEnemy("1", "Slime", 40, 0, {100, 100}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipantById(participants, "1", false);
    BattleParticipant* slime = findParticipantById(participants, "1", true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    const auto baseDamage = urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*hero, *slime));

    BattleScene::BattleAction buffAction{};
    buffAction.subject = hero;
    buffAction.target = hero;
    buffAction.command = "skill";
    buffAction.isSkill = true;
    buffAction.skillId = 2;

    BattleScene::BattleAction debuffAction{};
    debuffAction.subject = hero;
    debuffAction.target = slime;
    debuffAction.command = "skill";
    debuffAction.isSkill = true;
    debuffAction.skillId = 2;

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(buffAction);
    battle.onUpdate(0.1f);

    REQUIRE_FALSE(hero->modifiers.empty());
    CHECK(hero->modifiers.front().paramId == 2);
    CHECK(hero->modifiers.front().stages == 1);

    battle.addActionToQueue(debuffAction);
    battle.onUpdate(0.9f);
    battle.onUpdate(0.1f);

    REQUIRE_FALSE(slime->modifiers.empty());
    CHECK(slime->modifiers.front().paramId == 3);
    CHECK(slime->modifiers.front().stages == -1);

    const auto buffedDamage = urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*hero, *slime));
    CHECK(buffedDamage > baseDamage);

    battle.setPhase(BattlePhase::TURN_END);
    CHECK(hero->modifiers.front().turnsRemaining == 1);
    CHECK(slime->modifiers.front().turnsRemaining == 1);

    battle.setPhase(BattlePhase::TURN_END);
    CHECK(hero->modifiers.empty());
    CHECK(slime->modifiers.empty());

    actorData->skills = originalSkills;
    skillData->mpCost = originalCost;
    skillData->damage.type = originalDamageType;
    skillData->damage.power = originalDamagePower;
    skillData->effects = originalEffects;
}

TEST_CASE("BattleScene draws bounded colored HUD cues for gauges, guard, states, and popups", "[battle][scene][draw]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    REQUIRE(participants.size() == 1);

    auto& actor = participants.front();
    actor.isGuarding = true;
    actor.states = {5, 6};
    actor.DamagePopupTimer = 0.75f;
    actor.DamagePopupValue = 42.0f;
    actor.DamagePopupColor = 0x00FF00FFu;

    urpg::SpriteBatcher batcher;
    batcher.begin();
    battle.draw(batcher);
    batcher.end();

    REQUIRE_FALSE(batcher.getBatches().empty());
    REQUIRE(batchContainsColor(batcher, 0x1B1B24CCu));
    REQUIRE(batchContainsColor(batcher, 0xFF0000FFu));
    REQUIRE(batchContainsColor(batcher, 0xAAAA00FFu));
    REQUIRE(batchContainsColor(batcher, 0xF4A261FFu));
    REQUIRE(batchContainsColor(batcher, 0x2E86ABFFu));
    REQUIRE(batchContainsColor(batcher, 0x6A994EFFu));
    REQUIRE(batchContainsColor(batcher, 0x00FF00FFu));
}

TEST_CASE("BattleScene diagnostics preview is unavailable without a queued action", "[battle][scene][diagnostics]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0, 0}, nullptr);
    battle.addEnemy("1", "Slime", 30, 0, {100, 100}, nullptr);

    REQUIRE_FALSE(battle.buildDiagnosticsPreview().has_value());
}
