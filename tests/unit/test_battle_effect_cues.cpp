#include "engine/core/battle/battle_core.h"
#include "engine/core/presentation/effects/effect_cue.h"
#include "engine/core/scene/battle_scene.h"
#include "runtimes/compat_js/data_manager.h"

#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

using namespace urpg::presentation::effects;
using namespace urpg::scene;

namespace {

class ActorParamsGuard {
public:
    ActorParamsGuard(urpg::compat::DataManager& dataManager, int actorId)
        : m_dataManager(dataManager), m_actorId(actorId) {
        if (auto* actor = m_dataManager.getActor(m_actorId)) {
            m_originalParams = actor->params;
            m_hasOriginalParams = true;
        }
    }

    ~ActorParamsGuard() {
        if (!m_hasOriginalParams) {
            return;
        }

        if (auto* actor = m_dataManager.getActor(m_actorId)) {
            actor->params = m_originalParams;
        }
    }

private:
    urpg::compat::DataManager& m_dataManager;
    int m_actorId = 0;
    std::vector<std::vector<int32_t>> m_originalParams;
    bool m_hasOriginalParams = false;
};

BattleParticipant* findParticipant(std::vector<BattleParticipant>& participants, bool isEnemy) {
    for (auto& participant : participants) {
        if (participant.isEnemy == isEnemy) {
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

} // namespace

TEST_CASE("BattleScene emits cast then hit cues for a successful attack", "[battle][effects][cue]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("1", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);
    REQUIRE(urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*hero, *slime)) > 0);

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "attack";

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    const auto& cues = battle.effectCues();
    REQUIRE(cues.size() == 2);

    CHECK(cues[0].frameTick == 1);
    CHECK(cues[0].sequenceIndex == 0);
    CHECK(cues[0].kind == EffectCueKind::HitConfirm);
    CHECK(cues[0].anchorMode == EffectAnchorMode::Owner);
    CHECK(cues[0].sourceId != cues[1].ownerId);
    CHECK(cues[0].sourceId == cues[0].ownerId);
    CHECK(cues[0].overlayEmphasis.value == Catch::Approx(0.0f));

    CHECK(cues[1].frameTick == 1);
    CHECK(cues[1].sequenceIndex == 1);
    CHECK(cues[1].kind == EffectCueKind::HitConfirm);
    CHECK(cues[1].anchorMode == EffectAnchorMode::Target);
    CHECK(cues[1].sourceId == cues[0].sourceId);
    CHECK(cues[1].ownerId != cues[1].sourceId);
    CHECK(cues[1].overlayEmphasis.value > 0.0f);
    CHECK(cues[1].intensity.value >= 1.0f);
}

TEST_CASE("BattleScene emits miss cues instead of impact when attack fails", "[battle][effects][cue]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    ActorParamsGuard actorParamsGuard(dm, 1);
    auto* actorData = dm.getActor(1);
    REQUIRE(actorData != nullptr);
    actorData->params = makeActorParams(100, 20, 1, 10, 1, 10, 12, 5);

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("1", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);
    REQUIRE(urpg::battle::BattleRuleResolver::resolveDamage(buildAttackContext(*hero, *slime)) == 0);

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "attack";

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    const auto& cues = battle.effectCues();
    REQUIRE(cues.size() == 2);
    CHECK(cues[0].anchorMode == EffectAnchorMode::Owner);
    CHECK(cues[1].anchorMode == EffectAnchorMode::Target);
    CHECK(cues[1].overlayEmphasis.value == Catch::Approx(0.0f));
    CHECK(cues[1].intensity.value < 1.0f);
    const auto priorSequence = cues.back().sequenceIndex;

    battle.clearEffectCues();

    EffectCue explicitCue;
    explicitCue.frameTick = 4;
    explicitCue.sourceId = 7;
    explicitCue.ownerId = 9;
    explicitCue.kind = EffectCueKind::Gameplay;
    explicitCue.anchorMode = EffectAnchorMode::Target;
    explicitCue.overlayEmphasis = {0.25f};
    explicitCue.intensity = {0.5f};

    battle.enqueueEffectCue(explicitCue);

    REQUIRE(battle.effectCues().size() == 1);
    CHECK(battle.effectCues().front().frameTick == 4);
    CHECK(battle.effectCues().front().sequenceIndex == priorSequence + 1);
    CHECK(battle.effectCues().front().sourceId == 7);
    CHECK(battle.effectCues().front().ownerId == 9);
    CHECK(battle.effectCues().front().overlayEmphasis.value == Catch::Approx(0.25f));
    CHECK(battle.effectCues().front().intensity.value == Catch::Approx(0.5f));
}

TEST_CASE("BattleScene emits CastStart for skill use", "[battle][effects][cue]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("1", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "skill";
    action.isSkill = true;
    action.skillId = 2;

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    const auto& cues = battle.effectCues();
    REQUIRE(cues.size() == 2);
    CHECK(cues[0].kind == EffectCueKind::CastStart);
}

TEST_CASE("BattleScene emits CriticalHit for high damage", "[battle][effects][cue]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    ActorParamsGuard actorParamsGuard(dm, 1);
    auto* actorData = dm.getActor(1);
    REQUIRE(actorData != nullptr);
    actorData->params = makeActorParams(100, 20, 999, 1, 1, 1, 12, 5);

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("1", "Slime", 5000, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "attack";

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    const auto& cues = battle.effectCues();
    REQUIRE(cues.size() == 2);
    CHECK(cues[1].kind == EffectCueKind::CriticalHit);
}

TEST_CASE("BattleScene emits MissSweep for zero damage", "[battle][effects][cue]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    ActorParamsGuard actorParamsGuard(dm, 1);
    auto* actorData = dm.getActor(1);
    REQUIRE(actorData != nullptr);
    actorData->params = makeActorParams(100, 20, 1, 10, 1, 10, 12, 5);

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("1", "Slime", 30, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "attack";

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    const auto& cues = battle.effectCues();
    REQUIRE(cues.size() == 2);
    CHECK(cues[1].kind == EffectCueKind::MissSweep);
}

TEST_CASE("BattleScene emits DefeatFade when target HP reaches zero", "[battle][effects][cue]") {
    auto& dm = urpg::compat::DataManager::instance();
    REQUIRE(dm.loadDatabase());
    dm.setupNewGame();

    ActorParamsGuard actorParamsGuard(dm, 1);
    auto* actorData = dm.getActor(1);
    REQUIRE(actorData != nullptr);
    actorData->params = makeActorParams(100, 20, 999, 1, 1, 1, 12, 5);

    BattleScene battle({"1"});
    battle.onStart();
    battle.addActor("1", "Hero", 100, 20, {0.0f, 0.0f}, nullptr);
    battle.addEnemy("1", "Slime", 5, 0, {100.0f, 0.0f}, nullptr);

    auto& participants = const_cast<std::vector<BattleParticipant>&>(battle.getParticipants());
    BattleParticipant* hero = findParticipant(participants, false);
    BattleParticipant* slime = findParticipant(participants, true);

    REQUIRE(hero != nullptr);
    REQUIRE(slime != nullptr);

    BattleScene::BattleAction action{};
    action.subject = hero;
    action.target = slime;
    action.command = "attack";

    battle.setPhase(BattlePhase::ACTION);
    battle.addActionToQueue(action);
    battle.onUpdate(0.1f);

    const auto& cues = battle.effectCues();
    REQUIRE(cues.size() == 2);
    CHECK(cues[1].kind == EffectCueKind::DefeatFade);
}
