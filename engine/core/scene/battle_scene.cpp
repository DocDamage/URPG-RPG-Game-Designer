#include "battle_scene.h"
#include "combat_formula.h"
#include "engine/core/sprite_batcher.h"
#include "runtimes/compat_js/data_manager.h"
#include "runtimes/compat_js/battle_manager.h"
#include "engine/core/render/asset_loader.h"
#include <algorithm>
#include <ctime>
#include <filesystem>
#include <iostream>
#include <optional>

namespace urpg::scene {

namespace {

constexpr const char* kMissingBattlebackDiagnostic = "MISSING_BATTLEBACK";

int parseDatabaseId(const std::string& rawId) {
    try {
        return std::stoi(rawId);
    } catch (...) {
        return -1;
    }
}

std::uint64_t participantCueId(const BattleParticipant* participant) {
    if (participant == nullptr) {
        return 0;
    }

    const int id = parseDatabaseId(participant->id);
    if (id <= 0) {
        return 0;
    }

    constexpr std::uint64_t kEnemyCueDomainBit = 1ull << 63;
    const std::uint64_t cueId = static_cast<std::uint64_t>(id);
    return participant->isEnemy ? (cueId | kEnemyCueDomainBit) : cueId;
}

std::shared_ptr<urpg::Texture> loadOptionalTexture(const std::string& path) {
    std::error_code error;
    if (!std::filesystem::exists(path, error) || error) {
        return nullptr;
    }

    return AssetLoader::loadTexture(path);
}

int resolveParticipantParam(const BattleParticipant& participant, int32_t param_index) {
    auto& dm = compat::DataManager::instance();
    const int id = parseDatabaseId(participant.id);
    if (id <= 0) {
        return 1;
    }

    if (participant.isEnemy) {
        const auto* enemy = dm.getEnemy(id);
        if (enemy == nullptr) {
            return 1;
        }

        switch (param_index) {
        case 0:
            return enemy->mhp;
        case 1:
            return enemy->mmp;
        case 2:
            return enemy->atk;
        case 3:
            return enemy->def;
        case 4:
            return enemy->mat;
        case 5:
            return enemy->mdf;
        case 6:
            return enemy->agi;
        case 7:
            return enemy->luk;
        default:
            return 1;
        }
    }

    return dm.getActorParam(id, param_index, 1);
}

int resolveActionPriority(const BattleScene::BattleAction& action) {
    if (action.command == "guard") {
        return -10;
    }
    return 0;
}

urpg::battle::BattleDamageContext buildDamageContext(const BattleScene::BattleAction& action,
                                                     const BattleParticipant& subject,
                                                     const BattleParticipant& target) {
    urpg::battle::BattleDamageContext context;
    context.subject.hp = subject.hp;
    context.subject.mhp = subject.maxHp;
    context.subject.atk = resolveParticipantParam(subject, 2);
    context.subject.def = resolveParticipantParam(subject, 3);
    context.subject.mat = resolveParticipantParam(subject, 4);
    context.subject.mdf = resolveParticipantParam(subject, 5);
    context.subject.agi = resolveParticipantParam(subject, 6);
    context.subject.luk = resolveParticipantParam(subject, 7);

    context.target.hp = target.hp;
    context.target.mhp = target.maxHp;
    context.target.atk = resolveParticipantParam(target, 2);
    context.target.def = resolveParticipantParam(target, 3);
    context.target.mat = resolveParticipantParam(target, 4);
    context.target.mdf = resolveParticipantParam(target, 5);
    context.target.agi = resolveParticipantParam(target, 6);
    context.target.luk = resolveParticipantParam(target, 7);
    context.target.guarding = target.isGuarding;

    if (action.isSkill) {
        const auto* skill = compat::DataManager::instance().getSkill(action.skillId);
        if (skill != nullptr) {
            context.power = skill->damage.power;
            context.variance_percent = skill->damage.variance;
            context.critical = skill->damage.canCrit;
            context.magical = skill->damage.type == 2;
        }
    } else if (action.isItem) {
        const auto* item = compat::DataManager::instance().getItem(action.itemId);
        if (item != nullptr) {
            context.power = item->damage.power;
            context.variance_percent = item->damage.variance;
            context.critical = item->damage.canCrit;
            context.magical = item->damage.type == 2;
        }
    }

    return context;
}

std::optional<BattleScene::BattleAction> popResolvedAction(std::vector<BattleScene::BattleAction>& staged_actions,
                                                           urpg::battle::BattleActionQueue& native_queue) {
    const auto next = native_queue.popNext();
    if (!next.has_value()) {
        return std::nullopt;
    }

    auto matches = [&](const BattleScene::BattleAction& action) {
        if (action.command != next->command || action.subject == nullptr) {
            return false;
        }
        if (action.subject->id != next->subject_id) {
            return false;
        }

        const std::string target_id = action.target != nullptr ? action.target->id : std::string();
        if (target_id != next->target_id) {
            return false;
        }
        return true;
    };

    const auto it = std::find_if(staged_actions.begin(), staged_actions.end(), matches);
    if (it == staged_actions.end()) {
        return std::nullopt;
    }

    BattleScene::BattleAction resolved = *it;
    staged_actions.erase(it);
    return resolved;
}

std::optional<BattleScene::BattleAction> findOrderedAction(const std::vector<BattleScene::BattleAction>& staged_actions,
                                                           const urpg::battle::BattleActionQueue& native_queue) {
    const auto ordered = native_queue.snapshotOrdered();
    if (ordered.empty()) {
        return std::nullopt;
    }

    const auto& next = ordered.front();
    auto matches = [&](const BattleScene::BattleAction& action) {
        if (action.command != next.command || action.subject == nullptr) {
            return false;
        }
        if (action.subject->id != next.subject_id) {
            return false;
        }

        const std::string target_id = action.target != nullptr ? action.target->id : std::string();
        return target_id == next.target_id;
    };

    const auto it = std::find_if(staged_actions.begin(), staged_actions.end(), matches);
    if (it == staged_actions.end()) {
        return std::nullopt;
    }
    return *it;
}

int sumAgility(const std::vector<BattleParticipant>& participants, bool is_enemy) {
    int total = 0;
    for (const auto& participant : participants) {
        if (participant.isEnemy != is_enemy || participant.hp <= 0) {
            continue;
        }
        total += resolveParticipantParam(participant, 6);
    }
    return std::max(total, 1);
}

} // namespace

BattleScene::BattleScene(const std::vector<std::string>& enemyIds)
    : m_enemyIds(enemyIds), m_currentPhase(BattlePhase::START)
{
    // Initialize common battle windows
    m_logWindow = std::make_shared<ui::UIWindow>();
    m_logWindow->setPosition(Vector2f(20.0f, 20.0f));
    m_logWindow->setSize(Vector2f(760.0f, 120.0f));
    m_logWindow->setZIndex(0.85f);

    m_statusWindow = std::make_shared<ui::UIWindow>();
    m_statusWindow->setPosition(Vector2f(20.0f, 440.0f));
    m_statusWindow->setSize(Vector2f(760.0f, 140.0f));
    m_statusWindow->setZIndex(0.85f);

    m_commandWindow = std::make_shared<ui::UICommandList>();
    m_commandWindow->setPosition(Vector2f(20.0f, 440.0f));
    m_commandWindow->setSize(Vector2f(160.0f, 140.0f));
    m_commandWindow->setZIndex(0.9f);
    m_commandWindow->setVisible(false);

    m_skillWindow = std::make_shared<ui::UICommandList>();
    m_skillWindow->setPosition(Vector2f(180.0f, 440.0f));
    m_skillWindow->setSize(Vector2f(300.0f, 140.0f));
    m_skillWindow->setZIndex(0.9f);
    m_skillWindow->setVisible(false);

    m_itemWindow = std::make_shared<ui::UICommandList>();
    m_itemWindow->setPosition(Vector2f(180.0f, 440.0f));
    m_itemWindow->setSize(Vector2f(300.0f, 140.0f));
    m_itemWindow->setZIndex(0.9f);
    m_itemWindow->setVisible(false);

    m_targetWindow = std::make_shared<ui::UICommandList>();
    m_targetWindow->setPosition(Vector2f(480.0f, 440.0f));
    m_targetWindow->setSize(Vector2f(200.0f, 140.0f));
    m_targetWindow->setZIndex(0.9f);
    m_targetWindow->setVisible(false);

    // Initial commands
    m_commandWindow->addItem("Attack", [this]() { onCommandSelected("attack"); });
    m_commandWindow->addItem("Guard", [this]() { onCommandSelected("guard"); });
    m_commandWindow->addItem("Skill", [this]() { onCommandSelected("skill"); });
    m_commandWindow->addItem("Item", [this]() { onCommandSelected("item"); });
}

void BattleScene::onStart() {
    m_flowController.beginBattle(true);
    m_currentPhase = BattlePhase::START;
    m_turnCount = m_flowController.turnCount();
    m_actionQueue.clear();
    m_nativeActionQueue.clear();
    m_effectSequence = 0;
    m_effectCues.clear();
    m_commandWindow->setVisible(false);

    // Phase 12: Load Background
    // In MZ, battle backgrounds are often determined by the map or troop.
    // Keep the fallback path explicit so missing content does not look like a valid default.
    constexpr const char* fallbackBattlebackPath = "img/battlebacks1/Grassland.png";
    m_backgroundTexture = loadOptionalTexture(fallbackBattlebackPath);
    if (!m_backgroundTexture) {
        std::cerr << "[" << kMissingBattlebackDiagnostic << "] No configured battleback was resolved; "
                  << "fallback asset is also unavailable: " << fallbackBattlebackPath << std::endl;
        if (m_logWindow) {
            m_logWindow->setText(kMissingBattlebackDiagnostic);
        }
    }
}

void BattleScene::setPhase(BattlePhase phase) {
    m_currentPhase = phase;
    switch (phase) {
    case BattlePhase::START: {
        urpg::presentation::effects::EffectCue banner;
        banner.frameTick = static_cast<std::uint64_t>(std::max(m_turnCount, 0));
        banner.kind = urpg::presentation::effects::EffectCueKind::PhaseBanner;
        banner.anchorMode = urpg::presentation::effects::EffectAnchorMode::Screen;
        banner.overlayEmphasis = {0.80f};
        banner.intensity = {1.5f};
        enqueueEffectCue(banner);
        break;
    }
    case BattlePhase::INPUT:
        m_flowController.enterInput();
        break;
    case BattlePhase::ACTION:
        m_flowController.enterAction();
        break;
    case BattlePhase::TURN_END:
        m_flowController.endTurn();
        m_turnCount = m_flowController.turnCount();
        break;
    case BattlePhase::VICTORY:
        m_flowController.markVictory();
        break;
    case BattlePhase::DEFEAT:
        m_flowController.markDefeat();
        break;
    }
}

urpg::battle::BattleQueuedAction BattleScene::makeQueuedAction(const BattleAction& action) const {
    urpg::battle::BattleQueuedAction queued;
    queued.subject_id = action.subject != nullptr ? action.subject->id : std::string();
    queued.target_id = action.target != nullptr ? action.target->id : std::string();
    queued.command = action.command;
    queued.speed = action.subject != nullptr ? resolveParticipantParam(*action.subject, 6) : 0;
    queued.priority = resolveActionPriority(action);
    return queued;
}

void BattleScene::addActionToQueue(const BattleAction& action) {
    m_actionQueue.push_back(action);
    m_nativeActionQueue.enqueue(makeQueuedAction(action));
}

void BattleScene::enqueueEffectCue(const urpg::presentation::effects::EffectCue& cue) {
    auto ordered = cue;
    ordered.sequenceIndex = m_effectSequence++;
    m_effectCues.push_back(ordered);
}

std::optional<BattleDiagnosticsPreview> BattleScene::buildDiagnosticsPreview() const {
    const auto action = findOrderedAction(m_actionQueue, m_nativeActionQueue);
    if (!action.has_value() || action->subject == nullptr) {
        return std::nullopt;
    }

    const BattleParticipant* target = action->target;
    if (target == nullptr || target->hp <= 0) {
        return std::nullopt;
    }

    BattleDiagnosticsPreview preview;
    preview.physical_preview = buildDamageContext(*action, *action->subject, *target);
    preview.physical_preview.magical = false;

    preview.magical_preview = buildDamageContext(*action, *action->subject, *target);
    preview.magical_preview.magical = true;

    preview.party_agi = sumAgility(m_participants, false);
    preview.troop_agi = sumAgility(m_participants, true);
    return preview;
}

void BattleScene::onUpdate(float dt) {
    for (auto& p : m_participants) {
        if (p.animator) {
            p.animator->update(dt);
        }
        
        // Update damage popups
        if (p.DamagePopupValue > 0 && p.DamagePopupTimer > 0) {
            p.DamagePopupTimer -= dt;
        }
    }

    m_logWindow->update(dt);
    m_statusWindow->update(dt);
    
    if (m_commandWindow->isVisible()) {
        m_commandWindow->update(dt);
    }
    if (m_skillWindow->isVisible()) m_skillWindow->update(dt);
    if (m_itemWindow->isVisible()) m_itemWindow->update(dt);
    if (m_targetWindow->isVisible()) m_targetWindow->update(dt);

    if (m_phaseTimer > 0) m_phaseTimer -= dt;
    if (m_shakeTimer > 0) m_shakeTimer -= dt;

    // Phase State Machine
    switch (m_currentPhase) {
        case BattlePhase::START:
            if (m_phaseTimer <= 0) {
                setPhase(BattlePhase::INPUT);
                m_currentActorIndex = 0;
                m_commandWindow->setVisible(true);

                // Phase 13: Signal JS Battle Start Hook
                // compat::BattleManager::instance().triggerHook(compat::BattleManager::HookPoint::ON_START, {});
            }
            break;

        case BattlePhase::INPUT:
            // Input is handled via callbacks from UICommandList
            break;

        case BattlePhase::ACTION:
            if (m_phaseTimer <= 0) {
                BattlePhase nextPhase = checkEndCondition();
                if (nextPhase != BattlePhase::ACTION) {
                    setPhase(nextPhase);
                    m_phaseTimer = 2.0f;
                    m_actionQueue.clear();
                    m_nativeActionQueue.clear();
                    break;
                }

                if (!m_nativeActionQueue.empty()) {
                    auto action = popResolvedAction(m_actionQueue, m_nativeActionQueue);
                    if (!action.has_value()) {
                        setPhase(BattlePhase::TURN_END);
                        m_phaseTimer = 0.5f;
                        break;
                    }

                    executeAction(*action);
                    
                    // Check for victory/defeat after each action
                    BattlePhase nextPhase = checkEndCondition();
                    if (nextPhase != BattlePhase::ACTION) {
                        setPhase(nextPhase);
                        m_phaseTimer = 2.0f; // Brief pause to show victory/defeat msg
                        m_actionQueue.clear();
                        m_nativeActionQueue.clear();
                    } else {
                        m_phaseTimer = 1.0f; // 1 second between actions
                    }
                } else {
                    setPhase(BattlePhase::TURN_END);
                    m_phaseTimer = 0.5f;
                }
            }
            break;

        case BattlePhase::VICTORY:
            if (m_phaseTimer <= 0) {
                // Return to previous scene or world map
                SceneManager::getInstance().popScene();
            } else {
                if (m_logWindow) m_logWindow->setText("Victory!");
            }
            break;

        case BattlePhase::DEFEAT:
            if (m_phaseTimer <= 0) {
                // Game over or return to main menu
                SceneManager::getInstance().popScene();
            } else {
                if (m_logWindow) m_logWindow->setText("Defeat...");
            }
            break;

        case BattlePhase::TURN_END:
            if (m_phaseTimer <= 0) {
                setPhase(BattlePhase::INPUT);
                m_currentActorIndex = 0;
                m_actionQueue.clear();
                m_nativeActionQueue.clear();
                m_commandWindow->setVisible(true);
            }
            break;

        default:
            break;
    }
}

void BattleScene::draw(urpg::SpriteBatcher& batcher) {
    // Offset drawing for shake effect
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    if (m_shakeTimer > 0) {
        offsetX = (float)(rand() % 10 - 5);
        offsetY = (float)(rand() % 10 - 5);
    }

    // 1. Draw Background (Phase 12)
    if (m_backgroundTexture) {
        batcher.submit(m_backgroundTexture->getId(), 
                       offsetX, offsetY, 800.0f, 600.0f, 
                       0, 0, 1, 1, 0.1f);
    }
    
    // 2. Draw Participants
    for (const auto& p : m_participants) {
        if (p.animator) {
            float width = p.isEnemy ? 128.0f : 48.0f;
            float height = p.isEnemy ? 128.0f : 48.0f;
            
            // Draw participant at its current position with shake offset
            p.animator->draw(batcher, p.position.x + offsetX, p.position.y + offsetY, width, height, 0.5f);
            
            // Draw Damage Popup (Phase 9)
            if (p.DamagePopupTimer > 0) {
                // Animate floating upwards
                float popupY = p.position.y - (1.0f - p.DamagePopupTimer) * 32.0f - 20.0f;
                // placeholder: solid color box using white texture (id 1 usually or 0)
                batcher.submit(1, p.position.x, popupY, 24.0f, 12.0f, 0, 0, 1, 1, 0.9f);
            }
        }
    }

    // 3. Draw UI
    if (m_logWindow && m_logWindow->isVisible()) {
        m_logWindow->draw(batcher);
    }

    if (m_statusWindow && m_statusWindow->isVisible()) {
        m_statusWindow->draw(batcher);
        
        // 3a. Draw specific status data inside status window
        float startX = m_statusWindow->getPosition().x + 10.0f;
        float startY = m_statusWindow->getPosition().y + 10.0f;
        
        size_t index = 0;
        for (auto& p : m_participants) {
            if (!p.isEnemy) {
                // Name (Conceptual)
                // batcher.drawText(p.name, startX, startY + (index * 24.0f));

                // HP Bar
                float hpRate = (float)p.hp / (float)std::max(1, p.maxHp);
                m_statusWindow->drawGauge(batcher, startX + 120.0f, startY + (index * 24.0f) + 8.0f, 100.0f, hpRate, 0xFF0000FF, 0xFF0000AA);

                // MP Bar
                float mpRate = (float)p.mp / (float)std::max(1, p.maxMp);
                m_statusWindow->drawGauge(batcher, startX + 240.0f, startY + (index * 24.0f) + 8.0f, 100.0f, mpRate, 0xFFFF00FF, 0xAAAA00FF);
                
                // Show guarding icon/text (Phase 10)
                if (p.isGuarding) {
                    batcher.submit(1, startX + 350.0f, startY + (index * 24.0f), 16.0f, 16.0f, 0, 0, 1, 1, 0.9f);
                }

                // Show State Icons (Phase 11)
                float stateX = startX + 372.0f;
                for ([[maybe_unused]] int32_t stateId : p.states) {
                    batcher.submit(1, stateX, startY + (index * 24.0f), 16.0f, 16.0f, 0, 0, 1, 1, 0.9f);
                    stateX += 20.0f;
                }

                index++;
            }
        }
    }

    // 3b. Action Selection Visuals
    if (m_targetWindow->isVisible()) {
        // Highlighting current selection would go here
    }
    
    if (m_commandWindow && m_commandWindow->isVisible()) {
        m_commandWindow->draw(batcher);
    }
    if (m_skillWindow && m_skillWindow->isVisible()) m_skillWindow->draw(batcher);
    if (m_itemWindow && m_itemWindow->isVisible()) m_itemWindow->draw(batcher);
    if (m_targetWindow && m_targetWindow->isVisible()) m_targetWindow->draw(batcher);
}

void BattleScene::processTurn() {
    // Phase 13: Signal Turn Start Hook
    compat::BattleManager::instance().triggerHook(compat::BattleManager::HookPoint::ON_TURN_START, {});

    setPhase(BattlePhase::ACTION);
    m_phaseTimer = 0.5f;
}

void BattleScene::onCommandSelected(const std::string& cmd) {
    // 1. Determine the subject
    BattleParticipant* subject = nullptr;
    size_t actorCount = 0;
    for (auto& p : m_participants) {
        if (!p.isEnemy) {
            if (actorCount == m_currentActorIndex) {
                subject = &p;
                break;
            }
            actorCount++;
        }
    }

    if (!subject) return;

    // Reset pending action
    m_pendingAction = BattleAction();
    m_pendingAction.subject = subject;
    m_pendingAction.command = cmd;

    if (cmd == "attack") {
        openTargetWindow(true); // Target enemies
    } else if (cmd == "guard") {
        onTargetSelected(subject); // Self-target for guard
    } else if (cmd == "skill") {
        openSkillWindow();
    } else if (cmd == "item") {
        openItemWindow();
    }
}

void BattleScene::openSkillWindow() {
    m_skillWindow->clearItems();
    auto& dm = compat::DataManager::instance();
    if (m_pendingAction.subject == nullptr) {
        return;
    }
    const int actorId = parseDatabaseId(m_pendingAction.subject->id);
    auto actor = actorId > 0 ? dm.getActor(actorId) : nullptr;
    
    if (actor) {
        // In MZ, skills come from classes and traits. 
        // Simplified: list first few skills for now
        for (int skillId : {1, 2, 3, 4, 5}) {
            auto skill = dm.getSkill(skillId);
            if (skill) {
                bool canUse = m_pendingAction.subject->mp >= skill->mpCost;
                m_skillWindow->addItem(skill->name + " (" + std::to_string(skill->mpCost) + ")", [this, skillId, skill]() {
                    m_pendingAction.isSkill = true;
                    m_pendingAction.skillId = skillId;
                    
                    // MZ Scope Handling
                    // 1=Single Enemy, 2=All Enemies, 7=Single Ally, 8=All Allies, 11=Self
                    if (skill->scope == 11) {
                        onTargetSelected(m_pendingAction.subject);
                    } else if (skill->scope == 2 || skill->scope == 8) {
                        onMultiTargetSelected(skill->scope == 2);
                    } else {
                        openTargetWindow(skill->scope <= 6); 
                    }
                }, canUse);
            }
        }
    }
    m_skillWindow->setVisible(true);
}

void BattleScene::openItemWindow() {
    m_itemWindow->clearItems();
    auto& dm = compat::DataManager::instance();
    auto& state = dm.getGlobalState();

    for (auto const& [itemId, count] : state.items) {
        if (count > 0) {
            auto item = dm.getItem(itemId);
            if (item && item->occasion <= 1) { // 0: Always, 1: Battle Only
                m_itemWindow->addItem(item->name + " x" + std::to_string(count), [this, itemId, item]() {
                    m_pendingAction.isItem = true;
                    m_pendingAction.itemId = itemId;
                    
                    // MZ Scope Handling for Items
                    if (item->scope == 11) {
                        onTargetSelected(m_pendingAction.subject);
                    } else if (item->scope == 2 || item->scope == 8) {
                        onMultiTargetSelected(item->scope == 2);
                    } else {
                        openTargetWindow(item->scope <= 6);
                    }
                });
            }
        }
    }
    m_itemWindow->setVisible(true);
}

void BattleScene::onMultiTargetSelected(bool targetEnemies) {
    m_pendingAction.isAoE = true;
    m_pendingAction.multiTargets.clear();
    for (auto& p : m_participants) {
        if (p.isEnemy == targetEnemies && p.hp > 0) {
            m_pendingAction.multiTargets.push_back(&p);
        }
    }
    
    // We can still use onTargetSelected to finalize, passing nullptr as single target
    onTargetSelected(nullptr);
}

void BattleScene::openTargetWindow(bool targetEnemies) {
    m_targetWindow->clearItems();
    for (auto& p : m_participants) {
        if (p.isEnemy == targetEnemies && p.hp > 0) {
            m_targetWindow->addItem(p.name, [this, &p]() {
                onTargetSelected(&p);
            });
        }
    }
    m_targetWindow->setVisible(true);
}

void BattleScene::onTargetSelected(BattleParticipant* target) {
    m_pendingAction.target = target;
    addActionToQueue(m_pendingAction);
    
    m_skillWindow->setVisible(false);
    m_itemWindow->setVisible(false);
    m_targetWindow->setVisible(false);

    // Move to next actor
    m_currentActorIndex++;
    
    size_t totalActors = 0;
    for (const auto& p : m_participants) if (!p.isEnemy) totalActors++;

    if (m_currentActorIndex >= totalActors) {
        m_commandWindow->setVisible(false);
        processTurn();
    }
}

void BattleScene::executeAction(const BattleAction& action) {
    if (!action.subject) return;

    // Phase 13: Signal Action Start Hook
    compat::BattleManager::instance().triggerHook(compat::BattleManager::HookPoint::ON_ACTION_START, {});

    // Phase 10: Multi-Target Logic
    std::vector<BattleParticipant*> currentTargets;
    if (action.isAoE) {
        // Use pre-selected multi-targets (all enemies or all allies)
        for (auto* t : action.multiTargets) {
            if (t && t->hp > 0) currentTargets.push_back(t);
        }
    } else if (action.target) {
        // Single target handling with redirection
        BattleParticipant* finalTarget = action.target;
        if (finalTarget->hp <= 0) {
            bool searchEnemy = finalTarget->isEnemy;
            for (auto& p : m_participants) {
                if (p.isEnemy == searchEnemy && p.hp > 0) {
                    finalTarget = &p;
                    break;
                }
            }
        }
        if (finalTarget->hp > 0) currentTargets.push_back(finalTarget);
    }

    if (currentTargets.empty()) return;

    // Check if subject is alive
    if (action.subject->hp <= 0) return;

    // Guard reset and processing...
    action.subject->isGuarding = false;

    if (action.command == "guard") {
        action.subject->isGuarding = true;
        if (m_logWindow) m_logWindow->setText(action.subject->name + " is guarding!");
        return;
    }

    auto& dm = compat::DataManager::instance();
    std::string skillName = "Attack";
    const compat::SkillData* skillData = nullptr;
    const compat::ItemData* itemData = nullptr;

    // 2. Resolve Data
    if (action.isSkill && action.skillId != -1) {
        skillData = dm.getSkill(action.skillId);
        if (skillData) {
            skillName = skillData->name;
            action.subject->mp = std::max(0, action.subject->mp - skillData->mpCost);
            
            // Phase 11 Polish: Log Skill Use
            if (m_logWindow) m_logWindow->setText(action.subject->name + " uses " + skillName + "!");
        }
    } else if (action.isItem && action.itemId != -1) {
        itemData = dm.getItem(action.itemId);
        if (itemData) {
            skillName = itemData->name;
            dm.loseItem(action.itemId, 1);

            // Phase 11 Polish: Log Item Use
            if (m_logWindow) m_logWindow->setText(action.subject->name + " uses " + skillName + "!");
        }
    } else if (action.command == "attack") {
        if (m_logWindow) m_logWindow->setText(action.subject->name + " attacks!");
    }

    if (action.command == "attack" || action.isSkill || action.isItem) {
        urpg::presentation::effects::EffectCue castCue;
        castCue.frameTick = static_cast<std::uint64_t>(std::max(m_turnCount, 0));
        if (action.isSkill || action.isItem) {
            castCue.kind = urpg::presentation::effects::EffectCueKind::CastStart;
        } else {
            castCue.kind = urpg::presentation::effects::EffectCueKind::HitConfirm;
        }
        castCue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Owner;
        castCue.sourceId = participantCueId(action.subject);
        castCue.ownerId = participantCueId(action.subject);
        castCue.overlayEmphasis = {0.0f};
        castCue.intensity = {1.0f};
        enqueueEffectCue(castCue);
    }

    // 2. Execute on all targets
    for (auto* target : currentTargets) {
        int32_t damage = 0;
        if (action.command == "attack" || action.isSkill || action.isItem) {
            damage = urpg::battle::BattleRuleResolver::resolveDamage(buildDamageContext(action, *action.subject, *target));
        } else {
            combat::CombatFormula::Context ctx;
            ctx.subject = action.subject;
            ctx.target = target;
            ctx.skill = skillData;
            ctx.item = itemData;
            damage = combat::CombatFormula::evaluateDamage(ctx);
            if (target->isGuarding && damage > 0) {
                damage /= 2;
            }
        }

        // 3. Apply Damage/HP change
        target->hp = std::max(0, target->hp - damage);
        if (damage > 0) {
            m_shakeTimer = 0.3f;
            target->DamagePopupValue = (float)damage;
            target->DamagePopupTimer = 1.0f;
            target->DamagePopupColor = 0xFFFFFFFF;
        } else if (damage < 0) {
            target->DamagePopupValue = (float)-damage;
            target->DamagePopupTimer = 1.0f;
            target->DamagePopupColor = 0x00FF00FF;
        }

        if (action.command == "attack" || action.isSkill || action.isItem) {
            urpg::presentation::effects::EffectCue resultCue;
            resultCue.frameTick = static_cast<std::uint64_t>(std::max(m_turnCount, 0));
            resultCue.anchorMode = urpg::presentation::effects::EffectAnchorMode::Target;
            resultCue.sourceId = participantCueId(action.subject);
            resultCue.ownerId = participantCueId(target);

            if (target->hp <= 0) {
                resultCue.kind = urpg::presentation::effects::EffectCueKind::DefeatFade;
                resultCue.overlayEmphasis = {0.40f};
                resultCue.intensity = {1.2f};
            } else if (damage < 0) {
                resultCue.kind = urpg::presentation::effects::EffectCueKind::HealPulse;
                resultCue.overlayEmphasis = {0.0f};
                resultCue.intensity = {0.8f};
            } else if (damage == 0 && target->isGuarding) {
                resultCue.kind = urpg::presentation::effects::EffectCueKind::GuardClash;
                resultCue.overlayEmphasis = {0.30f};
                resultCue.intensity = {1.1f};
            } else if (damage == 0) {
                resultCue.kind = urpg::presentation::effects::EffectCueKind::MissSweep;
                resultCue.overlayEmphasis = {0.0f};
                resultCue.intensity = {0.5f};
            } else if (damage > 20) {
                resultCue.kind = urpg::presentation::effects::EffectCueKind::CriticalHit;
                resultCue.overlayEmphasis = {0.95f};
                resultCue.intensity = {2.0f};
            } else {
                resultCue.kind = urpg::presentation::effects::EffectCueKind::HitConfirm;
                resultCue.overlayEmphasis = {damage > 0 ? 1.0f : 0.0f};
                resultCue.intensity = {damage > 20 ? 1.5f : (damage > 0 ? 1.0f : 0.5f)};
            }

            enqueueEffectCue(resultCue);
        }

        // 3a. Persist state
        if (!target->isEnemy) {
            const int actorId = parseDatabaseId(target->id);
            if (actorId > 0) {
                dm.updateActorHp(actorId, target->hp);
                dm.updateActorMp(actorId, target->mp);
            }
        }
    }

    if (!action.subject->isEnemy) {
        const int actorId = parseDatabaseId(action.subject->id);
        if (actorId > 0) {
            dm.updateActorMp(actorId, action.subject->mp);
        }
    }

    // 4. Visual/Log
    if (action.subject->animator) {
        if (action.isSkill || action.isItem) action.subject->animator->setRow(3); // Skill/Item pose
        else if (action.command == "attack") action.subject->animator->setRow(2); // Attack pose
    }

    if (m_logWindow) {
        std::string targetText = currentTargets.size() > 1 ? "multiple targets" : currentTargets[0]->name;
        m_logWindow->setText(action.subject->name + " uses " + skillName + " on " + targetText + "!");
    }

    // Set a small delay after execution to let popups be visible
    m_phaseTimer = 0.8f;

    // Phase 13: Signal Action End Hook
    compat::BattleManager::instance().triggerHook(compat::BattleManager::HookPoint::ON_ACTION_END, {});
}

BattlePhase BattleScene::checkEndCondition() {
    bool anyEnemyAlive = false;
    bool anyActorAlive = false;

    for (const auto& p : m_participants) {
        if (p.isEnemy && p.hp > 0) anyEnemyAlive = true;
        if (!p.isEnemy && p.hp > 0) anyActorAlive = true;
    }

    if (!anyEnemyAlive) {
        processVictoryRewards();
        return BattlePhase::VICTORY;
    }
    if (!anyActorAlive) return BattlePhase::DEFEAT;

    return BattlePhase::ACTION;
}

void BattleScene::processVictoryRewards() {
    auto& dm = compat::DataManager::instance();
    int32_t totalExp = 0;
    int32_t totalGold = 0;
    std::vector<std::pair<int32_t, int32_t>> itemsDropped; // itemId, count

    // Aggregate rewards from all enemies (even if dead, they were part of the troop)
    for (const auto& p : m_participants) {
        if (p.isEnemy) {
            const int enemyId = parseDatabaseId(p.id);
            auto enemyData = enemyId > 0 ? dm.getEnemy(enemyId) : nullptr;
            if (enemyData) {
                totalExp += enemyData->exp;
                totalGold += enemyData->gold;

                // Process drop items
                // EnemyData::dropItems is [itemId, rate, ...] - based on my read_file of data_manager.h
                for (size_t i = 0; i < enemyData->dropItems.size(); i += 2) {
                    int32_t itemId = enemyData->dropItems[i];
                    int32_t denominator = (i + 1 < enemyData->dropItems.size()) ? enemyData->dropItems[i+1] : 1;
                    
                    // Simple probability check
                    if (denominator > 0 && (rand() % denominator == 0)) {
                        itemsDropped.push_back({itemId, 1});
                    }
                }
            }
        }
    }

    // Apply rewards to GlobalState
    dm.gainGold(totalGold);
    for (auto& drop : itemsDropped) {
        dm.gainItem(drop.first, drop.second);
    }

    // Apply EXP & Level Up Check (Phase 8/11)
    std::vector<std::string> levelUpMsgs;
    for (auto& p : m_participants) {
        if (!p.isEnemy && p.hp > 0) {
            const int actorId = parseDatabaseId(p.id);
            if (actorId <= 0) {
                continue;
            }
            const auto* actorBefore = dm.getActor(actorId);
            if (actorBefore == nullptr) {
                continue;
            }
            const int oldLevel = actorBefore->level;
            dm.gainExp(actorId, totalExp);
            const auto* actorAfter = dm.getActor(actorId);
            if (actorAfter == nullptr) {
                continue;
            }
            const int newLevel = actorAfter->level;

            if (newLevel > oldLevel) {
                levelUpMsgs.push_back(p.name + " reached Level " + std::to_string(newLevel) + "!");
            }
        }
    }

    // Update Log with rewards
    std::string rewardMsg = "Victory! Gained " + std::to_string(totalGold) + " G and " + std::to_string(totalExp) + " EXP.";
    if (!itemsDropped.empty()) {
        rewardMsg += " Found " + std::to_string(itemsDropped.size()) + " items!";
    }
    
    // Add level up messages to log
    for (const auto& msg : levelUpMsgs) {
        rewardMsg += "\n" + msg;
    }

    if (m_logWindow) m_logWindow->setText(rewardMsg);
}

void BattleScene::setupTroop(int32_t troopId) {
    auto& dm = compat::DataManager::instance();
    auto troop = dm.getTroop(troopId);
    if (!troop) return;

    m_participants.clear();

    // 1. Load Enemies from Troop
    // Phase 12 Layout: Centralized cluster for enemies
    float enemyBaseX = 550.0f;
    float enemyBaseY = 250.0f;
    for (size_t i = 0; i < troop->members.size(); ++i) {
        int enemyId = troop->members[i];
        auto enemyData = dm.getEnemy(enemyId);
        if (enemyData) {
            std::string texturePath = "img/enemies/enemy" + std::to_string(enemyId) + ".png";
            auto texture = AssetLoader::loadTexture(texturePath);
            
            // Layout: simple vertical stack, offset for depth
            float posX = enemyBaseX + (i * 20.0f);
            float posY = enemyBaseY + (i * 80.0f) - (troop->members.size() / 2.0f * 80.0f);

            addEnemy(std::to_string(enemyId), enemyData->name, enemyData->mhp, enemyData->mmp, 
                     {posX, posY}, texture);
        }
    }

    // 2. Load Actors from Party
    // Phase 12 Layout: Classic Side-view right-aligned layout
    auto& state = dm.getGlobalState();
    float actorBaseX = 150.0f;
    float actorBaseY = 250.0f;
    for (size_t i = 0; i < state.partyMembers.size(); ++i) {
        int actorId = state.partyMembers[i];
        auto actorData = dm.getActor(actorId);
        if (actorData) {
            std::string texturePath = "img/sv_actors/" + actorData->battlerName + ".png";
            auto texture = AssetLoader::loadTexture(texturePath);

            // Layout: slanted column (MZ style)
            float posX = actorBaseX - (i * 30.0f);
            float posY = actorBaseY + (i * 70.0f) - (state.partyMembers.size() / 2.0f * 70.0f);

            int mhp = dm.getActorParam(actorId, 0, 1);
            int mmp = dm.getActorParam(actorId, 1, 1);
            
            addActor(std::to_string(actorId), actorData->name, mhp, mmp, 
                     {posX, posY}, texture);
        }
    }
}

void BattleScene::addActor(const std::string& id, const std::string& name, int hp, int mp, Vector2f pos, std::shared_ptr<urpg::Texture> texture) {
    BattleParticipant p;
    p.id = id;
    p.name = name;
    p.hp = p.maxHp = hp;
    p.mp = p.maxMp = mp;
    p.position = pos;
    p.isEnemy = false;
    
    if (texture) {
        p.animator = std::make_unique<urpg::SpriteAnimator>(texture);
        p.animator->setRow(2); // Face Left (Right sheet column) for actors facing enemies
    }
    
    m_participants.push_back(std::move(p));
}

void BattleScene::addEnemy(const std::string& id, const std::string& name, int hp, int mp, Vector2f pos, std::shared_ptr<urpg::Texture> texture) {
    BattleParticipant p;
    p.id = id;
    p.name = name;
    p.hp = p.maxHp = hp;
    p.mp = p.maxMp = mp;
    p.position = pos;
    p.isEnemy = true;
    
    if (texture) {
        // Enemis usually use a single frame or different config
        p.animator = std::make_unique<urpg::SpriteAnimator>(texture);
    }
    
    m_participants.push_back(std::move(p));
}

} // namespace urpg::scene
