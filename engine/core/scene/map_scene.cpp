#include "map_scene.h"
#include "engine/core/animation/animation_ai_bridge.h"
#include "engine/core/audio/audio_ai_bridge.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/diagnostics/runtime_diagnostics.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/global_state_hub.h"
#include "engine/core/assets/texture_registry.h"
#include "engine/core/render/asset_loader.h"
#include "engine/core/save/runtime_save_startup.h"
#include "engine/core/save/save_runtime.h"
#include "engine/core/save/save_serialization_hub.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <limits>
#include <nlohmann/json.hpp>
#include <utility>

namespace urpg::scene {

namespace {

constexpr const char* kMissingPlayerSpriteId = "missing_player_sprite";
constexpr const char* kMissingTilesetId = "missing_tileset";

std::optional<bool> evaluateAuthoredDialogueCondition(const urpg::dialogue::DialogueCondition& condition) {
    if (condition.key.empty()) {
        return std::nullopt;
    }
    const auto value = urpg::GlobalStateHub::getInstance().getVariable(condition.key);
    int32_t actual = 0;
    if (const auto* integer = std::get_if<int32_t>(&value)) {
        actual = *integer;
    } else if (const auto* decimal = std::get_if<float>(&value)) {
        actual = static_cast<int32_t>(*decimal);
    } else if (const auto* boolean = std::get_if<bool>(&value)) {
        actual = *boolean ? 1 : 0;
    } else {
        return std::nullopt;
    }
    if (condition.op == "=" || condition.op == "==") return actual == condition.value;
    if (condition.op == "!=") return actual != condition.value;
    if (condition.op == ">") return actual > condition.value;
    if (condition.op == ">=") return actual >= condition.value;
    if (condition.op == "<") return actual < condition.value;
    if (condition.op == "<=") return actual <= condition.value;
    return std::nullopt;
}

int32_t authoredDialogueVariableValue(const std::string& key) {
    const auto value = urpg::GlobalStateHub::getInstance().getVariable(key);
    if (const auto* integer = std::get_if<int32_t>(&value)) return *integer;
    if (const auto* decimal = std::get_if<float>(&value)) return static_cast<int32_t>(*decimal);
    if (const auto* boolean = std::get_if<bool>(&value)) return *boolean ? 1 : 0;
    return 0;
}

int32_t saturatingDialogueEffectDelta(const int32_t current, const int32_t delta) {
    const int64_t result = static_cast<int64_t>(current) + static_cast<int64_t>(delta);
    if (result > std::numeric_limits<int32_t>::max()) {
        return std::numeric_limits<int32_t>::max();
    }
    if (result < std::numeric_limits<int32_t>::min()) {
        return std::numeric_limits<int32_t>::min();
    }
    return static_cast<int32_t>(result);
}

bool isStableDialogueProjectId(const std::string& dialogue_id) {
    return !dialogue_id.empty() &&
           std::all_of(dialogue_id.begin(), dialogue_id.end(), [](const unsigned char character) {
               return std::isalnum(character) || character == '_' || character == '-';
           });
}

std::string mapEventSelfSwitchStateKey(const std::string& map_id,
                                       const std::string& event_id,
                                       const std::string& key) {
    return "authored_map_self_switch:" + map_id + ":" + event_id + ":" + key;
}

std::string authoredDialogueStateValueText(const urpg::GlobalStateHub::Value& value) {
    if (const auto* integer = std::get_if<int32_t>(&value)) return std::to_string(*integer);
    if (const auto* decimal = std::get_if<float>(&value)) return std::to_string(*decimal);
    if (const auto* boolean = std::get_if<bool>(&value)) return *boolean ? "true" : "false";
    if (const auto* string = std::get_if<std::string>(&value)) return *string;
    return {};
}

bool isSupportedPerspectivePageComparison(const std::string& comparison) {
    return comparison.empty() || comparison == "equals" || comparison == "not_equals" ||
           comparison == "greater_equal" || comparison == "greater_than" || comparison == "less_equal" ||
           comparison == "less_than";
}

bool perspectivePageConditionMatches(const std::string& actual_value,
                                     const std::string& comparison,
                                     const std::string& expected_value) {
    if (comparison.empty() || comparison == "equals") {
        return actual_value == expected_value;
    }
    if (comparison == "not_equals") {
        return actual_value != expected_value;
    }

    char* actual_end = nullptr;
    char* expected_end = nullptr;
    const double actual = std::strtod(actual_value.c_str(), &actual_end);
    const double expected = std::strtod(expected_value.c_str(), &expected_end);
    if (actual_end == actual_value.c_str() || expected_end == expected_value.c_str() || actual_end == nullptr ||
        expected_end == nullptr || *actual_end != '\0' || *expected_end != '\0') {
        return false;
    }
    if (comparison == "greater_equal") return actual >= expected;
    if (comparison == "greater_than") return actual > expected;
    if (comparison == "less_equal") return actual <= expected;
    if (comparison == "less_than") return actual < expected;
    return false;
}

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

            if (!m_activeAuthoredDialogueCaption.empty()) {
                urpg::TextCommand captionCmd;
                captionCmd.text = m_activeAuthoredDialogueCaption;
                captionCmd.x = 40.0f;
                captionCmd.y = 252.0f;
                captionCmd.fontSize = 18;
                captionCmd.maxWidth = 560;
                captionCmd.zOrder = 49;
                layer.submit(urpg::toFrameRenderCommand(captionCmd));
            }

            if (m_messageRunner.state() == urpg::message::MessageFlowState::AwaitingChoice) {
                const auto& choice_prompt = m_messageRunner.choicePrompt();
                const auto& choices = choice_prompt.options();
                for (size_t index = 0; index < choices.size(); ++index) {
                    const auto& choice = choices[index];
                    urpg::TextCommand choiceCmd;
                    choiceCmd.text = std::string(choice.enabled && index == choice_prompt.selectedIndex() ? "> " : "  ") +
                                     (choice.enabled ? choice.label : "[Unavailable] " + choice.label);
                    choiceCmd.x = 40.0f;
                    choiceCmd.y = 332.0f + static_cast<float>(index) * 22.0f;
                    choiceCmd.fontSize = 18;
                    choiceCmd.maxWidth = 560;
                    choiceCmd.zOrder = 52;
                    layer.submit(urpg::toFrameRenderCommand(choiceCmd));
                }
            }
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

    const float animation_delta = std::max(deltaTime, 0.0f);
    for (size_t index = 0; index < m_eventSprites.size() && index < m_eventSpriteElapsedSeconds.size(); ++index) {
        if (m_eventSprites[index].frame_count > 1) {
            m_eventSpriteElapsedSeconds[index] += animation_delta;
        }
    }

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

    for (size_t index = 0; index < m_eventSprites.size(); ++index) {
        const auto& event_sprite = m_eventSprites[index];
        bool visible = event_sprite.default_visible;
        for (const auto& page : event_sprite.page_candidates) {
            if (authoredDialoguePageConditionsMatch(event_sprite.event_id, page.conditions) && page.has_visible_override) {
                visible = page.visible;
            }
        }
        if (!visible) {
            continue;
        }
        urpg::SpriteCommand eventCmd;
        eventCmd.textureId = event_sprite.asset.id;
        eventCmd.srcX = currentEventSpriteFrame(index) * event_sprite.frame_width;
        eventCmd.srcY = 0;
        eventCmd.x = static_cast<float>(event_sprite.tile_x) * kTileSize;
        eventCmd.y = static_cast<float>(event_sprite.tile_y) * kTileSize;
        eventCmd.width = event_sprite.frame_width;
        eventCmd.height = event_sprite.frame_height;
        eventCmd.zOrder = 2;
        layer.submit(urpg::toFrameRenderCommand(eventCmd));
    }
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
                    if (m_activeAuthoredDialogueGraph.has_value()) {
                        const auto* node = m_activeAuthoredDialogueGraph->findNode(m_activeAuthoredDialogueNodeId);
                        if (node != nullptr) {
                            const auto choice = std::find_if(node->choices.begin(), node->choices.end(),
                                                             [&](const auto& candidate) {
                                                                 return candidate.id == *selectedId;
                                                             });
                            if (choice != node->choices.end()) {
                                for (const auto& effect : choice->effects) {
                                    const int32_t current = authoredDialogueVariableValue(effect.key);
                                    urpg::GlobalStateHub::getInstance().setVariable(
                                        effect.key, saturatingDialogueEffectDelta(current, effect.delta));
                                }
                                if (!choice->effects.empty()) {
                                    ++m_authoredDialogueStateRevision;
                                }
                                if (!beginActiveAuthoredDialogueNode(choice->target_node_id)) {
                                    m_dialogueRuntimeDiagnostics.push_back(
                                        "authored_dialogue_target_runtime_admission_failed:" + choice->target_node_id);
                                }
                            }
                        }
                    } else {
                        auto& registry = urpg::message::DialogueRegistry::getInstance();
                        auto nextPages = registry.flattenConversation("intro_elder", *selectedId);
                        if (!nextPages.empty()) {
                            m_messageRunner.begin(std::move(nextPages));
                        }
                    }
                }
            }
        } else if (m_messageRunner.state() == urpg::message::MessageFlowState::AwaitingChoice) {
            if (input.isActionJustPressed(urpg::input::InputAction::MoveUp))
                m_messageRunner.moveChoicePrev();
            if (input.isActionJustPressed(urpg::input::InputAction::MoveDown))
                m_messageRunner.moveChoiceNext();
        }
        if (!m_messageRunner.isActive() && m_pendingAuthoredDialogue.has_value() && !beginPendingAuthoredDialogue()) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_event_pending_start_failed");
        }
        return; // Block character movement during dialogue
    }

    if (m_playerMovement.isMoving)
        return;

    if (input.isActionJustPressed(urpg::input::InputAction::Confirm)) {
        if (activateInteractionAbilityAtTile("confirm_interact", m_playerMovement.gridPos.x,
                                             m_playerMovement.gridPos.y) ||
            activateInteractionAbility("confirm_interact") ||
            triggerAuthoredDialogueInteractionAtTile("confirm_interact", m_playerMovement.gridPos.x,
                                                     m_playerMovement.gridPos.y)) {
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

bool MapScene::setEventSprites(std::vector<MapEventSprite> sprites) {
    std::sort(sprites.begin(), sprites.end(), [](const MapEventSprite& lhs, const MapEventSprite& rhs) {
        return lhs.event_id < rhs.event_id;
    });

    std::string previous_event_id;
    for (size_t index = 0; index < sprites.size(); ++index) {
        const auto& sprite = sprites[index];
        if (sprite.event_id.empty() || sprite.asset.id.empty() || sprite.asset.path.empty() ||
            sprite.tile_x < 0 || sprite.tile_x >= m_width || sprite.tile_y < 0 || sprite.tile_y >= m_height ||
            sprite.frame_width <= 0 || sprite.frame_height <= 0 || sprite.frame_count <= 0 || sprite.frame_count > 64 ||
            !std::isfinite(sprite.frame_duration) || sprite.frame_duration < 0.01f || sprite.frame_duration > 10.0f ||
            (!previous_event_id.empty() && previous_event_id == sprite.event_id)) {
            return false;
        }
        for (const auto& page : sprite.page_candidates) {
            if (page.page_id.empty() ||
                std::any_of(page.conditions.begin(), page.conditions.end(), [](const auto& condition) {
                    return (condition.type != "switch" && condition.type != "variable" &&
                            condition.type != "self_switch") ||
                           condition.key.empty() || !isSupportedPerspectivePageComparison(condition.comparison);
                })) {
                return false;
            }
        }
        for (size_t previous_index = 0; previous_index < index; ++previous_index) {
            const auto& previous = sprites[previous_index];
            if (previous.asset.id == sprite.asset.id && previous.asset.path != sprite.asset.path) {
                return false;
            }
        }
        previous_event_id = sprite.event_id;
    }

    std::vector<float> elapsed_seconds;
    elapsed_seconds.reserve(sprites.size());
    for (const auto& sprite : sprites) {
        const auto previous = std::find_if(m_eventSprites.begin(), m_eventSprites.end(),
                                           [&](const MapEventSprite& candidate) {
                                               return candidate.event_id == sprite.event_id;
                                           });
        if (previous == m_eventSprites.end()) {
            elapsed_seconds.push_back(0.0f);
            continue;
        }
        const size_t previous_index = static_cast<size_t>(std::distance(m_eventSprites.begin(), previous));
        const bool animation_unchanged = previous->asset.id == sprite.asset.id &&
                                         previous->asset.path == sprite.asset.path &&
                                         previous->frame_width == sprite.frame_width &&
                                         previous->frame_height == sprite.frame_height &&
                                         previous->frame_count == sprite.frame_count &&
                                         previous->frame_duration == sprite.frame_duration &&
                                         previous->loop == sprite.loop;
        elapsed_seconds.push_back(animation_unchanged && previous_index < m_eventSpriteElapsedSeconds.size()
                                      ? m_eventSpriteElapsedSeconds[previous_index]
                                      : 0.0f);
    }
    m_eventSprites = std::move(sprites);
    m_eventSpriteElapsedSeconds = std::move(elapsed_seconds);
    registerEventSpriteTextures();
    return true;
}

int32_t MapScene::currentEventSpriteFrame(const size_t index) const {
    if (index >= m_eventSprites.size() || index >= m_eventSpriteElapsedSeconds.size()) {
        return 0;
    }
    const auto& sprite = m_eventSprites[index];
    if (sprite.frame_count <= 1) {
        return 0;
    }
    const float cycle_duration = sprite.frame_duration * static_cast<float>(sprite.frame_count);
    float elapsed = m_eventSpriteElapsedSeconds[index];
    if (sprite.loop && cycle_duration > 0.0f) {
        elapsed = std::fmod(elapsed, cycle_duration);
    }
    const int32_t frame = static_cast<int32_t>(elapsed / sprite.frame_duration);
    return std::clamp(frame, 0, sprite.frame_count - 1);
}

bool MapScene::setEventColliders(std::vector<MapEventCollider> colliders) {
    std::sort(colliders.begin(), colliders.end(), [](const auto& lhs, const auto& rhs) {
        return lhs.event_id < rhs.event_id;
    });
    for (size_t index = 0; index < colliders.size(); ++index) {
        const auto& collider = colliders[index];
        if (collider.event_id.empty() || collider.tile_x < 0 || collider.tile_x >= m_width || collider.tile_y < 0 ||
            collider.tile_y >= m_height ||
            (index > 0 && colliders[index - 1].event_id == collider.event_id)) {
            return false;
        }
        for (const auto& page : collider.page_candidates) {
            if (page.page_id.empty() ||
                std::any_of(page.conditions.begin(), page.conditions.end(), [](const auto& condition) {
                    return (condition.type != "switch" && condition.type != "variable" &&
                            condition.type != "self_switch") ||
                           condition.key.empty() || !isSupportedPerspectivePageComparison(condition.comparison);
                })) {
                return false;
            }
        }
    }
    m_eventColliders = std::move(colliders);
    return true;
}

bool MapScene::checkCollision(int x, int y) const {
    if (x < 0 || x >= m_width || y < 0 || y >= m_height) {
        return true;
    }
    if (!m_tiles[y * m_width + x].isPassable) {
        return true;
    }
    return std::any_of(m_eventColliders.begin(), m_eventColliders.end(), [&](const MapEventCollider& collider) {
        if (collider.tile_x != x || collider.tile_y != y) {
            return false;
        }
        bool blocks_movement = collider.default_blocks_movement;
        for (const auto& page : collider.page_candidates) {
            if (authoredDialoguePageConditionsMatch(collider.event_id, page.conditions) &&
                page.has_blocks_movement_override) {
                blocks_movement = page.blocks_movement;
            }
        }
        return blocks_movement;
    });
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
    m_pendingAuthoredDialogue.reset();
    m_activeDialogueConversationId.clear();
    m_dialogueRuntimeDiagnostics.clear();
    m_activeAuthoredDialogueGraph.reset();
    m_activeAuthoredDialogueNodeId.clear();
    m_activeAuthoredDialogueCaption.clear();
    m_activeAuthoredDialogueVoiceAssetId.clear();
    m_messageRunner.begin(pages);
}

bool MapScene::startAuthoredDialogue(const urpg::dialogue::DialogueGraph& graph, std::string conversation_id) {
    m_pendingAuthoredDialogue.reset();
    if (!validateAuthoredDialogueAdmission(graph, conversation_id)) {
        return false;
    }

    m_activeAuthoredDialogueGraph = graph;
    m_activeDialogueConversationId = std::move(conversation_id);
    return beginActiveAuthoredDialogueNode(m_activeAuthoredDialogueGraph->startNode());
}

bool MapScene::validateAuthoredDialogueAdmission(const urpg::dialogue::DialogueGraph& graph,
                                                 const std::string& conversation_id) {
    m_dialogueRuntimeDiagnostics.clear();
    if (conversation_id.empty()) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_conversation_id_missing");
        return false;
    }

    const auto structural_diagnostics = graph.validate();
    const auto flow_diagnostics = graph.analyzeFlow();
    if (!structural_diagnostics.empty() || !flow_diagnostics.empty()) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_graph_invalid");
        return false;
    }
    for (const auto& [node_id, node] : graph.nodes()) {
        for (const auto& choice : node.choices) {
            if (std::any_of(choice.effects.begin(), choice.effects.end(),
                            [](const urpg::dialogue::DialogueEffect& effect) { return effect.key.empty(); })) {
                m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_effect_key_missing:" + node_id + ":" +
                                                       choice.id);
                return false;
            }
        }
    }
    return true;
}

bool MapScene::startAuthoredDialogueFromProject(const std::string& dialogue_id) {
    return startAuthoredDialogueFromProjectWithStateWrites(dialogue_id, {});
}

bool MapScene::startAuthoredDialogueFromProjectWithStateWrites(
    const std::string& dialogue_id, std::vector<AuthoredDialogueInteraction::StateWrite> state_writes) {
    const auto graph = loadAuthoredDialogueFromProject(dialogue_id);
    const std::string conversation_id = "project.dialogue." + dialogue_id;
    if (!graph.has_value() || !validateAuthoredDialogueAdmission(*graph, conversation_id) ||
        !validateAuthoredDialogueStateWrites(state_writes)) {
        return false;
    }
    applyAuthoredDialogueStateWrites(state_writes);
    return startAuthoredDialogue(*graph, conversation_id);
}

std::optional<urpg::dialogue::DialogueGraph> MapScene::loadAuthoredDialogueFromProject(const std::string& dialogue_id) {
    m_dialogueRuntimeDiagnostics.clear();
    if (m_projectRoot.empty()) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_project_root_missing");
        return std::nullopt;
    }
    if (!isStableDialogueProjectId(dialogue_id)) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_project_id_invalid:" + dialogue_id);
        return std::nullopt;
    }

    const auto dialogue_path = m_projectRoot / "content" / "dialogues" / (dialogue_id + ".json");
    std::ifstream input(dialogue_path, std::ios::binary);
    if (!input.good()) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_project_file_missing:" + dialogue_id);
        return std::nullopt;
    }

    const auto json = nlohmann::json::parse(input, nullptr, false);
    if (json.is_discarded()) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_project_file_invalid:" + dialogue_id);
        return std::nullopt;
    }
    const auto graph = urpg::dialogue::DialogueGraph::fromJson(json);
    if (!graph.has_value()) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_project_graph_invalid:" + dialogue_id);
        return std::nullopt;
    }
    return graph;
}

bool MapScene::setAuthoredDialogueInteractions(std::vector<AuthoredDialogueInteraction> interactions) {
    std::sort(interactions.begin(), interactions.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.trigger_id != rhs.trigger_id) return lhs.trigger_id < rhs.trigger_id;
        if (lhs.tile_y != rhs.tile_y) return lhs.tile_y < rhs.tile_y;
        if (lhs.tile_x != rhs.tile_x) return lhs.tile_x < rhs.tile_x;
        return lhs.event_id < rhs.event_id;
    });

    for (size_t index = 0; index < interactions.size(); ++index) {
        const auto& interaction = interactions[index];
        if (interaction.event_id.empty() || interaction.trigger_id.empty() ||
            (interaction.dialogue_id.empty() && interaction.page_candidates.empty()) ||
            (!interaction.dialogue_id.empty() && !isStableDialogueProjectId(interaction.dialogue_id)) ||
            interaction.tile_x < 0 ||
            interaction.tile_x >= m_width || interaction.tile_y < 0 || interaction.tile_y >= m_height) {
            return false;
        }
        if (std::any_of(interaction.state_writes.begin(), interaction.state_writes.end(),
                        [](const AuthoredDialogueInteraction::StateWrite& write) {
                            return write.key.empty() ||
                                   (write.kind == AuthoredDialogueInteraction::StateWriteKind::SetEventSelfSwitch &&
                                    write.event_id.empty());
                        })) {
            return false;
        }
        for (const auto& page : interaction.page_candidates) {
            if (page.page_id.empty() ||
                (page.dialogue_id.empty() && page.message_pages.empty() && page.state_writes.empty() &&
                 !page.transfer.has_value()) ||
                (!page.dialogue_id.empty() && !isStableDialogueProjectId(page.dialogue_id)) ||
                (page.transfer.has_value() && !page.dialogue_id.empty() && page.message_pages.empty()) ||
                (page.transfer.has_value() &&
                 (page.transfer->map_id != m_mapId || page.transfer->tile_x < 0 || page.transfer->tile_x >= m_width ||
                  page.transfer->tile_y < 0 || page.transfer->tile_y >= m_height)) ||
                std::any_of(page.message_pages.begin(), page.message_pages.end(),
                            [](const std::string& message) { return message.empty(); }) ||
                std::any_of(page.state_writes.begin(), page.state_writes.end(),
                            [](const AuthoredDialogueInteraction::StateWrite& write) {
                                return write.key.empty() ||
                                       (write.kind == AuthoredDialogueInteraction::StateWriteKind::SetEventSelfSwitch &&
                                        write.event_id.empty());
                            }) ||
                std::any_of(page.conditions.begin(), page.conditions.end(), [](const auto& condition) {
                    return (condition.type != "switch" && condition.type != "variable" &&
                            condition.type != "self_switch") || condition.key.empty() ||
                           !isSupportedPerspectivePageComparison(condition.comparison);
                })) {
                return false;
            }
        }
        if (index > 0) {
            const auto& previous = interactions[index - 1];
            if (previous.trigger_id == interaction.trigger_id && previous.tile_x == interaction.tile_x &&
                previous.tile_y == interaction.tile_y) {
                return false;
            }
        }
    }

    m_authoredDialogueInteractions = std::move(interactions);
    return true;
}

bool MapScene::validateAuthoredDialogueStateWrites(
    const std::vector<AuthoredDialogueInteraction::StateWrite>& state_writes) {
    if (std::any_of(state_writes.begin(), state_writes.end(),
                    [](const AuthoredDialogueInteraction::StateWrite& write) {
                        return write.key.empty() ||
                               (write.kind == AuthoredDialogueInteraction::StateWriteKind::SetEventSelfSwitch &&
                                write.event_id.empty());
                    })) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_state_write_key_missing");
        return false;
    }
    return true;
}

void MapScene::applyAuthoredDialogueStateWrites(
    const std::vector<AuthoredDialogueInteraction::StateWrite>& state_writes) {
    auto& state = urpg::GlobalStateHub::getInstance();
    for (const auto& write : state_writes) {
        switch (write.kind) {
        case AuthoredDialogueInteraction::StateWriteKind::SetSwitch:
            state.setSwitch(write.key, write.value != 0);
            break;
        case AuthoredDialogueInteraction::StateWriteKind::SetVariable:
            state.setVariable(write.key, write.value);
            break;
        case AuthoredDialogueInteraction::StateWriteKind::AddVariable:
            state.setVariable(write.key, saturatingDialogueEffectDelta(authoredDialogueVariableValue(write.key),
                                                                         write.value));
            break;
        case AuthoredDialogueInteraction::StateWriteKind::SetEventSelfSwitch:
            state.setSwitch(mapEventSelfSwitchStateKey(m_mapId, write.event_id, write.key), write.value != 0);
            break;
        }
    }
    if (!state_writes.empty()) {
        ++m_authoredDialogueStateRevision;
    }
}

bool MapScene::applyAuthoredDialogueTransfer(const AuthoredDialogueInteraction::Transfer& transfer) {
    if (transfer.map_id != m_mapId || transfer.tile_x < 0 || transfer.tile_x >= m_width || transfer.tile_y < 0 ||
        transfer.tile_y >= m_height) {
        return false;
    }
    m_playerMovement.lastGridPos = m_playerMovement.gridPos;
    m_playerMovement.gridPos = {transfer.tile_x, transfer.tile_y};
    m_playerMovement.isMoving = false;
    m_playerMovement.moveProgress = 0.0f;
    return true;
}

MapScene::AuthoredDialogueStateSnapshot MapScene::authoredDialogueStateSnapshot() const {
    AuthoredDialogueStateSnapshot snapshot;
    snapshot.revision = m_authoredDialogueStateRevision;
    const auto& state = urpg::GlobalStateHub::getInstance();
    const auto self_switch_prefix = "authored_map_self_switch:" + m_mapId + ":";
    for (const auto& [key, value] : state.getAllSwitches()) {
        if (key.starts_with("authored_map_self_switch:")) {
            if (key.starts_with(self_switch_prefix)) {
                const std::string local_key = key.substr(self_switch_prefix.size());
                if (const size_t separator = local_key.find(':'); separator != std::string::npos && separator > 0 &&
                    separator + 1 < local_key.size()) {
                    snapshot.self_switches.push_back({local_key, value ? "true" : "false"});
                }
            }
            continue;
        }
        snapshot.switches.push_back({key, value ? "true" : "false"});
    }
    for (const auto& [key, value] : state.getAllVariables()) {
        snapshot.variables.push_back({key, authoredDialogueStateValueText(value)});
    }
    const auto sort_entries = [](std::vector<AuthoredDialogueStateEntry>& entries) {
        std::sort(entries.begin(), entries.end(), [](const auto& lhs, const auto& rhs) { return lhs.key < rhs.key; });
    };
    sort_entries(snapshot.switches);
    sort_entries(snapshot.variables);
    sort_entries(snapshot.self_switches);
    return snapshot;
}

bool MapScene::authoredDialoguePageConditionsMatch(const std::string& event_id,
                                                   const std::vector<MapEventPageCondition>& conditions) const {
    const auto& state = urpg::GlobalStateHub::getInstance();
    const auto switches = state.getAllSwitches();
    const auto variables = state.getAllVariables();
    for (const auto& condition : conditions) {
        std::string actual_value;
        if (condition.type == "switch") {
            const auto value = switches.find(condition.key);
            if (value == switches.end()) {
                return false;
            }
            actual_value = value->second ? "true" : "false";
        } else if (condition.type == "variable") {
            const auto value = variables.find(condition.key);
            if (value == variables.end()) {
                return false;
            }
            if (const auto* integer = std::get_if<int32_t>(&value->second)) {
                actual_value = std::to_string(*integer);
            } else if (const auto* decimal = std::get_if<float>(&value->second)) {
                actual_value = std::to_string(*decimal);
            } else if (const auto* boolean = std::get_if<bool>(&value->second)) {
                actual_value = *boolean ? "true" : "false";
            } else if (const auto* string = std::get_if<std::string>(&value->second)) {
                actual_value = *string;
            } else {
                return false;
            }
        } else if (condition.type == "self_switch") {
            std::string condition_event_id = event_id;
            std::string condition_key = condition.key;
            const size_t separator = condition_key.find(':');
            if (separator != std::string::npos) {
                condition_event_id = condition_key.substr(0, separator);
                condition_key = condition_key.substr(separator + 1);
            }
            if (condition_event_id.empty() || condition_key.empty()) {
                return false;
            }
            const auto value = switches.find(mapEventSelfSwitchStateKey(m_mapId, condition_event_id, condition_key));
            if (value == switches.end()) {
                return false;
            }
            actual_value = value->second ? "true" : "false";
        } else {
            return false;
        }
        if (!perspectivePageConditionMatches(actual_value, condition.comparison, condition.value)) {
            return false;
        }
    }
    return true;
}

bool MapScene::triggerAuthoredDialogueInteractionAtTile(const std::string& trigger_id, int tile_x, int tile_y) {
    const auto interaction = std::find_if(
        m_authoredDialogueInteractions.begin(), m_authoredDialogueInteractions.end(), [&](const auto& candidate) {
            return candidate.trigger_id == trigger_id && candidate.tile_x == tile_x && candidate.tile_y == tile_y;
        });
    if (interaction == m_authoredDialogueInteractions.end()) {
        return false;
    }
    const AuthoredDialogueInteraction::PageCandidate* selected_page = nullptr;
    for (const auto& page : interaction->page_candidates) {
        if (authoredDialoguePageConditionsMatch(interaction->event_id, page.conditions)) {
            selected_page = &page;
        }
    }
    if (selected_page == nullptr && interaction->dialogue_id.empty()) {
        return false;
    }
    const std::string& dialogue_id = selected_page != nullptr ? selected_page->dialogue_id : interaction->dialogue_id;
    const auto& state_writes = selected_page != nullptr ? selected_page->state_writes : interaction->state_writes;
    const auto* transfer = selected_page != nullptr && selected_page->transfer.has_value()
                               ? &*selected_page->transfer
                               : nullptr;
    if (selected_page != nullptr && !selected_page->message_pages.empty()) {
        m_dialogueRuntimeDiagnostics.clear();
        std::optional<urpg::dialogue::DialogueGraph> pending_graph;
        std::string pending_conversation_id;
        if (!dialogue_id.empty()) {
            pending_graph = loadAuthoredDialogueFromProject(dialogue_id);
            pending_conversation_id = "project.dialogue." + dialogue_id;
        }
        if (!validateAuthoredDialogueStateWrites(state_writes) ||
            (!dialogue_id.empty() &&
             (!pending_graph.has_value() || !validateAuthoredDialogueAdmission(*pending_graph, pending_conversation_id)))) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_event_trigger_failed:" + interaction->event_id);
            return true;
        }
        applyAuthoredDialogueStateWrites(state_writes);
        if (transfer != nullptr && !applyAuthoredDialogueTransfer(*transfer)) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_event_transfer_failed:" + interaction->event_id);
            return true;
        }
        std::vector<urpg::message::DialoguePage> pages;
        pages.reserve(selected_page->message_pages.size());
        for (size_t index = 0; index < selected_page->message_pages.size(); ++index) {
            pages.push_back({"authored_event." + interaction->event_id + "." + selected_page->page_id + "." +
                                 std::to_string(index),
                             selected_page->message_pages[index], {}, true, {}, 0});
        }
        m_activeDialogueConversationId.clear();
        m_activeAuthoredDialogueGraph.reset();
        m_activeAuthoredDialogueNodeId.clear();
        m_activeAuthoredDialogueCaption.clear();
        m_activeAuthoredDialogueVoiceAssetId.clear();
        m_pendingAuthoredDialogue.reset();
        if (pending_graph.has_value()) {
            m_pendingAuthoredDialogue = {std::move(*pending_graph), std::move(pending_conversation_id)};
        }
        m_messageRunner.begin(std::move(pages));
        return true;
    }
    if (selected_page != nullptr && selected_page->dialogue_id.empty()) {
        m_dialogueRuntimeDiagnostics.clear();
        if (!validateAuthoredDialogueStateWrites(state_writes)) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_event_trigger_failed:" + interaction->event_id);
            return true;
        }
        applyAuthoredDialogueStateWrites(state_writes);
        if (transfer != nullptr && !applyAuthoredDialogueTransfer(*transfer)) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_event_transfer_failed:" + interaction->event_id);
        }
        return true;
    }
    if (!startAuthoredDialogueFromProjectWithStateWrites(dialogue_id, state_writes)) {
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_event_trigger_failed:" + interaction->event_id);
    }
    return true;
}

bool MapScene::beginPendingAuthoredDialogue() {
    if (!m_pendingAuthoredDialogue.has_value()) {
        return false;
    }
    PendingAuthoredDialogue pending = std::move(*m_pendingAuthoredDialogue);
    m_pendingAuthoredDialogue.reset();
    return startAuthoredDialogue(pending.graph, std::move(pending.conversation_id));
}

void MapScene::setDialogueLocaleCatalog(std::optional<urpg::localization::LocaleCatalog> catalog) {
    m_dialogueLocaleCatalog = std::move(catalog);
}

std::string MapScene::dialogueLocaleCode() const {
    return m_dialogueLocaleCatalog.has_value() ? m_dialogueLocaleCatalog->getLocaleCode() : std::string{};
}

bool MapScene::beginActiveAuthoredDialogueNode(const std::string& node_id) {
    if (!m_activeAuthoredDialogueGraph.has_value()) {
        return false;
    }
    const auto* node = m_activeAuthoredDialogueGraph->findNode(node_id);
    if (node == nullptr) {
        return false;
    }

    const auto resolve_text = [this](const std::string& localization_key, const std::string& fallback,
                                     const std::string& reference_id) {
        if (localization_key.empty() || !m_dialogueLocaleCatalog.has_value()) {
            return fallback;
        }
        const auto resolved = m_dialogueLocaleCatalog->getKey(localization_key);
        if (resolved.has_value()) {
            return *resolved;
        }
        m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_locale_key_missing:" +
                                               m_dialogueLocaleCatalog->getLocaleCode() + ":" + localization_key +
                                               ":" + reference_id);
        return fallback;
    };

    urpg::message::DialoguePage page;
    page.id = node->id;
    page.body = resolve_text(node->localization_key, node->text_preview, node->id);
    page.variant.speaker = node->speaker_name.empty() ? node->speaker_id : node->speaker_name;
    page.variant.route_token = "native_dialogue_graph";
    m_activeAuthoredDialogueCaption = node->caption_localization_key.empty()
                                         ? std::string{}
                                         : resolve_text(node->caption_localization_key, page.body, node->id + ":caption");
    m_activeAuthoredDialogueVoiceAssetId = node->voice_asset_id;
    if (!m_activeAuthoredDialogueVoiceAssetId.empty()) {
        if (m_audioCore == nullptr) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_voice_audio_core_missing:" + node->id);
        } else if (m_audioCore->playSound(m_activeAuthoredDialogueVoiceAssetId, urpg::audio::AudioCategory::SE) == 0) {
            m_dialogueRuntimeDiagnostics.push_back("authored_dialogue_voice_playback_failed:" + node->id + ":" +
                                                   m_activeAuthoredDialogueVoiceAssetId);
        }
    }
    for (const auto& choice : node->choices) {
        bool enabled = true;
        std::string disabled_reason;
        for (const auto& condition : choice.conditions) {
            const auto matches = evaluateAuthoredDialogueCondition(condition);
            if (!matches.has_value()) {
                enabled = false;
                disabled_reason = "Unsupported dialogue condition.";
                break;
            }
            if (!*matches) {
                enabled = false;
                disabled_reason = "Dialogue condition is not met.";
                break;
            }
        }
        page.choices.push_back({choice.id,
                                resolve_text(choice.localization_key, choice.label, node->id + ":" + choice.id),
                                enabled,
                                std::move(disabled_reason)});
    }

    m_activeAuthoredDialogueNodeId = node->id;
    m_messageRunner.begin({std::move(page)});
    return true;
}

void MapScene::startChatbot(const std::string& systemPrompt, std::shared_ptr<urpg::ai::IChatService> service) {
    m_activeChatbot = std::make_shared<urpg::ai::ChatbotComponent>(service);
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
    const std::string project_dialogue_prefix = "project.dialogue.";
    const std::string dialogue_id = m_activeDialogueConversationId.starts_with(project_dialogue_prefix)
                                        ? m_activeDialogueConversationId.substr(project_dialogue_prefix.size())
                                        : std::string{};
    if (m_activeAuthoredDialogueGraph.has_value() && isStableDialogueProjectId(dialogue_id) &&
        !m_activeAuthoredDialogueNodeId.empty()) {
        auto root = nlohmann::json::parse(snapshot, nullptr, false);
        if (!root.is_discarded() && root.is_object()) {
            root["map_scene_dialogue_checkpoint"] = {
                {"version", 1},
                {"conversation_id", m_activeDialogueConversationId},
                {"dialogue_id", dialogue_id},
                {"node_id", m_activeAuthoredDialogueNodeId},
            };
            snapshot = root.dump();
        }
    }

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
        startDialogue({});

        const auto root = nlohmann::json::parse(loadResult.payload, nullptr, false);
        const auto checkpoint = root.is_object()
                                    ? root.value("map_scene_dialogue_checkpoint", nlohmann::json::object())
                                    : nlohmann::json::object();
        if (checkpoint.is_object() && !checkpoint.empty()) {
            const bool checkpoint_shape_valid =
                checkpoint.contains("version") && checkpoint["version"].is_number_integer() &&
                checkpoint.contains("dialogue_id") && checkpoint["dialogue_id"].is_string() &&
                checkpoint.contains("conversation_id") && checkpoint["conversation_id"].is_string() &&
                checkpoint.contains("node_id") && checkpoint["node_id"].is_string();
            if (!checkpoint_shape_valid || checkpoint["version"].get<int>() != 1) {
                result.diagnostics.push_back("map_dialogue_checkpoint_restore_failed");
            } else {
                const std::string dialogue_id = checkpoint["dialogue_id"].get<std::string>();
                const std::string conversation_id = checkpoint["conversation_id"].get<std::string>();
                const std::string node_id = checkpoint["node_id"].get<std::string>();
                const std::string expected_conversation_id = "project.dialogue." + dialogue_id;
                const auto graph = loadAuthoredDialogueFromProject(dialogue_id);
                if (!graph.has_value() || conversation_id != expected_conversation_id || node_id.empty() ||
                    !validateAuthoredDialogueAdmission(*graph, conversation_id) || graph->findNode(node_id) == nullptr) {
                    result.diagnostics.push_back("map_dialogue_checkpoint_restore_failed");
                } else {
                    m_activeAuthoredDialogueGraph = *graph;
                    m_activeDialogueConversationId = conversation_id;
                    if (!beginActiveAuthoredDialogueNode(node_id)) {
                        m_activeAuthoredDialogueGraph.reset();
                        m_activeDialogueConversationId.clear();
                        m_activeAuthoredDialogueNodeId.clear();
                        result.diagnostics.push_back("map_dialogue_checkpoint_restore_failed");
                    }
                }
            }
        }
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
    registerEventSpriteTextures();
}

void MapScene::registerEventSpriteTextures() {
    for (const auto& sprite : m_eventSprites) {
        std::filesystem::path asset_path = sprite.asset.path;
        if (asset_path.is_relative() && !m_projectRoot.empty()) {
            asset_path = m_projectRoot / asset_path;
        }
        urpg::TextureMeta meta;
        meta.filePath = asset_path.lexically_normal().generic_string();
        urpg::TextureRegistry::getInstance().registerTexture(sprite.asset.id, meta);
    }
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

urpg::level::RoutedPathRequest MapScene::routePathRequest(urpg::level::PathRequest request) const {
    urpg::level::PathfindingGraph graph(m_width, m_height);
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            if (checkCollision(x, y)) {
                graph.setBlocked(x, y, true, "map_collision");
            }
        }
    }

    request.surface_id = m_mapId;
    return urpg::level::RoutePathRequest(graph, request);
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
