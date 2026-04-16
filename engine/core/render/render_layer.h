#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>

namespace urpg {

/**
 * @brief Command types for the native renderer.
 */
enum class RenderCmdType : uint8_t {
    Sprite = 0,
    Tile = 1,
    Text = 2,
    Rect = 3,
    Clear = 4
};

/**
 * @brief Base structure for a generic render command.
 */
struct RenderCommand {
    virtual ~RenderCommand() = default;
    RenderCmdType type;
    int32_t zOrder = 0;
    float x = 0.0f;
    float y = 0.0f;
};

/**
 * @brief Command to draw a specific sprite from a texture.
 */
struct SpriteCommand : public RenderCommand {
    SpriteCommand() { type = RenderCmdType::Sprite; }
    std::string textureId;
    int32_t srcX = 0;
    int32_t srcY = 0;
    int32_t width = 0;
    int32_t height = 0;
    float opacity = 1.0f;
};

/**
 * @brief Command to draw a specific tile.
 */
struct TileCommand : public RenderCommand {
    TileCommand() { type = RenderCmdType::Tile; }
    std::string tilesetId;
    int32_t tileIndex = 0;
};

/**
 * @brief Command to draw text via the renderer backend.
 */
struct TextCommand : public RenderCommand {
    TextCommand() { type = RenderCmdType::Text; }
    std::string text;
    std::string fontFace;
    int32_t fontSize = 22;
    int32_t maxWidth = 0;
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;
};

/**
 * @brief The bridge between the Map/Menu scenes and the low-level rendering backend.
 */
class RenderLayer {
public:
    static RenderLayer& getInstance() {
        static RenderLayer instance;
        return instance;
    }

    /**
     * @brief Adds a command to the current frame's batch.
     */
    void submit(const std::shared_ptr<RenderCommand>& cmd) {
        m_commands.push_back(cmd);
    }

    /**
     * @brief Clears the current frame's commands.
     */
    void flush() {
        m_commands.clear();
    }

    /**
     * @brief Returns the current frame's command queue.
     */
    const std::vector<std::shared_ptr<RenderCommand>>& getCommands() const {
        return m_commands;
    }

private:
    RenderLayer() = default;
    std::vector<std::shared_ptr<RenderCommand>> m_commands;
};

} // namespace urpg
