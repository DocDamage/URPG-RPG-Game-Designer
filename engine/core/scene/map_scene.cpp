#include "map_scene.h"
#include "engine/core/animation/animation_ai_bridge.h"
#include "engine/core/audio/audio_ai_bridge.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/render/asset_loader.h"
#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/save/save_runtime.h"
#include "engine/core/save/save_serialization_hub.h"
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>
#include <utility>

namespace urpg::scene {

namespace {

constexpr const char* kMissingPlayerSpriteId = "missing_player_sprite";
constexpr const char* kMissingTilesetId = "missing_tileset";

bool BindingMatches(const MapScene::InteractionAbilityBinding& binding, const std::string& trigger_id,
                    std::optional<std::pair<int, int>> tile, std::optional<std::string_view> prop_asset_id,
                    std::optional<std::string_view> prop_instance_id = std::nullopt) {
    if (binding.trigger_id != trigger_id) {
        return false;
    }

    switch (binding.scope) {
    case MapScene::InteractionBindingScope::Global:
        return true;
    case MapScene::InteractionBindingScope::Tile:
        return tile.has_value() && binding.tile_x == tile->first && binding.tile_y == tile->second;
    case MapScene::InteractionBindingScope::Prop:
        if (!binding.prop_instance_id.empty()) {
            return prop_instance_id.has_value() && binding.prop_instance_id == *prop_instance_id;
        }
        return prop_asset_id.has_value() && binding.prop_asset_id == *prop_asset_id;
    case MapScene::InteractionBindingScope::Region:
        return tile.has_value() && tile->first >= binding.region_min_x && tile->first <= binding.region_max_x &&
               tile->second >= binding.region_min_y && tile->second <= binding.region_max_y;
    }

    return false;
}

bool BindingKeyMatches(const MapScene::InteractionAbilityBinding& binding, MapScene::InteractionBindingScope scope,
                       const std::string& trigger_id, std::optional<std::pair<int, int>> tile,
                       std::optional<std::string_view> prop_asset_id,
                       std::optional<std::string_view> prop_instance_id = std::nullopt) {
    if (binding.scope != scope || binding.trigger_id != trigger_id) {
        return false;
    }

    switch (scope) {
    case MapScene::InteractionBindingScope::Global:
        return true;
    case MapScene::InteractionBindingScope::Tile:
        return tile.has_value() && binding.tile_x == tile->first && binding.tile_y == tile->second;
    case MapScene::InteractionBindingScope::Prop:
        if (prop_instance_id.has_value()) {
            return binding.prop_instance_id == *prop_instance_id;
        }
        return binding.prop_instance_id.empty() && prop_asset_id.has_value() && binding.prop_asset_id == *prop_asset_id;
    case MapScene::InteractionBindingScope::Region:
        return tile.has_value() && binding.region_min_x == tile->first && binding.region_min_y == tile->second &&
               binding.region_max_x == tile->first && binding.region_max_y == tile->second;
    }

    return false;
}

std::filesystem::path resolveProjectPath(const std::filesystem::path& project_root, const std::filesystem::path& path) {
    if (path.empty() || path.is_absolute()) {
        return path;
    }
    return project_root / path;
}

MapAssetReference readAssetReference(const nlohmann::json& root, const char* key) {
    MapAssetReference reference;
    if (!root.contains(key) || !root.at(key).is_object()) {
        return reference;
    }

    const auto& asset = root.at(key);
    if (asset.contains("id") && asset.at("id").is_string()) {
        reference.id = asset.at("id").get<std::string>();
    }
    if (asset.contains("path") && asset.at("path").is_string()) {
        reference.path = asset.at("path").get<std::string>();
    }
    return reference;
}

const nlohmann::json* findMapAssets(const nlohmann::json& project, const std::string& map_id) {
    if (project.contains("startup") && project.at("startup").is_object()) {
        const auto& startup = project.at("startup");
        if (startup.contains("map_assets") && startup.at("map_assets").is_object()) {
            return &startup.at("map_assets");
        }
    }

    if (!project.contains("maps") || !project.at("maps").is_array()) {
        return nullptr;
    }

    for (const auto& map : project.at("maps")) {
        if (!map.is_object() || map.value("id", "") != map_id) {
            continue;
        }
        if (map.contains("assets") && map.at("assets").is_object()) {
            return &map.at("assets");
        }
    }
    return nullptr;
}

urpg::Vector3 interpolateAnimationTrack(const std::vector<urpg::AnimationKeyframe>& track, urpg::Fixed32 time) {
    if (track.empty()) {
        return urpg::Vector3::Zero();
    }
    if (time <= track.front().time) {
        return track.front().value;
    }
    if (time >= track.back().time) {
        return track.back().value;
    }

    for (size_t i = 0; i + 1 < track.size(); ++i) {
        const auto& from = track[i];
        const auto& to = track[i + 1];
        if (time < from.time || time > to.time) {
            continue;
        }

        const auto range = to.time - from.time;
        if (range.raw == 0) {
            return to.value;
        }

        const float t = (time - from.time).ToFloat() / range.ToFloat();
        urpg::Vector3 value;
        value.x =
            urpg::Fixed32::FromFloat(from.value.x.ToFloat() + (to.value.x.ToFloat() - from.value.x.ToFloat()) * t);
        value.y =
            urpg::Fixed32::FromFloat(from.value.y.ToFloat() + (to.value.y.ToFloat() - from.value.y.ToFloat()) * t);
        value.z =
            urpg::Fixed32::FromFloat(from.value.z.ToFloat() + (to.value.z.ToFloat() - from.value.z.ToFloat()) * t);
        return value;
    }

    return track.back().value;
}

std::string trimWhitespace(std::string value);

std::string extractAiAnimationTarget(const std::string& aiResponse) {
    const size_t marker = aiResponse.find("[TARGET:");
    if (marker == std::string::npos) {
        return "player";
    }
    const size_t close = aiResponse.find(']', marker);
    if (close == std::string::npos) {
        return "unknown";
    }
    return trimWhitespace(
        aiResponse.substr(marker + std::string("[TARGET:").size(), close - marker - std::string("[TARGET:").size()));
}

bool isSupportedPlayerAnimationTarget(std::string target) {
    std::transform(target.begin(), target.end(), target.begin(),
                   [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return target == "player" || target == "hero";
}

std::string trimWhitespace(std::string value) {
    const auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

void eraseLastUtf8Codepoint(std::string& text) {
    if (text.empty()) {
        return;
    }

    size_t eraseFrom = text.size() - 1;
    while (eraseFrom > 0 && (static_cast<unsigned char>(text[eraseFrom]) & 0xC0U) == 0x80U) {
        --eraseFrom;
    }
    text.erase(eraseFrom);
}

urpg::RuntimeSaveLoadRequest makeMapSceneSaveRequest(const std::filesystem::path& projectRoot, int slotId) {
    const auto saveRoot = urpg::defaultRuntimeSaveRoot(projectRoot);
    const auto slotName = "slot_" + std::to_string(slotId);

    urpg::RuntimeSaveLoadRequest request;
    request.primary_save_path = saveRoot / (slotName + ".json");
    request.autosave_path = saveRoot / "autosave.json";
    request.metadata_path = saveRoot / (slotName + "_meta.json");
    request.variables_path = saveRoot / (slotName + "_vars.json");
    return request;
}

} // namespace

MapScene::MapScene(const std::string& mapId, int width, int height)
    : m_mapId(mapId), m_width(std::max(0, width)), m_height(std::max(0, height)) {
    m_tiles.resize(static_cast<size_t>(m_width * m_height), {0, true});
    m_renderer = std::make_unique<TilemapRenderer>(m_width, m_height);

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
    validateRenderAssetReferences();

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

    if (m_playerAiAnimation.has_value()) {
        auto& anim = *m_playerAiAnimation;
        if (anim.isPlaying) {
            anim.currentTime = anim.currentTime + urpg::Fixed32::FromFloat(deltaTime);
            if (anim.currentTime >= anim.duration) {
                if (anim.isLooping && anim.duration.raw > 0) {
                    anim.currentTime = urpg::Fixed32::FromRaw(anim.currentTime.raw % anim.duration.raw);
                } else {
                    anim.currentTime = anim.duration;
                    anim.isPlaying = false;
                }
            }
            m_playerAiAnimationOffset = interpolateAnimationTrack(anim.positionTrack, anim.currentTime);
        }
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
    playerX += m_playerAiAnimationOffset.x.ToFloat();
    playerY += m_playerAiAnimationOffset.y.ToFloat();

    urpg::SpriteCommand playerCmd;
    playerCmd.textureId =
        m_assetReferences.player_sprite.id.empty() ? kMissingPlayerSpriteId : m_assetReferences.player_sprite.id;
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
            tileCmd.tilesetId = m_assetReferences.tileset.id.empty() ? kMissingTilesetId : m_assetReferences.tileset.id;
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
        for (size_t i = 0; i < input.backspaceCount() && !m_currentInputBuffer.empty(); ++i) {
            eraseLastUtf8Codepoint(m_currentInputBuffer);
        }
        if (!input.textInput().empty()) {
            m_currentInputBuffer += input.textInput();
        }

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

        // Fallback project interaction path used when no authored interaction ability handles the tile.
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
        drawX += m_playerAiAnimationOffset.x.ToFloat();
        drawY += m_playerAiAnimationOffset.y.ToFloat();

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

void MapScene::setAssetReferences(MapAssetReferences references) {
    m_assetReferences = std::move(references);
    m_assetDiagnostics.clear();
    m_assetReferencesValidated = false;
    m_renderLayerDirty = true;
}

void MapScene::setRuntimeAssetMode(urpg::RuntimeAssetMode mode) {
    if (m_runtimeAssetMode == mode) {
        return;
    }

    m_runtimeAssetMode = mode;
    m_assetDiagnostics.clear();
    m_assetReferencesValidated = false;
}

void MapScene::validateRenderAssetReferences() {
    if (m_assetReferencesValidated) {
        return;
    }
    m_assetReferencesValidated = true;

    const auto validate = [this](const MapAssetReference& reference, const char* role, const char* code,
                                 const char* missingIdDiagnostic) {
        if (reference.id.empty()) {
            const std::string message = std::string("Map '") + m_mapId + "' has no " + role +
                                        " asset id; rendering will use a diagnostic fallback visual.";
            if (m_runtimeAssetMode == urpg::RuntimeAssetMode::Release) {
                m_assetDiagnostics.push_back(std::string(missingIdDiagnostic) + "_release_blocking");
                urpg::diagnostics::RuntimeDiagnostics::error("scene.map", code, message);
            } else {
                m_assetDiagnostics.push_back(missingIdDiagnostic);
                urpg::diagnostics::RuntimeDiagnostics::warning("scene.map", code, message);
            }
            return;
        }

        if (reference.path.empty()) {
            return;
        }

        std::error_code ec;
        if (!std::filesystem::is_regular_file(reference.path, ec) || ec) {
            const std::string message = std::string("Map '") + m_mapId + "' references missing " + role +
                                        " asset path: " + reference.path.generic_string() +
                                        "; rendering will use logical id '" + reference.id + "'.";
            if (m_runtimeAssetMode == urpg::RuntimeAssetMode::Release) {
                m_assetDiagnostics.push_back(std::string(role) + "_path_missing_release_blocking");
                urpg::diagnostics::RuntimeDiagnostics::error("scene.map", code, message);
            } else {
                m_assetDiagnostics.push_back(std::string(role) + "_path_missing");
                urpg::diagnostics::RuntimeDiagnostics::warning("scene.map", code, message);
            }
        }
    };

    validate(m_assetReferences.player_sprite, "player_sprite", "map.player_sprite_missing", "player_sprite_id_missing");
    validate(m_assetReferences.tileset, "tileset", "map.tileset_missing", "tileset_id_missing");
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

    const auto target = extractAiAnimationTarget(aiResponse);
    if (!isSupportedPlayerAnimationTarget(target)) {
        m_aiAnimationDiagnostics.push_back("unsupported_animation_target:" + target);
        urpg::diagnostics::RuntimeDiagnostics::warning("scene.map", "map.ai_animation_target_unsupported",
                                                       "AI animation command target '" + target +
                                                           "' is not supported by the MapScene runtime.");
        return;
    }

    urpg::AnimationComponent anim;
    anim.positionTrack = std::move(keyframes);
    anim.duration = anim.positionTrack.back().time;
    anim.currentTime = urpg::Fixed32::FromInt(0);
    anim.isPlaying = true;
    anim.isLooping = false;
    m_playerAiAnimationOffset = interpolateAnimationTrack(anim.positionTrack, anim.currentTime);
    m_playerAiAnimation = std::move(anim);
}

bool MapScene::saveGame(int slotId) {
    return saveGameDetailed(slotId).ok;
}

MapSceneSaveLoadResult MapScene::saveGameDetailed(int slotId) {
    auto& hub = urpg::GlobalStateHub::getInstance();
    std::string snapshot = urpg::save::SaveSerializationHub::snapshotGlobalState(hub);

    const auto request = makeMapSceneSaveRequest(m_projectRoot, slotId);
    MapSceneSaveLoadResult result;
    result.operation = MapSceneSaveLoadOperation::Save;
    result.slot_id = slotId;
    result.primary_path = request.primary_save_path;
    result.ok = urpg::RuntimeSaveLoader::Save(request, snapshot);
    if (!result.ok) {
        result.failure_reason = "runtime_save_write_failed";
        result.diagnostics.push_back("save_write_failed");
        urpg::diagnostics::RuntimeDiagnostics::error("scene.map", "map.save_failed",
                                                     "Failed to save map scene slot " + std::to_string(slotId) +
                                                         " to '" + request.primary_save_path.generic_string() + "'.");
    }

    m_lastSaveLoadResult = result;
    return result;
}

bool MapScene::loadGame(int slotId) {
    return loadGameDetailed(slotId).ok;
}

MapSceneSaveLoadResult MapScene::loadGameDetailed(int slotId) {
    const auto request = makeMapSceneSaveRequest(m_projectRoot, slotId);

    auto loadResult = urpg::RuntimeSaveLoader::Load(request);
    MapSceneSaveLoadResult result;
    result.operation = MapSceneSaveLoadOperation::Load;
    result.slot_id = slotId;
    result.primary_path = request.primary_save_path;
    result.ok = loadResult.ok;
    result.recovery_tier = loadResult.recovery_tier;
    result.loaded_from_recovery = loadResult.loaded_from_recovery;
    result.boot_safe_mode = loadResult.boot_safe_mode;
    result.failure_reason = loadResult.error;
    result.diagnostics = loadResult.diagnostics;

    if (loadResult.ok) {
        auto& hub = urpg::GlobalStateHub::getInstance();
        urpg::save::SaveSerializationHub::restoreGlobalState(hub, loadResult.payload);
        if (loadResult.loaded_from_recovery) {
            urpg::diagnostics::RuntimeDiagnostics::warning(
                "scene.map", "map.load_recovered",
                "Loaded map scene slot " + std::to_string(slotId) + " from recovery after primary save '" +
                    request.primary_save_path.generic_string() + "' was unavailable or invalid.");
        }
    } else {
        if (result.failure_reason.empty()) {
            result.failure_reason = "runtime_save_load_failed";
        }
        result.diagnostics.push_back(result.failure_reason);
        urpg::diagnostics::RuntimeDiagnostics::error("scene.map", "map.load_failed",
                                                     "Failed to load map scene slot " + std::to_string(slotId) +
                                                         " from '" + request.primary_save_path.generic_string() +
                                                         "': " + result.failure_reason);
    }

    m_lastSaveLoadResult = result;
    return result;
}

void MapScene::setProjectRoot(std::filesystem::path project_root) {
    m_projectRoot = std::move(project_root);
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

bool MapScene::bindPropInstanceInteractionAbility(const std::string& trigger_id, const std::string& prop_instance_id,
                                                  const std::string& prop_asset_id, const std::string& asset_path,
                                                  const urpg::ability::AuthoredAbilityAsset& asset) {
    if (trigger_id.empty() || prop_instance_id.empty() || asset.ability_id.empty()) {
        return false;
    }

    grantPlayerAbility(asset);

    for (auto& binding : m_interaction_ability_bindings) {
        if (BindingKeyMatches(binding, InteractionBindingScope::Prop, trigger_id, std::nullopt, prop_asset_id,
                              prop_instance_id)) {
            binding.asset_path = asset_path;
            binding.ability_id = asset.ability_id;
            binding.prop_asset_id = prop_asset_id;
            return true;
        }
    }

    InteractionAbilityBinding binding;
    binding.scope = InteractionBindingScope::Prop;
    binding.trigger_id = trigger_id;
    binding.asset_path = asset_path;
    binding.ability_id = asset.ability_id;
    binding.prop_instance_id = prop_instance_id;
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

bool MapScene::activateInteractionAbilityForPropInstance(const std::string& trigger_id,
                                                         const std::string& prop_instance_id,
                                                         const std::string& prop_asset_id) {
    for (const auto& binding : m_interaction_ability_bindings) {
        if (!binding.prop_instance_id.empty() &&
            BindingMatches(binding, trigger_id, std::nullopt, prop_asset_id, prop_instance_id)) {
            return tryActivatePlayerAbility(binding.ability_id);
        }
    }

    if (!prop_asset_id.empty()) {
        return activateInteractionAbilityForProp(trigger_id, prop_asset_id);
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

MapAssetReferences loadRuntimeMapAssetReferences(const std::filesystem::path& project_root, const std::string& map_id) {
    MapAssetReferences references;
    const auto project_path = project_root / "project.json";
    std::ifstream in(project_path, std::ios::binary);
    if (!in) {
        urpg::diagnostics::RuntimeDiagnostics::warning(
            "scene.map", "map.project_manifest_missing",
            "Runtime project manifest was not found while resolving map assets: " + project_path.generic_string());
        return references;
    }

    const auto project = nlohmann::json::parse(in, nullptr, false);
    if (project.is_discarded() || !project.is_object()) {
        urpg::diagnostics::RuntimeDiagnostics::warning(
            "scene.map", "map.project_manifest_invalid",
            "Runtime project manifest is invalid while resolving map assets: " + project_path.generic_string());
        return references;
    }

    const auto* assets = findMapAssets(project, map_id);
    if (assets == nullptr) {
        urpg::diagnostics::RuntimeDiagnostics::warning(
            "scene.map", "map.asset_references_missing",
            "Runtime project manifest has no map asset references for map '" + map_id + "'.");
        return references;
    }

    references.player_sprite = readAssetReference(*assets, "player_sprite");
    references.tileset = readAssetReference(*assets, "tileset");
    references.player_sprite.path = resolveProjectPath(project_root, references.player_sprite.path);
    references.tileset.path = resolveProjectPath(project_root, references.tileset.path);
    return references;
}

} // namespace urpg::scene
