#include "map_scene.h"
#include "engine/core/animation/animation_ai_bridge.h"
#include "engine/core/audio/audio_ai_bridge.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/render/asset_loader.h"
#include "engine/core/save/save_runtime.h"
#include "engine/core/save/save_serialization_hub.h"
#include <utility>

namespace urpg::scene {

namespace {

bool BindingMatches(const MapScene::InteractionAbilityBinding& binding, const std::string& trigger_id,
                    std::optional<std::pair<int, int>> tile, std::optional<std::string_view> prop_asset_id) {
    if (binding.trigger_id != trigger_id) {
        return false;
    }

    switch (binding.scope) {
    case MapScene::InteractionBindingScope::Global:
        return true;
    case MapScene::InteractionBindingScope::Tile:
        return tile.has_value() && binding.tile_x == tile->first && binding.tile_y == tile->second;
    case MapScene::InteractionBindingScope::Prop:
        return prop_asset_id.has_value() && binding.prop_asset_id == *prop_asset_id;
    case MapScene::InteractionBindingScope::Region:
        return tile.has_value() && tile->first >= binding.region_min_x && tile->first <= binding.region_max_x &&
               tile->second >= binding.region_min_y && tile->second <= binding.region_max_y;
    }

    return false;
}

bool BindingKeyMatches(const MapScene::InteractionAbilityBinding& binding, MapScene::InteractionBindingScope scope,
                       const std::string& trigger_id, std::optional<std::pair<int, int>> tile,
                       std::optional<std::string_view> prop_asset_id) {
    if (binding.scope != scope || binding.trigger_id != trigger_id) {
        return false;
    }

    switch (scope) {
    case MapScene::InteractionBindingScope::Global:
        return true;
    case MapScene::InteractionBindingScope::Tile:
        return tile.has_value() && binding.tile_x == tile->first && binding.tile_y == tile->second;
    case MapScene::InteractionBindingScope::Prop:
        return prop_asset_id.has_value() && binding.prop_asset_id == *prop_asset_id;
    case MapScene::InteractionBindingScope::Region:
        return tile.has_value() && binding.region_min_x == tile->first && binding.region_min_y == tile->second &&
               binding.region_max_x == tile->first && binding.region_max_y == tile->second;
    }

    return false;
}

} // namespace

MapScene::MapScene(const std::string& mapId, int width, int height) : m_mapId(mapId), m_width(width), m_height(height) {
    m_tiles.resize(width * height, {0, true});
    m_renderer = std::make_unique<TilemapRenderer>(width, height);

    // Initialize player movement component
    m_playerMovement.gridPos = {0, 0};
    m_playerMovement.lastGridPos = {0, 0};
    m_playerMovement.moveSpeed = 4.0f;
    m_playerMovement.isMoving = false;
    m_playerAbilitySystem.setAttribute("MP", 30.0f);
    m_playerAbilitySystem.setAttribute("Attack", 100.0f);
    m_playerAbilitySystem.setAttribute("Defense", 100.0f);
    m_playerAbilitySystem.setAttribute("MagicDefense", 100.0f);
}

void MapScene::onUpdate(float deltaTime) {
    // Keep RenderLayer in sync for scene/engine tests and headless render pipelines.
    auto& layer = urpg::RenderLayer::getInstance();
    layer.flush();

    // 0. Update message runner and UI components
    if (m_messageRunner.isActive()) {
        const auto* page = m_messageRunner.currentPage();
        if (page != nullptr) {
            // Message window background
            urpg::RectCommand rectCmd;
            rectCmd.x = 20.0f;
            rectCmd.y = 280.0f;
            rectCmd.w = 600.0f;
            rectCmd.h = 120.0f;
            rectCmd.r = 0.1f;
            rectCmd.g = 0.1f;
            rectCmd.b = 0.15f;
            rectCmd.a = 0.9f;
            rectCmd.zOrder = 50;
            layer.submit(urpg::toFrameRenderCommand(rectCmd));

            // Message body text
            urpg::TextCommand textCmd;
            textCmd.text = page->body;
            textCmd.x = 40.0f;
            textCmd.y = 300.0f;
            textCmd.fontSize = 22;
            textCmd.maxWidth = 560;
            textCmd.zOrder = 51;
            layer.submit(urpg::toFrameRenderCommand(textCmd));
        }
    }

    if (m_chatUI && m_isChatInputOpen) {
        m_chatUI->update(deltaTime);
        m_chatUI->setInputBuffer(m_currentInputBuffer);
    }

    // 1. Process movement transitions
    urpg::MovementSystem::Update(m_playerMovement, deltaTime);

    // 2. Sync animator state to movement
    if (m_playerAnimator) {
        m_playerAnimator->setMoving(m_playerMovement.isMoving);
        m_playerAnimator->setDirection(m_playerMovement.direction);
        m_playerAnimator->update(deltaTime);
    }

    m_playerAbilitySystem.update(deltaTime);

    // 3. Submit tile and player render commands
    if (m_renderLayerDirty) {
        rebuildTileRenderCache();
    }
    submitCachedTileCommands(layer);

    constexpr float kTileSize = 48.0f;
    float playerX = static_cast<float>(m_playerMovement.gridPos.x) * kTileSize;
    float playerY = static_cast<float>(m_playerMovement.gridPos.y) * kTileSize;
    if (m_playerMovement.isMoving) {
        const float lastX = static_cast<float>(m_playerMovement.lastGridPos.x) * kTileSize;
        const float lastY = static_cast<float>(m_playerMovement.lastGridPos.y) * kTileSize;
        playerX = lastX + (playerX - lastX) * m_playerMovement.moveProgress;
        playerY = lastY + (playerY - lastY) * m_playerMovement.moveProgress;
    }

    urpg::SpriteCommand playerCmd;
    playerCmd.textureId = "hero_sprite";
    playerCmd.x = playerX;
    playerCmd.y = playerY;
    playerCmd.width = 48;
    playerCmd.height = 48;
    playerCmd.zOrder = 1;
    layer.submit(urpg::toFrameRenderCommand(playerCmd));
}

void MapScene::rebuildTileRenderCache() {
    constexpr float kTileSize = 48.0f;

    m_cachedTileCommands.clear();
    m_cachedTileCommands.reserve(static_cast<size_t>(m_width * m_height));

    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const auto& tile = m_tiles[static_cast<size_t>(y * m_width + x)];
            urpg::TileCommand tileCmd;
            tileCmd.tilesetId = "default_tileset";
            tileCmd.tileIndex = tile.tileId;
            tileCmd.x = static_cast<float>(x) * kTileSize;
            tileCmd.y = static_cast<float>(y) * kTileSize;
            tileCmd.zOrder = 0;
            m_cachedTileCommands.push_back(std::move(tileCmd));
        }
    }

    m_renderLayerDirty = false;
}

void MapScene::submitCachedTileCommands(urpg::RenderLayer& layer) const {
    for (const auto& tileCmd : m_cachedTileCommands) {
        layer.submit(urpg::toFrameRenderCommand(tileCmd));
    }
}

void MapScene::handleInput(const urpg::input::InputCore& input) {
    // 0. Chat Input Handling (High Priority)
    if (m_isChatInputOpen) {
        if (input.isActionJustPressed(urpg::input::InputAction::Cancel)) {
            m_isChatInputOpen = false;
            m_currentInputBuffer.clear();
        } else if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
            if (!m_currentInputBuffer.empty() && m_activeChatbot) {
                std::string question = m_currentInputBuffer;
                if (m_chatUI)
                    m_chatUI->addMessage("Player", question);

                // Submit to AI
                m_activeChatbot->getResponse(question, [this](urpg::message::DialoguePage page) {
                    const std::string& response = page.body;
                    if (m_chatUI) {
                        m_chatUI->addMessage("Guide", response);
                    } else {
                        // Fallback to dialogue pages if UI is disabled
                        page.variant.speaker = "Game Guide";
                        this->startDialogue({page});
                    }

                    // Parse and execute hidden audio and animation commands
                    this->processAiAudioCommands(response);
                    this->processAiAnimationCommands(response);
                });

                m_currentInputBuffer.clear();
                // Stay in chat mode until Cancel is pressed, allowing a back-and-forth
            }
        } else {
            // Mocking text entry since InputAction doesn't have character scan yet
            // In a real environment, we'd poll the engine's text event queue.
            // For now, let's keep it abstract.
        }
        return;
    }

    // 1. Normal Dialogue Handling
    if (m_messageRunner.isActive()) {
        if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
            if (m_messageRunner.state() == urpg::message::MessageFlowState::Presenting) {
                m_messageRunner.markPagePresented();
            } else if (m_messageRunner.state() == urpg::message::MessageFlowState::AwaitingAdvance) {
                m_messageRunner.advance();
            } else if (m_messageRunner.state() == urpg::message::MessageFlowState::AwaitingChoice) {
                auto selectedId = m_messageRunner.confirmChoice();
                if (selectedId.has_value() && !selectedId->empty()) {
                    // Start the next part of the conversation based on choice
                    auto& registry = urpg::message::DialogueRegistry::getInstance();
                    // In this model, the choice.id IS the next node_id
                    auto nextPages = registry.flattenConversation("intro_elder", *selectedId);
                    if (!nextPages.empty()) {
                        startDialogue(nextPages);
                    }
                }
            }
        } else if (m_messageRunner.state() == urpg::message::MessageFlowState::AwaitingChoice) {
            if (input.isActionJustPressed(urpg::input::InputAction::MoveUp))
                m_messageRunner.moveChoicePrev();
            if (input.isActionJustPressed(urpg::input::InputAction::MoveDown))
                m_messageRunner.moveChoiceNext();
        }
        return; // Block character movement during dialogue
    }

    if (m_playerMovement.isMoving)
        return;

    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
        if (activateInteractionAbilityAtTile("confirm_interact", m_playerMovement.gridPos.x,
                                             m_playerMovement.gridPos.y) ||
            activateInteractionAbility("confirm_interact")) {
            return;
        }

        // In a real game, we'd check for the interact target tile here.
        // For testing, let's trigger the mock "intro_elder" dialogue if we interact with anything
        auto& registry = urpg::message::DialogueRegistry::getInstance();
        auto pages = registry.flattenConversation("intro_elder");
        if (!pages.empty()) {
            startDialogue(pages);
        }
    }

    urpg::Direction moveDir = urpg::Direction::Down;
    bool shouldMove = false;

    if (input.isActionActive(urpg::input::InputAction::MoveUp)) {
        moveDir = urpg::Direction::Up;
        shouldMove = true;
    } else if (input.isActionActive(urpg::input::InputAction::MoveDown)) {
        moveDir = urpg::Direction::Down;
        shouldMove = true;
    } else if (input.isActionActive(urpg::input::InputAction::MoveLeft)) {
        moveDir = urpg::Direction::Left;
        shouldMove = true;
    } else if (input.isActionActive(urpg::input::InputAction::MoveRight)) {
        moveDir = urpg::Direction::Right;
        shouldMove = true;
    }

    if (shouldMove) {
        auto collisionCheck = [this](int x, int y) { return this->checkCollision(x, y); };
        urpg::MovementSystem::TryMove(m_playerMovement, moveDir, collisionCheck);
    }
}

void MapScene::draw(SpriteBatcher& batcher) {
    if (m_renderer) {
        m_renderer->draw(batcher);
    }

    if (m_playerAnimator) {
        constexpr float kTileSize = 48.0f;
        float drawX = static_cast<float>(m_playerMovement.gridPos.x) * kTileSize;
        float drawY = static_cast<float>(m_playerMovement.gridPos.y) * kTileSize;
        if (m_playerMovement.isMoving) {
            const float lastX = static_cast<float>(m_playerMovement.lastGridPos.x) * kTileSize;
            const float lastY = static_cast<float>(m_playerMovement.lastGridPos.y) * kTileSize;
            drawX = lastX + (drawX - lastX) * m_playerMovement.moveProgress;
            drawY = lastY + (drawY - lastY) * m_playerMovement.moveProgress;
        }

        m_playerAnimator->draw(batcher, drawX, drawY, kTileSize, kTileSize, 1.0f);
    }

    // Draw UI components on top of the world
    if (m_isChatInputOpen && m_chatUI) {
        m_chatUI->draw(batcher);
    }
}

void MapScene::setLayerData(int layer, const std::vector<int>& data) {
    if (m_renderer) {
        m_renderer->setLayer(layer, data);
    }
}

void MapScene::setTileset(const std::shared_ptr<Texture>& tileset) {
    if (m_renderer) {
        m_renderer->setTileset(tileset);
    }
}

void MapScene::setPlayerCharacter(const std::string& name, int /*index*/) {
    auto texture = urpg::AssetLoader::loadTexture("img/characters/" + name + ".png");
    m_playerAnimator = std::make_unique<SpriteAnimator>(texture);
    // index * 3 is typical for character sheet offset but simplified here
}

void MapScene::startDialogue(const std::vector<urpg::message::DialoguePage>& pages) {
    m_messageRunner.begin(pages);
}

void MapScene::startChatbot(const std::string& systemPrompt, std::shared_ptr<urpg::ai::IChatService> service) {
    m_activeChatbot = std::make_unique<urpg::ai::ChatbotComponent>(service);
    m_activeChatbot->setSystemPrompt(systemPrompt);
}

void MapScene::openChatInput() {
    m_isChatInputOpen = true;
    m_currentInputBuffer = ""; // Reset for new question
    if (!m_chatUI) {
        m_chatUI = std::make_unique<urpg::ui::ChatWindow>();
    }
}

void MapScene::processAiAudioCommands(const std::string& aiResponse) {
    auto commands = urpg::ai::AudioKnowledgeBridge::parseAudioCommands(aiResponse);
    if (commands.empty())
        return;
    if (!m_audioCore)
        return;

    for (const auto& cmd : commands) {
        if (cmd.action == "PLAY_BGM") {
            m_audioCore->playBGM(cmd.assetId, 0.0f); // Immediate
        } else if (cmd.action == "CROSSFADE") {
            m_audioCore->playBGM(cmd.assetId, cmd.fadeTime);
        } else if (cmd.action == "PLAY_SE") {
            m_audioCore->playSound(cmd.assetId, urpg::audio::AudioCategory::SE);
        } else if (cmd.action == "STOP") {
            m_audioCore->stopAll();
        }
    }
}

void MapScene::processAiAnimationCommands(const std::string& aiResponse) {
    auto keyframes = urpg::ai::AnimationKnowledgeBridge::parseKeyframes(aiResponse);
    if (keyframes.empty())
        return;

    // Apply the keyframes to the player's animation component
    // In a full ECS implementation, we would look up the target entity
    if (m_playerAnimator) {
        // Mocking the injection into a component-based system
        [[maybe_unused]] urpg::AnimationComponent anim;
        anim.positionTrack = keyframes;
        anim.duration = keyframes.back().time;
        anim.isPlaying = true;
        anim.isLooping = false;

        // m_playerAnimator->applySequence(anim);
    }
}

bool MapScene::saveGame(int slotId) {
    auto& hub = urpg::GlobalStateHub::getInstance();
    std::string snapshot = urpg::save::SaveSerializationHub::snapshotGlobalState(hub);

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = "saves/slot_" + std::to_string(slotId) + ".usr";

    return urpg::RuntimeSaveLoader::Save(request, snapshot);
}

bool MapScene::loadGame(int slotId) {
    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = "saves/slot_" + std::to_string(slotId) + ".usr";

    auto result = urpg::RuntimeSaveLoader::Load(request);
    if (result.ok) {
        auto& hub = urpg::GlobalStateHub::getInstance();
        urpg::save::SaveSerializationHub::restoreGlobalState(hub, result.payload);
        return true;
    }
    return false;
}

void MapScene::grantPlayerAbility(const urpg::ability::AuthoredAbilityAsset& asset) {
    m_playerAbilitySystem.grantOrReplaceAbility(urpg::ability::makeGameplayAbilityFromAsset(asset));
}

bool MapScene::tryActivatePlayerAbility(const std::string& ability_id) {
    for (const auto& ability : m_playerAbilitySystem.getAbilities()) {
        if (ability && ability->getId() == ability_id) {
            return m_playerAbilitySystem.tryActivateAbility(*ability);
        }
    }

    return false;
}

bool MapScene::bindInteractionAbility(const std::string& trigger_id, const std::string& asset_path,
                                      const urpg::ability::AuthoredAbilityAsset& asset) {
    if (trigger_id.empty() || asset.ability_id.empty()) {
        return false;
    }

    grantPlayerAbility(asset);

    for (auto& binding : m_interaction_ability_bindings) {
        if (BindingKeyMatches(binding, InteractionBindingScope::Global, trigger_id, std::nullopt, std::nullopt)) {
            binding.asset_path = asset_path;
            binding.ability_id = asset.ability_id;
            return true;
        }
    }

    InteractionAbilityBinding binding;
    binding.scope = InteractionBindingScope::Global;
    binding.trigger_id = trigger_id;
    binding.asset_path = asset_path;
    binding.ability_id = asset.ability_id;
    m_interaction_ability_bindings.push_back(std::move(binding));
    return true;
}

bool MapScene::bindTileInteractionAbility(const std::string& trigger_id, int tile_x, int tile_y,
                                          const std::string& asset_path,
                                          const urpg::ability::AuthoredAbilityAsset& asset) {
    if (trigger_id.empty() || asset.ability_id.empty()) {
        return false;
    }

    grantPlayerAbility(asset);

    for (auto& binding : m_interaction_ability_bindings) {
        if (BindingKeyMatches(binding, InteractionBindingScope::Tile, trigger_id, std::make_pair(tile_x, tile_y),
                              std::nullopt)) {
            binding.asset_path = asset_path;
            binding.ability_id = asset.ability_id;
            return true;
        }
    }

    InteractionAbilityBinding binding;
    binding.scope = InteractionBindingScope::Tile;
    binding.trigger_id = trigger_id;
    binding.asset_path = asset_path;
    binding.ability_id = asset.ability_id;
    binding.tile_x = tile_x;
    binding.tile_y = tile_y;
    m_interaction_ability_bindings.push_back(std::move(binding));
    return true;
}

bool MapScene::bindPropInteractionAbility(const std::string& trigger_id, const std::string& prop_asset_id,
                                          const std::string& asset_path,
                                          const urpg::ability::AuthoredAbilityAsset& asset) {
    if (trigger_id.empty() || prop_asset_id.empty() || asset.ability_id.empty()) {
        return false;
    }

    grantPlayerAbility(asset);

    for (auto& binding : m_interaction_ability_bindings) {
        if (BindingKeyMatches(binding, InteractionBindingScope::Prop, trigger_id, std::nullopt, prop_asset_id)) {
            binding.asset_path = asset_path;
            binding.ability_id = asset.ability_id;
            return true;
        }
    }

    InteractionAbilityBinding binding;
    binding.scope = InteractionBindingScope::Prop;
    binding.trigger_id = trigger_id;
    binding.asset_path = asset_path;
    binding.ability_id = asset.ability_id;
    binding.prop_asset_id = prop_asset_id;
    m_interaction_ability_bindings.push_back(std::move(binding));
    return true;
}

bool MapScene::bindRegionInteractionAbility(const std::string& trigger_id, int min_tile_x, int min_tile_y,
                                            int max_tile_x, int max_tile_y, const std::string& asset_path,
                                            const urpg::ability::AuthoredAbilityAsset& asset) {
    if (trigger_id.empty() || asset.ability_id.empty()) {
        return false;
    }

    const int resolved_min_x = std::min(min_tile_x, max_tile_x);
    const int resolved_min_y = std::min(min_tile_y, max_tile_y);
    const int resolved_max_x = std::max(min_tile_x, max_tile_x);
    const int resolved_max_y = std::max(min_tile_y, max_tile_y);

    grantPlayerAbility(asset);

    for (auto& binding : m_interaction_ability_bindings) {
        if (binding.scope == InteractionBindingScope::Region && binding.trigger_id == trigger_id &&
            binding.region_min_x == resolved_min_x && binding.region_min_y == resolved_min_y &&
            binding.region_max_x == resolved_max_x && binding.region_max_y == resolved_max_y) {
            binding.asset_path = asset_path;
            binding.ability_id = asset.ability_id;
            return true;
        }
    }

    InteractionAbilityBinding binding;
    binding.scope = InteractionBindingScope::Region;
    binding.trigger_id = trigger_id;
    binding.asset_path = asset_path;
    binding.ability_id = asset.ability_id;
    binding.region_min_x = resolved_min_x;
    binding.region_min_y = resolved_min_y;
    binding.region_max_x = resolved_max_x;
    binding.region_max_y = resolved_max_y;
    m_interaction_ability_bindings.push_back(std::move(binding));
    return true;
}

bool MapScene::unbindTileInteractionAbility(const std::string& trigger_id, int tile_x, int tile_y) {
    const auto before = m_interaction_ability_bindings.size();
    m_interaction_ability_bindings.erase(
        std::remove_if(m_interaction_ability_bindings.begin(), m_interaction_ability_bindings.end(),
                       [&](const InteractionAbilityBinding& binding) {
                           return BindingKeyMatches(binding, InteractionBindingScope::Tile, trigger_id,
                                                    std::make_pair(tile_x, tile_y), std::nullopt);
                       }),
        m_interaction_ability_bindings.end());
    return m_interaction_ability_bindings.size() != before;
}

bool MapScene::unbindPropInteractionAbility(const std::string& trigger_id, const std::string& prop_asset_id) {
    const auto before = m_interaction_ability_bindings.size();
    m_interaction_ability_bindings.erase(
        std::remove_if(m_interaction_ability_bindings.begin(), m_interaction_ability_bindings.end(),
                       [&](const InteractionAbilityBinding& binding) {
                           return BindingKeyMatches(binding, InteractionBindingScope::Prop, trigger_id, std::nullopt,
                                                    prop_asset_id);
                       }),
        m_interaction_ability_bindings.end());
    return m_interaction_ability_bindings.size() != before;
}

bool MapScene::unbindRegionInteractionAbility(const std::string& trigger_id, int min_tile_x, int min_tile_y,
                                              int max_tile_x, int max_tile_y) {
    const int resolved_min_x = std::min(min_tile_x, max_tile_x);
    const int resolved_min_y = std::min(min_tile_y, max_tile_y);
    const int resolved_max_x = std::max(min_tile_x, max_tile_x);
    const int resolved_max_y = std::max(min_tile_y, max_tile_y);
    const auto before = m_interaction_ability_bindings.size();
    m_interaction_ability_bindings.erase(
        std::remove_if(m_interaction_ability_bindings.begin(), m_interaction_ability_bindings.end(),
                       [&](const InteractionAbilityBinding& binding) {
                           return binding.scope == InteractionBindingScope::Region &&
                                  binding.trigger_id == trigger_id && binding.region_min_x == resolved_min_x &&
                                  binding.region_min_y == resolved_min_y && binding.region_max_x == resolved_max_x &&
                                  binding.region_max_y == resolved_max_y;
                       }),
        m_interaction_ability_bindings.end());
    return m_interaction_ability_bindings.size() != before;
}

bool MapScene::activateInteractionAbility(const std::string& trigger_id) {
    for (const auto& binding : m_interaction_ability_bindings) {
        if (BindingMatches(binding, trigger_id, std::nullopt, std::nullopt)) {
            return tryActivatePlayerAbility(binding.ability_id);
        }
    }

    return false;
}

bool MapScene::activateInteractionAbilityAtTile(const std::string& trigger_id, int tile_x, int tile_y) {
    for (const auto& binding : m_interaction_ability_bindings) {
        if (BindingMatches(binding, trigger_id, std::make_pair(tile_x, tile_y), std::nullopt)) {
            return tryActivatePlayerAbility(binding.ability_id);
        }
    }

    return false;
}

bool MapScene::activateInteractionAbilityForProp(const std::string& trigger_id, const std::string& prop_asset_id) {
    for (const auto& binding : m_interaction_ability_bindings) {
        if (BindingMatches(binding, trigger_id, std::nullopt, prop_asset_id)) {
            return tryActivatePlayerAbility(binding.ability_id);
        }
    }

    return false;
}

bool MapScene::hasInteractionAbilityBinding(const std::string& trigger_id) const {
    for (const auto& binding : m_interaction_ability_bindings) {
        if (binding.trigger_id == trigger_id) {
            return true;
        }
    }

    return false;
}

} // namespace urpg::scene
