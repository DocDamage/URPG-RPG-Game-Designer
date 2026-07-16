#pragma once

#include "engine/core/ability/ability_system_component.h"
#include "engine/core/accessibility/inclusive_experience.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/animation/animation_components.h"
#include "engine/core/audio/audio_core.h"
#include "engine/core/dialogue/dialogue_graph.h"
#include "engine/core/input/input_core.h"
#include "engine/core/level/path_request_router.h"
#include "engine/core/localization/locale_catalog.h"
#include "engine/core/message/chatbot_component.h"
#include "engine/core/message/dialogue_registry.h"
#include "engine/core/message/message_core.h"
#include "engine/core/presentation/exploration_feedback.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/render/sprite_animator.h"
#include "engine/core/render/tilemap_renderer.h"
#include "engine/core/runtime_asset_mode.h"
#include "engine/core/save/save_recovery.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/ui/chat_window.h"
#include "scene_manager.h"
#include <cstdint>
#include <filesystem>
#include <memory>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace urpg::scene {

/**
 * @brief Represents a tile in the map.
 */
struct TileData {
    uint16_t tileId = 0;
    bool isPassable = true;
    bool passableDown = true;
    bool passableLeft = true;
    bool passableRight = true;
    bool passableUp = true;
    bool hasVisual = false;
    // Empty keeps the map-level tileset reference for legacy/compat maps.
    // Native authored maps may select a packed atlas per painted tile.
    std::string tilesetId;
};

struct MapAssetReference {
    std::string id;
    std::filesystem::path path;
};

struct MapAssetReferences {
    MapAssetReference player_sprite;
    MapAssetReference tileset;
};

// A condition shared by native projections of one persisted Perspective 2D
// event page. The event document remains the persistence authority.
struct MapEventPageCondition {
    std::string type;
    std::string key;
    std::string comparison = "equals";
    std::string value;
};

// A native Map-owned visual projection of an authored event. The Perspective
// 2D document remains the persistence authority; this record only supplies
// the runtime renderer with the approved stable asset reference and position.
struct MapEventSprite {
    struct PageCandidate {
        std::string page_id;
        bool has_visible_override = false;
        bool visible = true;
        std::vector<MapEventPageCondition> conditions;
    };

    std::string event_id;
    MapAssetReference asset;
    int tile_x = 0;
    int tile_y = 0;
    int32_t frame_width = 48;
    int32_t frame_height = 48;
    int32_t frame_count = 1;
    float frame_duration = 0.15f;
    bool loop = true;
    bool default_visible = true;
    std::vector<PageCandidate> page_candidates;
};

// A native collision projection of an authored event. Event documents remain
// authoritative; MapScene uses this record only for movement and path checks.
struct MapEventCollider {
    struct PageCandidate {
        std::string page_id;
        bool has_blocks_movement_override = false;
        bool blocks_movement = false;
        std::vector<MapEventPageCondition> conditions;
    };

    std::string event_id;
    int tile_x = 0;
    int tile_y = 0;
    bool default_blocks_movement = false;
    std::vector<PageCandidate> page_candidates;
};

enum class MapSceneSaveLoadOperation : uint8_t {
    Save,
    Load,
};

struct MapSceneSaveLoadResult {
    bool ok = false;
    MapSceneSaveLoadOperation operation = MapSceneSaveLoadOperation::Save;
    int slot_id = -1;
    std::filesystem::path primary_path;
    urpg::SaveRecoveryTier recovery_tier = urpg::SaveRecoveryTier::None;
    bool loaded_from_recovery = false;
    bool boot_safe_mode = false;
    std::string failure_reason;
    std::vector<std::string> diagnostics;
};

/**
 * @brief Native authority for Map data, coordinates, and collision.
 */
class MapScene : public GameScene {
  public:
    enum class InteractionBindingScope : uint8_t {
        Global = 0,
        Tile = 1,
        Prop = 2,
        Region = 3,
    };

    struct InteractionAbilityBinding {
        InteractionBindingScope scope = InteractionBindingScope::Global;
        std::string trigger_id;
        std::string asset_path;
        std::string ability_id;
        int tile_x = -1;
        int tile_y = -1;
        int region_min_x = -1;
        int region_min_y = -1;
        int region_max_x = -1;
        int region_max_y = -1;
        std::string prop_instance_id;
        std::string prop_asset_id;
    };

    // Runtime projection of one persisted map-event dialogue command. The
    // map scene owns input dispatch and graph admission; the source document
    // remains the authoring/history owner.
    struct AuthoredDialogueInteraction {
        enum class StateWriteKind : uint8_t {
            SetSwitch,
            SetVariable,
            AddVariable,
            SetEventSelfSwitch,
        };

        struct StateWrite {
            StateWriteKind kind = StateWriteKind::SetVariable;
            std::string key;
            int32_t value = 0;
            std::string event_id;
        };

        struct Transfer {
            std::string map_id;
            int tile_x = -1;
            int tile_y = -1;
        };

        // A conditional projection of a single Perspective 2D event page.
        // The source page order is retained so the final matching page has
        // the same precedence as the authoring preview.
        using PageCondition = MapEventPageCondition;

        struct PageCandidate {
            std::string page_id;
            std::string dialogue_id;
            std::vector<std::string> message_pages;
            std::vector<PageCondition> conditions;
            std::vector<StateWrite> state_writes;
            std::optional<Transfer> transfer;
        };

        std::string event_id;
        std::string trigger_id;
        std::string dialogue_id;
        int tile_x = -1;
        int tile_y = -1;
        std::vector<StateWrite> state_writes;
        std::vector<PageCandidate> page_candidates;
    };

    struct AuthoredDialogueStateEntry {
        std::string key;
        std::string value;
    };

    struct AuthoredDialogueStateSnapshot {
        uint64_t revision = 0;
        std::vector<AuthoredDialogueStateEntry> switches;
        std::vector<AuthoredDialogueStateEntry> variables;
        std::vector<AuthoredDialogueStateEntry> self_switches;
    };

    MapScene(const std::string& mapId, int width, int height);
    virtual ~MapScene() = default;

    SceneType getType() const override { return SceneType::MAP; }
    std::string getName() const override { return "Map_" + m_mapId; }

    // Lifecycle hooks
    void onUpdate(float deltaTime) override;
    void handleInput(const urpg::input::InputCore& input) override;

    /**
     * @brief Draw the map using the provided batcher.
     */
    void draw(SpriteBatcher& batcher) override;

    /**
     * @brief Set map data for a specific layer.
     */
    void setLayerData(int layer, const std::vector<int>& data);

    /**
     * @brief Associate a tileset texture with this map.
     */
    void setTileset(const std::shared_ptr<Texture>& tileset);

    /**
     * @brief Setup the player's visual sprite.
     */
    void setPlayerCharacter(const std::string& name, int index);
    void setAssetReferences(MapAssetReferences references);
    const MapAssetReferences& assetReferences() const { return m_assetReferences; }
    // Replaces the complete event-sprite projection after validating stable
    // IDs and map bounds. Invalid batches leave the current runtime view intact.
    bool setEventSprites(std::vector<MapEventSprite> sprites);
    const std::vector<MapEventSprite>& eventSprites() const { return m_eventSprites; }
    bool setEventColliders(std::vector<MapEventCollider> colliders);
    const std::vector<MapEventCollider>& eventColliders() const { return m_eventColliders; }
    const std::vector<std::string>& assetDiagnostics() const { return m_assetDiagnostics; }
    void setRuntimeAssetMode(urpg::RuntimeAssetMode mode);
    urpg::RuntimeAssetMode runtimeAssetMode() const { return m_runtimeAssetMode; }

    /**
     * @brief Manually override passability for a specific tile.
     */
    void setTilePassable(int x, int y, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            auto& tile = m_tiles[y * m_width + x];
            tile.isPassable = passable;
            tile.passableDown = passable;
            tile.passableLeft = passable;
            tile.passableRight = passable;
            tile.passableUp = passable;
            m_renderLayerDirty = true;
        }
    }

    void setTile(int x, int y, uint16_t tileId, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            auto& tile = m_tiles[y * m_width + x];
            tile.tileId = tileId;
            tile.isPassable = passable;
            tile.passableDown = passable;
            tile.passableLeft = passable;
            tile.passableRight = passable;
            tile.passableUp = passable;
            tile.hasVisual = true;
            tile.tilesetId.clear();
            m_renderLayerDirty = true;
        }
    }

    void setTileWithTileset(int x, int y, uint16_t tileId, bool passable, std::string tileset_id,
                            bool passable_down = true, bool passable_left = true,
                            bool passable_right = true, bool passable_up = true) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            auto& tile = m_tiles[y * m_width + x];
            tile.tileId = tileId;
            tile.isPassable = passable;
            tile.passableDown = passable_down;
            tile.passableLeft = passable_left;
            tile.passableRight = passable_right;
            tile.passableUp = passable_up;
            tile.hasVisual = true;
            tile.tilesetId = std::move(tileset_id);
            m_renderLayerDirty = true;
        }
    }

    void clearTiles(bool passable = true);

    // Coordinate Authority
    bool checkCollision(int x, int y) const;
    bool canMove(int x, int y, urpg::Direction direction) const;

    MovementComponent& getPlayerMovement() { return m_playerMovement; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    urpg::level::RoutedPathRequest routePathRequest(urpg::level::PathRequest request) const;

    /**
     * @brief Triggers a dialogue flow in this scene.
     */
    void startDialogue(const std::vector<urpg::message::DialoguePage>& pages);
    // Admits a structurally valid native Dialogue Graph into the existing
    // MessageFlowRunner, evaluating its integer conditions and applying its
    // effects through the native GlobalStateHub.
    bool startAuthoredDialogue(const urpg::dialogue::DialogueGraph& graph, std::string conversation_id);
    // Starts a saved authored dialogue graph from content/dialogues/<dialogue_id>.json.
    // Dialogue IDs are deliberately restricted to stable project identifiers so
    // map events cannot use this as a general file-loading escape hatch.
    bool startAuthoredDialogueFromProject(const std::string& dialogue_id);
    // Applies bounded preflighted state writes and an optional current-map
    // transfer only after the saved graph is valid for native admission, then
    // starts it.
    bool startAuthoredDialogueFromProjectWithStateWrites(
        const std::string& dialogue_id, std::vector<AuthoredDialogueInteraction::StateWrite> state_writes,
        std::optional<AuthoredDialogueInteraction::Transfer> transfer = std::nullopt);
    // Replaces the complete projected map-event dialogue interaction batch.
    // Invalid batches leave the current runtime projection intact.
    bool setAuthoredDialogueInteractions(std::vector<AuthoredDialogueInteraction> interactions);
    // Returns true when a matching interaction consumes the trigger, even if
    // its saved graph cannot be admitted. That prevents legacy fallback
    // dialogue from masking an authored event failure.
    bool triggerAuthoredDialogueInteractionAtTile(const std::string& trigger_id, int tile_x, int tile_y);
    const std::vector<AuthoredDialogueInteraction>& authoredDialogueInteractions() const {
        return m_authoredDialogueInteractions;
    }
    AuthoredDialogueStateSnapshot authoredDialogueStateSnapshot() const;
    void setDialogueLocaleCatalog(std::optional<urpg::localization::LocaleCatalog> catalog);
    bool setDialogueInclusiveSettings(const urpg::accessibility::InclusiveSettings& settings);
    bool setDialogueCaptionTrack(
        std::vector<urpg::accessibility::CaptionCue> cues,
        std::map<std::string, std::string> voice_asset_by_take_id = {});
    std::string dialogueLocaleCode() const;
    const std::vector<std::string>& dialogueRuntimeDiagnostics() const { return m_dialogueRuntimeDiagnostics; }
    const std::string& activeDialogueConversationId() const { return m_activeDialogueConversationId; }
    const std::string& activeAuthoredDialogueNodeId() const { return m_activeAuthoredDialogueNodeId; }
    const std::string& activeAuthoredDialogueCaption() const { return m_activeAuthoredDialogueCaption; }
    const std::string& activeAuthoredDialogueVoiceAssetId() const { return m_activeAuthoredDialogueVoiceAssetId; }
    const std::optional<urpg::accessibility::CaptionCue>& activeAuthoredDialogueCaptionCue() const {
        return m_activeAuthoredDialogueCaptionCue;
    }
    float authoredDialogueCaptionScale() const { return m_dialogueInclusiveSettings.caption_scale; }
    urpg::presentation::ExplorationFeedbackDirector& explorationFeedback() { return m_explorationFeedback; }
    const urpg::presentation::ExplorationFeedbackDirector& explorationFeedback() const { return m_explorationFeedback; }
    urpg::presentation::ExplorationFeedbackResult notifyExplorationFeedback(
        urpg::presentation::ExplorationFeedbackRequest request) {
        return m_explorationFeedback.submit(std::move(request));
    }

    /**
     * @brief Starts a chatbot-driven conversation.
     */
    void startChatbot(const std::string& systemPrompt, std::shared_ptr<urpg::ai::IChatService> service);

    /**
     * @brief Opens the 'Ask AI' input modal.
     */
    void openChatInput();

    void setAudioCore(std::shared_ptr<urpg::audio::AudioCore> audioCore) { m_audioCore = std::move(audioCore); }
    std::shared_ptr<urpg::audio::AudioCore> audioCore() const { return m_audioCore; }

    /**
     * @brief Injects the AI audio bridge into the scene logic.
     */
    void processAiAudioCommands(const std::string& aiResponse);

    /**
     * @brief Injects the AI animation bridge into the scene logic.
     */
    void processAiAnimationCommands(const std::string& aiResponse);
    const urpg::Vector3& playerAiAnimationOffset() const { return m_playerAiAnimationOffset; }
    bool hasActivePlayerAiAnimation() const {
        return m_playerAiAnimation.has_value() && m_playerAiAnimation->isPlaying;
    }
    const std::vector<std::string>& aiAnimationDiagnostics() const { return m_aiAnimationDiagnostics; }

    /**
     * @brief Performs a manual save of the current world state.
     */
    bool saveGame(int slotId = 0);
    MapSceneSaveLoadResult saveGameDetailed(int slotId = 0);

    /**
     * @brief Attempts to load a saved game state.
     */
    bool loadGame(int slotId = 0);
    MapSceneSaveLoadResult loadGameDetailed(int slotId = 0);
    const MapSceneSaveLoadResult& lastSaveLoadResult() const { return m_lastSaveLoadResult; }
    void setProjectRoot(std::filesystem::path project_root);
    const std::filesystem::path& projectRoot() const { return m_projectRoot; }

    /**
     * @brief Checks if a dialogue is currently active, blocking movement.
     */
    bool isDialogueActive() const { return m_messageRunner.isActive() || m_isChatInputOpen; }
    bool isChatInputOpen() const { return m_isChatInputOpen; }
    const std::string& currentChatInputBuffer() const { return m_currentInputBuffer; }

    urpg::ability::AbilitySystemComponent& playerAbilitySystem() { return m_playerAbilitySystem; }
    const urpg::ability::AbilitySystemComponent& playerAbilitySystem() const { return m_playerAbilitySystem; }
    void grantPlayerAbility(const urpg::ability::AuthoredAbilityAsset& asset);
    bool tryActivatePlayerAbility(const std::string& ability_id);
    bool bindInteractionAbility(const std::string& trigger_id, const std::string& asset_path,
                                const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindTileInteractionAbility(const std::string& trigger_id, int tile_x, int tile_y,
                                    const std::string& asset_path, const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindPropInteractionAbility(const std::string& trigger_id, const std::string& prop_asset_id,
                                    const std::string& asset_path, const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindPropInstanceInteractionAbility(const std::string& trigger_id, const std::string& prop_instance_id,
                                            const std::string& prop_asset_id, const std::string& asset_path,
                                            const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindRegionInteractionAbility(const std::string& trigger_id, int min_tile_x, int min_tile_y, int max_tile_x,
                                      int max_tile_y, const std::string& asset_path,
                                      const urpg::ability::AuthoredAbilityAsset& asset);
    bool unbindTileInteractionAbility(const std::string& trigger_id, int tile_x, int tile_y);
    bool unbindPropInteractionAbility(const std::string& trigger_id, const std::string& prop_asset_id);
    bool unbindRegionInteractionAbility(const std::string& trigger_id, int min_tile_x, int min_tile_y, int max_tile_x,
                                        int max_tile_y);
    bool activateInteractionAbility(const std::string& trigger_id);
    bool activateInteractionAbilityAtTile(const std::string& trigger_id, int tile_x, int tile_y);
    bool activateInteractionAbilityForProp(const std::string& trigger_id, const std::string& prop_asset_id);
    bool activateInteractionAbilityForPropInstance(const std::string& trigger_id, const std::string& prop_instance_id,
                                                   const std::string& prop_asset_id = "");
    bool hasInteractionAbilityBinding(const std::string& trigger_id) const;
    const std::vector<InteractionAbilityBinding>& interactionAbilityBindings() const {
        return m_interaction_ability_bindings;
    }

  private:
    void rebuildTileRenderCache();
    void submitCachedTileCommands(urpg::RenderLayer& layer) const;
    void validateRenderAssetReferences();
    void registerEventSpriteTextures();
    int32_t currentEventSpriteFrame(size_t index) const;
    std::optional<urpg::dialogue::DialogueGraph> loadAuthoredDialogueFromProject(const std::string& dialogue_id);
    bool validateAuthoredDialogueAdmission(const urpg::dialogue::DialogueGraph& graph,
                                           const std::string& conversation_id);
    bool validateAuthoredDialogueStateWrites(const std::vector<AuthoredDialogueInteraction::StateWrite>& state_writes);
    void applyAuthoredDialogueStateWrites(const std::vector<AuthoredDialogueInteraction::StateWrite>& state_writes);
    bool authoredDialoguePageConditionsMatch(const std::string& event_id,
                                             const std::vector<MapEventPageCondition>& conditions) const;
    bool applyAuthoredDialogueTransfer(const AuthoredDialogueInteraction::Transfer& transfer);
    bool beginActiveAuthoredDialogueNode(const std::string& node_id);
    bool beginPendingAuthoredDialogue();
    std::string nextExplorationFeedbackRequestId(const std::string& prefix);

    struct PendingAuthoredDialogue {
        urpg::dialogue::DialogueGraph graph;
        std::string conversation_id;
    };

    std::string m_mapId;
    int m_width;
    int m_height;
    std::vector<TileData> m_tiles;
    std::vector<urpg::TileCommand> m_cachedTileCommands;
    bool m_renderLayerDirty = true;
    MapAssetReferences m_assetReferences;
    std::vector<MapEventSprite> m_eventSprites;
    std::vector<float> m_eventSpriteElapsedSeconds;
    std::vector<MapEventCollider> m_eventColliders;
    std::vector<std::string> m_assetDiagnostics;
    bool m_assetReferencesValidated = false;
    urpg::RuntimeAssetMode m_runtimeAssetMode = urpg::RuntimeAssetMode::Development;
    std::filesystem::path m_projectRoot;
    MapSceneSaveLoadResult m_lastSaveLoadResult;

    // Components
    urpg::MovementComponent m_playerMovement;
    std::unique_ptr<TilemapRenderer> m_renderer;
    std::unique_ptr<SpriteAnimator> m_playerAnimator;
    std::optional<urpg::AnimationComponent> m_playerAiAnimation;
    urpg::Vector3 m_playerAiAnimationOffset = urpg::Vector3::Zero();
    std::vector<std::string> m_aiAnimationDiagnostics;

    // Dialogue & AI Runtime
    urpg::message::MessageFlowRunner m_messageRunner;
    std::string m_activeDialogueConversationId;
    std::vector<std::string> m_dialogueRuntimeDiagnostics;
    std::optional<urpg::dialogue::DialogueGraph> m_activeAuthoredDialogueGraph;
    std::optional<PendingAuthoredDialogue> m_pendingAuthoredDialogue;
    std::string m_activeAuthoredDialogueNodeId;
    std::optional<urpg::localization::LocaleCatalog> m_dialogueLocaleCatalog;
    urpg::accessibility::InclusiveSettings m_dialogueInclusiveSettings =
        urpg::accessibility::InclusiveSettings::safeDefaults();
    std::vector<urpg::accessibility::CaptionCue> m_authoredDialogueCaptionTrack;
    std::map<std::string, std::string> m_authoredDialogueVoiceAssetByTakeId;
    std::optional<urpg::accessibility::CaptionCue> m_activeAuthoredDialogueCaptionCue;
    std::string m_activeAuthoredDialogueCaption;
    std::string m_activeAuthoredDialogueVoiceAssetId;
    std::vector<AuthoredDialogueInteraction> m_authoredDialogueInteractions;
    uint64_t m_authoredDialogueStateRevision = 0;
    std::shared_ptr<urpg::ai::ChatbotComponent> m_activeChatbot;
    std::unique_ptr<urpg::ui::ChatWindow> m_chatUI;
    std::shared_ptr<urpg::audio::AudioCore> m_audioCore;
    urpg::presentation::ExplorationFeedbackDirector m_explorationFeedback;
    uint64_t m_explorationFeedbackRequestSequence = 1;
    bool m_isChatInputOpen = false;
    std::string m_currentInputBuffer;
    urpg::ability::AbilitySystemComponent m_playerAbilitySystem;
    std::vector<InteractionAbilityBinding> m_interaction_ability_bindings;
};

MapAssetReferences loadRuntimeMapAssetReferences(const std::filesystem::path& project_root, const std::string& map_id);

} // namespace urpg::scene
