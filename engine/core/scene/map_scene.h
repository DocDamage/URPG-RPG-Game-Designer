#pragma once

#include "scene_manager.h"
#include "engine/core/ability/ability_system_component.h"
#include "engine/core/ability/authored_ability_asset.h"
#include "engine/core/input/input_core.h"
#include "engine/core/scene/movement_authority.h"
#include "engine/core/render/render_layer.h"
#include "engine/core/render/tilemap_renderer.h"
#include "engine/core/render/sprite_animator.h"
#include "engine/core/message/message_core.h"
#include "engine/core/message/dialogue_registry.h"
#include "engine/core/message/chatbot_component.h"
#include "engine/core/ui/chat_window.h"
#include "engine/core/audio/audio_core.h"
#include <vector>
#include <cstdint>
#include <string>
#include <memory>

namespace urpg::scene {

/**
 * @brief Represents a tile in the map.
 */
struct TileData {
    uint16_t tileId = 0;
    bool isPassable = true;
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
        std::string prop_asset_id;
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

    /**
     * @brief Manually override passability for a specific tile.
     */
    void setTilePassable(int x, int y, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_tiles[y * m_width + x].isPassable = passable;
            m_renderLayerDirty = true;
        }
    }

    void setTile(int x, int y, uint16_t tileId, bool passable) {
        if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
            m_tiles[y * m_width + x].tileId = tileId;
            m_tiles[y * m_width + x].isPassable = passable;
            m_renderLayerDirty = true;
        }
    }

    // Coordinate Authority
    bool checkCollision(int x, int y) const {
        if (x < 0 || x >= m_width || y < 0 || y >= m_height) return true;
        return !m_tiles[y * m_width + x].isPassable;
    }

    MovementComponent& getPlayerMovement() { return m_playerMovement; }

    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    /**
     * @brief Triggers a dialogue flow in this scene.
     */
    void startDialogue(const std::vector<urpg::message::DialoguePage>& pages);

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

    /**
     * @brief Performs a manual save of the current world state.
     */
    bool saveGame(int slotId = 0);

    /**
     * @brief Attempts to load a saved game state.
     */
    bool loadGame(int slotId = 0);

    /**
     * @brief Checks if a dialogue is currently active, blocking movement.
     */
    bool isDialogueActive() const { return m_messageRunner.isActive() || m_isChatInputOpen; }

    urpg::ability::AbilitySystemComponent& playerAbilitySystem() { return m_playerAbilitySystem; }
    const urpg::ability::AbilitySystemComponent& playerAbilitySystem() const { return m_playerAbilitySystem; }
    void grantPlayerAbility(const urpg::ability::AuthoredAbilityAsset& asset);
    bool tryActivatePlayerAbility(const std::string& ability_id);
    bool bindInteractionAbility(const std::string& trigger_id,
                                const std::string& asset_path,
                                const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindTileInteractionAbility(const std::string& trigger_id,
                                    int tile_x,
                                    int tile_y,
                                    const std::string& asset_path,
                                    const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindPropInteractionAbility(const std::string& trigger_id,
                                    const std::string& prop_asset_id,
                                    const std::string& asset_path,
                                    const urpg::ability::AuthoredAbilityAsset& asset);
    bool bindRegionInteractionAbility(const std::string& trigger_id,
                                      int min_tile_x,
                                      int min_tile_y,
                                      int max_tile_x,
                                      int max_tile_y,
                                      const std::string& asset_path,
                                      const urpg::ability::AuthoredAbilityAsset& asset);
    bool unbindTileInteractionAbility(const std::string& trigger_id, int tile_x, int tile_y);
    bool unbindPropInteractionAbility(const std::string& trigger_id, const std::string& prop_asset_id);
    bool unbindRegionInteractionAbility(const std::string& trigger_id,
                                        int min_tile_x,
                                        int min_tile_y,
                                        int max_tile_x,
                                        int max_tile_y);
    bool activateInteractionAbility(const std::string& trigger_id);
    bool activateInteractionAbilityAtTile(const std::string& trigger_id, int tile_x, int tile_y);
    bool activateInteractionAbilityForProp(const std::string& trigger_id, const std::string& prop_asset_id);
    bool hasInteractionAbilityBinding(const std::string& trigger_id) const;
    const std::vector<InteractionAbilityBinding>& interactionAbilityBindings() const { return m_interaction_ability_bindings; }

private:
    void rebuildTileRenderCache();
    void submitCachedTileCommands(urpg::RenderLayer& layer) const;

    std::string m_mapId;
    int m_width;
    int m_height;
    std::vector<TileData> m_tiles;
    std::vector<urpg::TileCommand> m_cachedTileCommands;
    bool m_renderLayerDirty = true;
    
    // Components
    urpg::MovementComponent m_playerMovement;
    std::unique_ptr<TilemapRenderer> m_renderer;
    std::unique_ptr<SpriteAnimator> m_playerAnimator;

    // Dialogue & AI Runtime
    urpg::message::MessageFlowRunner m_messageRunner;
    std::unique_ptr<urpg::ai::ChatbotComponent> m_activeChatbot;
    std::unique_ptr<urpg::ui::ChatWindow> m_chatUI;
    std::shared_ptr<urpg::audio::AudioCore> m_audioCore;
    bool m_isChatInputOpen = false;
    std::string m_currentInputBuffer;
    urpg::ability::AbilitySystemComponent m_playerAbilitySystem;
    std::vector<InteractionAbilityBinding> m_interaction_ability_bindings;
};

} // namespace urpg::scene
